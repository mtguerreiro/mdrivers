/*
 * @file onewirehl.h
 * @brief Provides a simple bare metal 1-wire driver.
 *
 */

#ifndef PICO_ONEWIREHL_H_
#define PICO_ONEWIREHL_H_

//===========================================================================
/*------------------------------- Includes --------------------------------*/
//===========================================================================
#include <stdint.h>

//===========================================================================

//===========================================================================
/*------------------------------ Definitions ------------------------------*/
//===========================================================================
/* Settings */
#define OWHL_CONFIG_FREERTOS_EN		0 /** Enables FreeRTOS integration. */

/* Error codes */
#define OWHL_ERR_SW_INIT			-0x01 /**< Failed to initialize semaphore. */
#define OWHL_ERR_RESET_ERR			-0x02 /**< Failed to detect presence pulse. */
#define OWHL_ERR_RESET_TO			-0x03 /**< Reset process timed-out. */
#define OWHL_ERR_WRITE_ERR			-0x04 /**< Failed to write data. */
#define OWHL_ERR_WRITE_TO			-0x05 /**< Write process timed-out. */
#define OWHL_ERR_READ_ERR			-0x06 /**< Failed to read data. */
#define OWHL_ERR_READ_TO			-0x07 /**< Read process timed-out. */
//===========================================================================

//===========================================================================
/*------------------------------- Functions -------------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
int32_t onewirehlInitialize(void *gpio, uint8_t pin);
//---------------------------------------------------------------------------
int32_t onewirehlReset(uint32_t to);
//---------------------------------------------------------------------------
int32_t onewirehlWrite(uint8_t *data, uint16_t size, uint32_t to);
//---------------------------------------------------------------------------
int32_t onewirehlRead(uint8_t *data, uint16_t size, uint32_t to);
//---------------------------------------------------------------------------
//===========================================================================

#endif /* PICO_ONEWIREHL_H_ */
