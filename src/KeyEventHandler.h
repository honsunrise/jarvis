//
// Created by zhsyourai on 4/20/17.
//

#ifndef JARVIS_KEYEVENTHANDLER_H
#define JARVIS_KEYEVENTHANDLER_H


#include <functional>
#include <thread>
#include <atomic>
#include <future>
#include <linux/input.h>

class KeyEventHandler {
public:
    KeyEventHandler(std::string device, std::function<void(int key, bool press)> callback,
    std::function<void(int)> error_callback);

    virtual ~KeyEventHandler();

    int start();

private:
    std::function<void(int key, bool press)> _callback;
    std::function<void(int)> _error_callback;
    std::string _device;
    std::future<int> _future;
    std::atomic_bool _exit;
    int fd;
};


#endif //JARVIS_KEYEVENTHANDLER_H
