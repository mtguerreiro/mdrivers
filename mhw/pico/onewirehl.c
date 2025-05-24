/*
 * onewirehl.c
 *
 *  Created on: 23 de abr de 2021
 *      Author: marco
 */

//===========================================================================
/*------------------------------- Includes --------------------------------*/
//===========================================================================
#include "onewirehl.h"

/* Device and drivers */
#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#include "pwm_irq_handler.h"

#if (OWHL_CONFIG_FREERTOS_EN == 1)
/* Kernel */
#include "FreeRTOS.h"
#include "semphr.h"
#endif
//===========================================================================

//===========================================================================
/*-------------------------------- Defines --------------------------------*/
//===========================================================================

#define OWHL_CONFIG_DBG         0
#define OWHL_CONFIG_DBG_PIN     11

#define OWHL_CONFIG_PWM_SLICE             6
#define OWHL_CONFIG_PWM_CLK_DIV           10.0f
#define OWHL_CONFIG_PWM_TICK_PERIOD_US    ( (float)(OWHL_CONFIG_PWM_CLK_DIV / 125.f) )
#define OWHL_CONFIG_PWM_IRQ_PRIO          PICO_DEFAULT_IRQ_PRIORITY

#define OWHL_CONFIG_RESET_PULSE                   (700.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)
#define OWHL_CONFIG_RESET_PRESENCE_CHECK_PULSE    (20.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)

#define OWHL_CONFIG_WRITE_START_PULSE             (5.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)
#define OWHL_CONFIG_WRITE_START_DURATION          (55.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)
#define OWHL_CONFIG_WRITE_RECOVER_PULSE           (10.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)

#define OWHL_CONFIG_READ_START_PULSE              (5.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)
#define OWHL_CONFIG_READ_SAMPLE_DELAY             (8.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)
#define OWHL_CONFIG_READ_DURATION                 (55.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)
#define OWHL_CONFIG_READ_RECOVER_PULSE            (10.f / OWHL_CONFIG_PWM_TICK_PERIOD_US)
//===========================================================================

//===========================================================================
/*--------------------------------- Enums ---------------------------------*/
//===========================================================================
typedef enum{
	OWHL_STATE_IDLE,
	OWHL_STATE_RESET,
	OWHL_STATE_RESET_WAITING_CLEAR,
	OWHL_STATE_RESET_WAITING_SET,
	OWHL_STATE_WRITE,
	OWHL_STATE_WRITE_RECOVER_SET,
	OWHL_STATE_WRITE_RECOVER_CLEAR,
	OWHL_STATE_READ,
	OWHL_STATE_READ_SAMPLE,
	OWHL_STATE_READ_RECOVER_SET,
	OWHL_STATE_READ_RECOVER_CLEAR,
}owhlStates_t;

typedef enum{
	OWHL_STATUS_IDLE,
	OWHL_STATUS_BUSY,
	OWHL_STATUS_RESET_OK,
	OWHL_STATUS_RESET_FAIL,
	OWHL_STATUS_WRITE_DONE,
	OWHL_STATUS_READ_DONE,
}owhlStatus_t;
//===========================================================================

//===========================================================================
/*-------------------------------- Structs --------------------------------*/
//===========================================================================
typedef struct{
	uint32_t pin;
}owhlGPIO_t;

typedef struct{
	volatile uint8_t bits;
	volatile uint8_t byte;
	volatile owhlStates_t state;
	volatile owhlStatus_t status;
    volatile uint32_t n;
#if (OWHL_CONFIG_FREERTOS_EN == 1)
	SemaphoreHandle_t semaphore;
#endif
}owhlControl_t;
//===========================================================================


//===========================================================================
/*-------------------------------- Globals --------------------------------*/
//===========================================================================
owhlControl_t owhlControl = {.bits = 0, .byte = 0,
		.status = OWHL_STATUS_IDLE, .state = OWHL_STATE_IDLE};
owhlGPIO_t owhlgpio;

//===========================================================================

//===========================================================================
/*------------------------------- Prototypes ------------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
static void onewirehlInitializeTimer(void);
//---------------------------------------------------------------------------
static void onewirehlTimerSet(uint16_t val);
//---------------------------------------------------------------------------
static void onewirehlTimerSetEnable(bool enable);
//---------------------------------------------------------------------------
#if (OWHL_CONFIG_FREERTOS_EN == 1)
static int32_t onewirehlInitializeSW(void);
#endif
//---------------------------------------------------------------------------
static void onewirehlGPIOConfigInit(uint32_t pin);
//---------------------------------------------------------------------------
static void onewirehlGPIOConfigOD(void);
//---------------------------------------------------------------------------
static void onewirehlGPIOConfigInput(void);
//---------------------------------------------------------------------------
static void onewirehlGPIOSet(void);
//---------------------------------------------------------------------------
static void onewirehlGPIOClear(void);
//---------------------------------------------------------------------------
static uint8_t onewirehlGPIORead(void);
//---------------------------------------------------------------------------
// static int32_t onewirehlWaitWhileBusy(uint32_t to);
static int32_t __attribute__((optimize("O0"))) onewirehlWaitWhileBusy(uint32_t to);
//---------------------------------------------------------------------------
static void onewirehlTimerHandler(void);
//---------------------------------------------------------------------------
//===========================================================================

//===========================================================================
/*------------------------------- Functions -------------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
int32_t onewirehlInitialize(void *gpio, uint8_t pin){

	onewirehlGPIOConfigInit(pin);

	onewirehlInitializeTimer();

#if (OWHL_CONFIG_FREERTOS_EN == 1)
	if( onewirehlInitializeSW() != 0 ) return 1;
#endif

	return 0;
}
//---------------------------------------------------------------------------
int32_t onewirehlReset(uint32_t to){

	int32_t ret;

	/* Sets state/status */
	owhlControl.state = OWHL_STATE_RESET;
	owhlControl.status = OWHL_STATUS_BUSY;
    owhlControl.n = 0;

#if (OWHL_CONFIG_FREERTOS_EN == 1)
	/*
	 * Takes the semaphore so we can assume that we can only take it if the
	 * IRQ released it.
	 */
	xSemaphoreTake(owhlControl.semaphore, 0);
#endif

    onewirehlTimerSet(OWHL_CONFIG_RESET_PULSE);

	/* Write 0 to the line and waits */
	//onewirehlGPIOConfigOD();
	onewirehlGPIOClear();

    /* Runs the timer */
    onewirehlTimerSetEnable(true);

	/* Wait until reset is finished */
	ret =  onewirehlWaitWhileBusy(to);
	owhlControl.state = OWHL_STATE_IDLE;

	if( ret != 0 ) return OWHL_ERR_RESET_TO;

	if( owhlControl.status == OWHL_STATUS_RESET_FAIL ) return OWHL_ERR_RESET_ERR;

	return 0;
}
//---------------------------------------------------------------------------
int32_t onewirehlWrite(uint8_t *data, uint16_t size, uint32_t to){

	int32_t ret;

    while(size--){

        /* Sets state/status */
        owhlControl.state = OWHL_STATE_WRITE;
        owhlControl.status = OWHL_STATUS_BUSY;
        owhlControl.n = 0;
        owhlControl.byte = *data++;
        owhlControl.bits = 0;

    #if (OWHL_CONFIG_FREERTOS_EN == 1)
        /*
        * Takes the semaphore so we can assume that we can only take it if the
        * IRQ released it.
        */
        xSemaphoreTake(owhlControl.semaphore, 0);
    #endif

        /* Sets timer to generate starting write pulse */
        onewirehlTimerSet(OWHL_CONFIG_WRITE_START_PULSE);

        /* Write 0 to the line and waits */
        //onewirehlGPIOConfigOD();
        onewirehlGPIOClear();

        /* Runs timer */
        onewirehlTimerSetEnable(true);

        /* Wait until writing is finished */
        ret =  onewirehlWaitWhileBusy(to);
        owhlControl.state = OWHL_STATE_IDLE;

        if( ret != 0 ) return OWHL_ERR_WRITE_TO;

        if( owhlControl.status != OWHL_STATUS_WRITE_DONE ) return OWHL_ERR_WRITE_ERR;
    }

	return 0;
}
//---------------------------------------------------------------------------
int32_t onewirehlRead(uint8_t *data, uint16_t size, uint32_t to){

	int32_t ret;

    while(size--){

        /* Sets state/status */
        owhlControl.state = OWHL_STATE_READ;
        owhlControl.status = OWHL_STATUS_BUSY;
        owhlControl.n = 0;
        owhlControl.byte = 0;
        owhlControl.bits = 0;

    #if (OWHL_CONFIG_FREERTOS_EN == 1)
        /*
        * Takes the semaphore so we can assume that we can only take it if the
        * IRQ released it.
        */
        xSemaphoreTake(owhlControl.semaphore, 0);
    #endif

        /* Sets timer to generate the starting read pulse delay*/
        onewirehlTimerSet(OWHL_CONFIG_READ_START_PULSE);

        /* Write 0 to the line */
        //onewirehlGPIOConfigOD();
        onewirehlGPIOClear();

        /* Runs timer */
        onewirehlTimerSetEnable(true);

        /* Wait until reading is finished */
        ret =  onewirehlWaitWhileBusy(to);
        owhlControl.state = OWHL_STATE_IDLE;

        if( ret != 0 ) return OWHL_ERR_READ_TO;

        if( owhlControl.status != OWHL_STATUS_READ_DONE ) return OWHL_ERR_READ_ERR;

        *data++ = owhlControl.byte;
    }

	return 0;
}
//---------------------------------------------------------------------------
//===========================================================================


//===========================================================================
/*--------------------------- Static functions ----------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
static void onewirehlInitializeTimer(void){

    /* Seths the handle and enables interrupt */
    pwmIrqHandlerAdd(OWHL_CONFIG_PWM_SLICE, onewirehlTimerHandler);
    pwmIrqHandlerSliceEnable(OWHL_CONFIG_PWM_SLICE);
  
    pwm_config config = pwm_get_default_config();

    /* Set divider and initial clock div */
    pwm_config_set_clkdiv(&config, (float)OWHL_CONFIG_PWM_CLK_DIV);
    pwm_config_set_wrap(&config, 0xFFF);

    /* Load configurations */
    pwm_init(OWHL_CONFIG_PWM_SLICE, &config, false);

#if OWHL_CONFIG_DBG == 1
    gpio_init(OWHL_CONFIG_DBG_PIN);
    gpio_set_dir(OWHL_CONFIG_DBG_PIN, GPIO_OUT);
    gpio_put(OWHL_CONFIG_DBG_PIN, 0);
#endif
}
//---------------------------------------------------------------------------
static void onewirehlTimerSet(uint16_t val){

    pwm_set_wrap(OWHL_CONFIG_PWM_SLICE, val - 1);
}
//---------------------------------------------------------------------------
static void onewirehlTimerSetEnable(bool enable){

    pwm_set_counter(OWHL_CONFIG_PWM_SLICE, 0);
    pwm_set_enabled(OWHL_CONFIG_PWM_SLICE, enable);
}
//---------------------------------------------------------------------------
#if (OWHL_CONFIG_FREERTOS_EN == 1)
static int32_t onewirehlInitializeSW(void){

	owhlControl.semaphore = xSemaphoreCreateBinary();

	if( owhlControl.semaphore == NULL ) return OWHL_ERR_SW_INIT;
	xSemaphoreTake(owhlControl.semaphore, 0);

	return 0;
}
#endif
//---------------------------------------------------------------------------
static void onewirehlGPIOConfigInit(uint32_t pin){

    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);

    owhlgpio.pin = pin;
}
//---------------------------------------------------------------------------
static void onewirehlGPIOConfigOD(void){

    gpio_set_dir(owhlgpio.pin, GPIO_OUT);
}
//---------------------------------------------------------------------------
static void onewirehlGPIOConfigInput(void){

    gpio_set_dir(owhlgpio.pin, GPIO_IN);
}
//---------------------------------------------------------------------------
static void onewirehlGPIOSet(void){

    gpio_set_dir(owhlgpio.pin, GPIO_IN);
}
//---------------------------------------------------------------------------
static void onewirehlGPIOClear(void){

    gpio_set_dir(owhlgpio.pin, GPIO_OUT);
    gpio_put(owhlgpio.pin, 0);
}
//---------------------------------------------------------------------------
static uint8_t onewirehlGPIORead(void){

    return gpio_get(owhlgpio.pin);
}
//---------------------------------------------------------------------------
static int32_t onewirehlWaitWhileBusy(uint32_t to){

#if (OWHL_CONFIG_FREERTOS_EN == 1)
	if( xSemaphoreTake(owhlControl.semaphore, to) != pdTRUE) return 1;
#else
	while( (owhlControl.status ==  OWHL_STATUS_BUSY) && (to != 0) ) to--;
	if( to == 0 ) return 1;
#endif

	return 0;
}
//---------------------------------------------------------------------------
//===========================================================================

//===========================================================================
/*-----------------------------  IRQ Handlers -----------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
static void onewirehlTimerHandler(void){

#if OWHL_CONFIG_DBG == 1
    gpio_put(OWHL_CONFIG_DBG_PIN, 1);
#endif

    onewirehlTimerSetEnable(false);

	if( owhlControl.state == OWHL_STATE_RESET ){
        /* Releases the line and goes to the next state */
		owhlControl.state = OWHL_STATE_RESET_WAITING_CLEAR;
        onewirehlTimerSet(OWHL_CONFIG_RESET_PRESENCE_CHECK_PULSE);
		onewirehlGPIOSet();
		//onewirehlGPIOConfigInput();
        onewirehlTimerSetEnable(true);

	}

	else if( owhlControl.state == OWHL_STATE_RESET_WAITING_CLEAR ){
        if( onewirehlGPIORead() == 0 ){
            /* Presence confirmed, next state is to waits until line is released again */
            owhlControl.n = 0;
            owhlControl.state = OWHL_STATE_RESET_WAITING_SET;
            onewirehlTimerSet(OWHL_CONFIG_RESET_PRESENCE_CHECK_PULSE);
            onewirehlTimerSetEnable(true);
        }
        else{
            /* Still waiting for response */
            owhlControl.n++;

            /* Checks if timed out. If so, reset has failed. */
            if(owhlControl.n == 10){
                owhlControl.n = 0;
                owhlControl.state = OWHL_STATE_IDLE;
                owhlControl.status = OWHL_STATUS_RESET_FAIL;
#if (OWHL_CONFIG_FREERTOS_EN == 1)
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xSemaphoreGiveFromISR(owhlControl.semaphore, &xHigherPriorityTaskWoken);
                if( xHigherPriorityTaskWoken == pdTRUE ) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
#endif
            }
            else{
                onewirehlTimerSet(OWHL_CONFIG_RESET_PRESENCE_CHECK_PULSE);
                onewirehlTimerSetEnable(true);
            }
        }

	}

	else if( owhlControl.state == OWHL_STATE_RESET_WAITING_SET ){
        if( onewirehlGPIORead() == 1){
            /* Sensor released the line, reset is successful */
            owhlControl.state = OWHL_STATE_IDLE;
            owhlControl.status = OWHL_STATUS_RESET_OK;
            owhlControl.n = 0;
#if (OWHL_CONFIG_FREERTOS_EN == 1)
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(owhlControl.semaphore, &xHigherPriorityTaskWoken);
            if( xHigherPriorityTaskWoken == pdTRUE ) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
#endif
        }
        else{
            /* Still waiting for response */
            owhlControl.n++;

            if(owhlControl.n == 10){
                owhlControl.n = 0;
                owhlControl.state = OWHL_STATE_IDLE;
                owhlControl.status = OWHL_STATUS_RESET_FAIL;
#if (OWHL_CONFIG_FREERTOS_EN == 1)
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xSemaphoreGiveFromISR(owhlControl.semaphore, &xHigherPriorityTaskWoken);
                if( xHigherPriorityTaskWoken == pdTRUE ) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
#endif
            }
            else{
                onewirehlTimerSet(OWHL_CONFIG_RESET_PRESENCE_CHECK_PULSE);
                onewirehlTimerSetEnable(true);
            }
        }
	}

	else if( owhlControl.state == OWHL_STATE_WRITE ){
        /* Releases or clears the line according to the next bit */
		owhlControl.state = OWHL_STATE_WRITE_RECOVER_SET;
        onewirehlTimerSet(OWHL_CONFIG_WRITE_START_DURATION);
        if( owhlControl.byte & 1 ) onewirehlGPIOSet();
        else onewirehlGPIOClear();
        onewirehlTimerSetEnable(true);
	}

	else if( owhlControl.state == OWHL_STATE_WRITE_RECOVER_SET ){
		owhlControl.state = OWHL_STATE_WRITE_RECOVER_CLEAR;
        onewirehlTimerSet(OWHL_CONFIG_WRITE_RECOVER_PULSE);
        onewirehlGPIOSet();
        onewirehlTimerSetEnable(true);
    }

	else if( owhlControl.state == OWHL_STATE_WRITE_RECOVER_CLEAR ){
		owhlControl.bits++;
		if( owhlControl.bits == 8 ){
			/* Wrote all bits */
			owhlControl.state = OWHL_STATE_IDLE;
			owhlControl.status = OWHL_STATUS_WRITE_DONE;
#if (OWHL_CONFIG_FREERTOS_EN == 1)
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			xSemaphoreGiveFromISR(owhlControl.semaphore, &xHigherPriorityTaskWoken);
			if( xHigherPriorityTaskWoken == pdTRUE ) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
#endif
		}
		else{
			/* Writes next bit */
			owhlControl.state = OWHL_STATE_WRITE;
			owhlControl.byte = owhlControl.byte >> 1;
            onewirehlTimerSet(OWHL_CONFIG_WRITE_START_PULSE);
			onewirehlGPIOClear();
            onewirehlTimerSetEnable(true);
		}
	}

	else if( owhlControl.state == OWHL_STATE_READ ){
        /* Releases the line and sets it as input */
        owhlControl.state = OWHL_STATE_READ_SAMPLE;
        onewirehlTimerSet(OWHL_CONFIG_READ_SAMPLE_DELAY);
        onewirehlGPIOSet();
        //onewirehlGPIOConfigInput();
        onewirehlTimerSetEnable(true);
	}

    else if( owhlControl.state == OWHL_STATE_READ_SAMPLE ){
        /* Samples the line */
        owhlControl.byte |= (uint8_t)(onewirehlGPIORead() << 7);
        owhlControl.state = OWHL_STATE_READ_RECOVER_SET;
        onewirehlTimerSet(OWHL_CONFIG_READ_DURATION);
        onewirehlTimerSetEnable(true);
    }

	else if( owhlControl.state == OWHL_STATE_READ_RECOVER_SET ){
		owhlControl.state = OWHL_STATE_READ_RECOVER_CLEAR;
        onewirehlTimerSet(OWHL_CONFIG_READ_RECOVER_PULSE);
        onewirehlGPIOSet();
        onewirehlTimerSetEnable(true);
    }

	else if( owhlControl.state == OWHL_STATE_READ_RECOVER_CLEAR ){
		owhlControl.bits++;
		if( owhlControl.bits == 8 ){
			/* Read all bits */
			owhlControl.state = OWHL_STATE_IDLE;
			owhlControl.status = OWHL_STATUS_READ_DONE;
#if (OWHL_CONFIG_FREERTOS_EN == 1)
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			xSemaphoreGiveFromISR(owhlControl.semaphore, &xHigherPriorityTaskWoken);
			if( xHigherPriorityTaskWoken == pdTRUE ) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
#endif
		}
		else{
			/* Reads next bit */
			owhlControl.state = OWHL_STATE_READ;
			owhlControl.byte = (uint8_t)(owhlControl.byte >> 1);
			onewirehlGPIOClear();
            onewirehlTimerSet(OWHL_CONFIG_READ_START_PULSE);
            onewirehlTimerSetEnable(true);
		}
	}

#if OWHL_CONFIG_DBG == 1
    gpio_put(OWHL_CONFIG_DBG_PIN, 0);
#endif
}
//---------------------------------------------------------------------------
//===========================================================================
