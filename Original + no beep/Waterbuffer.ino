#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define pinRX                2 
#define pinTX                3 
#define GroeneLed            4
#define OranjeLed            5
#define RodeLed              6
#define Badhoogte           730
#define Alarmhoogte         680
#define Overloophoogte      530
#define Droogloophoogte     100
#define Volumegevuld        2889

int buzzerPin = 9;
int data_buffer[4] = {0};
long distance=0;
unsigned char CS;
long vulgraad;
long liter;
SoftwareSerial mySerial(pinRX, pinTX);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
lcd.init();
lcd.backlight();
pinMode(buzzerPin, OUTPUT);
pinMode(GroeneLed, OUTPUT);
pinMode(OranjeLed, OUTPUT);
pinMode(RodeLed, OUTPUT);
Serial.begin(115200);
mySerial.begin(9600);
}


void loop() {
  if (mySerial.available()>0) {
  delay(100);
  if (mySerial.read()==0xff){
  data_buffer[0]=0xff;
  for (int i = 1; i<4; i++){
  data_buffer[i] = mySerial.read();
  }

  CS=data_buffer[0]+data_buffer[1]+data_buffer[2];
  if (data_buffer[3] == CS){
  distance = ((data_buffer[1]<<8)+data_buffer[2]);
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.println(" mm");

vulgraad = 100*(Badhoogte-distance)/Overloophoogte; 
liter = Volumegevuld*(Badhoogte-distance)/Overloophoogte;

  Serial.print(vulgraad);
  Serial.print("/");
  Serial.println(liter);


//display vullen
delay(4);
lcd.setCursor(0,1);
lcd.print(liter);
lcd.print("lt ");
lcd.print(vulgraad);
lcd.print("% ");
lcd.print(distance);
lcd.print ("mm");


//droogloopcheck --> oranjke knipperen
if (distance>(Badhoogte-Droogloophoogte)){
digitalWrite(GroeneLed, LOW);
digitalWrite(RodeLed, LOW);
digitalWrite(OranjeLed, HIGH);
lcd.setCursor(0,0);
lcd.println();
lcd.print(" droogloop! ");
lcd.println();
delay(250);
digitalWrite(OranjeLed, LOW);
lcd.setCursor(0,0);
lcd.println();
lcd.print("              ");
lcd.println();
}

else{
//gewenst niveau --> groen
    if (distance<Badhoogte-Droogloophoogte & distance>Badhoogte-Overloophoogte){
    digitalWrite(GroeneLed, HIGH);
    digitalWrite(OranjeLed, LOW);
    digitalWrite(RodeLed, LOW);
    digitalWrite(buzzerPin, LOW);
    lcd.setCursor(0,0);
    lcd.println();
    lcd.print("     ok!    ");
    lcd.println();

    }

    else{
      //bad loopt over in de overloop -> oranje
        if (distance<Badhoogte-Overloophoogte & distance>Badhoogte-Alarmhoogte){
        digitalWrite(GroeneLed, HIGH);
        digitalWrite(OranjeLed, LOW);
        digitalWrite(RodeLed, LOW);
        lcd.setCursor(0,0);
        lcd.println();
        lcd.print("  overloop  ");
        lcd.println();
        }

        else{
          //bad loopt over over de overloop -> rood knipperen
          if (distance< Badhoogte-Alarmhoogte && distance > 100){
            digitalWrite(GroeneLed, LOW);
            digitalWrite(OranjeLed, LOW);
            digitalWrite(RodeLed, HIGH);
            digitalWrite(buzzerPin, HIGH);
            lcd.setCursor(0,0);
            lcd.println();
            lcd.print("    alarm   ");
            lcd.println();
       
            delay(250);
            digitalWrite(RodeLed, LOW);
            lcd.setCursor(0,0);
            lcd.println();
            lcd.print("            ");
            lcd.println();
            digitalWrite(buzzerPin, LOW);
            delay(250);
          }
    }}}}}}}