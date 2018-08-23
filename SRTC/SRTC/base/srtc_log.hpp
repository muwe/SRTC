//
//  srtc_log.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/5.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef srtc_log_hpp
#define srtc_log_hpp

#include <stdio.h>


    typedef enum Log_Level{
        NONE_LOG = 0,
        ERROR_LOG,
        WARNING_LOG,
        INFO_LOG,
        DEBUG_LOG
    }Log_Level;
    
    static Log_Level SRTC_LOG_LEVEL = INFO_LOG;
    
    void SRTC_LOG(Log_Level level, const char *format, ...);

    void SRTC_INFO_LOG(const char *format, ...);
    void SRTC_ERROR_LOG(const char *format, ...);

#endif /* srtc_log_hpp */
