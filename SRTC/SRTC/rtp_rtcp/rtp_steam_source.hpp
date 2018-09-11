//
//  rtp_steam_source.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/8/24.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef rtp_steam_source_hpp
#define rtp_steam_source_hpp

#include <stdio.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

namespace SRTC {
    
    
    struct SSRC_STATES
    {
        unsigned long receivedall;    /* all the received packet from session start*/
        unsigned long received;       /* the received packet from last SR/RR sent */
        unsigned long receivedbyteall; /* all the received rtp payload bytes from session start*/
        unsigned long receivedbyte;    /* the received rtp payload bytes from last SR/RR sent */
        unsigned long base_seq;       /* the seq of first packet from this source */
        unsigned long curr_seq;    /* the current sequence of received packet */
        unsigned long seq_cycles;    /* shifted count of seq. number cycles */
        unsigned long last_srrr_seq;    /* the sequence of last SR/RR sent */
        
        unsigned long currrtptime;    /* rtp time of last rtp packet */
        unsigned long currrtptime_reach; /* reach rtp time of last rtp packet */
        unsigned long jitter;         /* estimated jitter */
        
        unsigned long brecvsr;        /* identify of receive SR packet */
        struct timeval lastsr;  /* last RTCP SR pkt reception time */
        unsigned long ntp_lastsr[2];  /* last RTCP SR pkt NTP reception time */
    };
    
    struct SSRC_SDES
    {
        
#define RTCP_SDES_STR_MAX_SIZE    255
#define RTCP_SDES_STR_NAME_SIZE   23
#define RTCP_SDES_STR_EMAIL_SIZE  47
#define RTCP_SDES_STR_PHONE_SIZE  23
#define RTCP_SDES_STR_LOC_SIZE    23
#define RTCP_SDES_STR_TOOL_SIZE   23
#define RTCP_SDES_STR_NOTE_SIZE   47
        
        char cname[RTCP_SDES_STR_MAX_SIZE+1];
        char name[RTCP_SDES_STR_NAME_SIZE+1];
        char email[RTCP_SDES_STR_EMAIL_SIZE+1];
        char phone[RTCP_SDES_STR_PHONE_SIZE+1];
        char loc[RTCP_SDES_STR_LOC_SIZE+1];
        char tool[RTCP_SDES_STR_TOOL_SIZE+1];
        char note[RTCP_SDES_STR_NOTE_SIZE+1];
    };

    class StreamSource{
    public:
        unsigned long GetSsrc();
        SSRC_STATES* GetSsrcStates();
        SSRC_SDES* GetSsrcSdes();
    private:
        unsigned long ssrc_;                     /* network byte order (big_endian) */
        struct sockaddr_in rtp_from_;     /* network byte order (big_endian) */
        struct sockaddr_in rtcp_from_;    /* network byte order (big_endian) */
        SSRC_STATES ssrc_states_;
        SSRC_SDES ssrc_sdes_;
//        struct STREAM_SOURCE *next;
    };
}


#endif /* rtp_steam_source_hpp */
