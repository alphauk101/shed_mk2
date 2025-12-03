#include "WVariant.h"
#include "api/Common.h"
#include <Arduino.h>
#include <MCP23017.h>
#include "IOEXP_drv.h"
#include "shdmk3_config.h"

MCP23017 mcp = MCP23017(0x21);

IOEXP_DRV::IOEXP_DRV() {
  //set defaults
  this->PORTA_PIN_STATE = 0xff;
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
  bool BA, BB;

  uint8_t port = mcp.readPort(MCP23017Port::B);
  if ((port & (1 << SWT_1_BP)) == 0) {
    BA = true;
  } else {
    BA = false;
  }
  if ((port & (1 << SWT_2_BP)) == 0) {
    BB = true;
  } else {
    BB = false;
  }

  if (BA != this->BTN_A_PRESSED) {
    this->BTN_A_PRESSED = BA;
    this->btn_evt_callback(BUTTON_A, this->BTN_A_PRESSED);
  }

  if (BB != this->BTN_B_PRESSED) {
    this->BTN_B_PRESSED = BB;
    this->btn_evt_callback(BUTTON_B, this->BTN_B_PRESSED);
  }

  return (this->BTN_B_PRESSED || this->BTN_A_PRESSED);
}


bool IOEXP_DRV::poweron_setup(void (*callback)(int, bool)) {
  // 0 = output
  mcp.portMode(MCP23017Port::A, 0);
  mcp.portMode(MCP23017Port::B, 0x03);

  this->btn_evt_callback = callback;
  //Set all the relays to off

  this->set_relay_pins(false, false, false, false);

  this->set_statusLED_pin(false);
  this->set_firmwareLED_pin(false);
  return true;
}


void IOEXP_DRV::set_statusLED_pin(bool L2) {

  uint8_t tmp_reg = this->PORTB_PIN_STATE;

  if (!L2) {
    tmp_reg |= (1 << LED_2_BP);
  } else {
    tmp_reg &= ~(1 << LED_2_BP);
  }

  if (tmp_reg != this->PORTB_PIN_STATE) {
    this->PORTB_PIN_STATE = tmp_reg;
    mcp.writePort(MCP23017Port::B, this->PORTB_PIN_STATE);
  }
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

//(g_shed_data.power_states.blower, g_shed_data.power_states.lights, g_shed_data.power_states.misc, g_shed_data.power_states.fan);
void IOEXP_DRV::set_relay_pins(bool blower, bool lights, bool misc, bool fan) {

  /*Arranged to fit the hardware outputs*/
  uint8_t temp_port = (this->PORTA_PIN_STATE & (RELAY_1_BP | RELAY_2_BP | RELAY_3_BP | RELAY_4_BP));

  if (blower == RELAY_BLOWER_OFF) {
    temp_port |= (1 << RELAY_1_BP);
  } else {
    temp_port &= ~(1 << RELAY_1_BP);
  }

  if (lights == RELAY_LIGHT_OFF) { //fan - inverted
    temp_port |= (1 << RELAY_3_BP);
  } else {
    temp_port &= ~(1 << RELAY_3_BP);
  }

  if (misc == RELAY_MISC_OFF) {
    temp_port |= (1 << RELAY_2_BP);
  } else {
    temp_port &= ~(1 << RELAY_2_BP);
  }

  if (fan == RELAY_FAN_OFF) {
    temp_port |= (1 << RELAY_4_BP);
  } else {
    temp_port &= ~(1 << RELAY_4_BP);
  }

  if (temp_port != this->PORTA_PIN_STATE){
    this->PORTA_PIN_STATE = temp_port;
    mcp.writePort(MCP23017Port::A, this->PORTA_PIN_STATE);
  }
}
