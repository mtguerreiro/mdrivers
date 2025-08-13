
//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "wiznet_init.h"

#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/unique_id.h"

#include "mdrivers/wiznet/dhcp.h"
#include "mdrivers/wiznet/socket.h"
#include "mdrivers/wiznet/wizchip_conf.h"
//=============================================================================

//=============================================================================
/*------------------------------- Prototypes --------------------------------*/
//=============================================================================
static void wiznetInitW5500(wiznetInitLock_t lock, wiznetInitLock_t unlock);
static void wiznetInitW5500DHCP(void);

static inline void wiznetInitW5500ChipSelect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(WIZNET_INIT_CFG_SPI_CSN_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void wiznetInitW5500ChipDeselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(WIZNET_INIT_CFG_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}

static void wiznetInitW5500IPAssignCB(void);
static void wiznetInitW5500IPConflictCB(void);

static uint8_t wiznetInitSPIRead(void);
static void wiznetInitSPIWrite(uint8_t);

static void wiznetInitSPIBurstRead(uint8_t *data, uint16_t size);
static void wiznetInitSPIBurstWrite(uint8_t *data, uint16_t size);

static bool wiznetInitDhcpTimer(struct repeating_timer *t);
//=============================================================================

//===========================================================================
/*-------------------------------- Globals --------------------------------*/
//===========================================================================
static wiz_NetInfo gWIZNETINFO = {
    .mac = {0, 0, 0, 0, 0, 0},
    .ip = {192, 168, 0, 231},
    .sn = {255,255,255,0},
    .gw = {192, 168, 0, 254},
    .dns = {0,0,0,0},
    .dhcp = NETINFO_DHCP
};

#if WIZNET_INIT_CFG_USE_DHCP == 1
static struct repeating_timer timer;
#endif
//===========================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t wiznetInit(wiznetInitConfig_t *config){

    spi_init(WIZNET_INIT_CFG_SPI, WIZNET_INIT_CFG_SPI_CLK);
    gpio_set_function(WIZNET_INIT_CFG_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(WIZNET_INIT_CFG_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(WIZNET_INIT_CFG_SPI_RX_PIN, GPIO_FUNC_SPI);

    gpio_init(WIZNET_INIT_CFG_SPI_CSN_PIN);
    gpio_set_dir(WIZNET_INIT_CFG_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(WIZNET_INIT_CFG_SPI_CSN_PIN, 1);

    gpio_init(WIZNET_INIT_CFG_RST_PIN);
    gpio_set_dir(WIZNET_INIT_CFG_RST_PIN, GPIO_OUT);

    gpio_put(WIZNET_INIT_CFG_RST_PIN, 0);
    sleep_ms(10);
    gpio_put(WIZNET_INIT_CFG_RST_PIN, 1);

#if WIZNET_INIT_CFG_USE_DHCP == 1
    add_repeating_timer_ms(1000, wiznetInitDhcpTimer, NULL, &timer);
#endif

    wiznetInitW5500(config->lock, config->unlock);

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void wiznetInitW5500(wiznetInitLock_t lock, wiznetInitUnlock_t unlock){

    reg_wizchip_cris_cbfunc(lock, unlock);
    reg_wizchip_cs_cbfunc(wiznetInitW5500ChipSelect, wiznetInitW5500ChipDeselect);
    reg_wizchip_spi_cbfunc(wiznetInitSPIRead, wiznetInitSPIWrite);
    reg_wizchip_spiburst_cbfunc(wiznetInitSPIBurstRead, wiznetInitSPIBurstWrite);

    wizchip_init(0, 0);

    while(wizphy_getphylink() == PHY_LINK_OFF) sleep_ms(1000);

    #if WIZNET_INIT_CFG_USE_DHCP == 1
    wiznetInitW5500DHCP();
    #else
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
    #endif

    #if WIZNET_INIT_CFG_DBG == 1
    printf("IP Address: %d.%d.%d.%d\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
    printf("Subnet Mask: %d.%d.%d.%d\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
    printf("Gateway: %d.%d.%d.%d\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
    #endif
}
//-----------------------------------------------------------------------------
static void wiznetInitW5500DHCP(void){

    uint8_t buf[2048];
    uint32_t dhcpStatus;

    pico_unique_board_id_t id;

    /* We set the MAC as pico's ID */
    pico_get_unique_board_id(&id);

    gWIZNETINFO.mac[0] = id.id[2];
    gWIZNETINFO.mac[1] = id.id[3];
    gWIZNETINFO.mac[2] = id.id[4];
    gWIZNETINFO.mac[3] = id.id[5];
    gWIZNETINFO.mac[4] = id.id[6];
    gWIZNETINFO.mac[5] = id.id[7];

    setSHAR(gWIZNETINFO.mac);

    DHCP_init(0, buf);
    reg_dhcp_cbfunc(wiznetInitW5500IPAssignCB, wiznetInitW5500IPAssignCB, wiznetInitW5500IPConflictCB);

    while(1){
        dhcpStatus = DHCP_run();

        printf("Running DHCP\n\r");

        if(dhcpStatus == DHCP_IP_LEASED) break;

        else if(dhcpStatus == DHCP_FAILED){
            DHCP_stop();
            ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
        }

        sleep_ms(500);
    }
}
//-----------------------------------------------------------------------------
static void wiznetInitW5500IPAssignCB(void){

    getIPfromDHCP(gWIZNETINFO.ip);
    getGWfromDHCP(gWIZNETINFO.gw);
    getSNfromDHCP(gWIZNETINFO.sn);
    getDNSfromDHCP(gWIZNETINFO.dns);
    gWIZNETINFO.dhcp = NETINFO_DHCP;

    /* Network initialization */
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
}
//-----------------------------------------------------------------------------
static void wiznetInitW5500IPConflictCB(void){
#if WIZNET_INIT_CFG_DBG == 1
    printf("CONFLICT IP from DHCP\r\n");
#endif
    //halt or reset or any...
    while(1); // this example is halt.
}
//-----------------------------------------------------------------------------
static uint8_t wiznetInitSPIRead(void){

    uint8_t data;

    spi_read_blocking(WIZNET_INIT_CFG_SPI, 0xFF, &data, 1);

    return data;
}
//-----------------------------------------------------------------------------
static void wiznetInitSPIWrite(uint8_t data){

    spi_write_blocking(WIZNET_INIT_CFG_SPI, &data, 1);
}
//-----------------------------------------------------------------------------
static void wiznetInitSPIBurstRead(uint8_t *data, uint16_t size){

    spi_read_blocking(WIZNET_INIT_CFG_SPI, 0xFF, data, size);
}
//-----------------------------------------------------------------------------
static void wiznetInitSPIBurstWrite(uint8_t *data, uint16_t size){

    spi_write_blocking(WIZNET_INIT_CFG_SPI, data, size);
}
//-----------------------------------------------------------------------------
static bool wiznetInitDhcpTimer(struct repeating_timer *t){

    DHCP_time_handler();

    return true;
}
//-----------------------------------------------------------------------------
//=============================================================================
