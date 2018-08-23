//
//  h264_encode.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef h264_encode_hpp
#define h264_encode_hpp

#include <string>
#include <stdio.h>
#include "/Users/aivensmac/work/study/thirdparty/openh264/codec/api/svc/codec_api.h"

using namespace std;

extern int EncodeFile(string inPutFile, string outPutFile);

namespace SRTC {
    
    class H264DataCallback{
    public:
        virtual void OnDate(const unsigned char* data, int length)= 0;
    };
    
    class H264Encoder
    {
    public:
        H264Encoder(string inPutFile, const H264DataCallback* pDataCallback);
        virtual ~H264Encoder();
        void Encode();
    private:
        H264DataCallback* data_callback_;
        FILE* input_file_;
    };
}

#endif /* h264_encode_hpp */
