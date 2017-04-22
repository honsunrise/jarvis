//
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JARVIS_ASYNCHTTPCLIENT_H
#define JARVIS_ASYNCHTTPCLIENT_H

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class AsyncHttpClient {
public:
    enum REQUEST_TYPE {
        GET,
        POST
    };
    AsyncHttpClient(boost::asio::io_service &io_service,
                    const std::string &server, REQUEST_TYPE type, const std::string &path, const std::string data,
                    std::function<void(std::string)> on_error,
                    std::function<void(unsigned int, std::vector<std::string>, std::string)> on_result)
            : resolver_(io_service),
              socket_(io_service),
              on_error(on_error),
              on_result(on_result) {
        if (io_service.stopped())
            io_service.reset();
        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        std::ostream request_stream(&request_);
        if(type == GET) {
            request_stream << "GET " << path << " HTTP/1.0\r\n";
            request_stream << "Host: " << server << "\r\n";
        } else {
            request_stream << "POST " << path << " HTTP/1.0\r\n";
            request_stream << "Host: " << server << "\r\n";
        }

        request_stream << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n";
        request_stream << "Cache-Control: max-age=0\r\n";
        request_stream << "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36\r\n";
        request_stream << "Cache-Control: max-age=0\r\n";
        request_stream << "Content-Length: " << data.length() << "\r\n";
        request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
        request_stream << "Connection: keep-alive\r\n\r\n";
        if(type == POST)
            request_stream << data;


        // Start an asynchronous resolve to translate the server and service names
        // into a list of endpoints.
        tcp::resolver::query query(server, "http");
        resolver_.async_resolve(query,
                                boost::bind(&AsyncHttpClient::handle_resolve, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::iterator));
    }

private:
    void handle_resolve(const boost::system::error_code &err,
                        tcp::resolver::iterator endpoint_iterator) {
        if (!err) {
            // Attempt a connection to the first endpoint in the list. Each endpoint
            // will be tried until we successfully establish a connection.
            tcp::endpoint endpoint = *endpoint_iterator;
            socket_.async_connect(endpoint,
                                  boost::bind(&AsyncHttpClient::handle_connect, this,
                                              boost::asio::placeholders::error, ++endpoint_iterator));
        } else {
            on_error(err.message());
        }
    }

    void handle_connect(const boost::system::error_code &err,
                        tcp::resolver::iterator endpoint_iterator) {
        if (!err) {
            // The connection was successful. Send the request.
            boost::asio::async_write(socket_, request_,
                                     boost::bind(&AsyncHttpClient::handle_write_request, this,
                                                 boost::asio::placeholders::error));
        } else if (endpoint_iterator != tcp::resolver::iterator()) {
            // The connection failed. Try the next endpoint in the list.
            socket_.close();
            tcp::endpoint endpoint = *endpoint_iterator;
            socket_.async_connect(endpoint,
                                  boost::bind(&AsyncHttpClient::handle_connect, this,
                                              boost::asio::placeholders::error, ++endpoint_iterator));
        } else {
            on_error(err.message());
        }
    }

    void handle_write_request(const boost::system::error_code &err) {
        if (!err) {
            // Read the response status line.
            boost::asio::async_read_until(socket_, response_, "\r\n",
                                          boost::bind(&AsyncHttpClient::handle_read_status_line, this,
                                                      boost::asio::placeholders::error));
        } else {
            on_error(err.message());
        }
    }

    void handle_read_status_line(const boost::system::error_code &err) {
        if (!err) {
            // Check that response is OK.
            std::istream response_stream(&response_);
            std::string http_version;
            response_stream >> http_version;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                on_error("Invalid response");
                return;
            }
            if (status_code != 200) {
                on_result(status_code, std::vector<std::string>(), "");
                return;
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
                                          boost::bind(&AsyncHttpClient::handle_read_headers, this,
                                                      boost::asio::placeholders::error));
        } else {
            on_error(err.message());
        }
    }

    void handle_read_headers(const boost::system::error_code &err) {
        if (!err) {
            // Process the response headers.
            std::istream response_stream(&response_);
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
                header_list.push_back(header);

            // Start reading remaining data until EOF.
            boost::asio::async_read(socket_, response_,
                                    boost::asio::transfer_all(),
                                    boost::bind(&AsyncHttpClient::handle_read_content, this,
                                                boost::asio::placeholders::error));
        } else {
            on_error(err.message());
        }
    }

    void handle_read_content(const boost::system::error_code &err) {
        if (!err || err == boost::asio::error::eof) {
            std::ostringstream ss;
            ss << &response_;
            on_result(status_code, header_list, ss.str());
        } else {
            on_error(err.message());
        }
    }

    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::asio::streambuf request_;
    boost::asio::streambuf response_;
    unsigned int status_code;

    std::function<void(std::string)> on_error;
    std::vector<std::string> header_list;
    std::string content;
    std::function<void(unsigned int, std::vector<std::string>, std::string)> on_result;
};

#endif //JARVIS_ASYNCHTTPCLIENT_H
