#ifndef HS300X_CODE_H
#define HS300X_CODE_H

#include "hal_data.h"
#include "common_data.h"
#include "user_app_thread.h"

/*HS3001 slave address- 0x44 in hex*/
#define HS3001_SLAVE_ADDRESS 0x44

/*Command to start measurement*/
#define HS3001_START_MEASUREMENT_CMD  0x00


/*Check i2c event*/
static i2c_master_event_t i2c_event = 0;

struct hs3001_raw_data{
    uint8_t humidity[2];
    uint8_t temperature [2];
};


struct Humidity
{
    int16_t integer_part;
    int16_t decimal_part;
};

struct Temperature
{
    int16_t integer_part;
    int16_t decimal_part;
};

struct sensor_data{
     struct Humidity humidity_data;
     struct Temperature temperature_data;
};


/*Function definitions*/
fsp_err_t i2c_masterInit(uint8_t slaveAddress);
fsp_err_t i2_masterDeinit(void);
fsp_err_t i2c_masterWrite(uint8_t len, uint8_t txdata[len]);
fsp_err_t i2c_masterRead(uint8_t len, uint8_t rxdata[len]);
fsp_err_t start_measurement(void);
fsp_err_t get_measurement(struct hs3001_raw_data * p_raw_data);
void calculateData ( struct sensor_data * hs3001_data, struct hs3001_raw_data * p_raw_data);

#endif
