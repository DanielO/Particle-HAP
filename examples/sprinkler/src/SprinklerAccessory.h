#ifndef SprinklerAccessory_hpp
#define SprinklerAccessory_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


// Part of XXXXXXXX-0000-1000-8000-0026BB765291
typedef enum {
// iOS 10.3+
    charType_active			= 0xB0, // uint8 RW
    charType_serviceLabelIndex	= 0xCB,
// iOS 11.2+
    charType_inUse			= 0xD2, // uint8 RO
    charType_valveType		= 0xD5, // uint8 RO
    charType_setDuration		= 0xD3, // uint32 RW (seconds)
    charType_remDuration		= 0xD4, // uint32 RO (seconds)
// iOS 12.0+
    charType_isConfigured		= 0xD6,
} extraCharTypes;

// Accessory categories
typedef enum {
    deviceType_sprinklers 		= 28,
} extraDeviceTypes;

// Part of UUID XXXXXXXX-0000-1000-8000-0026BB765291
typedef enum {
    serviceType_valve		= 0xD0,
} extraServiceTypes;

class SprinklerAccessory: public HAPAccessoryDescriptor {
 private:
    bool on = false;

    int pin = D0;

    int REPORT_PERIOD = 2000;
    int lastMS = 0;

    void identify(bool oldValue, bool newValue, HKConnection *sender);

    std::string getActive (HKConnection *sender, int index);
    void setActive (bool oldValue, bool newValue, HKConnection *sender, int index);
    std::string getInUse (HKConnection *sender, int index);
    std::string getValveType (HKConnection *sender, int index);
 public:
    SprinklerAccessory(int pinOutput, bool initialValue) {
	this->pin = pinOutput;
	this->on = initialValue;
    }

    virtual void initAccessorySet();

    virtual int getDeviceType() {
	return deviceType_sprinklers;
    }
    virtual bool handle();
};

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
