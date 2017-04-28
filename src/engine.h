//
// Created by zhsyourai on 4/28/17.
//

#ifndef JARVIS_ENGINE_H
#define JARVIS_ENGINE_H
#include <msp_cmn.h>
#include <msp_errors.h>
#include <boost/log/trivial.hpp>

typedef struct _processor_product_info_ {
    const char *name;
    const char *version;
    const char *manufacturer_name;
    const char *home_page;
    void *custom_info;
} processor_product_info;

inline void global_engine_init() {
    const char *login_params = "appid = 58e74906, work_dir = .";

    int errcode = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != errcode) {
        BOOST_LOG_TRIVIAL(error) << "MSPLogin failed , Error code %d" << errcode;
    }
}

inline void global_engine_uninit() {
    int errcode = MSPLogout();
    if (MSP_SUCCESS != errcode) {
        BOOST_LOG_TRIVIAL(error) << "MSPLogin failed , Error code %d" << errcode;
    }
}

#endif //JARVIS_ENGINE_H
