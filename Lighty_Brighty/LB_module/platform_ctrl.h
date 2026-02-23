#ifndef __PLATFORM_CTRL__
#define __PLATFORM_CTRL__

#define PWM_FREQ    60

#define X_AXIS_SERVO    0
#define Y_AXIS_SERVO    1

//Absolute max and mins to prevent mech damage.
#define SERVO_X_MIN   135
#define SERVO_X_MAX   700

#define SERVO_Y_MAX   800
#define SERVO_Y_MIN   390

#define SERVO_Y_CENTER_POS      500
#define SERVO_X_CENTER_POS      380

class platform_ctrl {
public:
  void init(void);
  void go_to_center(void);
  void do_full_sweep(void);
private:
};


#endif
