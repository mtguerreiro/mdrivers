/*
 * ds18b20.c
 *
 */

//===========================================================================
/*------------------------------- Includes --------------------------------*/
//===========================================================================
#include "ds18b20.h"

//===========================================================================

//===========================================================================
/*------------------------------- Prototypes ------------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
static uint8_t ds18b20crc8(uint8_t *data, uint16_t len);
//---------------------------------------------------------------------------
static int32_t ds18b20ReadSP(uint8_t *buffer, uint32_t to);
//---------------------------------------------------------------------------
static int32_t ds18b20WriteSP(uint8_t *buffer, uint32_t to);
//---------------------------------------------------------------------------
static int32_t ds18b20SkipROM(uint32_t to);
//---------------------------------------------------------------------------
//===========================================================================

//===========================================================================
/*------------------------------ Definitions ------------------------------*/
//===========================================================================
#define DS18B20_CRC_POLY 			0x18 	/** Polynomial for CRC computation. */

/* Commands */
#define DS18B20_CMD_READ_ROM		0x33	/** Read ROM command. */
#define DS18B20_CMD_READ_ROM_SIZE	8 		/** Number of bytes expected from read ROM command. */

#define DS18B20_CMD_READ_SP			0xBE	/** Read scratch pad command. */
#define DS18B20_CMD_READ_SP_SIZE	9 		/** Number of bytes expected from read SP command. */

#define DS18B20_CMD_SKIP_ROM		0xCC	/** Skip ROM command. */

#define DS18B20_CMD_WRITE_SP		0x4E	/** Write scratch pad command. */
#define DS18B20_CMD_WRITE_SP_SIZE	3		/** Write scratch pad command. */

#define DS18B20_CMD_CONVERT_T		0x44	/** Initiate temperature conversion command. */

typedef struct{
    ds18b20OneWireReset_t reset;
    ds18b20OneWireWrite_t write;
    ds18b20OneWireRead_t read;
}ds18b20OW_t;
//===========================================================================

//===========================================================================
/*-------------------------------- Globals --------------------------------*/
//===========================================================================
static ds18b20OW_t gxow;
//===========================================================================

//===========================================================================
/*------------------------------- Functions -------------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
int32_t ds18b20Initialize(ds18b20Config_t *config){

    gxow.reset = config->owReset;
    gxow.write = config->owWrite;
    gxow.read = config->owRead;

	return 0;
}
//---------------------------------------------------------------------------
int32_t ds18b20ReadROM(uint8_t *buffer, uint32_t to){

	int32_t ret;
	uint8_t cmd;

	ret = gxow.reset(to);
	if( ret != 0 ) return DS18B20_ERR_RESET;

	cmd = DS18B20_CMD_READ_ROM;
	ret = gxow.write(&cmd, 1, to);
	if( ret != 0 ) return DS18B20_ERR_WRITE;

	ret = gxow.read(buffer, DS18B20_CMD_READ_ROM_SIZE, to);
	if( ret != 0 ) return DS18B20_ERR_READ;

	ret = ds18b20crc8(buffer, DS18B20_CMD_READ_ROM_SIZE);
	if( ret != 0 ) return DS18B20_ERR_READ_CRC;

	return 0;
}
//---------------------------------------------------------------------------
int32_t ds18b20SetTempRes(uint8_t res, uint32_t to){

	int32_t ret;
	uint8_t cmd[3];

	cmd[0] = 0;
	cmd[1] = 0;
	cmd[2] = (uint8_t)( ((res & 3U) << 5) | 0x1FU);

	ret = ds18b20WriteSP(cmd, to);
	if( ret != 0 ) return ret;

	return 0;
}
//---------------------------------------------------------------------------
int32_t ds18b20StartConversion(uint32_t to){

	uint8_t cmd;
	int32_t ret;

	ret = ds18b20SkipROM(to);
	if( ret != 0 ) return ret;

	cmd = DS18B20_CMD_CONVERT_T;
	ret = gxow.write(&cmd, 1, to);
	if( ret != 0 ) return DS18B20_ERR_WRITE;

	return 0;
}
//---------------------------------------------------------------------------
int32_t ds18b20ReadTemp(int8_t *temp, uint32_t to){

	int32_t ret;
	uint8_t buffer[9];

	ret = ds18b20ReadSP(buffer, to);
	if( ret != 0 ) return ret;

	/*
	 * The integer part of the temperature is:
	 * 	tempInt = (LSB_B1 << 4) | (MSB_B0 >> 4),
	 * where MSB_B0 are the 4 most significant bits of byte 0, and LSB_B1
	 * are the 4 least significant bits of byte 1.
	 */
	*temp = (int8_t)((buffer[0] & 0xF0U) >> 4U) | ((buffer[1] & 0x0FU) << 4U);

	return 0;
}
//---------------------------------------------------------------------------
int32_t ds18b20ReadTempFull(int8_t *dez, uint16_t *dec, uint32_t to){

	int32_t ret;
	uint8_t buffer[9];
	int8_t tempDez;
	uint16_t tempDec;

	ret = ds18b20ReadSP(buffer, to);
	if( ret != 0 ) return ret;

	/*
	 * The integer part of the temperature is:
	 * 	tempInt = (LSB_B1 << 4) | (MSB_B0 >> 4),
	 * where MSB_B0 are the 4 most significant bits of byte 0, and LSB_B1
	 * are the 4 least significant bits of byte 1.
	 */
	tempDez = (int8_t)((buffer[0] & 0xF0U) >> 4U) | ((buffer[1] & 0x0FU) << 4U);

	/*
	 * The decimal part of the temperature is:
	 * 	((2**3 + 2**2 + 2**1 + 2**0) << 6) - (12 + 6 + 3 + 1.5),
	 * where 2**3 is the 4th bit of byte 0, 2**2 is the second bit of byte 0
	 * and so on. For each bit, we have to subtract 12, 6, 3 or 2, if that
	 * bit is 1.
	 */
	tempDec = 0;
	if( buffer[0] & 0x08 ) tempDec = (0x08 << 6) - 12;
	if( buffer[0] & 0x04 ) tempDec = (uint16_t)( tempDec + ((0x04 << 6) - 6) );
	if( buffer[0] & 0x02 ) tempDec = (uint16_t)( tempDec + ((0x02 << 6) - 3) );
	if( buffer[0] & 0x01 ) tempDec = (uint16_t)( tempDec + ((0x01 << 6) - 1) );

	*dez = tempDez;
	*dec = tempDec;

	return 0;
}
//---------------------------------------------------------------------------
//===========================================================================

//===========================================================================
/*--------------------------- Static functions ----------------------------*/
//===========================================================================
//---------------------------------------------------------------------------
static int32_t ds18b20SkipROM(uint32_t to){

	int32_t ret;
	uint8_t cmd;

	ret = gxow.reset(to);
	if( ret != 0 ) return DS18B20_ERR_RESET;

	cmd = DS18B20_CMD_SKIP_ROM;
	ret = gxow.write(&cmd, 1, to);
	if( ret != 0 ) return DS18B20_ERR_WRITE;

	return 0;
}
//---------------------------------------------------------------------------
static int32_t ds18b20ReadSP(uint8_t *buffer, uint32_t to){

	int32_t ret;
	uint8_t cmd;

	ret = ds18b20SkipROM(to);
	if( ret != 0 ) return ret;

	cmd = DS18B20_CMD_READ_SP;
	ret = gxow.write(&cmd, 1, to);
	if( ret != 0 ) return DS18B20_ERR_WRITE;

	ret = gxow.read(buffer, DS18B20_CMD_READ_SP_SIZE, to);
	if( ret != 0 ) return DS18B20_ERR_READ;

	ret = ds18b20crc8(buffer, DS18B20_CMD_READ_SP_SIZE);
	if( ret != 0 ) return DS18B20_ERR_READ_CRC;

	return 0;
}
//---------------------------------------------------------------------------
static int32_t ds18b20WriteSP(uint8_t *buffer, uint32_t to){

	int32_t ret;
	uint8_t cmd[4];
	uint8_t rbuffer[9];
	uint8_t *p;

	/* First, writes the scratch pad */
	ret = ds18b20SkipROM(to);
	if( ret != 0 ) return ret;

	p = buffer;
	cmd[0] = DS18B20_CMD_WRITE_SP;
	cmd[1] = *p++;
	cmd[2] = *p++;
	cmd[3] = *p++;
	ret = gxow.write(cmd, 4, to);
	if( ret != 0 ) return DS18B20_ERR_WRITE;

	/* Now, reads the scratch pad to check if data is written correctly */
	ret = ds18b20ReadSP(rbuffer, to);
	if( ret != 0 ) return ret;

	if(cmd[1] != rbuffer[2]) return DS18B20_ERR_WRITE_SP;
	if(cmd[2] != rbuffer[3]) return DS18B20_ERR_WRITE_SP;
	if(cmd[3] != rbuffer[4]) return DS18B20_ERR_WRITE_SP;

	return 0;
}
//---------------------------------------------------------------------------
static uint8_t ds18b20crc8(uint8_t *data, uint16_t len){

	uint8_t *p;
	uint16_t k, i;
	uint8_t crc;
	uint8_t d;

	crc = 0;
	p = data;
	for(i = 0; i < len; i++){
		d = *p++;
		for(k = 0; k < 8; k++){
			if( (d ^ crc) & 1 ){
				crc = crc ^ DS18B20_CRC_POLY;
				crc = crc >> 1;
				crc = crc | 0x80;
			}
			else{
				crc = crc >> 1;
			}
			d = d >> 1;
		}
	}

	return crc;
}
//---------------------------------------------------------------------------
//===========================================================================
