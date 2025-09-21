//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "led.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static int32_t (*lock)(uint32_t timeout) = 0;
static int32_t (*unlock)(void) = 0;
static ledDriver_t leds[LED_CFG_MAX_LEDS] = {0};
static uint32_t n = 0;
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t ledInitialize(ledConfig_t *config){

    lock = config->lock;
    unlock = config->unlock;

    return 0;
}
//-----------------------------------------------------------------------------
int32_t ledRegister(ledDriver_t *driver, uint32_t to){

    int32_t idx;

    if( lock && (lock(to) != 0) )
        return LED_ERROR_LOCK;

    if( n >= LED_CFG_MAX_LEDS ) return LED_ERROR_MAX_REACHED;

    leds[n] = *driver;
    idx = n;
    n++;

    if( unlock && (unlock() != 0) )
        return LED_ERROR_UNLOCK;

    return idx;
}
//-----------------------------------------------------------------------------
int32_t ledSetIntensity(int32_t idx, void*p, uint8_t intensity, uint32_t to){

    if( lock && (lock(to) != 0) )
        return LED_ERROR_LOCK;

    if( idx < 0 ) return LED_ERROR_INVALID_IDX;
    if( idx >= n ) return LED_ERROR_MAX_REACHED;

    if( leds[idx].setIntensity )
        leds[idx].setIntensity(p, intensity);

    if( unlock && (unlock() != 0) )
        return LED_ERROR_UNLOCK;

    return 0;
}
//-----------------------------------------------------------------------------
int32_t ledSetColor(int32_t idx, void*p, uint8_t r, uint8_t g, uint8_t b, uint32_t to){

    if( lock && (lock(to) != 0) )
        return LED_ERROR_LOCK;

    if( idx < 0 ) return LED_ERROR_INVALID_IDX;
    if( idx >= n ) return LED_ERROR_MAX_REACHED;

    if( leds[idx].setColor )
        leds[idx].setColor(p, r, g, b);

    if( unlock && (unlock() != 0) )
        return LED_ERROR_UNLOCK;

    return 0;
}
//-----------------------------------------------------------------------------
uint32_t ledGetNumberLeds(void){

    return n;
}
//-----------------------------------------------------------------------------
//=============================================================================
