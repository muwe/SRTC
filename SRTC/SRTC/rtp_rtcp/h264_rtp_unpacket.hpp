//
//  h264_rtp_unpacket.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/4.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef h264_rtp_unpacket_hpp
#define h264_rtp_unpacket_hpp

#include <stdio.h>
#include "h264_rtp.h"

#define BUFFER_LENGTH 1024*4

namespace SRTC {
    
    class H264Receiver
    {
    public:
        virtual void OnH264RawData(unsigned char* h264, int length) = 0;
    };
    
    class H264RtpUnpacket
    {
    public:
        H264RtpUnpacket(H264Receiver* receiver);
        ~H264RtpUnpacket();
        int RtpToH264(const unsigned char* buffer, int length);
        int FindStartCode4Byte (const unsigned char *Buf);
        int FindRtpPacket(const unsigned char* buffer, int length, unsigned char** rtp_buffer, int* rtp_length);
        int ParseRtpPacket(const unsigned char* buffer, int length);
    private:
        H264Receiver* receiver_;
        unsigned char buffer_pool_[BUFFER_LENGTH];
        int buffer_pool_length_;
        unsigned char* h264_buffer_;
        int h264_buffer_length_;
    };
};

#endif /* h264_rtp_unpacket_hpp */
