//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

/* Kernel */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Device and drivers */
#include "stdio.h"
#include "pico/stdlib.h"

/* Temperature module */
#include "mdrivers/temperature/temperature.h"
#include "mhw/pico/temperatureHw.h"

//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static SemaphoreHandle_t lock;

//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskTemperatureInitialize(void);
static void taskTemperatureInitializeLock(void);
static int32_t taskTemperatureLock(uint32_t to);
static void taskTemperatureUnlock(void);
static void taskTemperatureUpdateMqtt(uint16_t temp);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void taskTemperature(void *param){

    int32_t status;
    int32_t temp;
    taskTemperatureInitialize();

    temperatureUpdate(0, 1000);

    while(1){
        vTaskDelay(3000);

        status = temperatureGet(0, &temp, 1000);
        printf("Status: %d, temperature %d.\n", status, temp);

        temperatureUpdate(0, 1000);
    }
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskTemperatureInitialize(void){

    taskTemperatureInitializeLock();

    temperatureHwInitialize();

    temperatureConfig_t config;
    config.hwTempUpdate = temperatureHwUpdate;
    config.hwTempGet = temperatureHwGet;
    config.hwGetNumberSensors = temperatureHwGetNumberSensors;
    config.lock = taskTemperatureLock;
    config.unlock = taskTemperatureUnlock;

    temperatureInitialize(&config);
}
//-----------------------------------------------------------------------------
static void taskTemperatureInitializeLock(void){

    lock = xSemaphoreCreateMutex();
}
//-----------------------------------------------------------------------------
static int32_t taskTemperatureLock(uint32_t to){

    if( xSemaphoreTake(lock, to) != pdTRUE ) return -1;

    return 0;
}
//-----------------------------------------------------------------------------
static void taskTemperatureUnlock(void){

    xSemaphoreGive( lock );
}
//-----------------------------------------------------------------------------
//=============================================================================
