//
//  h264_rtp_packet.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/4.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef h264_rtp_packet_hpp
#define h264_rtp_packet_hpp

#include <stdio.h>
#include "h264_rtp.h"

namespace SRTC {
    
    class RtpReveiver{
    public:
        virtual void OnRtpData(char* rtp, int length) = 0;
    };
    
    class H264RtpPacket{
    public:
        H264RtpPacket(RtpReveiver* receiver);
        virtual ~H264RtpPacket(){}
        int H264ToRtp(const unsigned char* buffer, int length);
    private:
        NALU_t *AllocNALU(int buffersize);
        void FreeNALU(NALU_t *n);
        int GetAnnexbNALU (unsigned char* inPutBuffer, int length, NALU_t *nalu);
        int FindStartCode2 (unsigned char *Buf);
        int FindStartCode3 (unsigned char *Buf);
        void dump(NALU_t *n);
    private:
        RtpReveiver* receiver_;
        unsigned short seq_num_;
        
    };
}

#endif /* h264_rtp_packet_hpp */
