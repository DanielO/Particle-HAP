#ifndef SprinklerAccessory_hpp
#define SprinklerAccessory_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


// Part of XXXXXXXX-0000-1000-8000-0026BB765291
typedef enum {
// iOS 10.3+
    charType_active		= 0xB0, // uint8 RW
    charType_serviceLabelIndex	= 0xCB,
// iOS 11.2+
    charType_inUse		= 0xD2, // uint8 RO
    charType_valveType		= 0xD5, // uint8 RO
    charType_setDuration	= 0xD3, // uint32 RW (seconds)
    charType_remDuration	= 0xD4, // uint32 RO (seconds)
// iOS 12.0+
    charType_isConfigured	= 0xD6,
} extraCharTypes;

// Accessory categories
typedef enum {
    deviceType_sprinklers 	= 28,
} extraDeviceTypes;

// Part of UUID XXXXXXXX-0000-1000-8000-0026BB765291
typedef enum {
    serviceType_valve		= 0xD0,
} extraServiceTypes;

typedef enum {
    valveTypes_generic		= 0,
    valveTypes_irrigation	= 1,
    valveTypes_showerHead	= 2,
    valveTypes_waterFaucet	= 3,
} valveTypes;

struct sprinkler {
    const char	*name;
    int		relayIO;
    int		valveType;
    bool	on;
};

class SprinklerAccessory: public HAPAccessoryDescriptor {
 private:
    struct sprinkler *sprinklers;
    int nsprink;
    int REPORT_PERIOD = 2000;
    int lastMS = 0;

    void identify(bool oldValue, bool newValue, HKConnection *sender);

    std::string getActive (HKConnection *sender, int index);
    void setActive (bool oldValue, bool newValue, HKConnection *sender, int index);
    std::string getInUse (HKConnection *sender, int index);
    std::string getValveType (HKConnection *sender, int index);
 public:
    SprinklerAccessory(struct sprinkler sprinklers[], int nsprink) {
	this->sprinklers = (struct sprinkler *)malloc(sizeof(struct sprinkler) * nsprink);
/* No exceptions enabled so YOLO
	if (this->sprinklers == NULL)
	    throw std::runtime_error("failed to allocate sprinkler data");
*/
	bcopy(sprinklers, this->sprinklers, sizeof(struct sprinkler) * nsprink);
	for (int i = 0; i < nsprink; i++) {
	    this->sprinklers[i].on = false;
	    pinMode(this->sprinklers[i].relayIO, OUTPUT);
	}
	this->nsprink = nsprink;
    }

    ~SprinklerAccessory() {
	free(this->sprinklers);
    }

    virtual void initAccessorySet();

    virtual int getDeviceType() {
	return deviceType_sprinklers;
    }
    virtual bool handle();

    void step(void);
};

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
