// Please uncomment #define PMS_DYNAMIC in pmsConfig.h file
// or use #define PMS_DYNAMIC before #include <pms.h>

#define PMS_DYNAMIC

#include <pms.h>

////////////////////////////////////////

PmsAltSerial pmsSerial;

#if defined PMS_DYNAMIC
pmsx::Pms* pms = nullptr;
#else
pmsx::Pms pms(&pmsSerial);
#endif

// * PMS5003 Pin 1 : VCC +5V
// * PMS5003 Pin 2 : GND
// Important: pms5003 uses 3.3V logic. Use converters if required or make sure your Arduino board uses 3.3V logic too.
// * PMS5003 Pin 4 : Digital pin 9 (there is no choice, forced by AltSerial)
// * PMS5003 Pin 5 : Digital pin 8 (there is no choice, forced by AltSerial)
// * Optional
//   * PMS5003 Pin 3 : Digital pin 7 (can be changed or not connected at all)
//   * PMS5003 Pin 6 : Digital pin 6 (can be changed or not connected at all)

// if PMS5003 Pin 3  and PMS5003 Pin 3 are not connected
// constexpr uint8_t pinReset = pmsx::Pms::pinNone;
// constexpr uint8_t pinSleepMode = pmsx::Pms::pinNone;

// if PMS5003 Pin 3  and PMS5003 Pin 3 are connected
constexpr uint8_t pinReset = 6;
constexpr uint8_t pinSleepMode = 7;

////////////////////////////////////////

float PM25[10];
int i=0,j=0;

void setup(void) {
	Serial.begin(9600);
	while (!Serial) {}
	Serial.println(pmsx::pmsxApiVersion);

#if defined PMS_DYNAMIC
	pms = new pmsx::Pms(&pmsSerial);
	if (!pms->initialized()) {
#else
	if (!pms->begin()) {
#endif
		Serial.println("Serial communication with PMS sensor failed");
		return;
	}

	pms->setPinReset(pinReset);
	pms->setPinSleepMode(pinSleepMode);

	if (!pms->write(pmsx::PmsCmd::CMD_RESET)) {
		pms->write(pmsx::PmsCmd::CMD_SLEEP);
		pms->write(pmsx::PmsCmd::CMD_WAKEUP);
	}
	pms->write(pmsx::PmsCmd::CMD_MODE_PASSIVE);
	pms->write(pmsx::PmsCmd::CMD_READ_DATA);
	pms->waitForData(pmsx::Pms::TIMEOUT_PASSIVE, pmsx::PmsData::FRAME_SIZE);
	pmsx::PmsData data;
	auto status = pms->read(data);
	if (status != pmsx::PmsStatus::OK) {
		Serial.print("PMS sensor: ");
		Serial.println(status.getErrorMsg());
	}
	pms->write(pmsx::PmsCmd::CMD_MODE_ACTIVE);
	if (!pms->isWorking()) {
		Serial.println("PMS sensor failed");
	}

	Serial.print("Time of setup(): ");
	Serial.println(millis());
}

////////////////////////////////////////
void loop(void) {

	static auto lastRead = millis();
	pmsx::PmsData data;
	auto status = pms->read(data);
	switch (status) {
	case pmsx::PmsStatus::OK: {
		for(i=0;i<10;i++){
		auto view = data.particles;
    auto data_PM25 = view.getLevel(3);
    PM25[i] = data_PM25;
    Serial.println(PM25[i]);
    }
  }
	case pmsx::PmsStatus::NO_DATA:
		break;
	default:
		Serial.print("!!! Pms error: ");
		Serial.println(status.getErrorMsg());
	}
}