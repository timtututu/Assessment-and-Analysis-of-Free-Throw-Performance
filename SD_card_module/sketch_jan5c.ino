#include "I2Cdev.h"
#include "MPU6050.h"
#include <SdFat.h>

// SD card read/write
// MOSI - pin 11
// MISO - pin 12 
// CLK  - pin 13
// CS   - pin 4
const int chipSelect = 10;

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

SdFat sd;
SdFile myFile;

int count = 0;
int idx = 0;

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    // initialize serial communication
    // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
    // it's really up to you depending on your project)
    Serial.begin(38400);

    // initialize device
    Serial.println("Initializing I2C devices...");
    mpu.initialize();

    // change accel sensitivity to 8g
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // Initialize SdFat or print a detailed error message and halt
    // Use half speed like the native library.
    // change to SPI_FULL_SPEED for more performance.
    if (!sd.begin(chipSelect, SPI_HALF_SPEED)) 
        sd.initErrorHalt();
}

void loop() 
{
    // read raw accel/gyro measurements from device
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // dynamic create a file
    if (count == 0)
    {
        // dynamic filename
        char filename[11];
        sprintf(filename, "data_%d.txt", idx);
        Serial.print("filename :");
        Serial.println(filename);

        // open the file for write at end like the Native SD library
        if (!myFile.open(filename, O_RDWR | O_CREAT | O_AT_END)) 
            sd.errorHalt("opening test.txt for write failed");        
    }

    // write data to file
    Serial.print(count); myFile.print(count); myFile.print(" ");
    Serial.print(", ax: "); Serial.print(ax); myFile.print(ax); myFile.print(", ");
    Serial.print(", ay: "); Serial.print(ay); myFile.print(ay); myFile.print(", ");
    Serial.print(", az: "); Serial.print(az); myFile.print(az);
    Serial.print(", gx: "); Serial.print(gx); myFile.print(gx); myFile.print(", ");
    Serial.print(", gy: "); Serial.print(gy); myFile.print(gy); myFile.print(", ");
    Serial.print(", gz: "); Serial.println(gz); myFile.println(gz);          

    // close file when every 1,000 records
    if (count > 1000)
    {
        myFile.close();
        Serial.println("Finish.");
        count = -1;
        idx++;
    }    

    count++;
}