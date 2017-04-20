//
// Created by zhsyourai on 4/20/17.
//

#ifndef JARVIS_KEYEVENTHANDLER_H
#define JARVIS_KEYEVENTHANDLER_H


#include <functional>
#include <thread>

class KeyEventHandler {
public:
    KeyEventHandler(std::string device, std::function<void(int key, bool press)> call_back);

    virtual ~KeyEventHandler();

    void start();

private:
    std::function<void(int key, bool press)> _call_back;
    std::string device;
    std::thread *_thread;
};


#endif //JARVIS_KEYEVENTHANDLER_H
