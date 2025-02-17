#include "relay_manager.h"
#include "Arduino.h"

//Shortening defines for tidiness
#define P_FAN FAN_RELAY_PIN
#define P_HEATER HEATER_RELAY_PIN
#define P_LIGHT LIGHT_RELAY_PIN
#define P_MISC MISC_RELAY_PIN

#define RELAY_OFF   true
#define RELAY_ON    false


void relay_manager::relay_init() {
  pinMode(P_FAN, OUTPUT);
  pinMode(P_HEATER, OUTPUT);
  pinMode(P_LIGHT, OUTPUT);
  pinMode(P_MISC, OUTPUT);

  //Ensure all relays are in the off state on initialisation.
  this->setall_state(RELAY_OFF);
  /*
  while (1) {
    delay(500);
    this->setall_state(RELAY_OFF);
    delay(500);
    this->setall_state(RELAY_ON);
  }
  */
}

void relay_manager::set_state_all(bool heater, bool light, bool fan, bool misc) {

  this->set_heater_state(heater);
  this->set_light_state(light);
  this->set_fan_state(fan);
  this->set_misc_state(misc);
}

void relay_manager::set_heater_state(bool state) {
  if (state) {
    digitalWrite(P_HEATER, RELAY_ON);
  } else {
    digitalWrite(P_HEATER, RELAY_OFF);
  }
}
void relay_manager::set_light_state(bool state) {
  if (state) {
    digitalWrite(P_LIGHT, RELAY_ON);
  } else {
    digitalWrite(P_LIGHT, RELAY_OFF);
  }
}
void relay_manager::set_fan_state(bool state) {
  if (state) {
    digitalWrite(P_FAN, RELAY_ON);
  } else {
    digitalWrite(P_FAN, RELAY_OFF);
  }
}

void relay_manager::set_misc_state(bool state) {
  if (state) {
    digitalWrite(P_MISC, RELAY_ON);
  } else {
    digitalWrite(P_MISC, RELAY_OFF);
  }
}



void relay_manager::setall_state(RELAY_STATE_t state) {

  digitalWrite(P_FAN, state);
  digitalWrite(P_HEATER, state);
  digitalWrite(P_LIGHT, state);
  digitalWrite(P_MISC, state);
}
