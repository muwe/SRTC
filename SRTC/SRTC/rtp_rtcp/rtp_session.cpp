//
//  rtp_session.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/9.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "rtp_session.hpp"
#include "../base/srtc_log.hpp"

using namespace SRTC;

unsigned long RtpSession:: GetLocalSsrc()
{
    SRTC_INFO_LOG("GetLocalSsrc");
    return local_ssrc;
}
