
#ifndef WIZNET_INIT_H_
#define WIZNET_INIT_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"

//=============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================

/* SPI configs */
#define WIZNET_INIT_CFG_SPI			    PICO_DEFAULT_SPI_INSTANCE	
#define WIZNET_INIT_CFG_SPI_CLK		    2 * 1000 * 1000 /* Clock is in Hz */
#define WIZNET_INIT_CFG_SPI_SCK_PIN	    PICO_DEFAULT_SPI_SCK_PIN
#define WIZNET_INIT_CFG_SPI_TX_PIN	    PICO_DEFAULT_SPI_TX_PIN
#define WIZNET_INIT_CFG_SPI_RX_PIN	    PICO_DEFAULT_SPI_RX_PIN
#define WIZNET_INIT_CFG_SPI_CSN_PIN	    PICO_DEFAULT_SPI_CSN_PIN
#define WIZNET_INIT_CFG_RST_PIN         20

/* If DHCP is set to 1, DHCP is used. Otherwise, static IP */
#define WIZNET_INIT_CFG_USE_DHCP		1

/* Prints debugging info if enabled */
#define WIZNET_INIT_CFG_DBG             1

typedef void (*wiznetInitLock_t)(void);
typedef void (*wiznetInitUnlock_t)(void);

typedef struct wiznetInitConfig_t{

    wiznetInitLock_t lock;
    wiznetInitUnlock_t unlock;
}wiznetInitConfig_t;
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t wiznetInit(wiznetInitConfig_t *config);
//-----------------------------------------------------------------------------
//=============================================================================

#endif /* WIZNET_INIT_H_ */
