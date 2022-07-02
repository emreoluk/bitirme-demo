#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "HUSKYLENS.h"
#include <PID_v1.h>
#include <Servo.h>

/////////////////////////////// PINLER ///////////////////////////////
#define ServoXPin 3 //X eksenindeki aciyi kontrol eden servo pini.
#define ServoYPin 5 //Y eksenindeki aciyi kontrol eden servo pini.
#define nrfCEPin 7  //Wireless modulu CE pini.
#define nrfCSNPin 8 //Wireless modulu CS pini.


/////////////////////////////// HUSKYLENS ///////////////////////////////
HUSKYLENS huskylens; //Olusturulan Huskylens objesi.


/////////////////////////////// NRF24L01 ///////////////////////////////
RF24 radio(nrfCEPin, nrfCSNPin); // CE, CSN
const byte nrfHaberlesmeKanali[6] = "00007";
uint16_t Xcm,Ycm;
uint16_t iletilecekMesaj;

uint16_t TopKonumX;
uint16_t TopKonumY;
uint16_t iletisimSayaci = 0;

double xValback;
double yValback;


/////////////////////////////// PID ///////////////////////////////
double PIDXGirisi, PIDXCikisi; //X ekseni
double PIDYGirisi, PIDYCikisi; //Y ekseni
double HedefDegeriX = 164;
double HedefDegeriY = 110;

float KpY = 0.2; // 0.1    0.2 Kirmizi Top icin
float KiY = 1.1; // 0.003     1  Kirmizi Top icin
float KdY = 0.0018; // 0.0025  0.0025Kirmizi Top icin
float KpX = 0.2; //PID x ekseni degerleri
float KiX = 1.2;
float KdX = 0.0018;

PID myPIDX(&PIDXGirisi, &PIDXCikisi, &HedefDegeriX, KpX, KiX, KdX, DIRECT);
PID myPIDY(&PIDYGirisi, &PIDYCikisi, &HedefDegeriY, KpY, KiY, KdY, REVERSE);

double Ts = 1.3; // Time Sample 1.5 Kirmizi Top icin


/////////////////////////////// SERVO DEGERLERI //////////////////////////9/////
Servo servoX; //X EKSEN
Servo servoY; //Y EKSEN
double ServoyaVerilecekX, ServoyaVerilecekY;
double servoXBaslangicAcisi = 95, servoYBaslangicAcisi = 100;


//grafik cizdirme
int x_hata;
int y_hata;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  NRFBaslangicAyarlariniYap();

  servoX.attach(ServoXPin);  // SERVO MOTORLARIN BAGLANDIGI PINLER
  servoY.attach(ServoYPin);
  servoX.write(servoXBaslangicAcisi);    //SERVOLARIN BASLANGIC ACILARI
  servoY.write(servoYBaslangicAcisi);

  myPIDX.SetMode(AUTOMATIC);             
  myPIDX.SetOutputLimits(-25, 25);
  myPIDX.SetSampleTime(Ts);
      
  myPIDY.SetMode(AUTOMATIC);             
  myPIDY.SetOutputLimits(-25, 25);
  myPIDY.SetSampleTime(Ts);

  huskylens.begin(Wire);
}

void loop(){
  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if(!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
  else while (huskylens.available()){
    HUSKYLENSResult result = huskylens.read();
    printResult(result);
    
    xValback = TopKonumX;
    yValback = TopKonumY;
    TopKonumX = result.xCenter;
    TopKonumY = result.yCenter;
    PIDXGirisi = (double)TopKonumX;
    PIDYGirisi = (double)TopKonumY;
    myPIDX.Compute();  // PIDX HESAPLAMALARI
    myPIDY.Compute();  // PIDY HESAPLAMALARI

    ServoyaVerilecekX =  servoXBaslangicAcisi + PIDXCikisi;   //TOPUN DENGEYE GELMESI ICIN GEREKEN X ACI DEGERLERI 
    ServoyaVerilecekY =  servoYBaslangicAcisi + PIDYCikisi;   //TOPUN DENGEYE GELMESI ICIN GEREKEN Y ACI DEGERLERI

    
    servoX.write(ServoyaVerilecekX); 
    servoY.write(ServoyaVerilecekY); 

    //kablosuz gonderme
    Xcm = (TopKonumX - 52 ) * 50 / 225;
    Ycm = (TopKonumY - 5) * 50 / 225;
    iletilecekMesaj = (Ycm << 8) + Xcm;
    if(iletisimSayaci == 10){
      radio.write(&iletilecekMesaj,sizeof(iletilecekMesaj));
      iletisimSayaci = 0;
    }
    iletisimSayaci += 1;

    grafikCiz();
  }    
}

void NRFBaslangicAyarlariniYap(){
  radio.begin();                              //Starting the radio communication
  radio.openWritingPipe(nrfHaberlesmeKanali); //Setting the address at which we will send the data
  radio.setPALevel(RF24_PA_MAX);              //You can set it as minimum or maximum depending on the distance between the transmitter and receiver. 
  radio.setRetries(3,5);
}

void grafikCiz(){
  x_hata = TopKonumX - HedefDegeriX;
  y_hata = TopKonumY - HedefDegeriY;
  Serial.print(x_hata);
  Serial.print(" ");
  Serial.print(y_hata);
  Serial.print(" ");
}

// Huskylensten alinan bilgileri serial konsola yazan fonksiyon.
void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}