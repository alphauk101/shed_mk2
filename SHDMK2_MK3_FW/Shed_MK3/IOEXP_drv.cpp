#include "WVariant.h"
#include "api/Common.h"
#include <Arduino.h>
#include <MCP23017.h>
#include "IOEXP_drv.h"

MCP23017 mcp = MCP23017(0x21);

IOEXP_DRV::IOEXP_DRV() {
  //set defaults
  this->PORTA_PIN_STATE = 0;
  this->PORTB_PIN_STATE = 0;
  this->BTN_B_PRESSED = false;
  this->BTN_A_PRESSED = false;
}


void IOEXP_DRV::get_pressed_buttons(bool *btnA, bool *btnB) {

  *btnA = this->BTN_A_PRESSED;
  *btnB = this->BTN_B_PRESSED;

  this->BTN_B_PRESSED = false;
  this->BTN_A_PRESSED = false;
}

bool IOEXP_DRV::task() {
  uint8_t port = mcp.readPort(MCP23017Port::B);
  if ((port & (1 << SWT_1_BP)) == 0) {
    this->BTN_A_PRESSED = true;
  }
  if ((port & (1 << SWT_2_BP)) == 0) {
    this->BTN_B_PRESSED = true;
  }
  return (this->BTN_B_PRESSED || this->BTN_A_PRESSED);
}


bool IOEXP_DRV::poweron_setup() {
  // 0 = output
  mcp.portMode(MCP23017Port::A, 0);
  mcp.portMode(MCP23017Port::B, 0x03);

  //Set all the relays to off
  this->set_relay_pins(false, false, false, false);
  this->set_statusLED_pin(false);
  this->set_firmwareLED_pin(false);
  return true;
}


void IOEXP_DRV::set_statusLED_pin(bool L2) {
  if (!L2) {
    this->PORTB_PIN_STATE |= (1 << LED_2_BP);
  } else {
    this->PORTB_PIN_STATE &= ~(1 << LED_2_BP);
  }
  mcp.writePort(MCP23017Port::B, this->PORTB_PIN_STATE);
}


void IOEXP_DRV::toggle_firmwareLED_pin() {

  if (this->PORTB_PIN_STATE & (1 << LED_1_BP)) {
    this->set_firmwareLED_pin(true);
  } else {
    this->set_firmwareLED_pin(false);
  }
}



void IOEXP_DRV::set_firmwareLED_pin(bool L1) {

  if (!L1) {
    this->PORTB_PIN_STATE |= (1 << LED_1_BP);
  } else {
    this->PORTB_PIN_STATE &= ~(1 << LED_1_BP);
  }
  mcp.writePort(MCP23017Port::B, this->PORTB_PIN_STATE);
}


void IOEXP_DRV::set_relay_pins(bool R1, bool R2, bool R3, bool R4) {

  //this->PORTA_PIN_STATE |= (((R1) ? 1 : 0) << 1);

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
