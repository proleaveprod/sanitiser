#include <Adafruit_NeoPixel.h>

//******************PINS**********************************
#define RUKA         7    //Емкостной датчик наличия руки
#define UROVEN       8 //Датчик уровня жидкости
#define DVIG         6    //Датчик движения 
#define RGB1         5     //Светодиодная лента
#define NASOS        16     //Насос
#define KLAPAN       10     //Клапан
#define BUZY         2      //Индикатор работы речевого модуля
#define BATTERY      A3     //Заряд батареи
#define DVER         A2     //Геркон
#define LED          12     //Подсветка
#define AUDIOBUTTON  3

//*****************AUDIO***********************************
#define AUDIO_HELLO     1
#define AUDIO_VPRISK    2
#define AUDIO_NEXTHAND  3
#define AUDIO_END       4
#define AUDIO_NOFLUID   5
#define AUDIO_DOOR      6
#define AUDIO_LOWBAT    7
#define AUDIO_CRIRBAT   8 
#define AUDIO_ERROR     9
#define AUDIO_START     10  

//*****************CONSTANTS*******************************
#define NUMPIXELS       16    //Количество светодиодов
#define MOTION_PERIOD   5000  // период до начала следующего приветствия
#define HAND_MAXPERIOD  7000

#define NASOS_TIME1       180
#define NASOS_TIME2       100
#define BATTERY_CONST   0.012890625
#define BATTERY_SPREAD  3
//**********************************************************

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, RGB1, NEO_GRB + NEO_KHZ800);

long LastDvig = 15000;
long TimeRuki=0 ;
double battery_voltage[4]={12,12,12,12};
int voltage_step;
bool audio_flag=0;

void setup(){
  pinsInit();
  while(digitalRead(BUZY));
  Serial.begin(9600);
  Serial1.begin(9600);
  pixels.begin();
  delay(2500);
  Battery_check();
  //Battery_check();
  Battery_check();
  Fluid_check();
  Door_check();
  digitalWrite(NASOS, 1); //прогон магистрали
  delay(1000);
  digitalWrite(NASOS, 0);
  Hello_world();
 
}
void loop() {
  Motion_check();
  Hand_check();
  Door_check();
}


  void Hello_world(){
  delay(50);
  if(!digitalRead(AUDIOBUTTON))AudioPlay(AUDIO_START);
  delay(50);
  while(digitalRead(BUZY))RGBrainbowCycle(1);
  delay(50);
}

void Door_check() {  //функция реакции на открытую дверь
  if(digitalRead(DVER))return;
  AudioPlay(AUDIO_DOOR);
   while(!digitalRead(DVER)){
    RGBsetColor(255,0,0);
    delay(100);
    RGBsetColor(0,0,0);
    delay(100);
   }

  Serial1.write(0x7E);
  Serial1.write(0x02);
  Serial1.write(0xA4);
  Serial1.write(0x7E);
   delay(1000);
   
}

void Hand_check() { //функция, когда нужно убрать руку
  if(!digitalRead(RUKA))return;
  
    VPRISK();
    if(!digitalRead(AUDIOBUTTON))AudioPlay(AUDIO_NEXTHAND);
                        while (digitalRead(RUKA))RGBrainbowCycle(1);
    
    TimeRuki=millis();
    delay(10);
    Serial.println("1");
    while(!digitalRead(RUKA)){
       Serial.println("2");
                        if(digitalRead(BUZY))RGBrainbowCycle(1);
                         Serial.println("3");
      if(millis()-TimeRuki > HAND_MAXPERIOD){
         Serial.println("3");
         Serial.print("delay ");Serial.println(millis()-TimeRuki);
        if(!digitalRead(AUDIOBUTTON))AudioPlay(AUDIO_ERROR);
                        while(digitalRead(BUZY))RGBrainbowCycle(1); 
                        RGBsetColor(0,0,0);
                        delay(10);
        return;
      }      
    }
                        RGBsetColor(0,0,0);
      
    VPRISK();
    if(!digitalRead(AUDIOBUTTON))AudioPlay(AUDIO_END);
    //delay(100);
    RGBrainbowCycle(6);
    RGBsetColor(0,0,0);
    
    Battery_check();
    Fluid_check();
    LastDvig=millis();
    
    if(digitalRead(RUKA))Hand_check();
    delay(500);
}

void Motion_check(){
  if(!digitalRead(DVIG))return;
  
  if(millis() - LastDvig > MOTION_PERIOD && !digitalRead(BUZY)){
    
    if(!digitalRead(AUDIOBUTTON))AudioPlay(AUDIO_HELLO);
    delay(100);
    while(digitalRead(BUZY) && !digitalRead(RUKA))RGBrainbowCycle(1);
    RGBsetColor(0,0,0);
    LastDvig=millis();
  }
  
}

void Fluid_check(){
  if(!digitalRead(UROVEN))return;
  while(digitalRead(BUZY));
  
  if(!digitalRead(AUDIOBUTTON))AudioPlay(AUDIO_NOFLUID);
  for(int i=0;i<15;i++){
    RGBsetColor(255,50,0);
    delay(200);
    RGBsetColor(0,0,0);
    delay(200);
  }  
  while(digitalRead(BUZY));
}
 
void Battery_check(){  // Проверка напряжения аккумулятора , в случае низкого заряда 
  battery_voltage[voltage_step] = analogRead(BATTERY)*BATTERY_CONST;
  voltage_step++;
  if(voltage_step==3){
    voltage_step=0;
    battery_voltage[3]=battery_voltage[0]+battery_voltage[1]+battery_voltage[2];
    battery_voltage[3]/=3;
  }
  Serial.println(battery_voltage[3]); 
  
  if( battery_voltage[3] <11.75){
    if(!digitalRead(AUDIOBUTTON))AudioPlay(8);
    
    for(int i=0;i<10;i++){
    RGBsetColor(255,0,0);
    delay(250);
    RGBsetColor(0,0,0);
    delay(250);
    }
    while(1);
        
  }else if(battery_voltage[3] <11.95){
    if(!digitalRead(AUDIOBUTTON))AudioPlay(7);   
    for(int i=0;i<5;i++){
    RGBsetColor(255,50,0);
    delay(500);
    RGBsetColor(0,0,0);
    delay(500);
    }
    
  }  
}

void VPRISK () { //Функция впрыска
  
  if(!digitalRead(RUKA))return;
  if(!digitalRead(AUDIOBUTTON))AudioPlay(AUDIO_VPRISK);
  
  RGBsetColor(255,100,0);
  delay(700);
  digitalWrite(NASOS, 1);
  delay(NASOS_TIME1);
  
//  digitalWrite(KLAPAN,1);
//  delay(NASOS_TIME2);
  digitalWrite(NASOS,0);
//  digitalWrite(KLAPAN,0);
  while(digitalRead(BUZY));
  
  RGBsetColor(0,0,0);
  delay(100);
  
}

void AudioPlay(byte track) {
  Serial1.write(0x7E);
  Serial1.write(0x02);
  Serial1.write(0xA4);
  Serial1.write(0x7E);
  delay(30);
  Serial1.write(0x7E);
  Serial1.write(0x04);
  Serial1.write(0xA1);

  Serial1.write(byte(0x00));
  Serial1.write(track);
  Serial1.write(0x7E);
  delay(120);
}

void RGBrainbow(uint8_t wait) {     //RGB радуга для всех светодиодов
  uint16_t i, j;
   delay(50);
  for(j=0; j<256; j++) {
    for(i=0; i<pixels.numPixels(); i++) {
     pixels.setPixelColor(i, RGBWheel((i+j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

void RGBrainbowCycle(uint8_t wait) {     //RGB радуга по кругу
  uint16_t i, j;
  delay(50);
  for(j=0; j<256; j++) { 
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, RGBWheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}


void RGBsetColor(uint8_t r,uint8_t g,uint8_t b){  // RGB установить цвет всей матрицы
    for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r, g, b)); 
  }
  pixels.show(); 
}

uint32_t RGBWheel(byte WheelPos) {  // Программная функция
  if(WheelPos < 85) {
   return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void pinsInit(){
  pinMode(RUKA, INPUT);
  pinMode(UROVEN,INPUT);
  pinMode(DVIG, INPUT);
  pinMode(RGB1, OUTPUT);
  pinMode(NASOS, OUTPUT);
  pinMode(KLAPAN, OUTPUT); 
  pinMode(BUZY,INPUT);
  pinMode(BATTERY,INPUT);
  pinMode(LED,OUTPUT); 
  pinMode(DVER,INPUT_PULLUP);
  pinMode(AUDIOBUTTON,INPUT_PULLUP);
}
