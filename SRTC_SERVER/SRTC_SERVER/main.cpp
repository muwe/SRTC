//
//  main.cpp
//  SRTC_SERVER
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "dump2file.hpp"
#include "h264_rtp.hpp"

#define SERVER_PORT 8060
#define SERVER_ADDRESS "127.0.0.1"

#define H264_FILE "/Users/aivensmac/work/study/SRTC/SRTC/resource/server_video.h264"

using namespace SRTC;


class H264FrameReceiver:public H264Receiver
{
public:
    H264FrameReceiver()
    {
        data_dump_.reset(new DumpFile(H264_FILE));
    }
    
    void OnH264RawData(unsigned char* h264, int length) override
    {
        data_dump_->Write((const unsigned char*)h264, (int)length);
    }
private:
    shared_ptr<DumpFile> data_dump_;
};

int main(int argc , char *argv[])
{
    //socket的建立
    char inputBuffer[BUFFER_LENGTH/2] = {0};
    char message[] = {"Hi,this is server.\n"};
    int sockfd = 0,forClientSockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    
    if (sockfd == -1){
        printf("Fail to create a socket.\n");
    }
    
    //socket的連線
    struct sockaddr_in serverInfo,clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    bzero(&serverInfo,sizeof(serverInfo));
    
    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serverInfo.sin_port = htons(SERVER_PORT);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,5);
    
    shared_ptr<H264RtpUnpacket> unpacket(new H264RtpUnpacket(new H264FrameReceiver()));
    //        shared_ptr<DumpFile>    dump_file(new DumpFile(H264_FILE));

    while(true){
        forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);

        printf("Start receive forClientSockfd = %d\n",forClientSockfd);
        ssize_t size = 0;
        
        while((size = recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0)) > 0){
            printf("Received Buffer Size is %ld\n", size);
            unpacket->ParseRtpPacket((unsigned char*)inputBuffer, (int)size);
//            dump_file->Write((unsigned char*)inputBuffer, (int)size);
        }
        printf("Stop receive forClientSockfd = %d\n",forClientSockfd);
    }
    return 0;
}
