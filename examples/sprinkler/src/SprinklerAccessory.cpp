
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
    if (index < 0 || index > this->nsprink)
	return "false";

    hkLog.info("getActive for index %d (%s) = %s", index, this->sprinklers[index].name, this->sprinklers[index].on ? "true" : "false");
    return this->sprinklers[index].on ? "true" : "false";
}

void SprinklerAccessory::setActive (bool oldValue, bool newValue, HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return;
    hkLog.info("setActive for index %d (%s) = %s", index, this->sprinklers[index].name, newValue ? "true" : "false1");
    this->sprinklers[index].on = newValue;
}

std::string SprinklerAccessory::getInUse (HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return "false";
    hkLog.info("getInUse for index %d (%s) = %s", index, this->sprinklers[index].name, this->sprinklers[index].on ? "true" : "false");
    return this->sprinklers[index].on ? "true" : "false";
}

std::string SprinklerAccessory::getValveType (HKConnection *sender, int index) {
    hkLog.info("getValveType for index %d", index);
    if (index < 0 || index > this->nsprink)
	return "";
    hkLog.info("getValveType for index %d (%s) = %d", index, this->sprinklers[index].name, this->sprinklers[index].valveType);
    return format("%d", this->sprinklers[index].valveType);
}

void SprinklerAccessory::identify(bool oldValue, bool newValue, HKConnection *sender) {
    hkLog.info("Start Identify");
}

bool SprinklerAccessory::handle() {
    for (int i = 0; i < this->nsprink; i++)
	digitalWrite(this->sprinklers[i].relayIO, this->sprinklers[i].on ? HIGH : LOW);
    return false;
}

void SprinklerAccessory::initAccessorySet() {
    hkLog.info("initAccessorySet called");
    // Our accessory
    Accessory *sprinklerAcc = new Accessory();
    addInfoServiceToAccessory(sprinklerAcc, "Sprinkler 1", "darius", "Model A", "1", "1.0.0",
      std::bind(&SprinklerAccessory::identify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3)
    );

    // The set which holds it
    AccessorySet *accSet = &AccessorySet::getInstance();
    accSet->addAccessory(sprinklerAcc);

    for (int i = 0; i < this->nsprink; i++) {
	hkLog.info("Creating service %s", this->sprinklers[i].name);
	// Service for this accessory
	Service *sprinklerService = new Service(serviceType_valve);
	sprinklerAcc->addService(sprinklerService);

	// Characteristics for this service
	// Name
	stringCharacteristics *sprinklerServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
	sprinklerServiceName->characteristics::setValue(this->sprinklers[i].name);
	sprinklerAcc->addCharacteristics(sprinklerService, sprinklerServiceName);

	// Active
	intCharacteristics *active = new intCharacteristics(charType_active, premission_read | premission_write | premission_notify, 0, 1, 1, unit_none);
	active->perUserQuery = std::bind(&SprinklerAccessory::getActive, this, std::placeholders::_1, i);
	active->valueChangeFunctionCall = std::bind(&SprinklerAccessory::setActive, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3, i);
	sprinklerAcc->addCharacteristics(sprinklerService, active);

	// In Use
	intCharacteristics *inUse = new intCharacteristics(charType_inUse, premission_read | premission_notify, 0, 1, 1, unit_none);
	inUse->perUserQuery = std::bind(&SprinklerAccessory::getInUse, this, std::placeholders::_1, i);
	sprinklerAcc->addCharacteristics(sprinklerService, inUse);

	// Valve Type
	intCharacteristics *valveType = new intCharacteristics(charType_valveType, premission_read | premission_notify, 0, 3, 1, unit_none);
	inUse->perUserQuery = std::bind(&SprinklerAccessory::getValveType, this, std::placeholders::_1, i);
	sprinklerAcc->addCharacteristics(sprinklerService, valveType);
    }
}

void SprinklerAccessory::step(void) {
}

/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
