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
private:
  void setBrightAllDisp(int);
  void convertTemperatureData(float , dispTempData *);
};




#endif