//#include <SD.h>
//#include <SPI.h>           // SPI library
#include <SdFat.h>         // SDFat Library
//#include <SdFatUtil.h>     // SDFat Util Library
#include <SFEMP3Shield.h>  // Mp3 Shield Library
#include <VS1053.h>
#include <LiquidCrystal_I2C.h> // LCD Library



SdFat sd; // Create object to handle SD functions
//Teste:
//SdFile file;
SdFile sd_file;
SdFile myFile;

ArduinoOutStream cout(Serial);


SFEMP3Shield MP3player; // Create Mp3 library object
// These variables are used in the MP3 initialization to set up
// some stereo options:
const uint8_t volume = 0; // MP3 Player volume 0=max, 255=lowest (off)
const uint16_t monoMode = 3;  // Mono setting 0=off, 3=max


//LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2);

/* Pin setup */
#define TRIGGER_COUNT 7
int triggerPins[TRIGGER_COUNT] = {0, 1, 5, A0, A1, A2, A3};
int stopPin = 1; // This pin triggers a track stop.
int lastTrigger = 0; // This variable keeps track of which tune is playing
int totalFaixas = 0;
int totalBancos = 0;
int banco =0;
int n_botoes = 4;
int bt = 0; // numero valor de incremento das faixas
int id1 = 1; // Variaveis de incremento dos titulos de cada banco
int id2 = 2;
int id3 = 3;
int id4 = 4;
char const divisor = '|';
byte cruz[8] = {B00000,B00100,B00100,B00100,B00100,B01110,B00100,B00000}; //desenha uma cruz
byte quadr[8] = {B11111,B11111,B11111,B11111,B11111,B11111,B11111,B11111}; //desenha um quadrado cheio

void setup()
{
  Serial.begin(115200);
 
  //LCD DISPLAY
  lcd.begin();
  lcd.backlight();
  //Splash Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   THIS IS MY   ");
  lcd.setCursor(0, 1);
  lcd.print("   BOOMSTICK!   ");
  lcd.setCursor(0, 0);
  Serial.println("LCD Good!");
  delay(2000); //Pausa de leitura
  lcd.clear();
  //Teste Triggers
  /*Serial.print("TRIG_0:");
  Serial.println(triggerPins[0]);
  Serial.print("TRIG_1:");
  Serial.println (triggerPins[1]);
  Serial.print("TRIG_2:");
  Serial.println (triggerPins[2]);
  Serial.print("TRIG_3:");
  Serial.println (triggerPins[3]);
  Serial.print("TRIG_4:");
  Serial.println (triggerPins[4]);
  Serial.print("TRIG_5:");
  Serial.println (triggerPins[5]);
  Serial.print("TRIG_6:");
  Serial.println (triggerPins[6]);*/
  
  
  /* Set up all trigger pins as inputs, with pull-ups activated: */
  for (int i=0; i<TRIGGER_COUNT; i++)
  {
    pinMode(triggerPins[i], INPUT_PULLUP);
  }
  pinMode(stopPin, INPUT_PULLUP);

  initSD();  // Initialize the SD card
  initMP3Player(); // Initialize the MP3 Shield

  
  COUNT();
  Serial.print ("Total de faixas:");
  Serial.println (totalFaixas);
  Serial.print ("Nº de bancos:");
  Serial.println (totalBancos);

  DESENHALCD();
}


// All the loop does is continuously step through the trigger
//  pins to see if one is pulled low. If it is, it'll stop any
//  currently playing track, and start playing a new one.
void loop()
{
  for (int i=0; i<TRIGGER_COUNT; i++)
  { 
    if ((digitalRead(triggerPins[i]) == LOW))// && ((i+1) != lastTrigger))
    {
      //lastTrigger = i+1; // Update lastTrigger variable to current trigger
      /* If another track is playing, stop it: */
      if (MP3player.isPlaying())
        MP3player.stopTrack();

      // BOTÕES
      // BT 1
      if ((triggerPins[i]) == 14)//Se for o botão 1 (Analog 0)
      {
        uint8_t result = MP3player.playTrack(bt+1);
        //uint8_t nomx = MP3player.trackTitle((char*)&title);
        //Serial.println (nomx);
                
        if (result != 0)  // Se não encontrar faixa
        {
            ERRO_1();
        }
       }
       
      //BT2
      if ((triggerPins[i]) == 15)//Se for o botão 2 (Analog 1)
      {
        uint8_t result = MP3player.playTrack(bt+2);
      }

      //BT3
      if ((triggerPins[i]) == 16)//Se for o botão 3 (Analog 2)
      {
        uint8_t result = MP3player.playTrack(bt+3);
      }

      //BT4
      if ((triggerPins[i]) == 17)//Se for o botão 4 (Analog 3)
      {
        uint8_t result = MP3player.playTrack(bt+4);
      }

      //BTESQ
      if ((triggerPins[i]) == 0)//Se for o botão ESQ (Digital 0)
      {
        Serial.print("ESQ - bt:");
        Serial.println (bt);
        if (bt > 0)
        {
          bt=bt-4;
          banco=banco-1;
        }
        delay(100);
        DESENHALCD();
      }

      //BTDIR
      if ((triggerPins[i]) == 5)//Se for o botão DIR (Digital 5)
      {
        Serial.print("DIR - bt:");
        Serial.println (bt);
        if (banco != totalBancos-1)
        {
          bt=bt+4;
          banco=banco+1;
        }   
        delay(100);
        DESENHALCD();     
      }
       
      delay(100);
      // An alternative here would be to use the
      //  playMP3(fileName) function, as long as you mapped
      //  the file names to trigger pins.

      
    }
  }
  // After looping through and checking trigger pins, check to
  //  see if the stopPin (A5) is triggered.
  if (digitalRead(stopPin) == LOW)
  {
    //lastTrigger = 0; // Reset lastTrigger
    // If another track is playing, stop it.
    if (MP3player.isPlaying())
      MP3player.stopTrack();
  }
}



//FUNÇÕES

//COUNTER
void COUNT() {
//char filename[20];
  int count = 0;

  // Count the files.
  SdFile file;
  sd.chdir("/", true);
  while (file.openNext(sd.vwd(), O_READ)) {
    count += 1;
    file.close();
  }
  totalFaixas = count-2;
  totalBancos = ceil(totalFaixas*0.25);
   
}


//DESENHA LCD
void DESENHALCD()                                                                                                      
  {

   //uint8_t prim = MP3player.playTrack(bt+1);
  //delay(200);
  char titulo1[30]; // Buffer para armazenas título da musica 1
  MP3player.trackTitle((char*)&titulo1);
  

  
  //LCD
  lcd.createChar(0, cruz);
  lcd.createChar(1, quadr);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOOMSTICK");
  lcd.setCursor(10, 0);
  lcd.print("BANK");     
  lcd.setCursor(0, 1);
  lcd.print(titulo1);
  lcd.setCursor(3, 1);
  lcd.write(0);
  lcd.setCursor(7, 1);
  lcd.write(0);
  lcd.setCursor(11, 1);
  lcd.write(0);  
  lcd.setCursor(15, 1);
  lcd.write(0);     
  lcd.setCursor(15, 0);
  lcd.print(banco+1);
  }


//ERRO SD CARD
void ERRO_1()
  {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">>>>>ERROR!<<<<<");
  lcd.setCursor(0, 1);
  lcd.print(" CHECK SD CARD");
  }


// initSD() initializes the SD card and checks for an error.
void initSD()
  {
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) 
    sd.initErrorHalt();
  if(!sd.chdir("/")) 
    sd.errorHalt("sd.chdir");
  }


// initMP3Player() sets up all of the initialization for the
// MP3 Player Shield. It runs the begin() function, checks
// for errors, applies a patch if found, and sets the volume/
// stero mode.
void initMP3Player()
  {
  uint8_t result = MP3player.begin(); // init the mp3 player shield
  if(result != 0) // check result, see readme for error codes.
  {
    // Error checking can go here!
  }
  MP3player.setVolume(volume, volume);
  MP3player.setMonoMode(monoMode); 
  }
