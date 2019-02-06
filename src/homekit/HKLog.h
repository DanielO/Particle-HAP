//
//  HKLog.hpp
//  HKTester
//
//  Created by Lukas Jezny on 06/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef HKLog_hpp
#define HKLog_hpp

#ifdef PARTICLE_COMPAT
#include "../../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif



class HKLog {
public:
    int printf(const char *format, ...);
    int println(const char *value);
};
extern HKLog HKLogger;
#endif /* HKLog_hpp */
