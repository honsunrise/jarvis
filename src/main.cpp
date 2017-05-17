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
#include "engine.h"
#include "VoiceRecord.h"
#include "asr/engine/IflytekRecognizer.h"
#include "nlp/engine/LPTProcessor.h"
#include "tts/engine/IflytekTTS.h"
#include "KeyEventHandler.h"
#include "VoicePlayer.h"
#include "chat/engine/TulingOTT.h"

int main(int argc, char *argv[]) {
    int status = 0;
    int index = 0;
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

    global_engine_init();

    Voice::VoicePlayer *voicePlayer = new Voice::VoicePlayer();

    std::vector<Voice::voice_dev> &&device_playback_list = Voice::list_playback_devices();
    Voice::voice_dev will_open;

    for (auto &dev : device_playback_list) {
        if (dev.name == "default") {
            will_open = dev;
        }
        BOOST_LOG_TRIVIAL(info) << "------------------------";
        BOOST_LOG_TRIVIAL(info) << "Device name " << dev.name;
        BOOST_LOG_TRIVIAL(info) << "Device desc " << dev.desc;
    }
    BOOST_LOG_TRIVIAL(info) << "------------------------";

    BOOST_LOG_TRIVIAL(info) << "Open player " << voicePlayer->open(will_open, DEFAULT_FORMAT);
    voicePlayer->start();

    TTS *tts = new IflytekTTS(
            [&](const char *data, unsigned int len) {
                if (data != nullptr)
                    voicePlayer->play(data, len);
            }, [](int code) {
            });

    chat *tuling = new TulingOTT([&tts](std::string text) {
        tts->start();
        tts->process(text);
        tts->end();
    }, [](int code) {
        BOOST_LOG_TRIVIAL(info) << "Chat Error[" << code << "]!";
    });

    NLP *nlp = new LPTProcessor([](std::vector<CONLL> conll) {
        BOOST_LOG_TRIVIAL(info) << "NLP [" << conll[1].content << "]!";
    }, [](int code) {
        BOOST_LOG_TRIVIAL(info) << "NLP Error[" << code << "]!";
    });

    static std::string end_flag[] = {"。", "？", "！"};

    SpeechRecognizer *recognizer = new IflytekRecognizer([&nlp, &tuling](char *result, char is_last) {
        static std::string last_recognizer = "";
        bool skip = false;
        for (auto &flag : end_flag) {
            if (flag == result) {
                skip = true;
                std::string recognize_result = last_recognizer + result;
                last_recognizer = "";
                BOOST_LOG_TRIVIAL(info) << "recognize something [" << recognize_result << "]!";

                tuling->start();
                tuling->process("贾维斯" + recognize_result);
                tuling->end();

                nlp->start();
                nlp->process("贾维斯" + recognize_result);
                nlp->end();
            }
        }
        if (!skip) {
            last_recognizer += result;
            skip = true;
        }
        delete[]result;
    }, [](int reason) {
        BOOST_LOG_TRIVIAL(info) << "happen something [" << reason << "]!";
    });

    Voice::VoiceRecord *voiceRecord;

    auto start_process = [&]() {
        bool expected = false;
        while (start_listing.compare_exchange_strong(expected, true)) {
            BOOST_LOG_TRIVIAL(info) << "Start listing";
            BOOST_LOG_TRIVIAL(info) << "Start cap " << voiceRecord->start();
            BOOST_LOG_TRIVIAL(info) << "Start recognizer " << recognizer->start();
        }
    };

    auto end_process = [&]() {
        bool expected = true;
        while (start_listing.compare_exchange_strong(expected, false)) {
            BOOST_LOG_TRIVIAL(info) << "End listing";
            BOOST_LOG_TRIVIAL(info) << "End recognizer " << recognizer->end();
            BOOST_LOG_TRIVIAL(info) << "Stop cap " << voiceRecord->stop();
        }
    };

    voiceRecord = new Voice::VoiceRecord([&recognizer](char *data, size_t len, void *param) {
        recognizer->listen(data, len);
    }, [&end_process]() {
        end_process();
    }, 0);

    tuling->initialize();

    tts->initialize();

    nlp->initialize();

    recognizer->initialize();

    std::vector<Voice::voice_dev> &&device_capture_list = Voice::list_capture_devices();

    for (auto &dev : device_capture_list) {
        if (dev.name == "default") {
            will_open = dev;
        }
        BOOST_LOG_TRIVIAL(info) << "------------------------";
        BOOST_LOG_TRIVIAL(info) << "Device name " << dev.name;
        BOOST_LOG_TRIVIAL(info) << "Device desc " << dev.desc;
    }
    BOOST_LOG_TRIVIAL(info) << "------------------------";

    BOOST_LOG_TRIVIAL(info) << "Open capture " << voiceRecord->open(will_open, DEFAULT_FORMAT);

    KeyEventHandler *keyEventHandler;
    keyEventHandler = new KeyEventHandler(
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

    std::vector<input_dev> &&input_device_list = keyEventHandler->list();
    BOOST_LOG_TRIVIAL(info) << "------input devicelist------";
    for (auto &dev : input_device_list) {
        BOOST_LOG_TRIVIAL(info) << dev.id << ". " << dev.name;
    }
    BOOST_LOG_TRIVIAL(info) << "----------------------------";

    if (argc < 2) {
        goto exit;
    }

    index = atoi(argv[1]);
    if ((status = keyEventHandler->start(input_device_list[index].path)) < 0) {
        BOOST_LOG_TRIVIAL(error) << "Key event handler register error : " << status;
        goto exit;
    }

    while (!global_exit) {
        BOOST_LOG_TRIVIAL(info) << "everything is ok!";
        sleep(5);
    }
    exit:
    delete keyEventHandler;
    tuling->uninitialize();
    recognizer->uninitialize();
    nlp->uninitialize();
    tts->uninitialize();
    voicePlayer->close();
    BOOST_LOG_TRIVIAL(info) << "Close cap " << voiceRecord->close();
    BOOST_LOG_TRIVIAL(info) << "Close player " << voicePlayer->close();
    delete nlp;
    delete recognizer;
    delete voiceRecord;
    delete voicePlayer;
    global_engine_uninit();
    BOOST_LOG_TRIVIAL(info) << "Jarvis exit.";
    return status;
}