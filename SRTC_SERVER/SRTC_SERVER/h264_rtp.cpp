//
//  h264_rtp.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/29.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "h264_rtp.hpp"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

using namespace SRTC;


H264RtpUnpacket::H264RtpUnpacket(H264Receiver* receiver)
{
    receiver_ = receiver;

    memset(buffer_pool_, 0x0, BUFFER_LENGTH);
    buffer_pool_length_ = 0;
    
    h264_buffer_ = nullptr;
    h264_buffer_length_ = 0;
    
}

H264RtpUnpacket::~H264RtpUnpacket()
{
    if(h264_buffer_){
        free(h264_buffer_);
    }
}


int H264RtpUnpacket::FindStartCode4Byte (const unsigned char *Buf)
{
    if(Buf[0]!='C' || Buf[1]!='D' || Buf[2] !='E' || Buf[3] !='F') return 0;//判断是否为0x00000001,如果是返回1
    else return 1;
}


int H264RtpUnpacket::FindRtpPacket(const unsigned char* buffer, int length, unsigned char** rtp_buffer, int* rtp_length)
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

int H264RtpUnpacket::ParseRtpPacket(const unsigned char* buffer, int length)
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
            RtpToH264(rtp_buffer, rtp_len);
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

int H264RtpUnpacket::RtpToH264(const unsigned char* buffer, int length)
{
    if(h264_buffer_length_ < length){
        if(h264_buffer_){
            free(h264_buffer_);
        }
        
        h264_buffer_ = (unsigned char*)calloc(length, sizeof(char));
        h264_buffer_length_ = length;
    }
    
    int pos = 0;

    // Parse RTP Header  12 byte
    RTP_FIXED_HEADER*  rtp_hdr =(RTP_FIXED_HEADER*)&buffer[0];
    
    printf("RtpToH264::rtp_hdr->version=%d, length=%d, seq_no=%d\n", rtp_hdr->version, length, ntohs(rtp_hdr->seq_no));

    if(rtp_hdr->version == 1){
    
        // Singl
        memset(h264_buffer_, 0x0, h264_buffer_length_);

        // Create the H264 SYS Header   3 byte
        h264_buffer_[0] = 0;
        h264_buffer_[1] = 0;
        h264_buffer_[2] = 0;
        h264_buffer_[3] = 1;
        pos = 4;
        
        // Create Nalu Header           1 byte
        NALU_HEADER* nalu_hdr =(NALU_HEADER*)&buffer[12]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
        printf("nalu_hdr->F=%d, nalu_hdr->NRI=%d, nalu_hdr->TYPE=%d\n", nalu_hdr->F, nalu_hdr->NRI, nalu_hdr->TYPE);
        h264_buffer_[pos] = buffer[12];
        pos++;
        
        // Append the raw data
        memcpy(&h264_buffer_[pos], &buffer[13], length - 13);
        pos += length - 13;
        
        if(receiver_){
            receiver_->OnH264RawData(h264_buffer_, pos);
        }
        
    }else if(rtp_hdr->version == 2){
        
        // FU-A
        memset(h264_buffer_, 0x0, buffer_pool_length_);

        FU_INDICATOR* fu_ind =(FU_INDICATOR*)&buffer[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
        assert(fu_ind->TYPE == 28);
        
        //设置FU HEADER,并将这个HEADER填入sendbuf[13]
        FU_HEADER* fu_hdr =(FU_HEADER*)&buffer[13];

        if(fu_hdr->S == 1){
            // 1：Create the H264 SYS Header   3 byte
            h264_buffer_[0] = 0;
            h264_buffer_[1] = 0;
            h264_buffer_[2] = 0;
            h264_buffer_[3] = 1;
            pos += 4;

            // 2：Create Nalu Header           1 byte
            NALU_HEADER* nalu_hdr =(NALU_HEADER*)&h264_buffer_[4]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
            nalu_hdr->F = fu_ind->F;
            nalu_hdr->NRI = fu_ind->NRI;
            nalu_hdr->TYPE = fu_hdr->TYPE;
            printf("nalu_hdr->F=%d, nalu_hdr->NRI=%d, nalu_hdr->TYPE=%d\n", nalu_hdr->F, nalu_hdr->NRI, nalu_hdr->TYPE);
            pos += 1;

        }else if(fu_hdr->E == 1){
            
        }else{
            
        }
        
        // 3：Append the raw data
        memcpy(&h264_buffer_[pos], &buffer[14], length - 14);
        pos += length - 14;

        if(receiver_){
            receiver_->OnH264RawData(h264_buffer_, pos);
        }

    }else{
        assert(0);
    }
        
    return 0;
}



