
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
#define WIZNET_INIT_CFG_SPI             spi0
#define WIZNET_INIT_CFG_SPI_CLK         10 * 1000 * 1000 /* Clock is in Hz */
#define WIZNET_INIT_CFG_SPI_SCK_PIN     18
#define WIZNET_INIT_CFG_SPI_TX_PIN      19
#define WIZNET_INIT_CFG_SPI_RX_PIN      16
#define WIZNET_INIT_CFG_SPI_CSN_PIN     17
#define WIZNET_INIT_CFG_RST_PIN         20

/* If DHCP is set to 1, DHCP is used. Otherwise, static IP */
#define WIZNET_INIT_CFG_USE_DHCP        1

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
