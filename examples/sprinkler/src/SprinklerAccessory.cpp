
#include "SprinklerAccessory.h"

#include "HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../../utils/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif

#include "HKLog.h"

std::string SprinklerAccessory::getActive (HKConnection *sender, int index) {
    Serial.printf("getActive for index %d\n", index);
    return on ? "true" : "false";
}

void SprinklerAccessory::setActive (bool oldValue, bool newValue, HKConnection *sender, int index) {
    Serial.printf("setActive for index %d\n", index);
    on = newValue;
}

std::string SprinklerAccessory::getInUse (HKConnection *sender, int index) {
    Serial.printf("getInUse for index %d\n", index);
    return on ? "true" : "false";
}

std::string SprinklerAccessory::getValveType (HKConnection *sender, int index) {
    Serial.printf("getValveType for index %d\n", index);
    return format("%d", 1);
}

void SprinklerAccessory::identify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify\n");
}


bool SprinklerAccessory::handle() {
    digitalWrite(pin, on ? HIGH : LOW);
    return false;
}

void SprinklerAccessory::initAccessorySet() {
    pinMode(pin, OUTPUT);

    // Our accessory
    Accessory *sprinklerAcc = new Accessory();
    addInfoServiceToAccessory(sprinklerAcc, "Sprinkler 1", "darius", "Model A", "1","1.0.0", std::bind(&SprinklerAccessory::identify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));

    // The set which holds it
    AccessorySet *accSet = &AccessorySet::getInstance();
    accSet->addAccessory(sprinklerAcc);

    // Service for this accessor
    Service *sprinklerService = new Service(serviceType_valve);
    sprinklerAcc->addService(sprinklerService);

    // Characteristics for this service
    // Name
    stringCharacteristics *sprinklerServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
    sprinklerServiceName->characteristics::setValue("Beds");
    sprinklerAcc->addCharacteristics(sprinklerService, sprinklerServiceName);

    // Active
    intCharacteristics *active = new intCharacteristics(charType_active, premission_read | premission_write | premission_notify, 0, 1, 1, unit_none);
    active->perUserQuery = std::bind(&SprinklerAccessory::getActive, this, std::placeholders::_1, 0);
    active->valueChangeFunctionCall = std::bind(&SprinklerAccessory::setActive, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3, 0);
    sprinklerAcc->addCharacteristics(sprinklerService, active);

    // In Use
    intCharacteristics *inUse = new intCharacteristics(charType_active, premission_read | premission_notify, 0, 1, 1, unit_none);
    inUse->perUserQuery = std::bind(&SprinklerAccessory::getInUse, this, std::placeholders::_1, 0);
    sprinklerAcc->addCharacteristics(sprinklerService, inUse);

    // Valve Type
    intCharacteristics *valveType = new intCharacteristics(charType_active, premission_read | premission_notify, 0, 3, 1, unit_none);
    inUse->perUserQuery = std::bind(&SprinklerAccessory::getValveType, this, std::placeholders::_1, 0);
    sprinklerAcc->addCharacteristics(sprinklerService, valveType);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
