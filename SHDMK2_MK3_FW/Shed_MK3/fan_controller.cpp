#include "api/Common.h"
#include "fan_controller.h"
#include <Arduino.h>
#include "SAMD_PWM.h"

#define PRINT_PWM_INFO
const byte interruptPin = FAN_IRQ_PIN;

//creates pwm instance
SAMD_PWM* PWM_Instance;
float frequency = 25000.0f;
int pinToUse = FAN_CNTRL_OUTPUT;

typedef unsigned long FANTime_t;

FANTime_t last_time = 0;

#define FAN_BUFFER_SIZE 6

volatile FANTime_t FANRPM_buffer[FAN_BUFFER_SIZE] = { 0 };
volatile uint8_t FANRPM_buff_cnt = 0;
volatile FANTime_t FANRPM = 0;


void fanIRQ() {
  FANTime_t now = micros();
  if (now - last_time > 500) {
    if (FANRPM_buff_cnt < FAN_BUFFER_SIZE) {
      FANRPM_buffer[FANRPM_buff_cnt] = micros();
      //FANRPM_buffer[FANRPM_buff_cnt] = getPreciseMicros();
      FANRPM_buff_cnt++;
    }
  }
  last_time = now;
}

void fan_cntrllr::init() {
  this->setup125kHz(FAN_CNTRL_OUTPUT);
  this->setFanLevel(this->FAN_OFF);
  //analogWrite(5, 20);

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), fanIRQ, FALLING);
}

String res_out;
String fan_cntrllr::convert_reason_code_string(int reason)
{
  res_out.reserve(17);

  switch (reason) {
    case FAN_UNKNOWN:
      res_out = "UNKNOWN";
      break;
    case FAN_DOOR_OPEN:
      res_out = "DOOR OPEN";
      break;
    case FAN_LOW_TEMP:
      res_out = "LOW EXTERNAL TEMP";
      break;
    case FAN_HOT_FLUSH:
      res_out = "FLUSHING HEAT";
      break;
    case FAN_HUMID_HIGH:
      res_out = "HIGH HUMIDITY";
      break;
    case FAN_HUMID_MED:
      res_out = "MEDIUM HUMIDITY";
      break;
    case FAN_HUMID_LOW:
      res_out = "LOW HUMIDITY";
      break;
    case FAN_DEWPOINT:
      res_out = "DEW POINT";
      break;
    case FAN_NIGHTMODE:
      res_out = "NIGHT MODE";
      break;
  }
  return res_out;
}



#define FAN_MODE_DEBUG
fan_cntrllr::FAN_REASON fan_cntrllr::task(SHED_APP* g_shed_ptr) {
  fan_cntrllr::FAN_SPEED target_level = this->FAN_OFF;
  fan_cntrllr::FAN_REASON reason = FAN_UNKNOWN;
  float internal_t = g_shed_ptr->environmentals.internal_temp;
  float external_t = g_shed_ptr->environmentals.external_temp;
  float humidity = g_shed_ptr->environmentals.internal_humidity;
  int current_hour = g_shed_ptr->last_timestamp.hour();

  if (g_shed_ptr->door_status.current_state == true) {
    this->setFanLevel(this->FAN_OFF);
#ifdef FAN_MODE_DEBUG
  Serial.print("Fan level: OFF");
  Serial.print("Fan reason: DOOR OPEN");
#endif
    return FAN_DOOR_OPEN;
  }

  if (external_t < FAN_OFF_TEMPERATURE) {
    this->setFanLevel(this->FAN_OFF);
#ifdef FAN_MODE_DEBUG
  Serial.print("Fan level: OFF");
  Serial.print("Fan reason: OUTSIDE TEMP TOO LOW");
#endif
    return FAN_LOW_TEMP;
  }

  float tmp_diff = internal_t - external_t;

  if (tmp_diff > FAN_IN_OUT_TEMP_DIFF) {
    target_level = this->FAN_ON;  // Heat flush (Hot bike)
    reason = FAN_HOT_FLUSH;
  } else if (g_shed_ptr->environmentals.internal_dewpoint > MAX_DEW_THRESHOLD) {
    target_level = this->FAN_75;  // Moisture prevention
    reason = FAN_DEWPOINT;
  } else if (humidity >= FAN_HUMIDITY_MAX) {
    target_level = this->FAN_75;
    reason = FAN_HUMID_HIGH;
  } else if (humidity >= FAN_HUMIDITY_MID) {
    target_level = this->FAN_50;
    reason = FAN_HUMID_MED;
  } else if (humidity >= FAN_HUMIDITY_LOW) {
    target_level = this->FAN_25;
    reason = FAN_HUMID_LOW;
  }

  bool is_night = (current_hour >= FAN_LOW_NIGHT_MODE_STARTHOUR || current_hour <= FAN_LOW_NIGHT_MODE_ENDHOUR);

  if (is_night && target_level > this->FAN_25) {
    target_level = this->FAN_25;
    reason = FAN_NIGHTMODE;
  }

#ifdef FAN_MODE_DEBUG
  Serial.print("Fan level: ");
  Serial.println(target_level);
  Serial.print("Fan reason: ");
  switch (reason) {
    case FAN_UNKNOWN:
      Serial.println("ERR -> Unknown");
      break;
    case FAN_DOOR_OPEN:
      Serial.println("FAN_DOOR_OPEN");
      break;
    case FAN_LOW_TEMP:
      Serial.println("FAN_LOW_TEMP");
      break;
    case FAN_HOT_FLUSH:
      Serial.println("FAN_HOT_FLUSH");
      break;
    case FAN_HUMID_HIGH:
      Serial.println("FAN_HUMID_HIGH");
      break;
    case FAN_HUMID_MED:
      Serial.println("FAN_HUMID_MED");
      break;
    case FAN_HUMID_LOW:
      Serial.println("FAN_HUMID_LOW");
      break;
    case FAN_DEWPOINT:
      Serial.println("FAN_DEWPOINT");
      break;
    case FAN_NIGHTMODE:
      Serial.println("FAN_NIGHTMODE");
      break;
  }
  Serial.println(target_level);


#endif

  this->setFanLevel(target_level);
  return reason;
}


FANTime_t fan_cntrllr::getRPM() {
  // 1. Handle empty buffer to avoid division by zero
  if (FANRPM_buff_cnt < 2) {
    FANRPM_buff_cnt = 0;
    return 0;
  }

  FANTime_t total_duration = 0;
  uint8_t valid_intervals = 0;

  // 2. Iterate through the buffer
  // We stop at 'FANRPM_buff_cnt - 1' to compare current index with next index
  for (int i = 0; i < FANRPM_buff_cnt - 1; i++) {
    FANTime_t start = FANRPM_buffer[i];
    FANTime_t end = FANRPM_buffer[i + 1];

    // 3. Ensure the timestamps are valid and chronological
    if (start > 0 && end > start) {
      total_duration += (end - start);
      valid_intervals++;
    }
  }

//#define PRINT_BUFFER
#ifdef PRINT_BUFFER
  Serial.print("Fan samples: ");
  Serial.println(FANRPM_buff_cnt);
  for (int i = 0; i < FANRPM_buff_cnt - 1; i++) {
    Serial.println(FANRPM_buffer[i]);
  }
#endif

  // 4. Calculate average interval
  FANTime_t avg_interval = 0;
  if (valid_intervals > 0) {
    avg_interval = total_duration / valid_intervals;
  }

  // 5. Reset buffer counter for next batch
  FANRPM_buff_cnt = 0;

  if (avg_interval == 0) return 0;
  return (30000000UL / avg_interval);
}



#ifdef PRINT_PWM_INFO
char dashLine[] = "=================================================================================================";
void printPWMInfo(SAMD_PWM* PWM_Instance) {
  Serial.println(dashLine);
  Serial.print("Actual data: pin = ");
  Serial.print(PWM_Instance->getPin());
  Serial.print(", PWM DC = ");
  Serial.print(PWM_Instance->getActualDutyCycle());
  Serial.print(", PWMPeriod = ");
  Serial.print(PWM_Instance->getPWMPeriod());
  Serial.print(", PWM Freq (Hz) = ");
  Serial.println(PWM_Instance->getActualFreq(), 4);
  Serial.println(dashLine);
}
#endif

void fan_cntrllr::setup125kHz(int pin) {
  PWM_Instance = new SAMD_PWM(pin, frequency, 0);
  //PWM_Instance->setResolution(16);
  if (PWM_Instance) {
    PWM_Instance->setPWM();
    //PWM_Instance->setPWM_DCPercentageInt_manual(pin, 50);
#ifdef PRINT_PWM_INFO
    printPWMInfo(PWM_Instance);
#endif
  } else {
#ifdef PRINT_PWM_INFO
    Serial.println("PWM NOT INITIALISED");
#endif
  }
}

fan_cntrllr::FAN_SPEED tmp_speed = fan_cntrllr::FAN_INIT;
void fan_cntrllr::setFanLevel(fan_cntrllr::FAN_SPEED fanspd) {

  if (fanspd != tmp_speed) {
    tmp_speed = fanspd;
    switch (fanspd) {
      case fan_cntrllr::FAN_OFF:
#ifdef PRINT_PWM_INFO
        Serial.println("Fan: OFF");
#endif
        PWM_Instance->setPWM_DCPercentage_manual(pinToUse, 0);
        break;
      case fan_cntrllr::FAN_25:
        PWM_Instance->setPWM_DCPercentage_manual(pinToUse, 40);
#ifdef PRINT_PWM_INFO
        Serial.println("Fan: 25%");
#endif
        break;
      case fan_cntrllr::FAN_50:
        PWM_Instance->setPWM_DCPercentage_manual(pinToUse, 65);
#ifdef PRINT_PWM_INFO
        Serial.println("Fan: 50%");
#endif
        break;
      case fan_cntrllr::FAN_75:
        PWM_Instance->setPWM_DCPercentage_manual(pinToUse, 90);
#ifdef PRINT_PWM_INFO
        Serial.println("Fan: 75%");
#endif
        break;
      case fan_cntrllr::FAN_ON:
        PWM_Instance->setPWM_DCPercentage_manual(pinToUse, 100);
#ifdef PRINT_PWM_INFO
        Serial.println("Fan:FULL");
#endif
        break;
      default:
#ifdef PRINT_PWM_INFO
        Serial.println("Fan: UNKNOWN STATE!");
#endif
        break;
    }
  } else {
#ifdef PRINT_PWM_INFO
    Serial.println("Fan set - failed!");
#endif
  }
}
