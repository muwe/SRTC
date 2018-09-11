//
//  rtp_steam_source.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/8/24.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "rtp_steam_source.hpp"

using namespace SRTC;

unsigned long StreamSource::GetSsrc()
{
    return ssrc_;
}

SSRC_STATES* StreamSource::GetSsrcStates()
{
    return &ssrc_states_;
}

SSRC_SDES* StreamSource::GetSsrcSdes()
{
    
    return &ssrc_sdes_;
}
