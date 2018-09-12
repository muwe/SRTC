//
//  rtp_session.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/9.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef rtp_session_hpp
#define rtp_session_hpp

#include <stdio.h>
#include "rtp_steam_source.hpp"
#include "vector"

namespace SRTC {
    
#define RTP_SESSION_SPEC_SIZE 63

    struct SESSION_STATES
    {
        struct timeval tp;      /* the last time an RTCP pkt was transmitted */
        struct timeval tn;      /* the next scheduled transmission time of an RTCP pkt */
        unsigned long pmembers;       /* the estimated number of session members at time tm was last recomputed */
        unsigned long members;        /* the most currente estimate for the number of the session members */
        unsigned long senders;        /* the most currente estimate for the number of senders in the session */
        unsigned long rtcp_bw;        /* the target RTCP bandwidht */
        unsigned long avg_rtcp_size;  /* the average Compound RTCP pkt size, in octets, over all RTCP pkts sent and received by this partecipant */
        unsigned long we_sent;        /* flag that is true if the app has sent data since the second previous RTCP Report was transmitted */
        unsigned long initial;    /* the flag that is true if the app has not yet sent an RTCP pkt */
        unsigned long sentpacketall;
        unsigned long sentbyteall;
        unsigned long sentpacket;
        unsigned long sentbyte;
    };

    
//    struct STREAM_SOURCE
//    {
//        unsigned long ssrc;                     /* network byte order (big_endian) */
//        struct sockaddr_in rtp_from;     /* network byte order (big_endian) */
//        struct sockaddr_in rtcp_from;    /* network byte order (big_endian) */
//        SSRC_STATES ssrc_states;
//        SSRC_SDES ssrc_sdes;
//        struct STREAM_SOURCE *next;
//    };

    class RtpSession{
    public:
        RtpSession();
        virtual ~RtpSession(){};
        unsigned long GetLocalSsrc();
        SESSION_STATES* GetSessionStates();
        
        
        StreamSource* pstream_source = nullptr;
//        std::vector<std::shared_ptr<StreamSource>> stream_source_list;
        char spec[RTP_SESSION_SPEC_SIZE+1] = {0};
        unsigned long rtptimestamp;
    private:
        SESSION_STATES sess_states_;
        unsigned long     local_ssrc_;     /* host byte order */;
//        int     rtpfd;
//        int     rtcpfd;
        int     rtp4who;
        int     payloadtype;

    };
}


#endif /* rtp_session_hpp */
