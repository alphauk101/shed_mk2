#ifndef __DISP_MANAGER__
#define __DISP_MANAGER__

typedef struct {
  bool positive;
  int dec;
  int frac;
} dispTempData;

class disp_manager {
public:
  void disp_init(void);
  void disp_environments(float, float, float);
  void system_sleeping(bool sleep);
private:
  void setBrightAllDisp(int);
  void convertTemperatureData(float , dispTempData *);
  void disp_historic_environments(float i_temp, float i_hum, float e_temp);
  void get_NVM_data(void);
  void set_NVM_data(void);
};




#endif