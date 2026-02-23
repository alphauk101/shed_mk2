#include "platform_ctrl.h"
#include <Adafruit_PWMServoDriver.h>
//https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

void platform_ctrl::init(void) {
  pwm.begin();
  pwm.setPWMFreq(PWM_FREQ);  // Servos run at ~60Hz

  //go to center to ensure servers are not in a unknown position
  this->go_to_center();
}


void platform_ctrl::go_to_center(void) {
  pwm.setPWM(X_AXIS_SERVO, 0, SERVO_X_CENTER_POS);
  pwm.setPWM(Y_AXIS_SERVO, 0, SERVO_Y_CENTER_POS);
}

void platform_ctrl::do_full_sweep(void) {
  uint16_t hold_time = 1000;
  pwm.setPWM(X_AXIS_SERVO, 0, SERVO_X_MAX);
  pwm.setPWM(Y_AXIS_SERVO, 0, SERVO_Y_MAX);
  delay(hold_time);

  pwm.setPWM(X_AXIS_SERVO, 0, SERVO_X_MAX);
  pwm.setPWM(Y_AXIS_SERVO, 0, SERVO_Y_MIN);
  delay(hold_time);

  pwm.setPWM(X_AXIS_SERVO, 0, SERVO_X_MIN);
  pwm.setPWM(Y_AXIS_SERVO, 0, SERVO_Y_MIN);
  delay(hold_time);

  pwm.setPWM(X_AXIS_SERVO, 0, SERVO_X_MIN);
  pwm.setPWM(Y_AXIS_SERVO, 0, SERVO_Y_MAX);
  delay(hold_time);

}
