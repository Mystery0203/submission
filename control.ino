// void MoveForeward(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right);
// void MoveForewardWithSpeed(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right,uint8_t pwmSignal);
// void MoveBack(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right);
// void MoveBackWithSpeed(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right,uint8_t pwmSignal);
// void MoveLeft(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right);
// void MoveLeftWithSpeed(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right,uint8_t pwmSignal);
// void MoveRight(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right);
// void MoveRighWithSpeed(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right,uint8_t pwmSignal);
// void stop(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right);



#include <Servo.h>

Servo servo;


#define FL1 8
#define FL2 9
#define BL1 4
#define BL2 5
#define FR1 6
#define FR2 7
#define BR1 2
#define BR2 3
#define GP1 10
#define GP2 11

#define LED_red 22
#define LED_green 24


 int LEFT;
 int RIGHT;
 int extra;
 int str ;
 int data1 = 90;
 int swtch = 1;
 int tick1 = 0;
 int tick2 = 0;
 double angle;

class Motor
{
private:
  uint8_t con1;
  uint8_t con2;
public:
  Motor(uint8_t con1,uint8_t con2);
 
  void move(uint8_t speed);
  void move();
  void moveRev(uint8_t speed);
  void moveRev();
  void stop();

};

Motor::Motor(uint8_t con1,uint8_t con2)
{
  this->con1 = con1;
  this->con2 = con2;
  pinMode(con1,OUTPUT);
  pinMode(con2,OUTPUT);
}


void Motor::move(uint8_t speed){
    analogWrite(con1,speed);
    digitalWrite(con2,LOW);
}

void Motor::move(){
  digitalWrite(con1,HIGH);
  digitalWrite(con2,LOW);
}

void Motor::moveRev(uint8_t speed){
  analogWrite(con2,speed);
  digitalWrite(con1,LOW);
}
void Motor::moveRev(){
  digitalWrite(con2,HIGH);
  digitalWrite(con1,LOW);
}

void Motor::stop(){
  digitalWrite(con1,LOW);
  digitalWrite(con2,LOW);
}


/* **************************motor class ends here******************************************/


/*************************Basic motor functions**********************************************/


void MoveForeward(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right){
  Front_left.move();
  Front_right.move();
  Back_left.move();
  Back_right.move();
}

void MoveForewardWithSpeed(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right,uint8_t speed){
  Front_left.move(speed);
  Back_left.move(speed);
  Front_right.move(speed);
  Back_right.move(speed);
}

void MoveLeft(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right ){
  Front_left.move();
  Back_left.move();
  Front_right.moveRev();
  Back_right.moveRev();
}

void MoveRight(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right ){
  Front_right.move();
  Back_right.move();
  Front_left.moveRev();
  Back_left.moveRev();
}
void MoveBack(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right ){
  Front_left.moveRev();
  Front_right.moveRev();
  Back_left.moveRev();
  Back_right.moveRev();
}
void MoveBackWithSpeed(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right ,uint8_t speed){
  Front_left.moveRev(speed);
  Front_right.moveRev(speed);
  Back_left.moveRev(speed);
  Back_right.moveRev(speed);
}
void Mstop(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right){
  Front_left.stop();
  Front_right.stop();
  Back_left.stop();
  Back_right.stop();
}

/*******************************basic functions end here**********************************************/


/*******************************Basic Motor drivere via phone bluetooth*******************************/

//static int count = 0;
void control(Motor Front_left,Motor Front_right,Motor Back_left,Motor Back_right,Motor Gripper,char data,uint8_t spd){
 
  if(data == 'w')
    MoveForewardWithSpeed(Front_left,Front_right,Back_left,Back_right,spd);
   
  if(data == 's')
    MoveBackWithSpeed(Front_left,Front_right,Back_left,Back_right,spd);
   
  if(data == 'a')
    MoveLeft(Front_left,Front_right,Back_left,Back_right);
   
  if(data == 'd')
    MoveRight(Front_left,Front_right,Back_left,Back_right);
   
  if(data == '0')
    Mstop(Front_left,Front_right,Back_left,Back_right);
   if(data == 'e')
     Gripper.move();
     
   if(data == 'c')
    Gripper.moveRev();
   
   if(data == 'o')
    Gripper.stop();

  if(data == 'j'){
    //open
    servo.write(90);

  }
  if(data == 'k'){
    //close
    
    servo.write(59);
  }
   
} 
/********************************basic Motor controls ends here**************************************/


/*********************************speed variation on left and right**********************************/

void leftWheels(Motor Front_left,Motor Back_left,int left){
 if(left<0){
  left = -left;
  Front_left.moveRev(left);
  Back_left.moveRev(left);
 }
 else{
  Front_left.move(left);
  Back_left.move(left);
  
 }
}
void rightWheels(Motor Front_right,Motor Back_right,int right){

  if(right<0){
    right = -right;
    Front_right.moveRev(right);
    Back_right.moveRev(right);
  }
  else{
    Front_right.move(right);
    Back_right.move(right);
  }
}
/*************************speed distribution ends here *****************************************/



/***************************advanced control*****************************************************/


void DataRecieved(Motor Front_left,Motor Back_left,Motor Front_right,Motor Back_right,int left,int right){
  
  rightWheels(Front_right,Back_right,left);
  leftWheels(Front_left,Back_left,right);

}





/*********************************speed variation ends here*******************************************/

Motor Front_left(FL1,FL2),Front_right(FR1,FR2),Back_left(BL1,BL2),Back_right(BR1,BR2),Gripper(GP1,GP2);
void setup() {
  pinMode(LED_green,OUTPUT);
  pinMode(LED_red,OUTPUT);
  Serial.begin(9600);
  Serial1.begin(9600);
  servo.attach(12);
  servo.write(0);
 
}

void loop() {
  
//  if(Serial1.available()){
//    char Data = Serial1.read();
// 
//  control(Front_left,Front_right,Back_left,Back_right,Gripper,Data,128);
//  }

  if(Serial1.available()){
    String value = Serial1.readStringUntil('#');
    if(value.length()==7){
    String ANGLE = value.substring(0,3);
    String STR = value.substring(3,6);
    String BUTTON = value.substring(6,7);

    angle = ANGLE.toInt();
    str = STR.toInt();
    
    if(str != 0){
      digitalWrite(LED_green,HIGH);
      digitalWrite(LED_red,LOW);
    }
    else{
      digitalWrite(LED_green,LOW);
      digitalWrite(LED_red,HIGH);
    }
  
  str = map(str,0,100,0,255);
  angle = angle * 3.14/180;

  int y;  //x = -255 to 255
  int x;  //y = -255 to 255 however

  y = (double)str*sin(angle);
  x = (double)str*cos(angle);
  
  LEFT = y - x;
  RIGHT = y + x;
  if(LEFT > 255){
    extra = 255 - LEFT;
    RIGHT+= extra;
    LEFT = 255;
  }
  if(RIGHT> 255){
    extra = 255 - RIGHT;
    LEFT += extra;
    RIGHT = 255;
  }
  if(RIGHT<-255){
    extra = -255 - RIGHT;
    LEFT += extra;
    RIGHT = -255;
  }
  if(LEFT < -255){
    extra = -255 - LEFT;
    RIGHT+= extra;
    LEFT = -255;
  }
//  Serial.print(left);//interchange left and right
//  Serial.print(" ");
//  Serial.println(right);
  DataRecieved(Front_left,Back_left,Front_right,Back_right,LEFT,RIGHT);
  if(tick1 ==0){
  if(BUTTON == "3"){
    if(data1 == 90){
      servo.write(data1);
      data1 = 56;
    }
    else if(data1 == 56){
      servo.write(data1);
      data1 = 90;
    }
      tick1++;
     BUTTON = "f";
     
  }}
  if(tick2 == 0){
  if(BUTTON == "1"){
    if(swtch == 1)
      swtch = 0;
    else if(swtch == 0){
      swtch = 1;}
      
      tick2++;
    BUTTON = "f";
  }
  }
  if(swtch == 1){
    if(BUTTON == "4"){
     // Serial.println("working");
      Gripper.move();
      delay(50);
      Gripper.stop();
    }
    if(BUTTON == "2"){
      Gripper.moveRev();
      delay(50);
      Gripper.stop();
    }
  }
  if(swtch == 0){
    if(BUTTON == "4"){
      Front_right.moveRev(55);
      Back_right.move(55);
      Front_left.move(55);
      Back_left.moveRev(55);
      digitalWrite(LED_green,HIGH);
      digitalWrite(LED_red,LOW);
      delay(50);
      Front_right.stop();
      Back_right.stop();
      Front_left.stop();
      Back_left.stop();
      digitalWrite(LED_green,LOW);
      digitalWrite(LED_red,HIGH);
    }
    if(BUTTON == "2"){
      Front_right.move(55);
      Back_right.moveRev(55);
      Front_left.moveRev(55);
      Back_left.move(55);
      digitalWrite(LED_green,HIGH);
      digitalWrite(LED_red,LOW);
      delay(50);
      Front_right.stop();
      Back_right.stop();
      Front_left.stop();
      Back_left.stop();
      digitalWrite(LED_green,LOW);
      digitalWrite(LED_red,HIGH);
    }
  }
  

}}
//Serial.println(tick2);
  if(tick2 !=0){
  tick2++;
  }
  if(tick1 !=0){
    tick1++;
    digitalWrite(LED_BUILTIN,HIGH);
  }
  if(tick2 == 5000){
    tick2 = 0;
  }
  if(tick1 == 5000){
    tick1 = 0;
    //digitalWrite(LED_BUILTIN,LOW);
  }
  }
