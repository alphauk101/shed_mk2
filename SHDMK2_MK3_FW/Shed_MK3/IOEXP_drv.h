#ifndef __IOEXP_h_
#define __IOEXP_h_

#define RELAY_1_BP 0
#define RELAY_2_BP 1
#define RELAY_3_BP 2
#define RELAY_4_BP 3

#define FAN_RELAY_BP RELAY_4_BP
#define BLOWER_RELAY_BP RELAY_3_BP
#define MISC_RELAY_BP RELAY_2_BP
#define LIGHT_RELAY_BP RELAY_1_BP


#define SWT_1_BP 0
#define SWT_2_BP 1

#define LED_1_BP 2
#define LED_2_BP 3


class IOEXP_DRV {
public:
  IOEXP_DRV();
  bool task(void);
  void get_pressed_buttons(bool *, bool *);
  bool poweron_setup(void (*callback)(int, bool));
  void set_relay_pins(bool, bool, bool, bool);
  void set_firmwareLED_pin(bool);
  void toggle_firmwareLED_pin(void);
  void set_statusLED_pin(bool);
private:
  void (*btn_evt_callback)(int, bool);
  uint8_t PORTA_PIN_STATE = 0;
  uint8_t PORTB_PIN_STATE = 0;
  bool BTN_A_PRESSED = false;
  bool BTN_B_PRESSED = false;
};
#endif