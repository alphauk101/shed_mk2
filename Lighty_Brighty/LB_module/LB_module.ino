#include <Wire.h>
#include "platform_ctrl.h"

static platform_ctrl PF_controller;

void setup() {

  Serial.begin(9600);
  Serial.println("Starting!");

  PF_controller = platform_ctrl();
  PF_controller.init();

  PF_controller.do_full_sweep();

  PF_controller.go_to_center();

  //pwm.setPWM(0, 0, 1000);
  //pwm.setPWM(1, 0, 300);
}


void loop() {
  
  //for (int angle = 0; angle <= 180; angle += 1) {
   // pwm.setPWM(1, 0, angleToPulse(angle));
    //pwm.setPWM(0, 0, angleToPulse(angle));
    //delay(10);
  //}
  
}