//
//  h264_rtp.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/29.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef h264_rtp_hpp
#define h264_rtp_hpp

#include <stdio.h>

#define H264                    96

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

    class RtpReveiver{
    public:
        virtual void OnRtpData(char* rtp, int length) = 0;
    };
    
    class H264RtpPacket{
    public:
        H264RtpPacket(RtpReveiver* receiver);
        virtual ~H264RtpPacket(){}
        int H264ToRtp(const unsigned char* buffer, int length);
    private:
        NALU_t *AllocNALU(int buffersize);
        void FreeNALU(NALU_t *n);
        int GetAnnexbNALU (unsigned char* inPutBuffer, int length, NALU_t *nalu);
        int FindStartCode2 (unsigned char *Buf);
        int FindStartCode3 (unsigned char *Buf);
        void dump(NALU_t *n);
    private:
        RtpReveiver* receiver_;
        unsigned short seq_num_;

    };
    
    
    
    
    class H264Receiver
    {
    public:
        virtual void OnH264RawData(unsigned char* h264, int length) = 0;
    };
    
    class H264RtpUnpacket
    {
    public:
        H264RtpUnpacket(H264Receiver* receiver);
        ~H264RtpUnpacket(){}
        int RtpToH264(const unsigned char* buffer, int length);
    private:
        H264Receiver* receiver_;
        unsigned char* h24_buffer_;
        int buffer_len_;
    };
}

#endif /* h264_rtp_hpp */
