#include <iostream>
#include <fstream>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "VoiceRecord.h"
#include "asr/engine/IflytekRecognizer.h"

int main(int, char *[]) {
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
                    << boost::log::expressions::smessage
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
    SpeechRecognizer *recognizer = new IflytekRecognizer([](const char *result, char is_last) {
        BOOST_LOG_TRIVIAL(info) << "Jarvis recognize something [" << result << "]!";
    }, [](int reason) {
        BOOST_LOG_TRIVIAL(info) << "Jarvis happen something [" << reason << "]!";
    });

    BOOST_LOG_TRIVIAL(info) << "Jarvis is started!";
    recognizer->initialize();
    VoiceRecord voiceRecord([&recognizer](char *data, size_t len, void *param) {
        BOOST_LOG_TRIVIAL(info) << "Jarvis listen something!";
        recognizer->listen(data, len);
    }, 0);
    std::vector<record_dev_id> &&device_list = voiceRecord.list();
    voiceRecord.open(device_list[0], DEFAULT_FORMAT);
    voiceRecord.start();
    recognizer->start();
    sleep(5);
    recognizer->end();
    voiceRecord.stop();
    sleep(1);
    return 0;
}