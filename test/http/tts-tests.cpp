//
// Created by zhsyourai on 4/27/17.
//
#include <gtest/gtest.h>
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
#include "tts/engine/IflytekTTS.h"
#include "VoicePlayer.h"

static long _tts_test() {
    std::vector<Voice::voice_dev> &&device_list = Voice::list_playback_devices();
    Voice::voice_dev will_open;

    for (auto &dev : device_list) {
        if (dev.name == "default") {
            will_open = dev;
        }
        BOOST_LOG_TRIVIAL(info) << "------------------------";
        BOOST_LOG_TRIVIAL(info) << "Device name " << dev.name;
        BOOST_LOG_TRIVIAL(info) << "Device desc " << dev.desc;
    }
    BOOST_LOG_TRIVIAL(info) << "------------------------";
    Voice::VoicePlayer voicePlayer;
    Voice::wave_format fmt = DEFAULT_FORMAT;
    voicePlayer.open(will_open, fmt);
    voicePlayer.start();
    TTS *tts = new IflytekTTS(
            [&](const char *data, unsigned int len) {
                if(data != nullptr)
                    voicePlayer.play(data, len);
            }, [](int code) {

            });
    tts->initialize();
    tts->start();
    tts->process("老子有一句妈买劈不知当讲不当讲老子有一句妈买劈不知当讲不当讲");
    sleep(5);
    tts->end();
    tts->uninitialize();
    voicePlayer.stop();
    voicePlayer.close();
    sleep(10);
    return 0;
}

TEST (HttpTest, GetTest) {
    EXPECT_EQ (0, _tts_test());
}

int main(int argc, char *argv[]) {

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
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}