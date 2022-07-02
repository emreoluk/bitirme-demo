#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27,16,2);
RF24 radio(9,10); // CE, CSN
const byte address [6] = "GRUP_2";
uint16_t gelenMesaj;
uint16_t gelenX;
uint16_t gelenY;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);    //baslangcta lcd'ye x ve y gostergeleri basilir
  lcd.print("X:");
  lcd.setCursor(0,1);
  lcd.print("Y:");

  Serial.begin(9600);    //konsola cikti basilacaksa serial baslatilmalidir.

  radio.begin();                       //nrf cihazi baslatilir. 
  radio.openReadingPipe(0, address);   //adres degeri kullanilarak okuma hatti acilir
  radio.setPALevel(RF24_PA_MIN);       //guc seviyesi belirlenir. belirtilen mesafe icin minimum degeri yeterli gorulmustur.
  radio.startListening();
  
}
void loop() 
{
  if (radio.available()){
    // Recieved data masking
    radio.read(&gelenMesaj, sizeof(gelenMesaj));
    gelenY = gelenMesaj/256;    //Gelen mesaj maskelenerek mesajdan x ve y degerleri elde edilir
    gelenX = gelenMesaj%256;

    /*
    //KONSOLA CIKTI BASILMAK ISTENIRSE BU KISIM YORUMDAN CIKARILMALIDIR
    // Console output
    Serial.println(gelenMesaj);
    Serial.print("X: ");
    Serial.print(gelenX);
    Serial.print(" ");
    Serial.print("Y: ");
    Serial.print(gelenY);
    Serial.print("\n");
    */
    // LCD output

    lcd.setCursor(2,0);    //x degerini lcd'ye basmak icin baslangicta yazilan X: ifadesi atlanir
    if(gelenX<10){         //gelen deger tek basamakli ise basina 0 koyulur bu sayede lcd uzerinde tum sayilar 2 basamak halinde yazilir
      lcd.print("0");
    }
    lcd.print(gelenX);
    
    lcd.setCursor(2,1);
    if(gelenY<10){
      lcd.print("0");
    }
    lcd.print(gelenY); 
  }
}
