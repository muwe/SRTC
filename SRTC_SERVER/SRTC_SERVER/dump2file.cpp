//
//  dump2file.cpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#include "dump2file.hpp"


using namespace SRTC;

DumpFile::DumpFile(string outPutFile)
:file_handle_(nullptr)
{
    file_handle_ = fopen(outPutFile.c_str(), "wb");
    assert(file_handle_!=nullptr);
}
DumpFile::~DumpFile()
{
    if(file_handle_){
        fclose(file_handle_);
        file_handle_ = nullptr;
    }
}


int DumpFile::Write(const unsigned char* pData, int length)
{
    size_t size = 0;
    
    if(file_handle_){
        size = fwrite(pData, 1, length, file_handle_);
        fflush(file_handle_);
    }
    
    return (int)size;
}

