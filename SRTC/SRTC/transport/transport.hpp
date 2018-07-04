//
//  transport.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef transport_hpp
#define transport_hpp

#include <stdio.h>
#include <string>

using namespace std;

namespace SRTC {
    
    enum Transport_Type{
        TCP = 0,
        UDP,
        RELAY
    };
    
    class Transport
    {
    public:
        Transport();
        virtual ~Transport();
        
        int Init(Transport_Type type, string address, int port);
        int Uninit();
        
        int Send(const unsigned char* data, int length);
        void OnReceived();
    private:
        int sockfd_;
    };
}

#endif /* transport_hpp */
