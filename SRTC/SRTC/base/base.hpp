//
//  base.hpp
//  SRTC
//
//  Created by Aiven's Mac on 2018/6/28.
//  Copyright © 2018年 Aiven's Mac. All rights reserved.
//

#ifndef base_hpp
#define base_hpp

#include <stdio.h>

typedef unsigned int uint;

int64_t GetSystemTime();
void Sleep(int time);

#define max(a,b) a>b ？a:b
#define min(a,b) a>b ? b:a

#endif /* base_hpp */
