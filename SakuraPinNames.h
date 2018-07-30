#ifndef __SAKURA_PIN_NAMES_H__
#define __SAKURA_PIN_NAMES_H__

#define DAP_UART_TX PB_6
#define DAP_UART_RX PB_7

// GPIO
#define SW1 PC_14
#define SW2 PH_0
#define SW3 PC_0
#define SW4 PC_1
#define SW5 PC_2
#define SW6 PC_3
#undef LED1
#undef LED2 
#undef LED3
#undef LED4
#define LED1 PC_13
#define LED2 PC_15
#define LED3 PH_1
#define LED4 PC_4

// LCD
#define LED_LCD PC_5

// MPU-9250
#define MPU9250_INT PB_2

// GPS
#define GPS_EN PA_11
#define GPS_INT PB_5
#define GPS_1PPS PB_4
#define GPS_WAKEUP PD_2
#define GPS_TX PC_6
#define GPS_RX PC_7

// sakura.io
#define SCM_WAKE_IN  PC_8
#define SCM_WAKE_OUT PA_9

/* Bus */

// I2C (LCD, BME280)
#define I2C_INTERNAL_SCL PB_8
#define I2C_INTERNAL_SDA PB_9

// I2C (sakura.io)
#undef I2C_SCL
#undef I2C_SDA
#define I2C_SCL PA_8
#define I2C_SDA PC_9

// I2C (External)
#define I2C_EXTERNAL_SCL PB_10
#define I2C_EXTERNAL_SDA PB_3

// SPI (MPU-9250)
#define SPI_MPU_MISO PA_12
#define SPI_MPU_MOSI PA_10
#define SPI_MPU_SCK PB_0
#define SPI_MPU_CS PB_1

#endif