//
//  h264_rtp_helper.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/6.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include <string.h>
#include <assert.h>
#include <memory.h>

#include "h264_rtp_helper.hpp"

using namespace SRTC;


H264RtpHelper::H264RtpHelper(H264RtpPacket* rtp_packet)
{
    rtp_packet_ = rtp_packet;
    
    memset(buffer_pool_, 0x0, BUFFER_LENGTH);
    buffer_pool_length_ = 0;
}

H264RtpHelper::~H264RtpHelper()
{
    
}


int H264RtpHelper::FindStartCode4Byte (const unsigned char *Buf)
{
    if(Buf[0]!='C' || Buf[1]!='D' || Buf[2] !='E' || Buf[3] !='F') return 0;//判断是否为0x00000001,如果是返回1
    else return 1;
}


int H264RtpHelper::FindRtpPacket(const unsigned char* buffer, int length, unsigned char** rtp_buffer, int* rtp_length)
{
    assert(buffer!=nullptr);
    assert(length!=0);
    assert(rtp_buffer!=nullptr);
    
    int pos = 0;
    
    //    unsigned char *Buf = buffer;
    int startcodeprefix_len = 4;
    //判断是否为0x000001
    if(FindStartCode4Byte (buffer) != 1){
        *rtp_buffer = nullptr;
        *rtp_length = 0;
        return 0;
    }
    
    pos = startcodeprefix_len;
    //查找下一个开始字符的标志位
    while (true)
    {
        if (pos > length)//到了文件尾部，还没有找到同步字，移到下一次再找
        {
            *rtp_buffer = nullptr;
            *rtp_length = 0;
            return pos-1;
        }
        pos++;//读一个字节到BUF中
        //判断是否为0x00000001
        if(FindStartCode4Byte(&buffer[pos-4]) == 1)
        {
            break;
        }
    }
    
    *rtp_length = (pos-startcodeprefix_len) - startcodeprefix_len;//4 is 00000001;  //NALU长度，不包括头部。
    *rtp_buffer = (unsigned char*)&buffer[startcodeprefix_len];    //一个完整NALU，不拷贝起始前缀0x000001或0x00000001
    
    return pos-startcodeprefix_len;//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
}

int H264RtpHelper::ParseRtpPacket(const unsigned char* buffer, int length)
{
    printf("ParseRtpPacket Start::length=%d, buffer_pool_length_=%d\n",length, buffer_pool_length_);
    
    if(length + buffer_pool_length_ <= BUFFER_LENGTH ){
        memcpy(&buffer_pool_[buffer_pool_length_], buffer, length);
        buffer_pool_length_ += length;
    }else{
        assert(0);
    }
    
    int pos = 0;
    unsigned char* rtp_buffer = nullptr;
    int rtp_len = 0;
    
    int packetbyte = 0;
    int processed_buffer_len = 0;
    while(pos < buffer_pool_length_){
        
        packetbyte = FindRtpPacket(&buffer_pool_[processed_buffer_len], buffer_pool_length_-processed_buffer_len, &rtp_buffer, &rtp_len);
        
        
        if(rtp_len && rtp_buffer){
            // 发现RTP包，解析
            rtp_packet_->ReceiveRtpPacket(rtp_buffer, rtp_len);
            processed_buffer_len += packetbyte;
            
        }else if(packetbyte){
            //没有完整数据，退出循环
            break;
        }else{
            // 数据错位，报错
            pos++;
            assert(0);
        }
    }
    
    //
    if(processed_buffer_len && ((buffer_pool_length_ - processed_buffer_len) > 0)){
        memcpy(&buffer_pool_[0], &buffer_pool_[processed_buffer_len], buffer_pool_length_ - processed_buffer_len);
        buffer_pool_length_ = buffer_pool_length_ - processed_buffer_len;
    }
    
    printf("ParseRtpPacket End::processed_buffer_len=%d, buffer_pool_length_=%d\n",processed_buffer_len, buffer_pool_length_);
    
    
    return 0;
}
