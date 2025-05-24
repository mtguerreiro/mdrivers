/*
 * temperature.c
 */

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "temperature.h"
//=============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
typedef struct{
    temperatureHwTempUpdate_t hwTempUpdate;
    temperatureHwTempGet_t hwTempGet;
    temperatureHwGetNumberSensors_t hwGetNumberSensors;

    temperatureLock_t lock;
    temperatureUnlock_t unlock;
}temperatureControl_t;
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static temperatureControl_t gxControl;
//=============================================================================


//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t temperatureInitialize(temperatureConfig_t *config){

    gxControl.hwTempUpdate = config->hwTempUpdate;
    gxControl.hwTempGet = config->hwTempGet;
    gxControl.hwGetNumberSensors = config->hwGetNumberSensors;

    gxControl.lock = config->lock;
    gxControl.unlock = config->unlock;

    return 0;
}
//-----------------------------------------------------------------------------
int32_t temperatureUpdate(uint32_t sensor, uint32_t to){

    int32_t status = 0;

    if( gxControl.lock ){
        status = gxControl.lock(to); 
        if( status != 0 ) return TEMPERATURE_ERROR_LOCK;
    }

    if( gxControl.hwTempUpdate ){
        status = gxControl.hwTempUpdate(sensor);
    }

    if( gxControl.unlock ) gxControl.unlock();

    return status;
}
//-----------------------------------------------------------------------------
int32_t temperatureGet(uint32_t sensor, int32_t *temp, uint32_t to){

    int32_t status = 0;

    if( gxControl.lock ){
        status = gxControl.lock(to); 
        if( status != 0 ) return TEMPERATURE_ERROR_LOCK;
    }

    status = gxControl.hwTempGet(sensor, temp);

    if( gxControl.unlock ) gxControl.unlock();

    return status;
}
//-----------------------------------------------------------------------------
int32_t temperatureGetNumberSensors(void){

    return gxControl.hwGetNumberSensors();
}
//-----------------------------------------------------------------------------
//=============================================================================
