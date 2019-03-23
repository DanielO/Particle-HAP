//
//  HomekitBridgeAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "HomekitBridgeAccessory.h"

bool HomekitBridgeAccessory::handle() {
    bool result = false;
    for(int i = 0; i < descriptors.size(); i++) {
        //process only one accessory per "loop" step. So we dont delay Particle connection, Bonjour so much. Never mind it will be process one step later
        result |= descriptors.at(i)->handle();
    }
    return result;
}

void HomekitBridgeAccessory::initAccessorySet(){
    for(int i = 0; i < descriptors.size(); i++) {
        descriptors.at(i)->initAccessorySet();
    }
}
