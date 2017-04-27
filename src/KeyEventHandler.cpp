//
// Created by zhsyourai on 4/20/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <boost/log/trivial.hpp>
#include "KeyEventHandler.h"


KeyEventHandler::KeyEventHandler(std::function<void(int key, bool press)> call_back,
                                 std::function<void(int)> error_callback)
        : _callback(call_back), _device(""), _exit(false), _error_callback(error_callback) {
    _prepare_device_list();
}

KeyEventHandler::~KeyEventHandler() {
    _exit = true;
    if (_future.valid())
        _future.wait();
}

int KeyEventHandler::start(std::string device) {
    _device = device;
    char name[256] = "Unknown";

    if ((getuid()) != 0)
        BOOST_LOG_TRIVIAL(info) << "You are not root! This may not work...";

    do {
        fd = open(_device.c_str(), O_RDONLY | O_NOCTTY | O_CLOEXEC);
    } while (fd == -1 && errno == EINTR);
    if (fd == -1) {
        BOOST_LOG_TRIVIAL(error) << _device << " is not a vaild device.";
        return -errno;
    }

//    errno = 0;
//    if (ioctl(fd, EVIOCGRAB, 1)) {
//        const int saved_errno = errno;
//        close(fd);
//        return errno = (saved_errno) ? -errno : -EACCES;
//    }

    //Print Device Name
    ioctl(fd, EVIOCGNAME (sizeof(name)), name);
    BOOST_LOG_TRIVIAL(info) << "Reading input event from : " << name;

    auto _process = [&]() {
        int status = 0;
        while (!_exit) {
            struct input_event ev;
            ssize_t n = read(fd, &ev, sizeof ev);
            if (n == (ssize_t) -1) {
                if (errno == EINTR)
                    continue;
                status = errno;
                break;

            } else if (n == sizeof ev) {
                /* We consider only key presses and auto repeats. */
                if (ev.type != EV_KEY || (ev.value != 0 && ev.value != 1 && ev.value != 2))
                    continue;
                std::thread([this, ev]() { _callback(ev.code, ev.value > 0); }).detach();
            } else if (n == (ssize_t) 0) {
                status = ENOENT;
                break;
            } else {
                status = EIO;
                break;
            }
        }
        if (!_exit)
            std::thread([this, status]() { _error_callback(status); }).detach();
        return -status;
    };

    std::packaged_task<int(void)> task(_process);
    _future = task.get_future();
    std::thread thread(std::move(task));
    thread.detach();
    return 0;
}

void KeyEventHandler::_prepare_device_list() {
    int version;
    char name[256];           /* RATS: Use ok, but could be better */
    char buf[256] = {0,};  /* RATS: Use ok */
    unsigned char mask[EV_MAX / 8 + 1]; /* RATS: Use ok */
    for (int i = 0; i < 32; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((fd = open(name, O_RDONLY, 0)) >= 0) {
            input_dev dev;
            ioctl(fd, EVIOCGVERSION, &version);
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
            ioctl(fd, EVIOCGBIT(0, sizeof(dev.mask)), dev.mask);
            dev.name = buf;
            dev.path = name;
            dev.id = i;
            dev.version = version;
            close(fd);
            _dev_list.push_back(dev);
        }
    }

}

std::vector<input_dev> KeyEventHandler::list() {
    return _dev_list;
}

