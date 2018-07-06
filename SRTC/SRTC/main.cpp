//
//  main.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/27.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include <iostream>
#include "dump2file.hpp"
#include "transport/transport.hpp"
#include "rtp_rtcp/h264_rtp_packet.hpp"
#include "codec/h264_encode.hpp"

#define YUV_FILE "/Users/aivensmac/work/study/SRTC/SRTC/resource/foreman_320x240.yuv"
#define H264_FILE "/Users/aivensmac/work/study/SRTC/SRTC/resource/video.h264"
#define RTP_FILE "/Users/aivensmac/work/study/SRTC/SRTC/resource/video.rtp"


#include "rtprtcp/inc/rtp_all.h"

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8070

using namespace SRTC;

class RtpDump:public RtpReveiver
{
public:
    RtpDump(string outPutFile)
    {
        dump_file_.reset(new DumpFile(outPutFile));
        transport_.reset(new Transport());
        
        transport_->Init(TCP, SERVER_ADDRESS, SERVER_PORT);
    }
    void OnRtpPacket(char* rtp, int length) override
    {
        dump_file_->Write((const unsigned char*)rtp, length);
        transport_->Send((const unsigned char*)rtp, length);
    }
private:
    shared_ptr<DumpFile> dump_file_;
    shared_ptr<Transport> transport_;
};

class H264Dump:public H264DataCallback
{
public:
    H264Dump(string outPutFile)
    {
        dump_file_.reset(new DumpFile(outPutFile));
//        transport_.reset(new Transport());
        rtp_dump_.reset(new RtpDump(RTP_FILE));
        h264_rtp_.reset(new H264RtpPacket(rtp_dump_.get(), nullptr));
        
//        transport_->Init(TCP, SERVER_ADDRESS, SERVER_PORT);
    }
    
    void OnDate(const unsigned char* data, int length) override
    {
        dump_file_->Write(data, length);
//        transport_->Send(data, length);
        
        h264_rtp_->ReceiveH264Packet(data, length);
    }
private:
    shared_ptr<DumpFile> dump_file_;
//    shared_ptr<Transport> transport_;
    shared_ptr<H264RtpPacket> h264_rtp_;
    shared_ptr<RtpDump> rtp_dump_;
    
};

int main(int argc, const char * argv[]) {
    
    shared_ptr<H264Dump> h264_dump_(new H264Dump(H264_FILE));
    shared_ptr<H264Encoder> h264_encoder_(new H264Encoder(YUV_FILE, h264_dump_.get()));

    h264_encoder_->Encode();
    
    return 0;
}


