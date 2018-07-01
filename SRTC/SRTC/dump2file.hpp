//
//  dump2file.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef dump2file_hpp
#define dump2file_hpp

#include <stdio.h>
#include <string>

using namespace std;

namespace SRTC {
    class DumpFile
    {
    public:
        DumpFile(string outPutFile);
        virtual ~DumpFile();
        int Write(const unsigned char* pData, int length);
    private:
        FILE* file_handle_;
    };
}

#endif /* dump2file_hpp */
