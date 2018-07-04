//
//  base.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "base.hpp"
#include <unistd.h>
#include <sys/time.h>

int64_t GetSystemTime()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    
    int64_t milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    
    return milliseconds;
}

void Sleep(int time)
{
    usleep(time*1000);
}
