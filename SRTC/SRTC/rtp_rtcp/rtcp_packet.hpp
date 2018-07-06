//
//  rtcp_packet.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/6.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef rtcp_packet_hpp
#define rtcp_packet_hpp

#include <stdio.h>

class RtcpPacket{
public:
    RtcpPacket();
    virtual ~RtcpPacket();
    
    int Send_Packages();
    int Receive_Package();
private:
    int Send_Bye_pkt(struct RTP_SESSION* psess);
    int Send_Single_Packet(struct RTP_SESSION* psess);
    
    int Parse_Sr_Packet();
    int Parse_Rr_Packet();
    int Parse_Sdes_Packet();
    int Parse_Bye_Packet();
    
};



#endif /* rtcp_packet_hpp */
