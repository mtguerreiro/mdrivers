/*
 * temperature.h
 *
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"

//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
typedef int32_t (*temperatureHwTempUpdate_t)(uint32_t sensor);
typedef int32_t (*temperatureHwTempGet_t)(uint32_t sensor, int32_t *temp);
typedef int32_t (*temperatureHwGetNumberSensors_t)(void);
typedef int32_t (*temperatureLock_t)(uint32_t to);
typedef void (*temperatureUnlock_t)(void);

typedef struct{

    temperatureHwTempUpdate_t hwTempUpdate;
    temperatureHwTempGet_t hwTempGet;
    temperatureHwGetNumberSensors_t hwGetNumberSensors;

    temperatureLock_t lock;
    temperatureUnlock_t unlock;

}temperatureConfig_t;

#define TEMPERATURE_ERROR_LOCK      -1 /** Unable to obtain lock */
#define TEMPERATURE_ERROR_UNLOCK    -2 /** Unable to unlock */
#define TEMPERATURE_ERROR_GET       -3 /** Unable to get temperature */
#define TEMPERATURE_ERROR_UPDATE    -4 /** Unable to update temperature */
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t temperatureInitialize(temperatureConfig_t *config);
//-----------------------------------------------------------------------------
int32_t temperatureUpdate(uint32_t sensor, uint32_t to);
//-----------------------------------------------------------------------------
int32_t temperatureGet(uint32_t sensor, int32_t *temp, uint32_t to);
//-----------------------------------------------------------------------------
int32_t temperatureGetNumberSensors(void);
//-----------------------------------------------------------------------------
//=============================================================================

#endif /* TEMPERATURE_H_ */
