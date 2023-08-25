#include <ArduinoSort.h>
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
long distance = 0;
unsigned char CS;
long vulgraad;
long liter;

/*
Hier maak ik 4 variables. 'lastDistance' is voor de vorige distance. Dus stel, hij meet de eerste keer 10mm en de tweede keer 11mm, dan is lastDistance, als hij 11mm (dus de 2e keer)
meet 10mm. 
Ook maak ik een array, met 'firstNumberLenght', de lengte van die array. In die array komen de eerste, in dit geval 50 getallen te staan. Dat doen we om de eerste 'lastDistance'
te bepalen. Dat moet goed gebeuren, want het systeem werkt als volgt:

  Als het programma wordt opgestart, gaat het programme eerst 50 keer naar de afstanden die hij meet kijken. Er staat dan 'loading...' op het scherm.
  Als hij dat 50 heeft gedaan gaat hij de array van klein naar groot sorteren. Daar heb ik een externe library voor gebruikt. Die heet 'ArduinoSort', en staat in dezelfde map 
  als dit Arduino bestand. Hier kan u vinden hoe u hem kan importeren in uw IDE: 'https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries'. Dat is genoeg, en
  vanaf dat punt werkt het hetzelfde als bijv. de 'LiquidCrystal_I2C' library.
  Dan haal ik aan het begin en eind x items uit de array weg. Hoeveel items, dat wordt berekend door x % van het totalte aantal items in de array te doen (in ons geval 50)
  Ik doe nu 10%. De ervaring moet laten zien of dat werkt. Dan worden er dus aan het begin en het eind 5 getallen weggehaald. En van de overige getallen nemen we het gemiddelde.
  En het gemiddelde is dan de lastDistance. En vanaf dan gaat hij iedere keer checken of de distance niet teveel uitwijkt met de lastDistance. Als dat wel het geval is, dan
  doet het programma niks. Alleen moet dus de eerste 'lastDistance' wel goed zijn, en geen 41%, of 185%, want dan denkt hij dat de echte afstand: bijv. 80% teveel afwijkt van de 41,
  en denkt hij dus dat dat niet mag, en dan klopt het systeem niet meer. Daarom dus het systeem met die firstDistance array.

*/
int lastDistance = -1;
const int firstDistancesLenght = 50;
int firstDistances[firstDistancesLenght] = {};
//Met 'firstDistanceIndex' bedoel bij welk item de array 'is'
int firstDistancesIndex = 0;

//Als er meer dan zoveel keer achter elkaar een foute afstand word gemeten, restart dan, deze variable checkt dat
int badDistance = 0;

//dsSerial; ds van distance - sensor
SoftwareSerial dsSerial(pinRX, pinTX);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT);
  pinMode(GroeneLed, OUTPUT);
  pinMode(OranjeLed, OUTPUT);
  pinMode(RodeLed, OUTPUT);
  Serial.begin(115200);
  dsSerial.begin(9600);
}

//Ik heb hier 1 functie voor het laten zien van de tekst, het buzzer geluid, of het lampje, omdat dit makkelijker is, dan hoeft u niet iedere keer opnieuw
//alle code voor het lichte te typen

void LcdWriteTop(String text, bool greenLedOn, bool orangeLedOn, bool redLedOn, bool buzzerOn, bool flashLight)
{
  bool leds[3] = {greenLedOn, orangeLedOn, redLedOn};
  int loop = 1;

  //|| betekent: of / or
  if(buzzerOn == true || flashLight == true)
  {
    loop = 2;
  }

  for(int i = 0; i < loop; i++)
  {
    digitalWrite(GroeneLed, leds[0]);
    digitalWrite(OranjeLed, leds[1]);
    digitalWrite(RodeLed, leds[2]);
    digitalWrite(buzzerPin, buzzerOn);
    lcd.setCursor(0,0);
    lcd.println();
    lcd.print(text); //************// 12 characters / spaces
    lcd.println();
    
    delay(250);
    
    buzzerOn = false;
    for(int i = 0; i < 3; i++)
    {
      leds[i] = false;
    }
  }
}

bool DistanceCheck(int distance, int firstDistances[])
{
  //Die onderstaande 11 is omgerekend iets meer dan 2%
  if(firstDistancesIndex > firstDistancesLenght - 1 && (distance < 0 || lastDistance - distance > 11 ||  - distance < -11))
  {
    return false;
  }else
  {
    int num;
    int sortedDistances[firstDistancesLenght] = {firstDistances};
    sortArray(firstDistances, firstDistancesLenght);
    int remove_items = 0.1 * firstDistancesLenght; //10% (10% = 0.1) van 50 is 5, dat betekend 5 items gaan aan allebij de kanten weg;
    int finalDistancesLenght = firstDistancesLenght - remove_items * 2; // = 40 in ons geval
    int finalDistances[finalDistancesLenght];
    int finelDistancesIndex = 0;

    //Dit haalt de eerste en laatste 'remove_items' weg (dus 5 bij ons). Die zet hij in een nieuwe array: 'finalDistances'.
    for(int i = remove_items; i < firstDistancesLenght - remove_items; i++)
    {
      finalDistances[finelDistancesIndex++] = sortedDistances[i];  
    }

    for(int i = 0; i < finalDistancesLenght; i++)
    {
      num += finalDistances[i];
    }

    lastDistance = num / finalDistancesLenght;
    
  }

  return true;
}

void loop() {
  if (dsSerial.available()>0)
  {
    delay(100);
    if(dsSerial.read()==0xff)
    {
      data_buffer[0]=0xff;
      for(int i = 1; i<4; i++)
      {
        data_buffer[i] = dsSerial.read();
      }

      CS=data_buffer[0]+data_buffer[1]+data_buffer[2];
      if(data_buffer[3] == CS)
      {
        distance = ((data_buffer[1]<<8)+data_buffer[2]);
        
        //Als de DistanceCheck 'true' returned, ga dan door
        if(DistanceCheck(distance, firstDistances))
        {
          Serial.print("Distance: ");
          Serial.print(distance);
          Serial.println(" mm");

          vulgraad = 100 * (Badhoogte - distance) / Overloophoogte; 
          liter = Volumegevuld * (Badhoogte - distance) / Overloophoogte;

          Serial.print(vulgraad);
          Serial.print("/");
          Serial.println(liter);

          //Display vullen
          delay(4);
          lcd.setCursor(0,1);
          lcd.print(liter);
          lcd.print("lt ");
          lcd.print(vulgraad);
          lcd.print("% ");
          lcd.print(distance);
          lcd.print ("mm");

          if(firstDistancesIndex > 50)
          {
            //LcdWriteTop heeft 5 variables nodig: 'text, moet de groene led aan, moet de oranje led aan, moet de rode led aan, moet hij piepen, moet het licht knipperen'

            //Gewenst niveau --> groen
            if(distance < Badhoogte - Droogloophoogte & distance > Badhoogte - Overloophoogte)
            {
              LcdWriteTop("     ok!    ", true, false, false, false, false);
            }
            //Droogloopcheck --> oranje knipperen
            else if(distance > (Badhoogte - Droogloophoogte))
            {
              LcdWriteTop(" droogloop! ", false, true, false, false, true);
            }
            //Bad loopt over in de overloop -> oranje
            else if(distance < Badhoogte - Overloophoogte & distance > Badhoogte - Alarmhoogte)
            {
              LcdWriteTop("  overloop  ", false, true, false, false, false);
            }
            //Bad loopt over over de overloop -> rood knipperen
            else if(distance < Badhoogte - Alarmhoogte)
            {
              LcdWriteTop("    alarm   ", false, false, true, true, true);
            }

            lastDistance = distance;
            badDistance = 0;
          }else
          {
            LcdWriteTop("loading: " + firstDistancesIndex, true, false, false, false, true);
            firstDistances[firstDistancesIndex] = distance;
            firstDistancesIndex ++;
          }
        }else
        {
          if(badDistance < 10)
          {
            badDistance ++;
          }else
          {
            LcdWriteTop("please restart", false, false, true, false, true);
          }
        }
      } 
    }
  }
}