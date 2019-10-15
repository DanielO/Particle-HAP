/*
 * Project sprinkler
 */

#include "HKServer.h"
#include "HKLog.h"

#include "SprinklerAccessory.h"

struct sprinkler sprinklers[] = {
    { .name = "Front", 		.relayIO = D0, .valveType = valveTypes_irrigation },
    { .name = "Garden Beds",	.relayIO = D1, .valveType = valveTypes_irrigation },
    { .name = "Side",		.relayIO = D2, .valveType = valveTypes_irrigation },
};

SerialLogHandler logHandler(LOG_LEVEL_ALL);

SprinklerAccessory *acc;

HKServer *hkServer = NULL;

void progress(Progress_t progress) {
    hkLog.info("Homekit progress callback: %d", progress);
}

// Cloud functions must return int and take one String
int restart(String extra) {
    System.reset();
    return 0;
}

// setup() runs once, when the device is first turned on.
void setup() {
    randomSeed(Time.now()); // we need to somehow init random seed, so device identity will be unique
    Serial.begin(115200);

    // Set all 4 otherwise the last relay will stay on
    pinMode(D0, OUTPUT);
    digitalWrite(D0, LOW);
    pinMode(D1, OUTPUT);
    digitalWrite(D1, LOW);
    pinMode(D2, OUTPUT);
    digitalWrite(D2, LOW);
    pinMode(D3, OUTPUT);
    digitalWrite(D3, LOW);

    acc = new SprinklerAccessory(sprinklers, sizeof(sprinklers) / sizeof(sprinklers[0]), 0);

    hkServer = new HKServer(acc->getDeviceType(), "Sprinkler", "523-12-643", progress);

    acc->initAccessorySet();

    hkServer->start();

    Particle.function("restart", restart);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
    bool didAnything = false; // !hkServer->hasConnections();
    didAnything |= hkServer->handle(); // handle connections, did anything (i.e processed some requests etc.)
    didAnything |= acc->handle(); // handle accessory, did anything (i.e read some sensors)
    if (didAnything) {
	//hkLog.info("Free memory %lu",System.freeMemory());
    }

    acc->step();
}
/*
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */

