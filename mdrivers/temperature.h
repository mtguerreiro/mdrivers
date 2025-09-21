/*
 * temperature.h
 *
 */

#ifndef MDRIVERS_TEMPERATURE_H_
#define MDRIVERS_TEMPERATURE_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"

//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
#define TEMPERATURE_CFG_MAX_SENSORS      2
#define TEMPERATURE_ERROR_LOCK          -1 /** Unable to obtain lock */
#define TEMPERATURE_ERROR_UNLOCK        -2 /** Unable to unlock */
#define TEMPERATURE_ERROR_GET           -3 /** Unable to get temperature */
#define TEMPERATURE_ERROR_UPDATE        -4 /** Unable to update temperature */
#define TEMPERATURE_ERROR_MAX_REACHED   -5 /** Unable to register new driver */
#define TEMPERATURE_ERROR_INVALID_IDX   -6 /** Invalid index */

typedef struct{
    int32_t (*lock)(uint32_t timeout);
    int32_t (*unlock)(void);
}temperatureConfig_t;

typedef struct{
    int32_t (*update)(void *p);
    int32_t (*read)(void *p, int32_t *temp);
}temperatureDriver_t;

//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t temperatureInitialize(temperatureConfig_t *config);
//-----------------------------------------------------------------------------
int32_t temperatureRegister(temperatureDriver_t *driver, uint32_t to);
//-----------------------------------------------------------------------------
int32_t temperatureUpdate(int32_t idx, void *p, uint32_t to);
//-----------------------------------------------------------------------------
int32_t temperatureRead(int32_t idx, void *p, int32_t *temp, uint32_t to);
//-----------------------------------------------------------------------------
int32_t temperatureGetNumberSensors(void);
//-----------------------------------------------------------------------------
//=============================================================================

#endif /* MDRIVERS_TEMPERATURE_H_ */
