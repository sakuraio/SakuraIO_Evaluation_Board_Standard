typedef struct {
    float temperature;
    float pressure;
    float humidity;
} BME280Data;

typedef struct {
    float temperature;
    float accelerometer[3];
    float gyroscope[3];
    float magnetometer[3];
} MPU9250Data;

typedef struct {
    BME280Data bme280;
    MPU9250Data mpu9250;
} SensorData;
