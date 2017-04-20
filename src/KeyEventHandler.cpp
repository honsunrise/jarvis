//
// Created by zhsyourai on 4/20/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <boost/log/trivial.hpp>
#include "KeyEventHandler.h"


KeyEventHandler::KeyEventHandler(std::string device, std::function<void(int key, bool press)> call_back)
        : _call_back(call_back), device(device) {

}

KeyEventHandler::~KeyEventHandler() {

}

void KeyEventHandler::start() {

    struct input_event ev[64];
    int fd, rd, size = sizeof(struct input_event);
    char name[256] = "Unknown";

    if ((getuid()) != 0)
        BOOST_LOG_TRIVIAL(info) << "You are not root! This may not work...n";

    //Open Device
    if ((fd = open(device.c_str(), O_RDONLY)) == -1)
        BOOST_LOG_TRIVIAL(info) << device << " is not a vaild device.n";

    //Print Device Name
    ioctl(fd, EVIOCGNAME (sizeof(name)), name);
    BOOST_LOG_TRIVIAL(info) << "Reading From : " << device << "(" << name << ")n";

    _thread = new std::thread([&]() {
                                  while (1) {
                                      if ((rd = read(fd, ev, size * 64)) < size) {
                                          BOOST_LOG_TRIVIAL(info) << "read error";
                                          return;
                                      }
                                      int value = ev[0].value;

                                      if (value != ' ' && ev[1].value == 1 && ev[1].type == 1) { // Only read the key press event
                                          _call_back(value, true);
                                      } else {
                                          _call_back(value, false);
                                      }
                                  }
                              }
    );
}

