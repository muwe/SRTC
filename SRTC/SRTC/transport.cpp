//
//  transport.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//
#include "transport.hpp"
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace SRTC;

Transport::Transport()
{
}

Transport::~Transport()
{
    Uninit();
}

int Transport::Init(Transport_Type type, string address, int port)
{
    int ret = 0;
    
    int sock_type = (type == TCP) ? SOCK_STREAM : SOCK_DGRAM;
    sockfd_ = socket(AF_INET , sock_type , 0);
    assert(sockfd_!=0);

    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET;
    
    //localhost test
    info.sin_addr.s_addr = inet_addr(address.c_str());
    info.sin_port = htons(port);
    
    int err = connect(sockfd_,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        printf("Connection error\n");
    }
    
    return ret;
}

int Transport::Uninit()
{
    if(sockfd_){
        shutdown(sockfd_, SHUT_RDWR);
        sockfd_ = 0;
    }

    return 0;
}

int Transport::Send(const unsigned char* data, int length)
{
    assert(sockfd_!=0);
    
    ssize_t size = 0;
    
    if(sockfd_){
        size = send(sockfd_,data,length,0);
        printf("Send::size=%d\n", size);
    }
    return 0;
}

void Transport::OnReceived()
{
    
    
}
