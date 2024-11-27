#include <Adafruit_NeoPixel.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Wire.h>

#define ledPin 2
#define ledCount 1

Adafruit_NeoPixel led(ledCount, ledPin, NEO_GRB + NEO_KHZ800);
Adafruit_MPU6050 mpu;

void mpuInit(){

  Wire.begin(7, 8);

  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("");
  delay(100);
}

void setup()
{
  Serial.begin(115200);
  mpuInit();

  led.begin();
  led.setPixelColor(0, led.Color(100, 100, 200));
  led.show();
}

void loop()
{
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.print(">AccelX:");
  Serial.println(a.acceleration.x);

  Serial.print(">AccelY:");
  Serial.println(a.acceleration.y);

  Serial.print(">AccelZ:");
  Serial.println(a.acceleration.z);

  Serial.print(">GyroX:");
  Serial.println(g.gyro.x);

  Serial.print(">GyroY:");
  Serial.println(g.gyro.y);

  Serial.print(">GyroZ:");
  Serial.println(g.gyro.z);

  Serial.print(">Temp: ");
  Serial.println(temp.temperature);

  float pitch = atan2(-a.acceleration.y, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180 / PI;

  float roll = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

  Serial.print(">Pitch: ");
  Serial.println(pitch);
  Serial.print(">Roll: ");
  Serial.println(roll);

  int ledR = map(a.acceleration.x, -10, 10, 0, 255);
  int ledG = map(a.acceleration.y, -10, 10, 0, 255);
  int ledB = map(a.acceleration.z, -10, 10, 0, 255);

  led.setPixelColor(0, led.Color(ledR, ledG, ledB));
  led.show();

  delay(10);
}
