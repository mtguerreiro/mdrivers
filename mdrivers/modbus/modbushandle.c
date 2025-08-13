
//===========================================================================
/*------------------------------- Includes --------------------------------*/
//===========================================================================
#include "modbushandle.h"

#include "modbusconfig.h"
#include "nanomodbus.h"

#if MODBUS_HANDLE_CFG_DEBUG == 1
#include "stdio.h"
#endif
//===========================================================================

//===========================================================================
/*------------------------------ Definitions ------------------------------*/
//===========================================================================
typedef struct modbusCallbackControl_t{

    modbusHandleCallback_t cb[MODBUS_CONFIG_ID_END];
    uint16_t address[MODBUS_CONFIG_ID_END];
    uint16_t range[MODBUS_CONFIG_ID_END];
}modbusCallbackControl_t;
//===========================================================================

//===========================================================================
/*------------------------------- Prototypes ------------------------------*/
//===========================================================================
 //---------------------------------------------------------------------------
 static int32_t modbusHandleTcpWrite(const uint8_t *buf, uint16_t size, int32_t to, void *arg);
//---------------------------------------------------------------------------
 static int32_t modbusHandleTcpRead(uint8_t *buf, uint16_t size, int32_t to, void *arg);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadCoils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out, uint8_t unit_id, void* arg);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteSingleCoil(uint16_t address, bool value, uint8_t unit_id, void* arg);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteMultipleCoils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils, uint8_t unit_id,
                                       void* arg);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadHoldingRegisters(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id,
                                          void* arg);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteSingleRegister(uint16_t address, uint16_t value, uint8_t unit_id, void* arg);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteMultipleRegisters(uint16_t address, uint16_t quantity, const uint16_t* registers,
                                           uint8_t unit_id, void* arg);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadDeviceIdentificationMap(nmbs_bitfield_256 map);
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadDeviceIdentification(uint8_t object_id, char buffer[NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH]);
//---------------------------------------------------------------------------
//===========================================================================


//===========================================================================
/*-------------------------------- Globals --------------------------------*/
//===========================================================================
static nmbs_t nmbs;

static nmbs_bitfield_32 modbusServerCoils = {0};
static uint16_t modbusServerRegisters[MODBUS_HANDLE_CFG_REGS_ADDR_MAX] = {0};

static modbusCallbackControl_t callbacks = {.cb = {0}};

static modbusHandleTcpWrite_t tcpWrite;
static modbusHandleTcpRead_t tcpRead;
//===========================================================================


//===========================================================================
/*------------------------------- Functions -------------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
int32_t modbusHandleInitialize(modbusHandleConfig_t *config){

    tcpWrite = config->tcpWrite;
    tcpRead = config->tcpRead;

    nmbs_platform_conf platform_conf = {0};
    platform_conf.transport = NMBS_TRANSPORT_TCP;
    platform_conf.read = modbusHandleTcpRead;
    platform_conf.write = modbusHandleTcpWrite;
    platform_conf.arg = NULL;    // We will set the arg (socket fd) later

    nmbs_callbacks callbacks = {0};
    callbacks.read_coils = modbusHandleReadCoils;
    callbacks.write_single_coil = modbusHandleWriteSingleCoil;
    callbacks.write_multiple_coils = modbusHandleWriteMultipleCoils;

    callbacks.read_holding_registers = modbusHandleReadHoldingRegisters;
    callbacks.write_single_register = modbusHandleWriteSingleRegister;
    callbacks.write_multiple_registers = modbusHandleWriteMultipleRegisters;

    callbacks.read_device_identification_map = modbusHandleReadDeviceIdentificationMap;
    callbacks.read_device_identification = modbusHandleReadDeviceIdentification;

    // Create the modbus server. It's ok to set address_rtu to 0 since we are on TCP
    nmbs_error err = nmbs_server_create(&nmbs, 0, &platform_conf, &callbacks);
    if (err != NMBS_ERROR_NONE) {
#if MODBUS_HANDLE_CFG_DEBUG == 1
        printf("Error creating modbus server. Error: %d\n\r", err);
#endif
        return err;
    }

    // Set only the polling timeout. Byte timeout will be handled by the TCP connection
    nmbs_set_read_timeout(&nmbs, 1000);

#if MODBUS_HANDLE_CFG_DEBUG == 1
    printf("Modbus TCP server started\n\r");
#endif

    return 0;
}
//---------------------------------------------------------------------------
int32_t modbusHandleRun(int32_t sn){

    nmbs_error err;
    nmbs.platform.arg = (void *)&sn;
    err = nmbs_server_poll(&nmbs);
    if (err != NMBS_ERROR_NONE) {
#if MODBUS_HANDLE_CFG_DEBUG == 1
        printf("Error on modbus connection - %s\n\r", nmbs_strerror(err));
#endif
        return -1;
        // In a more complete example, we would handle this error by checking its nmbs_error value
    }

    return 0;
}
//---------------------------------------------------------------------------
int32_t modbusHandleUpdateRegisters(uint16_t address, uint16_t *data, uint16_t size){

    uint16_t k;

    if( (address + size) > MODBUS_HANDLE_CFG_REGS_ADDR_MAX ) return -1;

    for(k = address; k < (address + size); k++){
        modbusServerRegisters[k] = *data++;
    }

    return 0;
}
//---------------------------------------------------------------------------
int32_t modbusHandleReadRegisters(uint16_t address, uint16_t *buffer, uint16_t size){
    
    uint16_t k;

    if( (address + size) > MODBUS_HANDLE_CFG_REGS_ADDR_MAX ) return -1;

    for(k = address; k < (address + size); k++){
        *buffer++ = modbusServerRegisters[k];
    }

    return 0;
}
//---------------------------------------------------------------------------
int32_t modbusHandleAssignCallback(uint16_t id, uint16_t address, uint16_t range, modbusHandleCallback_t callback){

    if( id >= MODBUS_CONFIG_ID_END ) return -1;

    callbacks.address[id] = address;
    callbacks.range[id] = range;
    callbacks.cb[id] = callback;

    return 0;
}
//---------------------------------------------------------------------------
//===========================================================================

//===========================================================================
/*--------------------------- Static functions ----------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
 static int32_t modbusHandleTcpWrite(const uint8_t *buf, uint16_t size, int32_t to, void *arg){

    int32_t ret;
    int32_t sn = *( (int32_t *) arg );

    ret = tcpWrite(sn, buf, size, to, arg);

    return ret;
}
//---------------------------------------------------------------------------
 static int32_t modbusHandleTcpRead(uint8_t *buf, uint16_t size, int32_t to, void *arg){

    int32_t ret;
    int32_t sn = *( (int32_t *) arg );

    ret = tcpRead(sn, buf, size, to, arg);

    return ret;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadCoils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out, uint8_t unit_id, void* arg) {
    //UNUSED_PARAM(arg);
    //UNUSED_PARAM(unit_id);

    if (address + quantity > MODBUS_HANDLE_CFG_COILS_ADDR_MAX + 1)
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    // Read our coils values into coils_out
    for (int i = 0; i < quantity; i++) {
        bool value = nmbs_bitfield_read(modbusServerCoils, address + i);
        nmbs_bitfield_write(coils_out, i, value);
    }

    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteSingleCoil(uint16_t address, bool value, uint8_t unit_id, void* arg) {
    //UNUSED_PARAM(arg);
    //UNUSED_PARAM(unit_id);

    // Write coil value to our server_coils
    if (address > MODBUS_HANDLE_CFG_COILS_ADDR_MAX + 1)
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    nmbs_bitfield_write(modbusServerCoils, address, value);

    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteMultipleCoils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils, uint8_t unit_id,
                                       void* arg) {
    //UNUSED_PARAM(arg);
    //UNUSED_PARAM(unit_id);

    if (address + quantity > MODBUS_HANDLE_CFG_COILS_ADDR_MAX + 1)
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    // Write coils values to our server_coils
    for (int i = 0; i < quantity; i++) {
        nmbs_bitfield_write(modbusServerCoils, address + i, nmbs_bitfield_read(coils, i));
    }

    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadHoldingRegisters(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id,
                                          void* arg) {
    //UNUSED_PARAM(arg);
    //UNUSED_PARAM(unit_id);

    if (address + quantity > MODBUS_HANDLE_CFG_REGS_ADDR_MAX + 1)
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    // Read our registers values into registers_out
    for (int i = 0; i < quantity; i++)
        registers_out[i] = modbusServerRegisters[address + i];

    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteSingleRegister(uint16_t address, uint16_t value, uint8_t unit_id, void* arg) {
    //UNUSED_PARAM(arg);
    //UNUSED_PARAM(unit_id);

    uint16_t k;
    uint16_t aHigh;
    uint16_t aLow;

    if (address > MODBUS_HANDLE_CFG_REGS_ADDR_MAX + 1)
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    // Write register values to our server_registers
    modbusServerRegisters[address] = value;

    for(k = 0; k < MODBUS_CONFIG_ID_END; k++){
        aLow  = callbacks.address[k];
        aHigh = callbacks.address[k] + callbacks.range[k];
        
        if( (address >= aLow ) && (address < aHigh) && (callbacks.cb[k] != 0) )
            callbacks.cb[k]( (const uint16_t *) &modbusServerRegisters[callbacks.address[k]] );
    }

    // put_pixel(urgb_u32(server_registers[1], server_registers[2], server_registers[3]));
    // put_pixel(urgb_u32(server_registers[1], server_registers[2], server_registers[3]));
    // put_pixel(urgb_u32(server_registers[1], server_registers[2], server_registers[3]));

    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleWriteMultipleRegisters(uint16_t address, uint16_t quantity, const uint16_t* registers,
                                           uint8_t unit_id, void* arg) {
    //UNUSED_PARAM(arg);
    //UNUSED_PARAM(unit_id);

    if (address + quantity > MODBUS_HANDLE_CFG_REGS_ADDR_MAX + 1)
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    // Write registers values to our server_registers
    for (int i = 0; i < quantity; i++)
        modbusServerRegisters[address + i] = registers[i];

    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadDeviceIdentificationMap(nmbs_bitfield_256 map) {
    // We support basic object ID and a couple of extended ones
    nmbs_bitfield_set(map, 0x00);
    nmbs_bitfield_set(map, 0x01);
    nmbs_bitfield_set(map, 0x02);
    nmbs_bitfield_set(map, 0x90);
    nmbs_bitfield_set(map, 0xA0);
    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
static nmbs_error modbusHandleReadDeviceIdentification(uint8_t object_id, char buffer[NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH]) {
    switch (object_id) {
        case 0x00:
            strcpy(buffer, "VendorName");
            break;
        case 0x01:
            strcpy(buffer, "ProductCode");
            break;
        case 0x02:
            strcpy(buffer, "MajorMinorRevision");
            break;
        case 0x90:
            strcpy(buffer, "Extended 1");
            break;
        case 0xA0:
            strcpy(buffer, "Extended 2");
            break;
        default:
            return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    return NMBS_ERROR_NONE;
}
//---------------------------------------------------------------------------
//===========================================================================
