
#ifndef MODULES_LED_H_
#define MODULES_LED_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"

//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================

typedef uint32_t (*ledHwGetNumberLeds_t)(void);
typedef int32_t (*ledHwSetIntensity_t)(uint8_t led, uint8_t intensity, uint32_t to);
typedef int32_t (*ledHwSetColor_t)(uint8_t led, uint8_t red, uint8_t green, uint8_t blue, uint32_t to);
typedef int32_t (*ledLock_t)(uint32_t to);
typedef int32_t (*ledUnlock_t)(void);
typedef int32_t (*ledHwIf_t)(void *in, uint32_t insize, void **out, uint32_t maxoutsize);

typedef struct ledConfig_t{

    ledHwGetNumberLeds_t hwGetNumberLeds;
    ledHwSetIntensity_t hwSetIntensity;
    ledHwSetColor_t hwSetColor;

    ledLock_t lock;
    ledUnlock_t unlock;

    ledHwIf_t hwIf;
}ledConfig_t;

#define LED_ERROR_LOCK      -1 /** Unable to obtain lock */
#define LED_ERROR_UNLOCK    -2 /** Unable to unlock */
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t ledInitialize(ledConfig_t *config);
//-----------------------------------------------------------------------------
uint32_t ledGetNumberLeds(void);
//-----------------------------------------------------------------------------
int32_t ledSetIntensity(uint8_t led, uint8_t intensity, uint32_t to);
//-----------------------------------------------------------------------------
int32_t ledSetColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue, uint32_t to);
//-----------------------------------------------------------------------------
int32_t ledInterface(
    void *in, uint32_t insize, 
    void **out, uint32_t maxoutsize);
//-----------------------------------------------------------------------------
//=============================================================================

#endif /* MODULES_LED_H */
