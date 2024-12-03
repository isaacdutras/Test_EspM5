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

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DHTDATA_CHAR_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" 
 
bool deviceConnected = false;

Adafruit_NeoPixel led(ledCount, ledPin, NEO_GRB + NEO_KHZ800);
Adafruit_MPU6050 mpu;
BLECharacteristic *pCharacteristic;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
 
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      Serial.println(rxValue[0]);
 
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
 
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");
      }
    }
};

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

uint8_t pins[2] = {1,2};

void setup()
{
  Serial.begin(115200);
  mpuInit();
  
  // Create the BLE Device
  BLEDevice::init("ESP32"); // Give it a name
 
  // Configura o dispositivo como Servidor BLE
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
 
  // Cria o servico UART
  BLEService *pService = pServer->createService(SERVICE_UUID);
 
  // Cria uma Característica BLE para envio dos dados
  pCharacteristic = pService->createCharacteristic(
                      DHTDATA_CHAR_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                       
  pCharacteristic->addDescriptor(new BLE2902());
 
  // cria uma característica BLE para recebimento dos dados
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
 
  pCharacteristic->setCallbacks(new MyCallbacks());
 
  // Inicia o serviço
  pService->start();
 
  // Inicia a descoberta do ESP32
  pServer->getAdvertising()->start();

  led.begin();
  led.setPixelColor(0, led.Color(100, 100, 200));
  led.show();

}

void loop()
{

  static unsigned long tempo = 0;

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // /* Print out the values */
  // Serial.print(">AccelX:");
  // Serial.println(a.acceleration.x);

  // Serial.print(">AccelY:");
  // Serial.println(a.acceleration.y);

  // Serial.print(">AccelZ:");
  // Serial.println(a.acceleration.z);

  // Serial.print(">GyroX:");
  // Serial.println(g.gyro.x);

  // Serial.print(">GyroY:");
  // Serial.println(g.gyro.y);

  // Serial.print(">GyroZ:");
  // Serial.println(g.gyro.z);

  // Serial.print(">Temp: ");
  // Serial.println(temp.temperature);

  float pitch = atan2(-a.acceleration.y, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180 / PI;

  float roll = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

  // Serial.print(">Pitch: ");
  // Serial.println(pitch);
  // Serial.print(">Roll: ");
  // Serial.println(roll);

  int ledR = map(a.acceleration.x, -10, 10, 0, 255);
  int ledG = map(a.acceleration.y, -10, 10, 0, 255);
  int ledB = map(a.acceleration.z, -10, 10, 0, 255);

  led.setPixelColor(0, led.Color(ledR, ledG, ledB));
  led.show();

  delay(10);

  if (millis() - tempo >= 1000)
  {
    tempo = millis();

    char DataString[16];
    sprintf(DataString, "%f \n" , temp.temperature);
     
    pCharacteristic->setValue(DataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
  }
    

}
