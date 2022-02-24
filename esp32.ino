#include <Arduino.h>
#include <BluetoothSerial.h>
#include <Wire.h>
#include <SPI.h>
//last left at interrupt and need to create spinlocks so that it receives angles 
//last left at bt
//need to create extra queue for angle isr with spinLock
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t appcpu = 0;
  #else 
    static const BaseType_t appcpu = 1;
#endif

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define TMR_DVDR 80
#define MAX_TMR 20000
#define MPU 0x68
#define A 0.962
#define dt 0.020
#define CRITICAL_ANGLE 20

//global
static SemaphoreHandle_t checkAngle = NULL;
//static SemaphoreHandle_t eulerSem = NULL;
static SemaphoreHandle_t example = NULL;

static hw_timer_t *timer = NULL;

static TaskHandle_t gyro_cal  = NULL;
//static TaskHandle_t TaskMpu = NULL;

// static QueueHandle_t gyroQueue = NULL;
static QueueHandle_t accelQueue = NULL;
static QueueHandle_t caliQueue = NULL;
static QueueHandle_t angleQueue = NULL;
static QueueHandle_t inputQueue = NULL;

BluetoothSerial bt;

void ConfigMpu();

void IRAM_ATTR mpu(){
  BaseType_t stablilizer = pdFALSE;
  float pitch;
  float roll; 
  float yaw;

  xQueueReceiveFromISR(angleQueue,(void*)&roll,NULL);
  xQueueReceiveFromISR(angleQueue,(void*)&pitch,NULL);
  xQueueReceiveFromISR(angleQueue,(void*)&yaw,NULL);
  
    if(pitch<CRITICAL_ANGLE){
      xSemaphoreGiveFromISR(checkAngle,&stablilizer);
      xQueueSendFromISR(angleQueue,(void*)&roll,NULL);
      xQueueSendFromISR(angleQueue,(void*)&pitch,NULL);
      xQueueSendFromISR(angleQueue,(void*)&yaw,NULL);
    }
  if(stablilizer){
    portYIELD_FROM_ISR();
  }
}

void calibration(void* PvParameters){
  //Serial.print("calibration started");
   pinMode(LED_BUILTIN,OUTPUT);

  
  float cal_gyro_x = 0;
  float cal_gyro_y = 0;
  float cal_gyro_z = 0;

  double correct_gyro_x = 0;
  double correct_gyro_y = 0;
  double correct_gyro_z = 0;
  
  short int gyro_x;
  short int gyro_y;
  short int gyro_z;
  
    ConfigMpu();

  digitalWrite(LED_BUILTIN,HIGH);
  for(int i = 0;i<2000;i++){
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission();
    Wire.requestFrom(MPU,6);

    gyro_x = Wire.read()<<8 | Wire.read();
    gyro_y = Wire.read()<<8 | Wire.read();
    gyro_z = Wire.read()<<8 | Wire.read();

    cal_gyro_x = (float)gyro_x/131;       //00 == 131
    cal_gyro_y = (float)gyro_y/131;       //08 == 65.5 
    cal_gyro_z = (float)gyro_z/131;       //10 == 32.8    18==16.4
   
    correct_gyro_x +=cal_gyro_x;
    correct_gyro_y +=cal_gyro_y;
    correct_gyro_z +=cal_gyro_z;

  }
  correct_gyro_x  /=2000;
  correct_gyro_y  /=2000;
  correct_gyro_z  /=2000;
  digitalWrite(LED_BUILTIN,LOW);
  xQueueSend(caliQueue,(void*)&correct_gyro_x,0);
  xQueueSend(caliQueue,(void*)&correct_gyro_y,0);
  xQueueSend(caliQueue,(void*)&correct_gyro_z,0);
  xSemaphoreGive(example);
  timerAlarmEnable(timer);
  vTaskDelete(NULL);
}

void MPU6050(void* PvParameters){
 

  short int accel_x;
  short int accel_y;
  short int accel_z;
  
  short int  Temp;
  
  short int gyro_x;
  short int gyro_y;
  short int gyro_z;

  float cal_accel_x;
  float cal_accel_y;
  float cal_accel_z;

  float cal_gyro_x;
  float cal_gyro_y;
  float cal_gyro_z;

  double cali_gyro_x;
  double cali_gyro_y;
  double cali_gyro_z;

  float calibrated_x;
  float calibrated_y;
  float calibrated_z;
  //float Tempe;
  float pitchangle;
  float rollangle;

  float pitch = 0.0;
  float roll = 0.0;
  float yaw = 0.0;
  xSemaphoreTake(example,portMAX_DELAY);
  xQueueReceive(caliQueue,(void*)&cali_gyro_x,0);
  xQueueReceive(caliQueue,(void*)&cali_gyro_y,0);
  xQueueReceive(caliQueue,(void*)&cali_gyro_z,0);
  while(1){
 
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission();
    Wire.requestFrom(MPU,14);
  
    accel_x = Wire.read()<<8 | Wire.read();
    accel_y = Wire.read()<<8 | Wire.read();
    accel_z = Wire.read()<<8 | Wire.read();
  
    Temp = Wire.read()<<8 | Wire.read();
  
    gyro_x = Wire.read()<<8 | Wire.read();
    gyro_y = Wire.read()<<8 | Wire.read();
    gyro_z = Wire.read()<<8 | Wire.read(); 

    accel_z = ~accel_z + 1;

    cal_accel_x = (float)accel_x/16384;       //2g==16384     
    cal_accel_y = (float)accel_y/16384;       //4g==8192
    cal_accel_z = (float)accel_z/16384;       //8g==4096  //16g == 2048

    cal_gyro_x = (float)gyro_x/131;       //00 == 131
    cal_gyro_y = (float)gyro_y/131;       //08 == 65.5 
    cal_gyro_z = (float)gyro_z/131;       //10 == 32.8    18==16.4

    //Tempe = (float)(Temp)/340 + 36.53;
    calibrated_x = cal_gyro_x - cali_gyro_x;  
    calibrated_y = cal_gyro_y - cali_gyro_y;  
    calibrated_z = cal_gyro_z - cali_gyro_z;  
   
    pitchangle=atan2(cal_accel_x,sqrt(cal_accel_y*cal_accel_y + cal_accel_z*cal_accel_z))*180/PI; //FORMULA FOUND ON INTERNET
    rollangle=atan2(cal_accel_y,cal_accel_z)*180/PI; // FORMULA FOUND ON INTERNET

  //Using Complemetary Filter
    roll=A*(roll+calibrated_x*dt)+(1-A)*rollangle;
    pitch=A*(pitch+calibrated_y*dt)+(1-A)*pitchangle;
    yaw = yaw+calibrated_z*dt;
    
    xQueueSend(angleQueue,(void*)&roll,0);
    xQueueSend(angleQueue,(void*)&pitch,0);
    xQueueSend(angleQueue,(void*)&yaw,0);

    xQueueSend(accelQueue,(void*)&cal_accel_x,0);
    xQueueSend(accelQueue,(void*)&cal_accel_y,0);
    xQueueSend(accelQueue,(void*)&cal_accel_z,0);
 
  }
}
void bluetooth(void* PvParameters){
  char input;
  
  while(1){
    if(bt.available()>0){
      input = bt.read();
      xQueueSend(inputQueue,(void*)&input,0);
    }
  }
}


void setup(){
  Wire.begin();
  Serial.begin(115200);
  bt.begin("esp32");

  timer = timerBegin(0,(uint16_t)TMR_DVDR,true);
  timerAttachInterrupt(timer,&mpu,true);
  timerAlarmWrite(timer,(uint64_t)MAX_TMR,true);

  checkAngle = xSemaphoreCreateBinary();
  example = xSemaphoreCreateBinary();

  caliQueue = xQueueCreate(3,sizeof(double));
  angleQueue = xQueueCreate(3,sizeof(float));
  // gyroQueue = xQueueCreate(3,sizeof(float));
  accelQueue = xQueueCreate(3,sizeof(float));
  inputQueue = xQueueCreate(2,sizeof(char));

  xTaskCreatePinnedToCore(calibration,"gyro calibration",1024,NULL,2,NULL,appcpu);
  xTaskCreatePinnedToCore(MPU6050,"raw input",2048,NULL,1,NULL,appcpu);
  xTaskCreatePinnedToCore(bluetooth,"bluetooth",1024,NULL,1,NULL,appcpu);
  
}
void loop(){
}
void ConfigMpu(){
    Wire.beginTransmission(MPU); //power on mpu6050
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(MPU); //accel config
    Wire.write(0x1C);
    Wire.write(0x00);            //00 == 2g   08 == 4g   10 == 8g   18 == 16g
    Wire.endTransmission();

    Wire.beginTransmission(MPU); // gyro config 
    Wire.write(0x1B);
    Wire.write(0x00);            //00 == 2g   08 == 500   10 == 1000  18 == 20000
    Wire.endTransmission();
}
