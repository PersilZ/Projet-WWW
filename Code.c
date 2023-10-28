#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <ChainableLED.h>
#include <EEPROM.h>
#include <SdFat.h>


#define SEALEVELPRESSURE_HPA (1013.25)


RTC_DS1307 rtc;
SoftwareSerial GPS(8, 9);


byte ErrorCode = 0;

byte LedTickCounter = 0;

bool GPSFlag = true;

unsigned long DataClock=0;
unsigned long LedControl=0;

byte ECOMUL =1;

ChainableLED Led(6,7,1);


// to store in EEPROM

//byte LOG_INTERVAL = 1;
//byte TIMEOUT = 2;
//bool TEMP_AIR = 3;
//bool HYGR = 4;
//bool PRESSURE = 5;
//bool LUMIN = 6;
//signed char MIN_TEMP_AIR = 7;
//signed char MAX_TEMP_AIR = 8;
//signed char HYGR_MINT = 9;
//signed char HYGR_MAXT = 10;
//word LUMIN_LOW = 11;
//word LUMIN_HIGH = 13;
//word PRESSURE_MIN = 15;
//word PRESSURE_MAX = 17;
//
//unsigned int FILE_MAX_SIZE = 19;

//[0-5]byte [6-9]signed char [10-13] word [14] unsigned int 
const  char Name[15][14]  PROGMEM = {{"LOG_INTERVAL"},{"TIMEOUT"},{"TEMP_AIR"},{"HYGR"},{"PRESSURE"},{"LUMIN"},{"MIN_TEMP_AIR"},{"MAX_TEMP_AIR"},{"HYGR_MINT"},{"HYGR_MAXT"},{"LUMIN_LOW"},{"LUMIN_HIGH"},{"PRESSURE_MIN"},{"PRESSURE_MAX"},{"FILE_MAX_SIZE"}};

const char NameF[7][10]  PROGMEM ={{"version"},{"reset"},{"getconfig"},{"help"},{"day"},{"date"},{"clock"}};



const byte NameSlot[] PROGMEM = {1,2,3,4,5,6,7,8,9,10,11,13,15,17,19};


byte Revision = 0 ;
byte mode = 0; //0:standard ; 1:configuration; 2:maintenance; 3:économique;

const  byte Version PROGMEM = 1;


typedef struct Time
{
  byte Hour;
  byte Minute;
  byte Second;
  byte Day;
  byte Month;
  unsigned short Year;
  byte NDay;
}Time;

typedef struct Data 
{
  float LightA;
  float LightI;
  float Temperature;
  float Humidity;
  float Pressure;
  String GPS;
  Time Date;

}Data;

Data Lecture;

String GetName(char * str)
{
  String r = "";
  for (byte k = 0; k < strlen_P(str); k++) 
  {
    r += (char)pgm_read_byte_near(str + k);
  }
  return r;
}


//####################################################################################Code config mode########################################################################
void serialEvent()//code pour commandes
{
  if(mode == 1)
  {
    DataClock = millis();
    String readString ="";

    while (Serial.available()) 
    {
      delay(2); 
      char c = Serial.read();
      readString += c;
    }

    if (readString.length() >0) 
    {

      readString.replace(" ","");

      Serial.println(readString);

      //readString="";
    } 

    if (readString.indexOf("=") == -1)
    {
      
      if (readString.equals(GetName(NameF[0]))) {Serial.print("V."); Serial.println(Version);}
      else 
      if (readString.equals(GetName(NameF[1]))) {Reset(); }
      else
      if (readString.equals(GetName(NameF[2]))) { GetConfig();}
      else 
      if (readString.equals(GetName(NameF[3]))) { GetHelp();}
      else 
      if (readString.equals(GetName(NameF[4]))) { Day();}
      else 
      if (readString.equals(GetName(NameF[5]))) { Date();}
      else 
      if (readString.equals(GetName(NameF[6]))) { Clock();}
      else 
      {
        Serial.println("unrecognize command try help");

      }
    }
    else 
    {
      byte tmp[15];
      memcpy_P(tmp,NameSlot,15);
      long value = readString.substring(readString.indexOf("=")+1,readString.length()).toInt();
      readString=readString.substring(0,readString.indexOf("="));

      //byte
      for(int i = 0 ; i <=5;i++)
      {
        if (readString.equals(GetName(NameF[i])))
        {
          byte var = value;
          EEPROM.put(tmp[i],var);
          break;
          
        }
      }

      //signed char

      {
      
        for(int i = 6 ; i <=9;i++)
        {
          if (readString.equals(GetName(NameF[i])))
          {
            signed char var = value;
            EEPROM.put(tmp[i],var);
            break;
            
          }
        }
          
      }//word
      {
        word var;
        
        for(int i = 10 ; i <=13;i++)
        {
          if (readString.equals(GetName(NameF[i])))
          {
            word var = value;
            EEPROM.put(tmp[i],var);
            break;
          }
        }     
      }
      if (readString.equals(GetName(NameF[14])))
      {
        unsigned int var = value;
        EEPROM.put(tmp[14],var);
      }
    }
  }

}

 

void GetConfig()//obtenir les valeurs des paramètres
{
  byte tmp[15];
  memcpy_P(tmp,NameSlot,15);

  //byte
  for(int i = 0 ; i <=5;i++)
  {
    Serial.println(GetName(Name[i]));
    Serial.println(EEPROM.read(tmp[i]));
  }

//signed char

{
    signed char var;
    for(int i = 6 ; i <=9;i++)
    {
      Serial.println(GetName(Name[i]));    
      EEPROM.get(tmp[i],var);
      Serial.println(var);
    }
    
}//word
{
    word var;
    
    for(int i = 10 ; i <=13;i++)
    {
      Serial.println(GetName(Name[i]));    
      EEPROM.get(tmp[i],var);
      Serial.println(var);
    }
    
}
unsigned int var;
Serial.println(GetName(Name[14]));
EEPROM.get(tmp[14],var);
Serial.println(var);
}

void Clock() //mettre à jour l'heure
{
  Serial.println("Hour?");
  Lecture.Date.Hour=Serial.read();
  Serial.println("Minute?");
  Lecture.Date.Minute=Serial.read();
  Serial.println("Second?");
  Lecture.Date.Second=Serial.read();
  rtc.adjust(DateTime(Lecture.Date.Year,Lecture.Date.Month,Lecture.Date.Day,Lecture.Date.Hour, Lecture.Date.Minute, Lecture.Date.Second));
}

void Date()//mettre à jour la date
{
  Serial.println("Year?");
  Lecture.Date.Year=Serial.read();
  Serial.println("Month?");
  Lecture.Date.Month=Serial.read();
  Serial.println("Day?");
  Lecture.Date.Day=Serial.read();
  rtc.adjust(DateTime(Lecture.Date.Year, Lecture.Date.Month, Lecture.Date.Day,Lecture.Date.Hour,Lecture.Date.Minute,Lecture.Date.Second));
}
void Day() //mettre à jour le jour de la semaine
{
  Serial.println("Week Day?");
  Lecture.Date.NDay=Serial.read();
} 

void Reset()// remettre les paramètres par défaut
{
//byte
EEPROM.write(1,10);
EEPROM.write(2,30);
EEPROM.write(3,1);
EEPROM.write(4,1);
EEPROM.write(5,1);
EEPROM.write(6,1);
//signed char
EEPROM.write(7,-10);
EEPROM.write(8,60);
EEPROM.write(9,0);  
EEPROM.write(10,50);
//word

word var;
var = 255;
EEPROM.put(11,var);
var = 768;
EEPROM.put(13,var);
var = 850;
EEPROM.put(15,var);
var = 1080;
EEPROM.put(17,var);

unsigned int FILE_MAX_SIZE = 4096;
EEPROM.put(19,FILE_MAX_SIZE);

}

void GetHelp()// obtenir les different paramètre et fonction disponibles
{
  Serial.println("var");
  for(int i = 0 ; i <=14;i++)
  {
    Serial.println(GetName(Name[i]));
  }

  Serial.println();
  Serial.println("func");
  for(int i = 0 ; i <=6;i++)
  {
    Serial.println(GetName(NameF[i]));
  }
}



//###########capteurs#################################

void GetRTC()//obtenir date et heure
{

  DateTime now = rtc.now();
  Lecture.Date.Year = now.year();
  Lecture.Date.Month = now.month();
  Lecture.Date.Day = now.day();
  Lecture.Date.Hour = now.hour();
  Lecture.Date.Minute = now.minute();
  Lecture.Date.Second = now.second(); 

}


void GetLum()//obtenir donnée de lumière
{
  if(EEPROM.read(6)==1)
  {
    Lecture.LightA =analogRead(A0);
    Lecture.LightI=(1023-(Lecture.LightA))*10/(Lecture.LightA);
  }else
  {
    Lecture.LightA =NULL;
    Lecture.LightI=NULL;

  }
}

void GetGPS()//obtenir donnée gps
{
  
  GPS.begin(9600);
  char c;
  Lecture.GPS = "";
  while (GPS.available()) 
  {

    c =GPS.read();
    Lecture.GPS += c;
  }
  
}


void GetHum() // obtenir humidité, température, pression
{
  Adafruit_BME280 bme; 
  bme.begin(0x76);
  
  if(EEPROM.read(3)==1)
  {
    Lecture.Temperature = bme.readTemperature();
  }else
  {
    Lecture.Temperature = NULL;
  }

  if(EEPROM.read(5)==1)
  {
    Lecture.Pressure = bme.readPressure() / 100.0;
  }else
  {
    Lecture.Pressure = NULL;
  }
  
  if(EEPROM.read(5)==1)
  {
    Lecture.Humidity = bme.readHumidity();
  }else
  {
    Lecture.Humidity =NULL;
  }

  if(!bme.begin(0x76))
  {
    unsigned long Start = millis();
    while (!bme.begin(0x76)) 
    {
      if(millis()-Start <=EEPROM.read(2)*1000)
      {
        ErrorCode = 3;
        break;
      }


    }
  }else
  if (ErrorCode == 3)
  {
    ErrorCode = 0;
  }
  
  
}

void GetData()// récuperer les donnée précedentes
{
  
  if(mode != 1 && millis()-DataClock>=EEPROM.read(1)*ECOMUL*150)
  {
    Erreur();
    if(mode != 3 || GPSFlag)
    {
      GetGPS(); 
    }
    GPSFlag = !GPSFlag;
    DataClock = millis();
    GetHum();
    GetRTC();
    GetLum();
    
    if(mode==0 || mode == 3) SaveData();
    if(mode==2 && LedTickCounter == 0) PrintData();
 
  }
  else if(millis()-DataClock>=30000)
  {
    Stand();
  
  }



}





//################################################################################################################### Changer mode#########
void SetupButtons()// SetupButtons
{
  pinMode(2,INPUT_PULLUP);
  pinMode(3,INPUT_PULLUP);//VERT
  attachInterrupt(digitalPinToInterrupt(2),ChangeModeRed, FALLING);
  
  attachInterrupt(digitalPinToInterrupt(3),ChangeModeGreen, FALLING);

}

void ChangeModeGreen()//fonction interruption bouton vert
{
  
  
  if(mode==0 && CountButtonTime(3,5000))
  {
    Eco();
  
  }
  
}

bool CountButtonTime(byte pin,short t)//fonction interruption bouton rouge
{
  sei();
  unsigned long ButtonTime = millis();
  while(1)
  {
    if(millis() - ButtonTime>=t)
    {
      break;
    }
    if(digitalRead(pin)== HIGH)
    {
      break;
    }
  }
  cli(); 
  
    //bouton vars
  return (millis() - ButtonTime>=t);
}

void ChangeModeRed()//ChangeModeRed
{
  
  if(CountButtonTime(2, 10))
  {
    if(CountButtonTime(2,5000))
    {
      if(mode == 0  )
      {
      Maint(); 
      }
      else
      {
        Stand();
      }
    }
    else if(mode==0) 
    {
      Config();
    }
  }

  
}

void Stand()//mode standard
{
  if(mode == 3)
  {
    ECOMUL = 1;
  }

  
  Led.setColorRGB(0, 0, 255, 0);
  delay(100);
  LedTickCounter = 200;
  mode = 0;
  
}

void Config()//mode configuration
{
  
  Led.setColorRGB(0, 255, 255, 0);
  delay(100);

  Serial.begin(9600);

  
  DataClock = millis();
  LedTickCounter = 200;
  mode=1;

}

void Maint()// mode maintenance
{

  //print data serie sans  et retirer sd corrompre données
 
  Led.setColorRGB(0, 255,100 , 0);
  delay(100);
  LedTickCounter = 200;
  mode=2;
}

void Eco()// mode économique
{
  
  GPSFlag = false;
  Led.setColorRGB(0, 0,0 , 255);
  delay(100);
  ECOMUL = 2;
  LedTickCounter = 200;
  mode=3;
}



void Erreur()//gestion des erreurs
{

  //Erreur RTC
  if(!rtc.begin())
  {
    unsigned long Start = millis();
    while (!rtc.begin()) 
    {
      Serial.println(!rtc.begin());
      Serial.println(millis()-Start);

      if(millis()-Start <=EEPROM.read(2)*1000)
      {
        ErrorCode = 1;
        break;
      }
      

    }

  }else
  if (ErrorCode == 1)
  {
    ErrorCode = 0;
  }
  



  {
    signed char MIN_TEMP_AIR;
    signed char MAX_TEMP_AIR;
    signed char HYGR_MINT;
    signed char HYGR_MAXT;
    word LUMIN_LOW;
    word LUMIN_HIGH;
    word PRESSURE_MIN;
    word PRESSURE_MAX ;

    EEPROM.get(7, MIN_TEMP_AIR);
    EEPROM.get(8, MAX_TEMP_AIR);
    EEPROM.get(9, HYGR_MINT);
    EEPROM.get(10, HYGR_MAXT);
    EEPROM.get(11, LUMIN_LOW);
    EEPROM.get(13, LUMIN_HIGH);
    EEPROM.get(15, PRESSURE_MIN);
    EEPROM.get(17, PRESSURE_MAX);

    if(( LUMIN_LOW>Lecture.LightA||Lecture.LightA>LUMIN_HIGH)||(PRESSURE_MIN>Lecture.Pressure||Lecture.Pressure>PRESSURE_MAX)||(HYGR_MINT>Lecture.Humidity||Lecture.Humidity>HYGR_MAXT)||(MIN_TEMP_AIR>Lecture.Temperature||Lecture.Temperature>MAX_TEMP_AIR))
    {
      ErrorCode = 4;
    }else
    if (ErrorCode == 4)
    {
      ErrorCode = 0;
    }
  }

 
} 

void ChageLeds() // changer les led
{
  LedTickCounter++;
  delay(16);
  
  switch(ErrorCode)
  {
    
    case 0:
      ModeLed();
      break;

    case 1:
      if(LedTickCounter<64)
      {
        Led.setColorRGB(0,255,0,0);
      }else 
      if(LedTickCounter<128)
      {
        Led.setColorRGB(0,0,0,255);
      }
      else
      {
        ModeLed();
      }
      break;

    case 2:

      if(LedTickCounter<64)
      {
        Led.setColorRGB(0,255,0,0);
      }else 
      if(LedTickCounter<128)
      {
        Led.setColorRGB(0,255,255,0);
      }
      else
      {
        ModeLed();
      }
      break;

    case 3:

      if(LedTickCounter<64)
      {
        Led.setColorRGB(0,255,0,0);
      }else 
      if(LedTickCounter<128)
      {
        Led.setColorRGB(0,0,255,0);
      }
      else
      {
        ModeLed();
      }
      break;

    case 4:
      if(LedTickCounter<64)
      {
        Led.setColorRGB(0,255,0,0);
      }else 
      if(LedTickCounter<192)
      {
        Led.setColorRGB(0,0,255,0);
      }
      else
      {
        ModeLed();
      }
      break;

    case 5:
      if(LedTickCounter<64)
      {
        Led.setColorRGB(0,255,0,0);
      }else 
      if(LedTickCounter<128)
      {
        Led.setColorRGB(0,255,255,255);
      }
      else
      {
        ModeLed();
      }
      
      break;

    case 6:
      if(LedTickCounter<64)
      {
        Led.setColorRGB(0,255,0,0);
      }else 
      if(LedTickCounter<192)
      {
        Led.setColorRGB(0,255,255,255);
      }
      else
      {
        ModeLed();
      }
      break;
  }


}

void ModeLed()
{

  switch (mode) 
  {
  case 0:
    Led.setColorRGB(0,0,255,0);
    break;
  case 1:
      Led.setColorRGB(0,255,255,0);
    
    break;
  case 2:
    
    Led.setColorRGB(0,255,100,0);
    break;
  case 3:
    Led.setColorRGB(0,0,0,255);
    break;
  
  }
}
void SaveData() // enregistrer les données
{
  SdFat sd;
  SdFile myFile;
  sd.begin(4, SPI_HALF_SPEED);


  {
  char FileName[17];
  snprintf(FileName, sizeof(FileName), "%d%d%d_%d.log", Lecture.Date.Year , Lecture.Date.Month, Lecture.Date.Day, Revision);
  
  myFile.open(FileName, O_RDWR | O_CREAT | O_AT_END);

  }
  {
    unsigned int var;
    EEPROM.get(19,var);
    if(myFile.fileSize()>=var)
    {
      Revision++;
    }

  }
  
 
  
  myFile.print(Lecture.Date.Hour);myFile.print(":");myFile.print(Lecture.Date.Minute);myFile.print(":");myFile.print(Lecture.Date.Second);myFile.println();
  myFile.print("\tGPS:");
  myFile.println(Lecture.GPS);
  myFile.print("\tHumidity: ");
  myFile.println(Lecture.Humidity);
  myFile.print("\tPressure: ");
  myFile.println(Lecture.Pressure);
  myFile.print("\tTemperature: ");
  myFile.println(Lecture.Temperature);
  myFile.print("\tLight: ");
  myFile.println(Lecture.LightI);
    

  // close the file:
  myFile.close();

}
 
//Debug

void PrintData() //renvoie donnée
{
  Serial.print(Lecture.Date.Hour);Serial.print(":");Serial.print(Lecture.Date.Minute);Serial.print(":");Serial.print(Lecture.Date.Second);Serial.println();

  Serial.println("GPS");
  Serial.println(Lecture.GPS);
  Serial.println("Humidity");
  Serial.println(Lecture.Humidity);
  Serial.println("Pressure");
  Serial.println(Lecture.Pressure);
  Serial.println("Temperature");
  Serial.println(Lecture.Temperature);
  Serial.println("LightA");
  Serial.println(Lecture.LightA);
  Serial.println("LightI");
  Serial.println(Lecture.LightI);
}


void setup()
{
  if(EEPROM.read(0)!=1)
  {
    Reset();
    EEPROM.write(0,1);
  }
  Serial.begin(9600);

 
  
  SetupButtons();
  Led.init();
  
  pinMode(A0,INPUT);

  Led.setColorRGB(0,255,255,255);
  delay(1000);

  
}

void loop()
{
  
  GetData();
  ChageLeds();
}
