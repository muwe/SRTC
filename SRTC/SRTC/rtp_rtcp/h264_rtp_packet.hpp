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
#include "rtp_session.hpp"

namespace SRTC {
    
    class RtpReveiver{
    public:
        virtual void OnRtpPacket(char* rtp, int length) = 0;
    };
    
    class H264Receiver
    {
    public:
        virtual void OnH264Frame(unsigned char* h264data, int length) = 0;
    };

    class H264RtpPacket{
    public:
        H264RtpPacket(RtpReveiver* rtp_receiver, H264Receiver* h264_receiver, RtpSession* rtp_session);
        virtual ~H264RtpPacket();
        
        int ReceiveH264Packet(const unsigned char* buffer, int length);
        int ReceiveRtpPacket(const unsigned char* buffer, int length);

    private:
        // h264 to rtp
        NALU_t *AllocNALU(int buffersize);
        void FreeNALU(NALU_t *n);
        int GetAnnexbNALU (unsigned char* inPutBuffer, int length, NALU_t *nalu);
        int FindStartCode2 (unsigned char *Buf);
        int FindStartCode3 (unsigned char *Buf);
        void dump(NALU_t *n);
        
    private:
        RtpReveiver* rtp_receiver_;
        unsigned short rtp_seq_num_;
        
        H264Receiver* h264_receiver_;
        unsigned char* h264_buffer_;
        int h264_buffer_length_;

        RtpSession* rtp_session_{nullptr};
    };
}

#endif /* h264_rtp_packet_hpp */
