
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
    hkLog.info("setActive for index %d (%s) = %s", index, this->sprinklers[index].name, newValue ? "true" : "false");
    this->sprinklers[index].on = newValue;
    if (!oldValue && newValue) {
	this->sprinklers[index].startedAt = Time.now();
	this->sprinklers[index].stopAt = this->sprinklers[index].startedAt + this->sprinklers[index].setDuration;
    }
}

std::string SprinklerAccessory::getInUse (HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return "false";
    hkLog.info("getInUse for index %d (%s) = %s", index, this->sprinklers[index].name, this->sprinklers[index].on ? "true" : "false");
    return this->sprinklers[index].on ? "true" : "false";
}

std::string SprinklerAccessory::getValveType (HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return "";
    hkLog.info("getValveType for index %d (%s) = %d", index, this->sprinklers[index].name, this->sprinklers[index].valveType);
    return format("%d", this->sprinklers[index].valveType);
}

std::string SprinklerAccessory::getSetDuration(HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return "";
    hkLog.info("getSetDuration for index %d (%s) = %d", index, this->sprinklers[index].name, this->sprinklers[index].setDuration);
    return format("%d", this->sprinklers[index].setDuration);
}

void SprinklerAccessory::setSetDuration(int oldValue, int newValue, HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return;
    hkLog.info("setSetDuration for index %d (%s) = %d", index, this->sprinklers[index].name, newValue);
    this->sprinklers[index].setDuration = newValue;
    uint16_t tmp = newValue;
    EEPROM.put(this->EEPROMBase + sizeof(tmp) * index, tmp);
}

std::string SprinklerAccessory::getRemDuration(HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return "";
    int remaining = this->sprinklers[index].stopAt - this->sprinklers[index].startedAt;
    hkLog.info("getRemDuration for index %d (%s) = %d", index, this->sprinklers[index].name, remaining);
    return format("%d", remaining);
}

bool SprinklerAccessory::handle() {
    return false;
}

void SprinklerAccessory::identify(bool oldValue, bool newValue, HKConnection *sender) {
    hkLog.info("Start Identify");
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
	valveType->perUserQuery = std::bind(&SprinklerAccessory::getValveType, this, std::placeholders::_1, i);
	sprinklerAcc->addCharacteristics(sprinklerService, valveType);

	// Set Duration
	intCharacteristics *setDuration = new intCharacteristics(charType_setDuration, premission_read | premission_write | premission_notify, 0, 3600, 1, unit_none);
	setDuration->perUserQuery = std::bind(&SprinklerAccessory::getSetDuration, this, std::placeholders::_1, i);
	setDuration->valueChangeFunctionCall = std::bind(&SprinklerAccessory::setSetDuration, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3, i);
	sprinklerAcc->addCharacteristics(sprinklerService, setDuration);

	// Remaining Duration (no notify, we never allow increasing it independently)
	intCharacteristics *remDuration = new intCharacteristics(charType_remDuration, premission_read, 0, 3600, 1, unit_none);
	remDuration->perUserQuery = std::bind(&SprinklerAccessory::getRemDuration, this, std::placeholders::_1, i);
	sprinklerAcc->addCharacteristics(sprinklerService, remDuration);
    }
}

void SprinklerAccessory::step(void) {
    static uint32_t nextLog = 0;
    uint32_t now = Time.now();
    bool dolog = false;

    if (now > nextLog) {
	dolog = true;
	nextLog = now + 30;
    }

    for (int i = 0; i < this->nsprink; i++) {
	if (this->sprinklers[i].on && this->sprinklers[i].stopAt <= now) {
	    hkLog.info("Stopping index %d (%s)", i, this->sprinklers[i].name);
	    this->sprinklers[i].on = false;
	    this->sprinklers[i].startedAt = 0;
	    this->sprinklers[i].stopAt = 0;
	}
	if (dolog)
	    hkLog.info("Sprinkler %d (%s) is %s", i, this->sprinklers[i].name, this->sprinklers[i].on ? "on" : "off");
	digitalWrite(this->sprinklers[i].relayIO, this->sprinklers[i].on ? HIGH : LOW);
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
