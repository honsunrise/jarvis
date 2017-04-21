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
    char name[256] = "Unknown";

    if ((getuid()) != 0)
        BOOST_LOG_TRIVIAL(info) << "You are not root! This may not work...n";


    _thread = new std::thread([&]() {
                                  int fd;

                                  //Open Device
                                  if ((fd = open(device.c_str(), O_RDWR, 0)) < 0)
                                      BOOST_LOG_TRIVIAL(info) << device << " is not a vaild device.n";

                                  //Print Device Nameggddggg
                                  ioctl(fd, EVIOCGNAME (sizeof(name)), name);
                                  BOOST_LOG_TRIVIAL(info) << "Reading From : " << name;
                                  while (1) {
                                      size_t size = sizeof(struct input_event);
                                      ssize_t rd;
                                      struct input_event ev[64];
                                      if ((rd = read(fd, ev, size * 64)) < 0) {
                                          BOOST_LOG_TRIVIAL(info) << "Read error " << rd;
                                      }

                                      if (ev[0].value != ' ' && ev[1].value == 1) { // Only read the key press event
                                          _call_back(ev[1].code, ev[1].type == 1);
                                      }

                                      n = read(dev->fd, &ev, sizeof ev);
                                      if (n == (ssize_t)-1) {
                                          if (errno == EINTR)
                                              continue;
                                          status = errno;
                                          break;

                                      } else
                                      if (n == sizeof ev) {

                                          /* We consider only key presses and autorepeats. */
                                          if (ev.type != EV_KEY || (ev.value != 1 && ev.value != 2))
                                              continue;

                                          switch (ev.code) {
                                              case KEY_0: digit = '0'; break;
                                              case KEY_1: digit = '1'; break;
                                              case KEY_2: digit = '2'; break;
                                              case KEY_3: digit = '3'; break;
                                              case KEY_4: digit = '4'; break;
                                              case KEY_5: digit = '5'; break;
                                              case KEY_6: digit = '6'; break;
                                              case KEY_7: digit = '7'; break;
                                              case KEY_8: digit = '8'; break;
                                              case KEY_9: digit = '9'; break;
                                              default:    digit = '\0';
                                          }

                                          /* Non-digit key ends the code, except at beginning of code. */
                                          if (digit == '\0') {
                                              if (!len)
                                                  continue;
                                              status = 0;
                                              break;
                                          }

                                          if (len < length)
                                              buffer[len] = digit;
                                          len++;

                                          continue;

                                      } else
                                      if (n == (ssize_t)0) {
                                          status = ENOENT;
                                          break;

                                      } else {
                                          status = EIO;
                                          break;
                                      }
                                  }
                              }
    );
}

