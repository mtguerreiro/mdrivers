/*
 * temperature.c
 */

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "temperature.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static int32_t (*lock)(uint32_t timeout) = 0;
static int32_t (*unlock)(void) = 0;
static temperatureDriver_t sensors[TEMPERATURE_CFG_MAX_SENSORS] = {0};
static uint32_t n = 0;
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t temperatureInitialize(temperatureConfig_t *config){

    lock = config->lock;
    unlock = config->unlock;

    return 0;
}
//-----------------------------------------------------------------------------
int32_t temperatureRegister(temperatureDriver_t *driver, uint32_t to){

    int32_t idx;

    if( lock && (lock(to) != 0) )
        return TEMPERATURE_ERROR_LOCK;

    if( n >= TEMPERATURE_CFG_MAX_SENSORS ) return TEMPERATURE_ERROR_MAX_REACHED;

    sensors[n] = *driver;
    idx = n;
    n++;

    if( unlock && (unlock() != 0) )
        return TEMPERATURE_ERROR_UNLOCK;

    return idx;
}
//-----------------------------------------------------------------------------
int32_t temperatureUpdate(int32_t idx, void *p, uint32_t to){

    int32_t status = 0;

    if( lock && (lock(to) != 0) )
        return TEMPERATURE_ERROR_LOCK;

    if( idx < 0 ) return TEMPERATURE_ERROR_INVALID_IDX;
    if( idx >= n ) return TEMPERATURE_ERROR_MAX_REACHED;

    if( sensors[idx].update )
        status = sensors[idx].update(p);

    if( unlock && (unlock() != 0) )
        return TEMPERATURE_ERROR_UNLOCK;

    return status;
}
//-----------------------------------------------------------------------------
int32_t temperatureRead(int32_t idx, void *p, int32_t *temp, uint32_t to){

    int32_t status = 0;

    if( lock && (lock(to) != 0) )
        return TEMPERATURE_ERROR_LOCK;

    if( idx < 0 ) return TEMPERATURE_ERROR_INVALID_IDX;
    if( idx >= n ) return TEMPERATURE_ERROR_MAX_REACHED;

    if( sensors[idx].read )
        status = sensors[idx].read(p, temp);

    if( unlock && (unlock() != 0) )
        return TEMPERATURE_ERROR_UNLOCK;

    return status;

}
//-----------------------------------------------------------------------------
int32_t temperatureGetNumberSensors(void){

    return n;
}
//-----------------------------------------------------------------------------
//=============================================================================
