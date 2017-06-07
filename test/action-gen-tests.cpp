//
// Created by zhsyourai on 4/27/17.
//
#include <gtest/gtest.h>
#include <fstream>
#include <atomic>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "nlp/NLP.h"
#include "nlp/ActionAnalytics.h"
#include "nlp/engine/LPTProcessor.h"
#include "VoicePlayer.h"

template<typename T>
boost::log::formatting_ostream &operator<<(boost::log::formatting_ostream &out, std::vector<T> &v) {
    out << "[";
    size_t last = v.size() - 1;
    for (size_t i = 0; i < v.size(); ++i) {
        out << v[i];
        if (i != last)
            out << ", ";
    }
    out << "]";
    return out;
}

static long _action_test() {
    ActionAnalytics *analytics = new ActionAnalytics();
    NLP *nlp = new LPTProcessor([&](std::vector<CONLL> conll) {
        Action &&action = analytics->analytics(conll);
        BOOST_LOG_TRIVIAL(info) << "-------------------";
        BOOST_LOG_TRIVIAL(info) << "Action:" << action.action;
        BOOST_LOG_TRIVIAL(info) << "Target:" << action.target;
        BOOST_LOG_TRIVIAL(info) << "Source:" << action.source;
        BOOST_LOG_TRIVIAL(info) << "Param:";
        for (auto p : action.params) {
            BOOST_LOG_TRIVIAL(info) << "\t" << p.first << "==>" << p.second;
        }
        BOOST_LOG_TRIVIAL(info) << "-------------------";

    }, [](int code) {
        BOOST_LOG_TRIVIAL(info) << "NLP Error[" << code << "]!";
    });
    nlp->initialize();

    nlp->start();
    nlp->process("小黑打开灯。");
    nlp->end();

    sleep(1);
    nlp->start();
    nlp->process("小黑关闭新风系统。");
    nlp->end();

    sleep(1);
    nlp->start();
    nlp->process("小黑把客厅灯设置成激情模式。");
    nlp->end();

    sleep(1);
    nlp->start();
    nlp->process("小黑把灯泡设置成绿色。");
    nlp->end();

    sleep(1);
    nlp->start();
    nlp->process("小黑空调多少度？");
    nlp->end();

    nlp->uninitialize();
    return 0;
}

TEST (ActionTests, ActionTest) {
    EXPECT_EQ (0, _action_test());
}

int main(int argc, char *argv[]) {

    // create sink backend
    boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend(
            new boost::log::sinks::text_ostream_backend());

    // add stream
    backend->add_stream(boost::shared_ptr<std::ostream>(
            &std::clog, boost::null_deleter()));

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