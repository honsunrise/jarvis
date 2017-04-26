#include <iostream>
#include <fstream>
#include <atomic>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "VoiceRecord.h"
#include "asr/engine/IflytekRecognizer.h"
#include "nlp/engine/LPTProcessor.h"
#include "KeyEventHandler.h"

int main(int, char *[]) {
    int status = 0;
    std::atomic_bool global_exit(false), start_listing(false);
    // create sink backend
    boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend(
            new boost::log::sinks::text_ostream_backend());

    // add stream
    backend->add_stream(boost::shared_ptr<std::ostream>(
            &std::clog, boost::null_deleter()));
    backend->add_stream(boost::shared_ptr<std::ostream>(
            new std::ofstream("sample.log", std::ofstream::app)));

    // other setting
    backend->auto_flush(true);

    // create sink frontend
    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> sink_t;
    boost::shared_ptr<sink_t> sink(new sink_t(backend));

    // sink format
    sink->set_formatter(
            boost::log::expressions::stream
                    << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %T")
                    << " [" << boost::log::trivial::severity << "]\t"
                    << "Jarvis " << boost::log::expressions::smessage
    );

    // sink filter
    sink->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
    );

    // add sink
    boost::shared_ptr<boost::log::core> core = boost::log::core::get();
    core->add_sink(sink);

    // setup common attributes
    boost::log::add_common_attributes();

    BOOST_LOG_TRIVIAL(info) << "is started!";
    NLP *nlp = new LPTProcessor([](std::vector<CONLL> conll) {
        BOOST_LOG_TRIVIAL(info) << "NLP [" << conll[1].content << "]!";
    }, [](int code) {

    });

    static std::string end_flag[] = {"。", "？", "！"};

    SpeechRecognizer *recognizer = new IflytekRecognizer([&nlp](const char *result, char is_last) {
        static std::string last_recognizer;
        for (auto &flag : end_flag) {
            if (flag == result) {
                std::string recognize_result = last_recognizer + result;
                last_recognizer = "";
                BOOST_LOG_TRIVIAL(info) << "recognize something [" << recognize_result << "]!";
                nlp->start();
                nlp->process(recognize_result);
                nlp->end();
            }
        }
        last_recognizer = result;
    }, [](int reason) {
        BOOST_LOG_TRIVIAL(info) << "happen something [" << reason << "]!";
    });

    VoiceRecord *voiceRecord;

    auto start_process = [&]() {
        bool expected = false;
        while(start_listing.compare_exchange_strong(expected, true)) {
            BOOST_LOG_TRIVIAL(info) << "Start listing";
            BOOST_LOG_TRIVIAL(info) << "Start cap " << voiceRecord->start();
            BOOST_LOG_TRIVIAL(info) << "Start recognizer " << recognizer->start();
        }
    };

    auto end_process = [&]() {
        bool expected = true;
        while(start_listing.compare_exchange_strong(expected, false)) {
            BOOST_LOG_TRIVIAL(info) << "End listing";
            BOOST_LOG_TRIVIAL(info) << "End recognizer " << recognizer->end();
            BOOST_LOG_TRIVIAL(info) << "Stop cap " << voiceRecord->stop();
        }
    };

    voiceRecord = new VoiceRecord([&recognizer](char *data, size_t len, void *param) {
        BOOST_LOG_TRIVIAL(info) << "listen something!";
        recognizer->listen(data, len);
    }, [&end_process](){
        end_process();
    }, 0);

    nlp->initialize();

    recognizer->initialize();

    std::vector<voice_record_dev> &&device_list = voiceRecord->list();

    BOOST_LOG_TRIVIAL(info) << "Open cap "
                            << voiceRecord->open(
                                    device_list[0],
                                    DEFAULT_FORMAT);;

    KeyEventHandler *keyEventHandler;
    keyEventHandler = new KeyEventHandler("/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd",
                                          [&](int key, bool press) {
                                              BOOST_LOG_TRIVIAL(info) << "Info event code : " << key
                                                                      << " " << press;
                                              if (press && key == KEY_HOME) {
                                                  start_process();
                                              } else if (!press && key == KEY_ESC) {
                                                  global_exit = true;
                                              }
                                          }, [&](int code) {
                BOOST_LOG_TRIVIAL(error) << "Key event handler error : " << code;
                global_exit = true;
            });

    if ((status = keyEventHandler->start()) < 0) {
        BOOST_LOG_TRIVIAL(error) << "Key event handler register error : " << status;
        goto exit;
    }
    while (!global_exit) {
        BOOST_LOG_TRIVIAL(info) << "everything is ok!";
        sleep(5);
    }
    exit:
    delete keyEventHandler;
    recognizer->uninitialize();
    nlp->uninitialize();
    BOOST_LOG_TRIVIAL(info) << "Close cap " << voiceRecord->close();
    delete nlp;
    delete recognizer;
    delete voiceRecord;
    BOOST_LOG_TRIVIAL(info) << "Jarvis exit.";
    return status;
}