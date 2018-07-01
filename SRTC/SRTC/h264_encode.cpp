//
//  h264_encode.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "h264_encode.hpp"
#include "base.hpp"

using namespace std;
using namespace SRTC;

#define PIC_WIDTH 320
#define PIC_HEIGH 240

H264Encoder::H264Encoder(string inPutFile, const H264DataCallback* pDataCallback)
:data_callback_(nullptr)
{
    input_file_ = fopen(inPutFile.c_str(), "rb");
    assert(input_file_!=0);
    
    data_callback_ = (H264DataCallback*)pDataCallback;
}

H264Encoder::~H264Encoder()
{
    if(input_file_){
        fclose(input_file_);
        input_file_ = nullptr;
    }
}

void H264Encoder::Encode()
{
    ISVCEncoder* encoder = nullptr;
    if(WelsCreateSVCEncoder(&encoder)){
        printf("WelsCreateSVCEncoder Error\n");
        return;
    }
    
    SEncParamBase EnxParam;
    SSourcePicture SrcPic;
    SFrameBSInfo BsInfo;
    long long last_time = 0;
    
    uint8_t buf[PIC_WIDTH*PIC_HEIGH*3/2+100] = {0};
    
    EnxParam.iPicWidth = PIC_WIDTH;
    EnxParam.iPicHeight = PIC_HEIGH;
    EnxParam.iRCMode = RC_BITRATE_MODE;
    EnxParam.fMaxFrameRate = 15;
    EnxParam.iTargetBitrate = 500*1024;
    
    if(encoder->Initialize(&EnxParam)){
        printf("Initialize Error\n");
        return;
    }
    
    int index = 0;
    while(input_file_) {
        
        size_t length = fread(buf, 1, PIC_WIDTH*PIC_HEIGH*3/2, input_file_);
        // FIX ME:: we need to check the data length which fread return.
        if(length){
            
            SrcPic.iStride[0] = PIC_WIDTH;
            SrcPic.iStride[1] = PIC_WIDTH>>1;
            SrcPic.iStride[2] = PIC_WIDTH>>1;
            SrcPic.pData[0]     = buf;
            SrcPic.pData[1]     = SrcPic.pData[0] + EnxParam.iPicWidth * EnxParam.iPicHeight;
            SrcPic.pData[2]     = SrcPic.pData[1] + (EnxParam.iPicWidth * EnxParam.iPicHeight >> 2);
            SrcPic.iColorFormat = videoFormatI420;
            SrcPic.iPicHeight = PIC_HEIGH;
            SrcPic.iPicWidth = PIC_WIDTH;
            SrcPic.uiTimeStamp = !last_time ? 0 : (GetSystemTime() - last_time);
            
            last_time = GetSystemTime();
            printf("input::index=%d, iColorFormat=%d, uiTimeStamp=%lld\n" , index, SrcPic.iColorFormat, SrcPic.uiTimeStamp);
            if(cmResultSuccess == encoder->EncodeFrame(&SrcPic, &BsInfo)){
                
                for (int i = 0; i < BsInfo.iLayerNum; ++i) {
                    const SLayerBSInfo& layerInfo = BsInfo.sLayerInfo[i];
                    int layerSize = 0;
                    for (int j = 0; j < layerInfo.iNalCount; ++j) {
                        layerSize += layerInfo.pNalLengthInByte[j];
                    }
                    
                    if(data_callback_){
                        data_callback_->OnDate(layerInfo.pBsBuf, layerSize);
                    }
                    printf("output::eFrameType=%d, BsInfo.uiTimeStamp=%lld, iFrameSizeInBytes=%d,iLayerNum=%d\n" , BsInfo.eFrameType, BsInfo.uiTimeStamp, layerSize, BsInfo.iLayerNum);
                }
                
            }
            index++;
            
            // sleep for 30-40ms one frame
            Sleep(20);
        }else{
            break;
        }
        
    };
    
    if(encoder){
        WelsDestroySVCEncoder(encoder);
    }
    
}
