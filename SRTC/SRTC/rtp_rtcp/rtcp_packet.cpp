//
//  rtcp_packet.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/6.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "rtcp_packet.hpp"
#include <memory.h>
#include <arpa/inet.h>
#include "../base/srtc_log.hpp"

using namespace SRTC;

int RtcpPacket::Send_Packages()
{
    return 0;
}

int RtcpPacket::Receive_Package(unsigned char * pkt, int len)
{
    if(pkt ==nullptr || len <= 0){
        SRTC_INFO_LOG("Receive_Package---no data.pkt=%ld, len=%d\n", pkt, len);
        return -1;
    }
    
    int pktlen = len;
    unsigned char* rtcp_rbuff = pkt;

    /* Check packet header and content*/
    
    if( Check_Rtcp_Packet(rtcp_rbuff,pktlen) < 0)
    {
        printf("packet check failed.\n");
        return -1;
    }
    
//    /* Check ssrc of rtcp packet */
//
//    if( rtcp_pkt_ssrc_check(rtcp_rbuff,&from) < 0 )
//    {
//        printf("rtcp packet ssrc check failed\n");
//        return -1;
//    }
    
    /* Parse rtcp packet */
    if( Parse_Rtcp_Packet(rtcp_rbuff, len) < 0 )
    {
        printf("rtcp packet parse failed\n");
        return -1;
    }

    return 0;
}

int RtcpPacket::Send_Bye_pkt()
{
    return 0;
}

int RtcpPacket::Send_Single_Packet()
{
    return 0;
}


int RtcpPacket::Check_Rtcp_Packet(unsigned char * pkt, int len)
{
    int ii,nn;
    int onelen;
    int bfind_sdes;
    
    RTCP_PACKET * rtcp_pkt;
    
    if(NULL == pkt || len <= 0)
    {
        printf("praramter error!!!\n");
        return -1;
    }
    
    memset(pkt_rtcp_combi_,0x0, sizeof(COMBI_RTCP_INFO)*RTCP_COMBINATION_MAX_COUNT);
    
    ii = 0;
    nn = 0;
    bfind_sdes = 0;
    while(1)
    {
        rtcp_pkt = (RTCP_PACKET *)(pkt+nn);
        if( rtcp_pkt->common.ver != RTP_VERSION )
        {
            printf("rtcp pakcet version mismatching %d\n",rtcp_pkt->common.ver);
            return -1;
        }
        
        if( rtcp_pkt->common.pt < RTCP_TYPE_SR || rtcp_pkt->common.pt > RTCP_TYPE_APP )
        {
            printf("rtcp pakcet type mismatching %d\n",rtcp_pkt->common.pt);
            return -1;
        }
#if 0
        /* first packet must be SR or RR type */
        if( ii == 0 && rtcp_pkt->common.pt != RTCP_TYPE_SR && rtcp_pkt->common.pt != RTCP_TYPE_RR )
        {
            DLOG(RTCP_PACKETVALCHK|0x04,LOGC_ERROR,"rtcp first pakcet type is not SR or RR");
            return -1;
        }
        
        if ( RTCP_TYPE_SDES == rtcp_pkt->common.pt )
        {
            bfind_sdes = 1;
        }
#endif
        /*
         RTCP 报文中长度字段的含意为:
         "The length of this RTCP packet in 32-bit words minus one,"
         参见 RFC1889 6.3.1
         */
        onelen = (ntohs((unsigned short)(rtcp_pkt->common.length))+1)*4;
        if( ( nn + onelen ) <= len )
        {
            pkt_rtcp_combi_[ii].type = rtcp_pkt->common.pt;
            pkt_rtcp_combi_[ii].offset = nn;
            pkt_rtcp_combi_[ii].len = onelen;
            
            ii++;
            nn += onelen;
            
            if ( nn == len )
            {
                printf("rtcp packet check ok\n");
                break;
            }
            
            if( ii >= RTCP_COMBINATION_MAX_COUNT )
            {
                printf("rtcp too many combination packet\n");
                return -1;
            }
        }
        else  /* nn + onelen ) > len */
        {
            printf("rtcp packet length mismatching\n");
            return -1;
        }
        
    }
    
    /* There is no SDES packet or no combination packet */
#if 0
    if( ii < 2 )
    {
        DLOG(RTCP_PACKETVALCHK|0x08,LOGC_ERROR,"rtcp only one packet, no combination");
        return -1;
    }
    
    if( 0 == bfind_sdes )
    {
        DLOG(RTCP_PACKETVALCHK|0x09,LOGC_ERROR,"rtcp no SDES packet");
        return -1;
    }
#endif
    
    return 0;
}

int RtcpPacket::Parse_Rtcp_Packet(unsigned char * pkt, int len)
{
    int ii;
    unsigned char* pbuf;
    
    if( NULL == pkt || 0 == len )
    {
        printf("rtcp pkt parse parameter error!!!\n");
        return -1;
    }
    
    for( ii = 0 ; ii< RTCP_COMBINATION_MAX_COUNT ; ii++)
    {
        if( pkt_rtcp_combi_[ii].type == 0) break;
        pbuf = pkt + pkt_rtcp_combi_[ii].offset;
        
        switch( pkt_rtcp_combi_[ii].type )
        {
            case RTCP_TYPE_SR:
                Parse_Sr_Packet(pbuf);
                break;
            case RTCP_TYPE_RR:
                Parse_Rr_Packet(pbuf);
                break;
            case RTCP_TYPE_SDES:
                Parse_Sdes_Packet(pbuf);
                break;
            case RTCP_TYPE_BYE:
                Parse_Bye_Packet(pbuf);
            default:
                break;
        }
    }

    return 0;
}

int RtcpPacket::Parse_Sr_Packet(unsigned char * pkt)
{
    int ii;
    RTCP_PACKET * rtcp_pkt;
    struct STREAM_SOURCE* pstream;
    PACKET_INFOS pinfo;
    
//    if( NULL == pkt )
//    {
//        printf("rtcp pkt SR parse paramter error\n");
//        return -1;
//    }
//    
//    rtcp_pkt = (RTCP_PACKET *)pkt;
//    if( unicast == psess->delivery )
//    {
//        pstream = psess->pstream_source;
//        if( NULL == pstream || rtcp_pkt->r.sr.ssrc != pstream->ssrc )
//        {
//            printf("rtcp unicast sr dealing ssrc error\n");
//            return -1;
//        }
//        
//        /* record sr infomation */
//        pstream->ssrc_states.brecvsr++;
//        rtprtcp_get_curr_time(&(pstream->ssrc_states.lastsr));
//        pstream->ssrc_states.ntp_lastsr[0] = ntohl(rtcp_pkt->r.sr.ntp_sec);
//        pstream->ssrc_states.ntp_lastsr[1] = ntohl(rtcp_pkt->r.sr.ntp_frac);
//        
//        DEBUG_RTP(("recv sr pakcet sent = %d", ntohl(rtcp_pkt->r.sr.psent)));
//        
//        /* dealing rr information */
//        if( 1 == rtcp_pkt->common.count && psess->local_ssrc == ntohl(rtcp_pkt->r.sr.rr[0].ssrc) )
//        {
//            /* Record information */
//            printf("rtcp unicast sr dealing rr lost pkt = %d ",ntohl24(rtcp_pkt->r.sr.rr[0].lost));
//        }
//        
//        memset(&pinfo,0,sizeof(PACKET_INFOS));
//        pinfo.type = PKT_RCV_RTCP;
//        pinfo.rtpsess = psess;
//        pinfo.fraction = rtcp_pkt->r.sr.rr[0].fraction;
//        if(g_rtp_callback.process_pkt)
//            g_rtp_callback.process_pkt(&pinfo);
//        
//    }
//    else /* dealing multicast */
//    {
//        pstream = rtp_stream_source_match_ssrc(psess,rtcp_pkt->r.sr.ssrc);
//        if( pstream != NULL )
//        {
//            /* record information*/
//            pstream->ssrc_states.brecvsr ++;
//            rtprtcp_get_curr_time(&(pstream->ssrc_states.lastsr));
//            pstream->ssrc_states.ntp_lastsr[0] = ntohl(rtcp_pkt->r.sr.ntp_sec);
//            pstream->ssrc_states.ntp_lastsr[1] = ntohl(rtcp_pkt->r.sr.ntp_frac);
//            
//            DEBUG_RTP(("rtcp multicast sr dealing sr \n"));
//        }
//        else
//        {
//            DEBUG_RTP(("rtcp multicast sr no dealing sr \n"));
//        }
//        
//        /* dealing rr information */
//        
//        for( ii = 0 ; (unsigned)ii < rtcp_pkt->common.count ; ii++ )
//        {
//            if( psess->local_ssrc == ntohl(rtcp_pkt->r.sr.rr[0].ssrc) )
//            {
//                /* record information*/
//                
//                DEBUG_RTP(("rtcp multicast sr dealing rr \n"));
//                break;
//            }
//        }
//    }

    return 0;
}

int RtcpPacket::Parse_Rr_Packet(unsigned char * pkt)
{
    return 0;
}

int RtcpPacket::Parse_Sdes_Packet(unsigned char * pkt)
{
    return 0;
}

int RtcpPacket::Parse_Bye_Packet(unsigned char * pkt)
{
    return 0;
}
