/*
 * hs300x_code.c
 *
 *  Created on: Jun 30, 2025
 *      Author: ikanari
 */
#include "hs300x_code.h"

/*Function to init I2C and set slave address*/
fsp_err_t i2c_masterInit(uint8_t slaveAddress)
{
    fsp_err_t initErr = FSP_SUCCESS;
    //Open I2C Module
    initErr = R_IIC_MASTER_Open (&g_i2c_master1_ctrl, &g_i2c_master1_cfg);
    if(initErr != FSP_SUCCESS)
    {
        return initErr;
    }

    //Set slave address
    initErr = R_IIC_MASTER_SlaveAddressSet (&g_i2c_master1_ctrl,(uint32_t)slaveAddress, I2C_MASTER_ADDR_MODE_7BIT);
    if(initErr != FSP_SUCCESS)
    {
        return initErr;
    }
    return initErr;
}

/*Close I2C bus*/
fsp_err_t i2_masterDeinit(void)
{
    fsp_err_t deinitErr = FSP_SUCCESS;
    deinitErr = R_IIC_MASTER_Close (&g_i2c_master1_ctrl);
    if (deinitErr != FSP_SUCCESS){
        return deinitErr;
    }

    return deinitErr;
}

/*I2C Master Write Function*/
fsp_err_t i2c_masterWrite(uint8_t len, uint8_t txdata[len])
{
    fsp_err_t writeErr = FSP_SUCCESS;
    i2c_event = 0;

    writeErr =  R_IIC_MASTER_Write(&g_i2c_master1_ctrl, txdata, len, false);
    if (writeErr != FSP_SUCCESS) {
        i2_masterDeinit();
        return writeErr;
    }
    uint32_t timeout = 1000;
    while ((i2c_event == 0) && timeout--) {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    }

    if (i2c_event == 0) {
        i2_masterDeinit();
        return FSP_ERR_TIMEOUT;
    }

    if (i2c_event != I2C_MASTER_EVENT_TX_COMPLETE) {
        i2_masterDeinit();
        return FSP_ERR_ABORTED;
    }
    return writeErr;
}

/*I2C master read function*/
fsp_err_t i2c_masterRead(uint8_t len, uint8_t rxdata[len])
{
    fsp_err_t readErr = FSP_SUCCESS;

    i2c_event = 0;
    readErr = R_IIC_MASTER_Read(&g_i2c_master1_ctrl, rxdata,len, false);
    if (readErr != FSP_SUCCESS) {
        i2_masterDeinit();
        return readErr;
    }

    uint32_t timeout = 1000;
    while ((i2c_event == 0) && timeout--) {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    }

    if (i2c_event == 0) {
        i2_masterDeinit();
        return FSP_ERR_TIMEOUT;
    }

    if (i2c_event != I2C_MASTER_EVENT_RX_COMPLETE) {
        i2_masterDeinit();
        return FSP_ERR_ABORTED;
    }

    return readErr;
}


fsp_err_t start_measurement(void)
{
    fsp_err_t err = FSP_SUCCESS;
    uint8_t cmd_data[1] = {HS3001_START_MEASUREMENT_CMD};
    err = i2c_masterWrite(1, cmd_data);
    return err;
}

fsp_err_t get_measurement(struct hs3001_raw_data * p_raw_data)
{
    fsp_err_t err = FSP_SUCCESS;
    uint8_t rx_data[4] ={0};
    err = i2c_masterRead(4,rx_data);
    if (err == FSP_SUCCESS)
    {
        p_raw_data->humidity[0] = rx_data[0];
        p_raw_data->humidity[1] = rx_data[1];
        p_raw_data->temperature[0] = rx_data[2];
        p_raw_data->temperature[1] = rx_data[3];
        return err;
    }
    return err;
}

void calculateData ( struct sensor_data * hs3001_data, struct hs3001_raw_data * p_raw_data)
{
   int32_t tmp_32 = 0;
   uint16_t tmp_u16 = 0x0000;

   /*Calculate humidity integer and decimal part- Humidity [RH%]*/

   tmp_u16 = (uint16_t) (((uint16_t)p_raw_data->humidity[0] & 0x3f) << 8);
   tmp_u16 =(uint16_t) (tmp_u16 | (uint16_t) (p_raw_data->humidity[1]));
   tmp_32 =(int32_t) (((int32_t) tmp_u16 * 100 * 100) /16383);

   hs3001_data->humidity_data.integer_part = (int16_t) (tmp_32 / 100);
   hs3001_data->humidity_data.decimal_part = (int16_t) (tmp_32 % 100);


   /*Calculate temperature integer and decimal part- Temperature [Celsius]*/
   tmp_u16 = (uint16_t) ((uint16_t) (p_raw_data->temperature[0]) << 8);
   tmp_u16 =(uint16_t) ((tmp_u16 | (uint16_t) (p_raw_data->temperature[1] & 0xfc)) >> 2);
   tmp_32 =(int32_t) ((((int32_t) tmp_u16 * 165 * 100) /16383) -(40 * 100));

   hs3001_data ->temperature_data.integer_part = (int16_t) (tmp_32 / 100);
   hs3001_data ->temperature_data.decimal_part = (int16_t) (tmp_32 % 100);

}

/* Callback function */
void g_i2c_master1_cb(i2c_master_callback_args_t *p_args)
{
    /* TODO: add your own code here */
    i2c_event = p_args->event;
}



