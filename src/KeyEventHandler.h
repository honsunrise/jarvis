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

#define test_bit(bit, mask) (mask[(bit)/8] & (1 << ((bit)%8)))

typedef struct _input_dev {
    int id;
    std::string name;
    std::string path;
    std::string version;
    unsigned char mask[EV_MAX / 8 + 1]; /* RATS: Use ok */
} input_dev;

class KeyEventHandler {
public:
    KeyEventHandler(std::function<void(int key, bool press)> callback,
    std::function<void(int)> error_callback);

    virtual ~KeyEventHandler();

    int start(std::string device);

    std::vector<input_dev> list();

private:
    void _prepare_device_list();

    std::function<void(int key, bool press)> _callback;
    std::function<void(int)> _error_callback;
    std::string _device;
    std::future<int> _future;
    std::atomic_bool _exit;
    std::vector<input_dev> _dev_list;
    int fd;
};


#endif //JARVIS_KEYEVENTHANDLER_H
