
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
    return this->sprinklers[index].on ? "1" : "0";
}

void SprinklerAccessory::setActive (bool oldValue, bool newValue, HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return;
    hkLog.info("setActive for index %d (%s) = %s", index, this->sprinklers[index].name, newValue ? "true" : "false");
    this->sprinklers[index].on = newValue;
    if (!oldValue && newValue) {
	this->sprinklers[index].startedAt = Time.now();
	this->sprinklers[index].stopAt = this->sprinklers[index].startedAt + this->sprinklers[index].setDuration;
	this->sprinklers[index].remChar->notify(NULL);
	this->sprinklers[index].inUseChar->notify(NULL);
    }

    if (oldValue && !newValue) {
	this->sprinklers[index].startedAt = 0;
	this->sprinklers[index].stopAt = 0;
	this->sprinklers[index].remChar->notify(NULL);
	this->sprinklers[index].inUseChar->notify(NULL);
  }
}

std::string SprinklerAccessory::getInUse (HKConnection *sender, int index) {
    if (index < 0 || index > this->nsprink)
	return "false";
    hkLog.info("getInUse for index %d (%s) = %s", index, this->sprinklers[index].name, this->sprinklers[index].on ? "true" : "false");
    return this->sprinklers[index].on ? "1" : "0";
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
    int remaining = this->sprinklers[index].stopAt - Time.now();
    if (remaining < 0)
	remaining = 0;
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

    int count = 1;
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

	// Service Label Index
	intCharacteristics *serviceLabelIndex = new intCharacteristics(charType_serviceLabelIndex, premission_read, 1, 255, 1, unit_none);
	serviceLabelIndex->characteristics::setValue(format("%d", count++));
	sprinklerAcc->addCharacteristics(sprinklerService, serviceLabelIndex);

	// Service Label Namespace
	intCharacteristics *serviceLabelNamespace = new intCharacteristics(charType_serviceLabelIndex, premission_read, 0, 1, 1, unit_none);
	serviceLabelNamespace->characteristics::setValue("1");
	sprinklerAcc->addCharacteristics(sprinklerService, serviceLabelNamespace);

	// Active
	intCharacteristics *active = new intCharacteristics(charType_active, premission_read | premission_write | premission_notify, 0, 1, 1, unit_none);
	active->perUserQuery = std::bind(&SprinklerAccessory::getActive, this, std::placeholders::_1, i);
	active->valueChangeFunctionCall = std::bind(&SprinklerAccessory::setActive, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3, i);
	this->sprinklers[i].activeChar = active;
	sprinklerAcc->addCharacteristics(sprinklerService, active);

	// In Use
	intCharacteristics *inUse = new intCharacteristics(charType_inUse, premission_read | premission_notify, 0, 1, 1, unit_none);
	inUse->perUserQuery = std::bind(&SprinklerAccessory::getInUse, this, std::placeholders::_1, i);
	this->sprinklers[i].inUseChar = inUse;
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

	// Remaining Duration
	intCharacteristics *remDuration = new intCharacteristics(charType_remDuration, premission_read | premission_notify, 0, 3600, 1, unit_none);
	remDuration->perUserQuery = std::bind(&SprinklerAccessory::getRemDuration, this, std::placeholders::_1, i);
	this->sprinklers[i].remChar = remDuration;
	sprinklerAcc->addCharacteristics(sprinklerService, remDuration);
    }
}

void SprinklerAccessory::step(void) {
    static uint32_t nextLog = 0;
    static uint32_t nextPublish = 0;
    uint32_t now = Time.now();
    bool dolog = false;
    bool dopublish = false;

    if (now > nextLog) {
	dolog = true;
	nextLog = now + 30;
    }

    if (now > nextPublish) {
	dopublish = true;
	nextPublish = now + 120;
    }

    for (int i = 0; i < this->nsprink; i++) {
	if (this->sprinklers[i].on && this->sprinklers[i].stopAt <= now) {
	    hkLog.info("Stopping index %d (%s)", i, this->sprinklers[i].name);
	    this->sprinklers[i].on = false;
	    this->sprinklers[i].startedAt = 0;
	    this->sprinklers[i].stopAt = 0;
	    this->sprinklers[i].activeChar->notify(NULL);
	    this->sprinklers[i].inUseChar->notify(NULL);
	}
	if (dolog)
	    hkLog.info("Sprinkler %d (%s) is %s", i, this->sprinklers[i].name, this->sprinklers[i].on ? "on" : "off");

	if (dopublish)
	    Particle.publish("sprinkler/status", format("Sprinkler %d (%s) is %s", i, this->sprinklers[i].name, this->sprinklers[i].on ? "on" : "off").c_str());

	digitalWrite(this->sprinklers[i].relayIO, this->sprinklers[i].on ? HIGH : LOW);
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
