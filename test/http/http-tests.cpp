//
// Created by zhsyourai on 4/17/17.
//
#include <gtest/gtest.h>
#include "utils/AsyncHttpClient.h"

static int _http_get_test() {
    try
    {
        boost::asio::io_service io_service;
        AsyncHttpClient c(io_service, "www.baidu.com", "/");
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }
    return 0;
}

TEST (HttpTest, GetTest) {
    EXPECT_EQ (0, _http_get_test());
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}