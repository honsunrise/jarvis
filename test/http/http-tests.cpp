//
// Created by zhsyourai on 4/17/17.
//
#include <gtest/gtest.h>
#include "utils/AsyncHttpClient.h"

static long _http_get_test() {
    try {
        boost::asio::io_service io_service;
        AsyncHttpClient c(io_service, "api.ltp-cloud.com", AsyncHttpClient::POST, "/analysis/",
                          "api_key=O111T730w3BZaUJshXbcFcuQrQuiwSBD5Bjl2cBe&text=把灯泡设置成绿色。&pattern=all&format=json",
                          AsyncHttpClient::URLENCODE,
                          [](std::string error) {
                              std::cout << error << std::endl;
                          },
                          [](unsigned int code, std::vector<std::string> headers, std::string content) {
                              std::cout << code << std::endl;
                              std::cout << headers.size() << std::endl;
                              std::cout << content << std::endl;
                          });
        io_service.run();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    return 0;
}

TEST (HttpTest, GetTest) {
    EXPECT_EQ (0, _http_get_test());
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}