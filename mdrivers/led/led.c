
//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "led.h"
//=============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
typedef struct{
    ledHwGetNumberLeds_t hwGetNumberLeds;
    ledHwSetIntensity_t setIntensity;
    ledHwSetColor_t setColor;

    ledLock_t lock;
    ledUnlock_t unlock;
}ledControl_t;
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static ledControl_t gxControl;
//=============================================================================


//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t ledInitialize(ledConfig_t *config){

    gxControl.hwGetNumberLeds = config->hwGetNumberLeds;
    gxControl.setIntensity = config->hwSetIntensity;
    gxControl.setColor = config->hwSetColor;

    gxControl.lock = config->lock;
    gxControl.unlock = config->unlock;

    return 0;
}
//-----------------------------------------------------------------------------
uint32_t ledGetNumberLeds(void){

    return gxControl.hwGetNumberLeds();
}
//-----------------------------------------------------------------------------
int32_t ledSetIntensity(uint8_t led, uint8_t intensity, uint32_t to){

    int32_t status;

    if( gxControl.lock ){
        status = gxControl.lock(to); 
        if( status != 0 ) return LED_ERROR_LOCK;
    }

    gxControl.setIntensity(led, intensity, to);

    if( gxControl.unlock ){
        status = gxControl.unlock(); 
        if( status != 0 ) return LED_ERROR_UNLOCK;
    }

    return 0;
}
//-----------------------------------------------------------------------------
int32_t ledSetColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue, uint32_t to){

    int32_t status;

    if( gxControl.lock ){
        status = gxControl.lock(to); 
        if( status != 0 ) return LED_ERROR_LOCK;
    }

    gxControl.setColor(led, red, green, blue, to);

    if( gxControl.unlock ){
        status = gxControl.unlock(); 
        if( status != 0 ) return LED_ERROR_UNLOCK;
    }

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
