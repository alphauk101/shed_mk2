#include "api/Common.h"
#include <Arduino.h>
#include <MCP23017.h>
#include "IOEXP_drv.h"

MCP23017 mcp = MCP23017(0x21);

IOEXP_DRV::IOEXP_DRV() {
  //set defaults
  this->PORTA_PIN_STATE = 0;
}


void IOEXP_DRV::poweron_setup() {
  // 0 = output
  mcp.portMode(MCP23017Port::A, 0);

  //Set all the relays to off
  this->set_relay_pins(false,false,false,false);
}


void IOEXP_DRV::set_relay_pins(bool R1, bool R2, bool R3, bool R4) {

  this->PORTA_PIN_STATE |= (((R1) ? 1 : 0) << 1);

  if (!R1) {
    this->PORTA_PIN_STATE |= (1 << RELAY_1_BP);
  } else {
    this->PORTA_PIN_STATE &= ~(1 << RELAY_1_BP);
  }

  if (!R2) {
    this->PORTA_PIN_STATE |= (1 << RELAY_2_BP);
  } else {
    this->PORTA_PIN_STATE &= ~(1 << RELAY_2_BP);
  }

  if (!R3) {
    this->PORTA_PIN_STATE |= (1 << RELAY_3_BP);
  } else {
    this->PORTA_PIN_STATE &= ~(1 << RELAY_3_BP);
  }

  if (!R4) {
    this->PORTA_PIN_STATE |= (1 << RELAY_4_BP);
  } else {
    this->PORTA_PIN_STATE &= ~(1 << RELAY_4_BP);
  }
  mcp.writePort(MCP23017Port::A, this->PORTA_PIN_STATE);
}