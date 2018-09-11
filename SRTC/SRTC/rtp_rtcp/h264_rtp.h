//
//  h264_rtp.h
//  SRTC
//
//  Created by Aiven's Mac on 2018/7/4.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//



#ifndef h264_rtp_h
#define h264_rtp_h



#define H264  96

#define RTP_VERSION    2


// Here we only support little endian

namespace SRTC {
    
    typedef struct
    {
        /**//* byte 0 */
        unsigned char csrc_len:4;        /**//* expect 0 */
        unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
        unsigned char padding:1;        /**//* expect 0 */
        unsigned char version:2;        /**//* expect 2 */
        /**//* byte 1 */
        unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
        unsigned char marker:1;        /**//* expect 1 */
        /**//* bytes 2, 3 */
        unsigned short seq_no;
        /**//* bytes 4-7 */
        unsigned  long timestamp;
        /**//* bytes 8-11 */
        unsigned long ssrc;            /**//* stream number is used here. */
    } RTP_FIXED_HEADER;
    
    typedef struct {
        //byte 0
        unsigned char TYPE:5;
        unsigned char NRI:2;
        unsigned char F:1;
        
    } NALU_HEADER; /**//* 1 BYTES */
    
    typedef struct {
        //byte 0
        unsigned char TYPE:5;
        unsigned char NRI:2;
        unsigned char F:1;
        
    } FU_INDICATOR; /**//* 1 BYTES */
    
    typedef struct {
        //byte 0
        unsigned char TYPE:5;
        unsigned char R:1;
        unsigned char E:1;
        unsigned char S:1;
    } FU_HEADER; /**//* 1 BYTES */
    
    typedef struct
    {
        int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
        unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
        unsigned max_size;            //! Nal Unit Buffer size
        int forbidden_bit;            //! should be always FALSE
        int nal_reference_idc;        //! NALU_PRIORITY_xxxx
        int nal_unit_type;            //! NALU_TYPE_xxxx
        char *buf;                    //! contains the first byte followed by the EBSP
        unsigned short lost_packets;  //! true, if packet loss is detected
    } NALU_t;

    
    
    // Following code is for RTCP
    
    enum RTCP_TYEP
    {
        RTCP_TYPE_SR   = 200,
        RTCP_TYPE_RR   = 201,
        RTCP_TYPE_SDES = 202,
        RTCP_TYPE_BYE  = 203,
        RTCP_TYPE_APP  = 204
    } ;

    enum RTCP_SDES_TYPE
    {
        RTCP_SDES_END   = 0,
        RTCP_SDES_CNAME = 1,
        RTCP_SDES_NAME  = 2,
        RTCP_SDES_EMAIL = 3,
        RTCP_SDES_PHONE = 4,
        RTCP_SDES_LOC   = 5,
        RTCP_SDES_TOOL  = 6,
        RTCP_SDES_NOTE  = 7,
        RTCP_SDES_PRIV  = 8
    };

    
    struct RTCP_COMMON_HEADER
    {
        int count:5;       /* varies by packet type */
        int pad:1;         /* padding flag */
        int ver:2;         /* protocol version */
        
        int pt:8;          /* RTCP packet type */
        int length:16;     /* pkt len in words, w/o this word */
    };
    
    /*
     RTCP Reception report block
     */
    struct RTCP_RR
    {
        unsigned long ssrc;             /* data source being reported */
        unsigned long fraction:8;       /* fraction lost since last SR/RR */
        int lost:24;                    /* cumul. no. pkts lost (signed!) */
        unsigned long last_seq;         /* extended last seq. no. received */
        unsigned long jitter;           /* interarrival jitter */
        unsigned long lsr;                 /* last SR packet from this source */
        unsigned long dlsr;             /* delay since last SR packet */
    } ;

    
    struct RTCP_SDES_ITEM
    {
        unsigned char type;              /* type of item (rtcp_sdes_type_t) */
        unsigned char length;            /* length of item (in octets) */
        char data[1];             /* text, not null-terminated */
    };
    
    struct RTCP_PACKET
    {
        RTCP_COMMON_HEADER common;     /* common header */
        union
        {
            /* sender report (SR) */
            struct rtcp_sr
            {
                int ssrc;     /* sender generating this report */
                int ntp_sec;  /* NTP timestamp */
                int ntp_frac;
                int rtp_ts;   /* RTP timestamp */
                int psent;    /* packets sent */
                int osent;    /* octets sent */
                RTCP_RR rr[1];    /* variable-length list */
            } sr;
            
            /* reception report (RR) */
            struct rtcp_rr
            {
                int ssrc;     /* receiver generating this report */
                RTCP_RR rr[1];    /* variable-length list */
            } rr;
            
            /* source description (SDES) */
            struct rtcp_sdes
            {
                int ssrc;      /* first SSRC/CSRC */
                RTCP_SDES_ITEM item[1]; /* list of SDES items */
            } sdes;
            
            /* BYE */
            struct
            {
                int ssrc[1];   /* list of sources */
                /* can't express trailing text for reason */
            } bye;
        } report;
    };

    
}


#endif /* h264_rtp_h */
