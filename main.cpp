#include <mbed.h>
#include <AQM0802A.h>
#include <BME280.h>
#include <MPU9250.h>
#include <SakuraIO.h>
#include "SakuraPinNames.h"
#include "sensors.h"
#include "gps.h"

// Serial over CMSIS_DAP
Serial pc(DAP_UART_TX, DAP_UART_RX, 9600);

// GPS
Serial gps(GPS_TX, GPS_RX, 9600);
DigitalOut gps_en(GPS_EN);

// LED
DigitalOut led_1(LED1);
DigitalOut led_2(LED2);
DigitalOut led_3(LED3);
DigitalOut led_4(LED4);

// LCD backlight
DigitalOut lcd_led(LED_LCD);

// Switch
DigitalIn sw_1(SW1);
DigitalIn sw_2(SW2);
DigitalIn sw_3(SW3);
DigitalIn sw_4(SW4);
DigitalIn sw_5(SW5);
DigitalIn sw_6(SW6);

// Internal I2C
I2C internal_i2c(I2C_INTERNAL_SDA, I2C_INTERNAL_SCL);
AQM0802A lcd(internal_i2c);
BME280 bme280(internal_i2c);

// SPI
SPI internal_mpu9250_spi(SPI_MPU_MOSI, SPI_MPU_MISO, SPI_MPU_SCK);
mpu9250_spi mpu9250(internal_mpu9250_spi, SPI_MPU_CS);

// sakura.io
SakuraIO_I2C sakuraio(I2C_SDA, I2C_SCL);

SensorData sensor_data;

// GPS UART buffer
char uart_buffer[128] = {0};
int uart_buffer_index = 0;

// NMEA Decoder
GPSDecoder gps_decoder;

void gps_uart_buffering_handler();

const int SEND_INTERVAL_TICKS_PAR_COUNT = 1500;

void setup()
{
    lcd_led = 1;
    pc.printf("Hello World !\r\n");
    lcd.cls();
    lcd.printf("Hello");

    // Initialize sensors
    bme280.initialize();
    pc.printf("BME280 ok.\r\n");
    mpu9250.init(1, BITS_DLPF_CFG_188HZ);
    pc.printf("MPU9250 ok. WHOAMI=%02x\r\n", mpu9250.whoami());
    if (mpu9250.whoami() != 0x71) {
        pc.printf("[ERROR] MPU9250 init fail.\r\n");
    }

    mpu9250.set_gyro_scale(BITS_FS_2000DPS);    //Set full scale range for gyros
    mpu9250.set_acc_scale(BITS_FS_16G);          //Set full scale range for accs
    mpu9250.calib_acc();
    mpu9250.AK8963_calib_Magnetometer();

    // active high
    gps_en = 1;
    gps.attach(&gps_uart_buffering_handler, Serial::RxIrq);

    led_1 = 1;
    led_2 = 0;
    
    pc.printf("Send par %d seconds.\r\n", (SEND_INTERVAL_TICKS_PAR_COUNT * 200) / 1000);
}

void read_sensor_data()
{
    sensor_data.bme280.temperature = bme280.getTemperature();
    sensor_data.bme280.pressure = bme280.getPressure();
    sensor_data.bme280.humidity = bme280.getHumidity();

    mpu9250.read_all();
    sensor_data.mpu9250.temperature = mpu9250.Temperature;
    for (int cnt_send = 0; cnt_send < 3; cnt_send++) {
        sensor_data.mpu9250.accelerometer[cnt_send] = mpu9250.accelerometer_data[cnt_send];
        sensor_data.mpu9250.gyroscope[cnt_send] = mpu9250.gyroscope_data[cnt_send];
        sensor_data.mpu9250.magnetometer[cnt_send] =  mpu9250.Magnetometer[cnt_send];
    }
}

void enqueue_sensor_data(int counter)
{
    sakuraio.enqueueTx(0, (int32_t)counter);
    sakuraio.enqueueTx(1, sensor_data.bme280.temperature);
    sakuraio.enqueueTx(2, sensor_data.bme280.pressure);
    sakuraio.enqueueTx(3, sensor_data.bme280.humidity);
    sakuraio.enqueueTx(4, sensor_data.mpu9250.accelerometer[0]);
    sakuraio.enqueueTx(5, sensor_data.mpu9250.accelerometer[1]);
    sakuraio.enqueueTx(6, sensor_data.mpu9250.accelerometer[2]);
    sakuraio.enqueueTx(7, sensor_data.mpu9250.gyroscope[0]);
    sakuraio.enqueueTx(8, sensor_data.mpu9250.gyroscope[1]);
    sakuraio.enqueueTx(9, sensor_data.mpu9250.gyroscope[2]);
    sakuraio.enqueueTx(10, sensor_data.mpu9250.magnetometer[0]);
    sakuraio.enqueueTx(11, sensor_data.mpu9250.magnetometer[1]);
    sakuraio.enqueueTx(12, sensor_data.mpu9250.magnetometer[2]);
    sakuraio.enqueueTx(13, gps_decoder.get_longitude());
    sakuraio.enqueueTx(14, gps_decoder.get_latitude());
    sakuraio.enqueueTx(15, gps_decoder.get_unixtime());
}

void gps_uart_buffering_handler()
{
    while (gps.readable() == 1) {
        char c = gps.getc();
        if (c == '\r') {
            continue;
        }
        uart_buffer[uart_buffer_index] = c;
        uart_buffer_index++;
        if (c == '\n') {
            uart_buffer[uart_buffer_index - 1] = '\0';
            gps_decoder.set_nmea_message(uart_buffer);
            gps_decoder.decode();
            uart_buffer_index = 0;
        }
    }
}


void loop()
{
    static int cnt_send = 1;
    static int tick_by_200ms = 0;
    static int stat_sw5 = -1;

    if((sakuraio.getConnectionStatus() & 0x80) == 0x00) {
        //Offline
        lcd.cls();
        lcd.printf("Offline");
        pc.printf("Network is offline.\r\n(After 1 sec to running retry.)\r\n");
        wait(1);
        return;    
    }

    if (stat_sw5 != sw_5) {
        stat_sw5 = sw_5;
        led_3 = stat_sw5;   //State: `Send Enable'
        if (stat_sw5 == 0) {
            lcd.cls();
            lcd.printf("Send:OFF");
        } else {
            cnt_send = 1;
            tick_by_200ms = 0;
            lcd.cls();
            lcd.printf("Send:ON");
        }
    }
    
    if (stat_sw5 == 1) {
        if ((tick_by_200ms % SEND_INTERVAL_TICKS_PAR_COUNT) == 0) {  //Send data intarval is 5 minutes.
            pc.printf("\r\n\r\n--------------------\r\n");
            read_sensor_data();
            pc.printf("BME280\r\n");
            pc.printf("\tTemp: %.2fC\r\n", sensor_data.bme280.temperature);
            pc.printf("\tPres: %.2fhPa\r\n", sensor_data.bme280.pressure);
            pc.printf("\tHum: %.2f%%\r\n", sensor_data.bme280.humidity);
            pc.printf("MPU9250\r\n");
            pc.printf("\tTemp: %.2fC\r\n", sensor_data.mpu9250.temperature);
            for (int j = 0; j < 3; j++) {
                pc.printf("\tacc[%d]: %.2f\r\n", j, sensor_data.mpu9250.accelerometer[j]);
                pc.printf("\tgyro[%d]: %.2f\r\n", j, sensor_data.mpu9250.gyroscope[j]);
                pc.printf("\tmag[%d]: %.2f\r\n", j, sensor_data.mpu9250.magnetometer[j]);
            }
            pc.printf("GPS\r\n");
            pc.printf("\tlat: %f%c\r\n", 
                    gps_decoder.get_latitude(), 
                    gps_decoder.get_latitude() >= 0 ? 'N' : 'S');
            pc.printf("\tlon: %f%c\r\n", 
                    gps_decoder.get_longitude(),
                    gps_decoder.get_longitude() >= 0 ? 'E' : 'W');
            pc.printf("\tspeed: %fkm/h\r\n", gps_decoder.get_speed());
            pc.printf("\tmove_direction: %f\r\n", gps_decoder.get_move_direction());
            pc.printf("\tdate: %d/%02d/%02d %02d:%02d:%02d (UTC)\r\n",
                    gps_decoder.get_year(), gps_decoder.get_month(), gps_decoder.get_day(),
                    gps_decoder.get_hour(), gps_decoder.get_min(), gps_decoder.get_sec());
            pc.printf("\tUNIX time: %d\r\n", gps_decoder.get_unixtime());
            if ((sakuraio.getConnectionStatus() & 0x80) == 0x80) {
                led_2 = 1;
                pc.printf("Send:%d\r\n", cnt_send);
                lcd.setCursor(0, 1);
                lcd.printf("%d", cnt_send);
                enqueue_sensor_data(cnt_send);
                sakuraio.send();
                cnt_send++;
                led_2 = 0;
                pc.printf("After %d sec to send.\r\n", (int)(SEND_INTERVAL_TICKS_PAR_COUNT * 0.2));
            } else {
                return;
            }
        }
    }
    led_1 = !led_1;
    led_4 = !sw_4;
    tick_by_200ms++;
    wait(0.2);
}


int main()
{
    setup();
    while(1) {
        loop();
    }
}
