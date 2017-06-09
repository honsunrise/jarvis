//
// Created by zhsyourai on 6/9/17.
//

#ifndef JARVIS_PLUGIN_INTERFACE_H
#define JARVIS_PLUGIN_INTERFACE_H

#include "common.h"

/**
 * 插件打开回调
 */
void plugin_open();

/**
 * 调用插件
 */
Response plugin_do(Action action);

/**
 * 插件关闭
 */
void plugin_close();


#endif //JARVIS_PLUGIN_INTERFACE_H
