
#ifndef DRIVERS_MODBUS_HANDLE_H_
#define DRIVERS_MODBUS_HANDLE_H_

//===========================================================================
/*------------------------------- Includes --------------------------------*/
//===========================================================================
#include <stdint.h>

//===========================================================================


//===========================================================================
/*--------------------------------- Enums ---------------------------------*/
//===========================================================================

//===========================================================================

//===========================================================================
/*------------------------------ Definitions ------------------------------*/
//===========================================================================
#define MODBUS_HANDLE_CFG_COILS_ADDR_MAX    32
#define MODBUS_HANDLE_CFG_REGS_ADDR_MAX     32
#define MODBUS_HANDLE_CFG_CALLBACKS_MAX     10

#define MODBUS_HANDLE_CFG_DEBUG     1

#if MODBUS_HANDLE_CFG_DEBUG == 1
#define NMBS_DEBUG
#endif

typedef int32_t (*modbusHandleTcpWrite_t)(int32_t sn, const uint8_t *buf, uint16_t size, int32_t to, void *arg);
typedef int32_t (*modbusHandleTcpRead_t)(int32_t sn, uint8_t *buf, uint16_t size, int32_t to, void *arg);

typedef void (*modbusHandleCallback_t)(const uint16_t *buf);

typedef struct modbusHandleConfig_t{
    modbusHandleTcpWrite_t tcpWrite;
    modbusHandleTcpRead_t tcpRead;
}modbusHandleConfig_t;
//===========================================================================

//===========================================================================
/*------------------------------- Functions -------------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
int32_t modbusHandleInitialize(modbusHandleConfig_t *config);
//---------------------------------------------------------------------------
int32_t modbusHandleRun(int32_t sn);
//---------------------------------------------------------------------------
int32_t modbusHandleUpdateRegisters(uint16_t address, uint16_t *data, uint16_t size);
//---------------------------------------------------------------------------
int32_t modbusHandleReadRegisters(uint16_t address, uint16_t *buffer, uint16_t size);
//---------------------------------------------------------------------------
int32_t modbusHandleAssignCallback(uint16_t id, uint16_t address, uint16_t range, modbusHandleCallback_t callback);
//---------------------------------------------------------------------------
//===========================================================================

#endif /* DRIVERS_MODBUS_HANDLE_H_ */
