#ifndef __IOEXP_h_
#define __IOEXP_h_

#define RELAY_1_BP    0
#define RELAY_2_BP    1
#define RELAY_3_BP    2
#define RELAY_4_BP    3


class IOEXP_DRV{
  public:
  IOEXP_DRV();
  bool poweron_setup(void);
  void set_relay_pins(bool R1, bool R2, bool R3, bool R4);

  private:
  uint8_t PORTA_PIN_STATE = 0;
};
#endif