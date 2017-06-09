//
// Created by zhsyourai on 6/9/17.
//

#ifndef JARVIS_COMMON_H
#define JARVIS_COMMON_H

#include <string>

typedef enum {
    OPEN, CLOSE, SETTING, QUERY, NONE, N
} ACTIONDEF;

typedef enum {
    LOCATION, TIME, COLOR, MODE, STATUS, DESC
} PARAM_KEY;

typedef struct _action {
    /*
     * 描述需要执行的动作
     *
     * 取值：OPEN, CLOSE, SETTING, QUERY
     */
    ACTIONDEF action;

    /*
     * 描述需要执行动作的发出者
     *
     * 取值：“控制端名称” 还没有起名字
     */
    std::string source;

    /*
     * 描述需要执行动作的执行者
     *
     * 取值：空调、电视、洗衣用户所说的话决定)
     */
    std::string target;
    /*
     * 描述需要执行
     *
     * 取值：依据key的不同，有所不同
     */
    std::map<PARAM_KEY, std::vector<std::string>> params;
} Action ;

typedef struct _response {
    bool success;
    std::string desc;
} Response;

#endif //JARVIS_COMMON_H
