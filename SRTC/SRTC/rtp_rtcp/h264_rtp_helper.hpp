//
//  h264_rtp_helper.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/6.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef h264_rtp_helper_hpp
#define h264_rtp_helper_hpp

#include <stdio.h>

#include "h264_rtp.h"
#include "h264_rtp_packet.hpp"

#define BUFFER_LENGTH 1024*4

namespace SRTC {
    
    class H264RtpHelper
    {
    public:
        H264RtpHelper(H264RtpPacket* rtp_packet);
        ~H264RtpHelper();
        
        int ParseRtpPacket(const unsigned char* buffer, int length);
        
    private:
        int FindStartCode4Byte (const unsigned char *Buf);
        int FindRtpPacket(const unsigned char* buffer, int length, unsigned char** rtp_buffer, int* rtp_length);
    private:
        unsigned char buffer_pool_[BUFFER_LENGTH];
        int buffer_pool_length_;
        
        H264RtpPacket* rtp_packet_;
    };
};


#endif /* h264_rtp_helper_hpp */
