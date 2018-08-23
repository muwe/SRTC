//
//  srtc_log.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/5.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "srtc_log.hpp"

void SRTC_LOG(Log_Level level, const char *format, ...)
{
    if(SRTC_LOG_LEVEL >= level){
        printf(format);
    }
}

void SRTC_INFO_LOG(const char *format, ...)
{
    if(SRTC_LOG_LEVEL >= INFO_LOG){
        printf(format);
    }

}

void SRTC_ERROR_LOG(const char *format, ...)
{
    if(SRTC_LOG_LEVEL >= ERROR_LOG){
        printf(format);
    }

}

