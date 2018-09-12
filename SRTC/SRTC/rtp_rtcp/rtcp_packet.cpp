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
#include "../base/base.hpp"


using namespace SRTC;

RtcpPacket::RtcpPacket(RtpSession* rtp_session)
:rtp_session_(rtp_session)
{
    
}

int RtcpPacket::Send_Packages()
{
    int ii;
    int nn,nmid;
    int fillzeronum;
    int bsr,brr;
    unsigned long tt1,tt2,tt3;
    unsigned short onelen;
    unsigned char* pbuf;
    struct sockaddr_in to;
    RTCP_PACKET* rtcp_pkt;
    
    // modified for multi thread by starhan
    unsigned char rtcp_sbuff[RTCP_PACKET_MAX_SIZE+16];
    
    nn = 0;
    pbuf = rtcp_sbuff;
    
    /* Judge need to send SR */
    bsr = 0;
    if( rtp_session_->GetSessionStates()->sentpacket > 0 )  bsr = 1;

    /* Judge need to send RR information */
//    brr = 0;
//    if( stream_source_calc_recv_source(rtp_session_) > 0 ) brr = 1;

    if( 0 == bsr && 0 == brr )
    {
        Send_Bye_Or_Emptyrr_Packet(rtp_session_,0);
        return 0;
    }

    /* Creating SR or RR Packet */
    
    if( 1 == bsr )
    {
        rtcp_pkt = (RTCP_PACKET *)pbuf;
        rtcp_pkt->common.ver = RTP_VERSION;
        rtcp_pkt->common.pad = 0;
        rtcp_pkt->common.pt = RTCP_TYPE_SR;
        rtcp_pkt->report.sr.ssrc = htonl(rtp_session_->GetLocalSsrc());
        if( Get_Curr_Ntp_Rtp_Time(rtp_session_,tt1,tt2,tt3) < 0 )
        {
            SRTC_ERROR_LOG("rtcp get ntp time error\n");
            return -1;
        }
        rtcp_pkt->report.sr.ntp_sec = htonl(tt1);
        rtcp_pkt->report.sr.ntp_frac = htonl(tt2);
        rtcp_pkt->report.sr.rtp_ts = htonl(tt3);
        
        rtcp_pkt->report.sr.psent = htonl(rtp_session_->GetSessionStates()->sentpacketall);
        rtcp_pkt->report.sr.osent = htonl(rtp_session_->GetSessionStates()->sentbyteall);
        
        if( 1 == brr && (ii = Stream_Source_Calc_RR(rtp_session_,rtcp_pkt->report.sr.rr)) >0 )
        {
            rtcp_pkt->common.count = (unsigned)ii;
            /* need to substract size of RTCP_PKT.sr.rr[0] */
            onelen = sizeof(RTCP_COMMON_HEADER)+sizeof(rtcp_pkt->report.rr)+(ii-1)*sizeof(RTCP_RR);
            rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
        }
        else
        {
            rtcp_pkt->common.count = 0;
            onelen = sizeof(RTCP_COMMON_HEADER)+sizeof(rtcp_pkt->report.rr)-sizeof(RTCP_RR);
            rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
        }
        
    }
    else /* RR type  */
    {
        rtcp_pkt = (RTCP_PACKET *)pbuf;
        rtcp_pkt->common.ver = RTP_VERSION;
        rtcp_pkt->common.pad = 0;
        rtcp_pkt->common.pt = RTCP_TYPE_RR;
        rtcp_pkt->report.rr.ssrc = htonl(rtp_session_->GetLocalSsrc());
        
        if( (ii = Stream_Source_Calc_RR(rtp_session_,rtcp_pkt->report.rr.rr)) >0 )
        {
            rtcp_pkt->common.count = (unsigned)ii;
            onelen = sizeof(RTCP_COMMON_HEADER)+4+ii*sizeof(RTCP_RR);
            rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
        }
        else
        {
            rtcp_pkt->common.count = 0;
            onelen = sizeof(RTCP_COMMON_HEADER)+4;
            rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
        }
        
    }
    
    nn += onelen;
    pbuf += onelen;
    
    /* Add SDES packet */
    
    nmid = strlen(rtp_session_->spec);
    fillzeronum = 4 - (nmid + 2)%4;
    nmid = (nmid+2+4)/4*4;
    
    rtcp_pkt = (RTCP_PACKET *)pbuf;
    rtcp_pkt->common.ver = RTP_VERSION;
    rtcp_pkt->common.pad = 0;
    rtcp_pkt->common.count = 1;
    onelen = sizeof(RTCP_COMMON_HEADER)+4+nmid;
    rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
    rtcp_pkt->common.pt = RTCP_TYPE_SDES;
    
    rtcp_pkt->report.sdes.ssrc = htonl(rtp_session_->GetLocalSsrc());
    rtcp_pkt->report.sdes.item[0].type = RTCP_SDES_CNAME;
    rtcp_pkt->report.sdes.item[0].length = strlen(rtp_session_->spec);
    strcpy(rtcp_pkt->report.sdes.item[0].data,rtp_session_->spec);
    
    bzero(pbuf+onelen-fillzeronum,fillzeronum);
    
    nn += onelen;
    pbuf += onelen;
    
//    /* send rtcp bye packet */
//    bzero(&to,sizeof(struct sockaddr_in));
//    to.sin_family = AF_INET;
//    to.sin_addr.s_addr = htonl(psess->dstaddr);
//    to.sin_port = htons(psess->dstports[1]);
//
//    if( sendto(psess->rtcpfd,rtcp_sbuff,nn,0,(struct sockaddr *)&to,sizeof(to)) < 0 )
//    {
//        DEBUG_RTP(("rtcp send bye packet error\n"));
//        return -1;
//    }
    
    
    rtp_session_->GetSessionStates()->sentpacket = 0;
    rtp_session_->GetSessionStates()->sentbyte = 0;
    
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

int RtcpPacket::Send_Bye_Or_Emptyrr_Packet(RtpSession* pSession,int bbyepkt)
{
    int nn,nmid;
    int fillzeronum;
    unsigned short onelen;
    unsigned char* pbuf;
    struct sockaddr_in to;
    RTCP_PACKET* rtcp_pkt;
    
    // modified for multi thread by starhan
    unsigned char rtcp_sbuff[RTCP_PACKET_MAX_SIZE+16];
    
    nn = 0;
    pbuf = rtcp_sbuff;
    
    /* add RR packet*/
    rtcp_pkt = (RTCP_PACKET *)pbuf;
    rtcp_pkt->common.ver = RTP_VERSION;
    rtcp_pkt->common.pad = 0;
    rtcp_pkt->common.count = 0;
    rtcp_pkt->common.pt = RTCP_TYPE_RR;
    rtcp_pkt->report.rr.ssrc = htonl(pSession->GetLocalSsrc());
    onelen = sizeof(RTCP_COMMON_HEADER) + 4;
    rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
    
    nn += onelen;
    pbuf += onelen;
    
    /* add SDES packet */
    
    nmid = strlen(pSession->spec);
    fillzeronum = 4 - (nmid + 2)%4;
    nmid = (nmid+2+4)/4*4;
    
    rtcp_pkt = (RTCP_PACKET *)pbuf;
    rtcp_pkt->common.ver = RTP_VERSION;
    rtcp_pkt->common.pad = 0;
    rtcp_pkt->common.count = 1;
    onelen = sizeof(RTCP_COMMON_HEADER) + 4 + nmid;
    rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
    rtcp_pkt->common.pt = RTCP_TYPE_SDES;
    
    rtcp_pkt->report.sdes.ssrc = htonl(pSession->GetLocalSsrc());
    rtcp_pkt->report.sdes.item[0].type = RTCP_SDES_CNAME;
    rtcp_pkt->report.sdes.item[0].length = strlen(pSession->spec);
    strcpy(rtcp_pkt->report.sdes.item[0].data,pSession->spec);
    
    bzero(pbuf+onelen-fillzeronum,fillzeronum);
    
    nn += onelen;
    pbuf += onelen;
    
    /* add BYE packet */
    if( 1 == bbyepkt )
    {
        rtcp_pkt = (RTCP_PACKET *)pbuf;
        rtcp_pkt->common.ver = RTP_VERSION;
        rtcp_pkt->common.pad = 0;
        rtcp_pkt->common.count = 1;
        onelen = sizeof(RTCP_COMMON_HEADER) + 4;;
        rtcp_pkt->common.length = htons((unsigned short)(onelen/4-1));
        rtcp_pkt->common.pt = RTCP_TYPE_BYE;
        
        rtcp_pkt->report.bye.ssrc[0] = htonl(pSession->GetLocalSsrc());
        
        nn += onelen;
    }
    
    //    /* send rtcp  packet */
    //    bzero(&to,sizeof(struct sockaddr_in));
    //    to.sin_family = AF_INET;
    //    to.sin_addr.s_addr = htonl(psess->dstaddr);
    //    to.sin_port = htons(psess->dstports[1]);
    //
    //    if( sendto(psess->rtcpfd,rtcp_sbuff,nn,0,(struct sockaddr *)&to,sizeof(to)) < 0 )
    //    {
    //        if( 1 == bbyepkt )
    //        {
    //            DEBUG_RTP(("rtcp send bye packet error\n"));
    //        }
    //        else
    //        {
    //            DEBUG_RTP(("rtcp send empty rr packet error\n"));
    //        }
    //        return -1;
    //    }
    
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
    StreamSource* pstream;
    PACKET_INFOS pinfo;
    struct RTP_SESSION* psess = NULL;

    if( NULL == pkt )
    {
        printf("rtcp pkt SR parse paramter error\n");
        return -1;
    }
    
    rtcp_pkt = (RTCP_PACKET *)pkt;
//    if( unicast == psess->delivery )
    {
//        pstream =rtp_session_->stream_source_list;
        pstream = rtp_session_->pstream_source;
        
        if( NULL == pstream || rtcp_pkt->report.sr.ssrc != pstream->GetSsrc() )
        {
            printf("rtcp unicast sr dealing ssrc error\n");
            return -1;
        }
        
        /* record sr infomation */
        pstream->GetSsrcStates()->brecvsr++;
        Get_Curr_Time(&(pstream->GetSsrcStates()->lastsr));
        pstream->GetSsrcStates()->ntp_lastsr[0] = ntohl(rtcp_pkt->report.sr.ntp_sec);
        pstream->GetSsrcStates()->ntp_lastsr[1] = ntohl(rtcp_pkt->report.sr.ntp_frac);
        
        SRTC_INFO_LOG("recv sr pakcet sent = %d", ntohl(rtcp_pkt->report.sr.psent));
        
        /* dealing rr information */
        if( 1 == rtcp_pkt->common.count && rtp_session_->GetLocalSsrc() == ntohl(rtcp_pkt->report.sr.rr[0].ssrc) )
        {
            /* Record information */
//            SRTC_INFO_LOG("rtcp unicast sr dealing rr lost pkt = %d ",ntohl24(rtcp_pkt->report.sr.rr[0].lost));
        }
        
        memset(&pinfo,0,sizeof(PACKET_INFOS));
        pinfo.type = PKT_RCV_RTCP;
        pinfo.rtp_session = rtp_session_;
        pinfo.fraction = rtcp_pkt->report.sr.rr[0].fraction;
//        if(g_rtp_callback.process_pkt)
//            g_rtp_callback.process_pkt(&pinfo);
        
    }
#if 0
    else /* dealing multicast */
    {
        pstream = rtp_stream_source_match_ssrc(psess,rtcp_pkt->report.sr.ssrc);
        if( pstream != NULL )
        {
            /* record information*/
            pstream->ssrc_states.brecvsr ++;
            rtprtcp_get_curr_time(&(pstream->ssrc_states.lastsr));
            pstream->ssrc_states.ntp_lastsr[0] = ntohl(rtcp_pkt->report.sr.ntp_sec);
            pstream->ssrc_states.ntp_lastsr[1] = ntohl(rtcp_pkt->report.sr.ntp_frac);
            
            DEBUG_RTP(("rtcp multicast sr dealing sr \n"));
        }
        else
        {
            DEBUG_RTP(("rtcp multicast sr no dealing sr \n"));
        }
        
        /* dealing rr information */
        
        for( ii = 0 ; (unsigned)ii < rtcp_pkt->common.count ; ii++ )
        {
            if( psess->local_ssrc == ntohl(rtcp_pkt->report.sr.rr[0].ssrc) )
            {
                /* record information*/
                
                SRTC_ERROR_LOG("rtcp multicast sr dealing rr.");
                break;
            }
        }
    }
#endif
    return 0;
}

int RtcpPacket::Parse_Rr_Packet(unsigned char * pkt)
{
    int ii;
    RTCP_PACKET * rtcp_pkt;
    
    if( NULL == pkt )
    {
        SRTC_ERROR_LOG("rtcp pkt parse RR paramter error");
        return -1;
    }
    
    rtcp_pkt = (RTCP_PACKET *)pkt;
//    if( unicast == rtp_session_->delivery )
    {
        /* dealing rr information */
        if( 1 == rtcp_pkt->common.count && rtp_session_->GetLocalSsrc() == ntohl(rtcp_pkt->report.rr.rr[0].ssrc) )
        {
            /* Record information */
            SRTC_ERROR_LOG("rtcp unicast rr dealing rr.");
        }
    }
#if 0
    else /* dealing multicast */
    {
        /* dealing rr information */
        for( ii = 0 ; (unsigned) ii < rtcp_pkt->common.count ; ii++ )
        {
            if( psess->local_ssrc == ntohl(rtcp_pkt->r.rr.rr[0].ssrc) )
            {
                /* record information*/
                DEBUG_RTP(("rtcp multicast sr dealing rr \n"));
                break;
            }
        }
    }
#endif
    
    return 0;
}

int RtcpPacket::Parse_Sdes_Packet(unsigned char * pkt)
{
    int nn;
    int jj;
    int nmid;
    int minlen;
    unsigned long *pssrc;
    RTCP_SDES_ITEM* psdesitem;
    StreamSource* pstream;
    RTCP_PACKET * rtcp_pkt;
    
    if(NULL == pkt )
    {
        SRTC_ERROR_LOG("rtcp pkt sdes parse paramter error.");
        return -1;
    }
    
    rtcp_pkt = (RTCP_PACKET *)pkt;
    
    nn = sizeof(RTCP_COMMON_HEADER);
    for( jj = 0 ; (unsigned)jj < rtcp_pkt->common.count; jj++)
    {
        pssrc = (unsigned long *)(pkt+nn);
        nn += 4;
        psdesitem = (RTCP_SDES_ITEM *)(pkt+nn);
        
        pstream = rtp_session_->pstream_source;
//        pstream = rtp_stream_source_match_ssrc(rtp_session_,(unsigned long)(*pssrc));
        
        nmid = 0;
        while(1)
        {
            psdesitem = (RTCP_SDES_ITEM *)(pkt+nn+nmid);
            if( RTCP_SDES_END == psdesitem->type )
                break;
            
            nmid += (psdesitem->length + 2);
            /* error source
             if( (nmid + sizeof(RTCP_COMMON_HEADER)+4) >= g_pkt_rtcp_combi[jj].len )
             {
             DLOG(RTCP_PKTSDESPARSE|0x01,LOGC_ERROR,"rtcp multicast sdes length error");
             return -1;
             }
             */
            if( pstream != NULL)
            {
                switch( psdesitem->type )
                {
                    case RTCP_SDES_CNAME:
                        minlen=(RTCP_SDES_STR_MAX_SIZE,psdesitem->length);
                        memcpy(pstream->GetSsrcSdes()->cname,psdesitem->data,minlen);
                        pstream->GetSsrcSdes()->cname[minlen] = '\0';
                        SRTC_INFO_LOG("sdes cname %s\n",pstream->GetSsrcSdes()->cname);
                        break;
                    case RTCP_SDES_NAME:
                        minlen=min(RTCP_SDES_STR_NAME_SIZE,psdesitem->length);
                        memcpy(pstream->GetSsrcSdes()->name,psdesitem->data,minlen);
                        pstream->GetSsrcSdes()->name[minlen] = '\0';
                        SRTC_INFO_LOG("sdes name %s\n",pstream->GetSsrcSdes()->name);
                        break;
                    case RTCP_SDES_EMAIL:
                        minlen=min(RTCP_SDES_STR_EMAIL_SIZE,psdesitem->length);
                        memcpy(pstream->GetSsrcSdes()->email,psdesitem->data,minlen);
                        pstream->GetSsrcSdes()->email[minlen] = '\0';
                        SRTC_INFO_LOG("sdes email name %s\n",pstream->GetSsrcSdes()->email);
                        break;
                    case RTCP_SDES_PHONE:
                        minlen=min(RTCP_SDES_STR_PHONE_SIZE,psdesitem->length);
                        memcpy(pstream->GetSsrcSdes()->phone,psdesitem->data,minlen);
                        pstream->GetSsrcSdes()->phone[minlen] = '\0';
                        SRTC_INFO_LOG("sdes phone %s\n",pstream->GetSsrcSdes()->phone);
                        break;
                    case RTCP_SDES_LOC:
                        minlen=min(RTCP_SDES_STR_LOC_SIZE,psdesitem->length);
                        memcpy(pstream->GetSsrcSdes()->loc,psdesitem->data,minlen);
                        pstream->GetSsrcSdes()->loc[minlen] = '\0';
                        SRTC_INFO_LOG("sdes loc %s\n",pstream->GetSsrcSdes()->loc);
                        break;
                    case RTCP_SDES_TOOL:
                        minlen=min(RTCP_SDES_STR_TOOL_SIZE,psdesitem->length);
                        memcpy(pstream->GetSsrcSdes()->tool,psdesitem->data,minlen);
                        pstream->GetSsrcSdes()->tool[minlen] = '\0';
                        SRTC_INFO_LOG("sdes tool %s\n",pstream->GetSsrcSdes()->tool);
                        break;
                    case RTCP_SDES_NOTE:
                        minlen=min(RTCP_SDES_STR_NOTE_SIZE,psdesitem->length);
                        memcpy(pstream->GetSsrcSdes()->note,psdesitem->data,minlen);
                        pstream->GetSsrcSdes()->note[minlen] = '\0';
                        SRTC_INFO_LOG("sdes note %s\n",pstream->GetSsrcSdes()->note);
                        break;
                }
            }
        }
        
        nn += (nmid+4)/4*4;
    }
    
    return 0;
    
}

int RtcpPacket::Parse_Bye_Packet(unsigned char * pkt)
{
    int jj;
    StreamSource* pstream;
    RTCP_PACKET * rtcp_pkt;
    
    if( NULL == pkt )
    {
        SRTC_ERROR_LOG("rtcp pkt bye parse paramter error.");
        return -1;
    }
    
    rtcp_pkt = (RTCP_PACKET *)pkt;
    
    for( jj = 0 ; (unsigned)jj < rtcp_pkt->common.count; jj++)
    {
//        pstream = NULL;
        pstream = rtp_session_->pstream_source;
//        pstream = rtp_stream_source_match_ssrc(pkt,rtcp_pkt->report.bye.ssrc[jj]);
        
        if( pstream != NULL )
        {
//            rtp_stream_source_chain_del(pkt,pkt);
        }
    }
    
    return 0;
}

int RtcpPacket::Get_Curr_Ntp_Rtp_Time(RtpSession* pSession,unsigned long &hntpt,unsigned long &lntpt,unsigned long &rtpt)
{
    
    unsigned long lmid;
#ifdef WIN32
    struct _timeb timebuffer;
#endif
    
    if( NULL == pSession )
    {
        SRTC_ERROR_LOG("rtcp_get_curr_ntp_rtp_time parameter error.");
        return -1;
    }
    
#ifdef WIN32
    _ftime( &timebuffer );
    *phntpt = timebuffer.time;
    
    lmid = (unsigned long)(timebuffer.millitm);
    lmid = (lmid*0x10000/1000)<<16;
    
    *plntpt = lmid;
    *prtpt = psess->ttime+0x20;
#else
    {
        struct timeval now;
        struct timezone tz;
        
        gettimeofday(&now, &tz);
        
        hntpt = now.tv_sec;
        lmid = (unsigned long)(now.tv_usec);
        lmid = (lmid/(1000000/0x10000))<<16;    /* usec -> 1/0x10000 sec */
        
        lntpt = lmid;
        rtpt = pSession->rtptimestamp;
    }
#endif
    return 0;
}

int RtcpPacket::Get_Curr_Time(timeval *ptv)
{
#ifdef WIN32
    struct _timeb timebuffer;
#endif
    
    if( NULL == ptv )
    {
        SRTC_ERROR_LOG("rtp rtprtcp_get_curr_time parameter error.");
        return -1;
    }
    
#ifdef WIN32
    _ftime( &timebuffer );
    ptv->tv_sec = timebuffer.time;
    ptv->tv_usec = timebuffer.millitm;
#else
    {
        struct timeval now;
        struct timezone tz;
        
        gettimeofday(&now, &tz);
        ptv->tv_sec = now.tv_sec;
        ptv->tv_usec = now.tv_usec;
    }
#endif
    return 0;
}


int RtcpPacket::Stream_Source_Calc_RR(RtpSession* pSession,RTCP_RR *prr)
{
    return 0;
}
