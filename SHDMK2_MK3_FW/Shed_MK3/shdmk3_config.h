#ifndef __SHDMKR_CFG__
#define __SHDMKR_CFG__

#define DEBUG_ENVIRONMENTS  1


/*
              Relays states
              =============
To optimise current, this version will set the relays to there most 
used state in the none-energised position ie.
Fan is on most of the time so, relay will be NC vs. NO for the fan this 
way the power will be less when the fan is on.

This will apply to the others ie. lights will be on NO ie. not on regularly 
use the below defines to set the states so it is synchronised with the hardware.
*/

#define RELAY_ON            false
#define RELAY_OFF           true


#define RELAY_FAN_OFF       RELAY_ON      //Spends most time in ON
#define RELAY_FAN_ON        RELAY_OFF     

#define RELAY_DRYER_OFF     RELAY_OFF     //Spends most time in OFF
#define RELAY_DRYER_ON      RELAY_ON     //Spends most time in OFF

#define REALY_LIGHT_OFF     RELAY_ON     //Spends most time in OFF
#define REALY_LIGHT_ON      RELAY_OFF     //Spends most time in OFF

#define RELAY_MISC_OFF      RELAY_ON     //Spends most time in OFF
#define RELAY_MISC_ON       RELAY_OFF     //Spends most time in OFF



/*
      APPLICATION TIME VALUES
*/

#define ENVIRONMENT_SAMPLE_TIME         5000


#endif