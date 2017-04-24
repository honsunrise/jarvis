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
    NLP *nlp = new LPTProcessor([](std::vector<CONLL> conll){
        BOOST_LOG_TRIVIAL(info) << "NLP [" << conll[1].content << "]!";
    }, [](int code){

    });

    SpeechRecognizer *recognizer = new IflytekRecognizer([&nlp](const char *result, char is_last) {
        BOOST_LOG_TRIVIAL(info) << "recognize something [" << result << "]!";
        nlp->start();
        nlp->process(result);
        nlp->end();
    }, [](int reason) {
        BOOST_LOG_TRIVIAL(info) << "happen something [" << reason << "]!";
    });

    VoiceRecord *voiceRecord = new VoiceRecord([&recognizer](char *data, size_t len, void *param) {
        BOOST_LOG_TRIVIAL(info) << "listen something!";
        recognizer->listen(data, len);
    }, 0);

    nlp->initialize();

    recognizer->initialize();

    std::vector<voice_record_dev> &&device_list = voiceRecord->list();

    BOOST_LOG_TRIVIAL(info) << "Open cap "
                            << voiceRecord->open(
                                    device_list[0],
                                    DEFAULT_FORMAT);;

    KeyEventHandler *keyEventHandler;
    keyEventHandler = new KeyEventHandler("/dev/input/by-id/usb-Heng_Yu_Technology_Poker-event-kbd",
                                          [&](int key, bool press) {
                                              BOOST_LOG_TRIVIAL(info) << "Info event code : " << key
                                                                      << " " << press;
                                              if (press && key == KEY_HOME) {
                                                  if (!start_listing) {
                                                      start_listing = true;
                                                      BOOST_LOG_TRIVIAL(info) << "Start listing";
                                                      BOOST_LOG_TRIVIAL(info) << "Start cap "
                                                                              << voiceRecord->start();
                                                      BOOST_LOG_TRIVIAL(info) << "Start recognizer "
                                                                              << recognizer->start();
                                                      sleep(5);
                                                      BOOST_LOG_TRIVIAL(info) << "End recognizer " << recognizer->end();
                                                      BOOST_LOG_TRIVIAL(info) << "Stop cap " << voiceRecord->stop();
                                                      start_listing = false;
                                                  }
                                              } else if (!press && key == KEY_Q) {
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
    return status;
}