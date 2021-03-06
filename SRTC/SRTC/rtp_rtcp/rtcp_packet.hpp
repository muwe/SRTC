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
#include "h264_rtp.h"
#include "rtp_session.hpp"

#define RTCP_COMBINATION_MAX_COUNT 30
#define RTCP_PACKET_MAX_SIZE 1460

namespace SRTC {
    
    struct COMBI_RTCP_INFO
    {
        int type;    /* RTCP packet type SR,RR,SDES,BYE,APP*/
        int offset;     /* the rtcp packet offset in buff */
        int len;    /* the length of this rtcp packet */
    };

#define  PKT_SND_MPEG4     1
#define  PKT_SND_H263      2
#define  PKT_RCV_RTCP      3
#define  PKT_RCV_RFC2833    4

    struct PACKET_INFOS
    {
        unsigned type;       /*packet type such as PKT_RCV_RFC2833 PKT_RCV_RTCP */
        RtpSession *rtp_session;
        unsigned fraction;         /*  fraction lost since last SR/RR */
        unsigned char* event;/*dtmf*/
    };

    class RtcpPacket{
    public:
        RtcpPacket(RtpSession* rtp_session);
        virtual ~RtcpPacket(){};
        
        int Send_Packages();
        int Receive_Package(unsigned char * pkt, int len);
    private:
        int Send_Bye_pkt();
        int Send_Single_Packet();
        int Send_Bye_Or_Emptyrr_Packet(RtpSession* pSession,int bbyepkt);

        int Check_Rtcp_Packet(unsigned char * pkt, int len);
        int Parse_Rtcp_Packet(unsigned char * pkt, int len);
        int Parse_Sr_Packet(unsigned char * pkt);
        int Parse_Rr_Packet(unsigned char * pkt);
        int Parse_Sdes_Packet(unsigned char * pkt);
        int Parse_Bye_Packet(unsigned char * pkt);

        int Get_Curr_Time(timeval *ptv);
        int Get_Curr_Ntp_Rtp_Time(RtpSession* pSession,unsigned long &hntpt,unsigned long &lntpt,unsigned long &rtpt);
        int Stream_Source_Calc_RR(RtpSession* pSession,RTCP_RR *prr);
        
    private:
        COMBI_RTCP_INFO pkt_rtcp_combi_[RTCP_COMBINATION_MAX_COUNT];
        RtpSession* rtp_session_{nullptr};
    };
    
}

#endif /* rtcp_packet_hpp */
