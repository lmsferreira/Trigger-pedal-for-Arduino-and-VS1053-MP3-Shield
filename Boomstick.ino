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
int banco = 0;
int n_botoes = 4;
int bt = 0; // numero valor de incremento das faixas
int id1 = 1; // Variaveis de incremento dos titulos de cada banco
int id2 = 2;
int id3 = 3;
int id4 = 4;
char const divisor = '|';
byte cruz[8] = {B00000, B00100, B00100, B00100, B00100, B01110, B00100, B00000}; //desenha uma cruz
byte quadr[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111}; //desenha um quadrado cheio

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
  for (int i = 0; i < TRIGGER_COUNT; i++)
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
  for (int i = 0; i < TRIGGER_COUNT; i++)
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
        uint8_t result = MP3player.playTrack(bt + 1);
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
        uint8_t result = MP3player.playTrack(bt + 2);
      }

      //BT3
      if ((triggerPins[i]) == 16)//Se for o botão 3 (Analog 2)
      {
        uint8_t result = MP3player.playTrack(bt + 3);
      }

      //BT4
      if ((triggerPins[i]) == 17)//Se for o botão 4 (Analog 3)
      {
        uint8_t result = MP3player.playTrack(bt + 4);
      }

      //BTESQ
      if ((triggerPins[i]) == 0)//Se for o botão ESQ (Digital 0)
      {
        Serial.print("ESQ - bt:");
        Serial.println (bt);
        if (bt > 0)
        {
          bt = bt - 4;
          banco = banco - 1;
        }
        delay(100);
        DESENHALCD();
      }

      //BTDIR
      if ((triggerPins[i]) == 5)//Se for o botão DIR (Digital 5)
      {
        Serial.print("DIR - bt:");
        Serial.println (bt);
        if (banco != totalBancos - 1)
        {
          bt = bt + 4;
          banco = banco + 1;
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
  totalFaixas = count - 2;
  totalBancos = ceil(totalFaixas * 0.25);

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
  lcd.print(banco + 1);
}


//Id3Tag

/*
   example sketch to play audio file(s) in a directory, using the VS1053 library
   for playback and the arduino sd library to read files from a microsd card.
   pins are setup to work well for Arduino Micro.

   originally based on frank zhao's player: http://frank.circleofcurrent.com/
   utilities adapted from previous versions of the functions by matthew seal.

   (c) 2011, 2012 david sirkin sirkin@cdr.stanford.edu
                  akil srinivasan akils@stanford.edu
*/
/*
  // this utility function reads id3v1 and id3v2 tags, if any are present, from
  // mp3 audio files. if no tags are found, just use the title of the file. :-|

  void get_title_from_id3tag () {
  unsigned char id3[3];       // pointer to the first 3 characters to read in

  // visit http://www.id3.org/id3v2.3.0 to learn all(!) about the id3v2 spec.
  // move the file pointer to the beginning, and read the first 3 characters.

  sd_file.seek(0);
  sd_file.read(id3, 3);

  // if these first 3 characters are 'ID3', then we have an id3v2 tag. if so,
  // a 'TIT2' (for ver2.3) or 'TT2' (for ver2.2) frame holds the song title.

  if (id3[0] == 'I' && id3[1] == 'D' && id3[2] == '3') {
    unsigned char pb[4];       // pointer to the last 4 characters we read in
    unsigned char c;           // the next 1 character in the file to be read

    // our first task is to find the length of the (whole) id3v2 tag. knowing
    // this means that we can look for 'TIT2' or 'TT2' frames only within the
    // tag's length, rather than the entire file (which can take a while).

    // skip 3 bytes (that we don't use), then read in the last 4 bytes of the
    // header, which contain the tag's length.

    sd_file.read(pb, 3);
    sd_file.read(pb, 4);

    // to combine these 4 bytes together into the single value, we first have
    // to shift each one over to get it into its correct 'digits' position. a
    // quirk of the spec is that bit 7 (the msb) of each byte is set to 0.

    unsigned long v2l = ((unsigned long) pb[0] << (7 * 3)) +
                        ((unsigned long) pb[1] << (7 * 2)) +
                        ((unsigned long) pb[2] << (7 * 1)) + pb[3];

    // we just moved the file pointer 10 bytes into the file, so we reset it.

    sd.seek(0);

    while (1) {
      // read in bytes of the file, one by one, so we can check for the tags.

      sd_file.read(&c, 1);

      // keep shifting over previously-read bytes as we read in each new one.
      // that way we keep testing if we've found a 'TIT2' or 'TT2' frame yet.

      pb[0] = pb[1];
      pb[1] = pb[2];
      pb[2] = pb[3];
      pb[3] = c;

      if (pb[0] == 'T' && pb[1] == 'I' && pb[2] == 'T' && pb[3] == '2') {
        // found an id3v2.3 frame! the title's length is in the next 4 bytes.

        sd_file.read(pb, 4);

        // only the last of these bytes is likely needed, as it can represent
        // titles up to 255 characters. but to combine these 4 bytes together
        // into the single value, we first have to shift each one over to get
        // it into its correct 'digits' position.

        unsigned long tl = ((unsigned long) pb[0] << (8 * 3)) +
                           ((unsigned long) pb[1] << (8 * 2)) +
                           ((unsigned long) pb[2] << (8 * 1)) + pb[3];
        tl--;

        // skip 2 bytes (we don't use), then read in 1 byte of text encoding.

        sd_file.read(pb, 2);
        sd_file.read(&c, 1);

        // if c=1, the title is in unicode, which uses 2 bytes per character.
        // skip the next 2 bytes (the byte order mark) and decrement tl by 2.

        if (c) {
          sd_file.read(pb, 2);
          tl -= 2;
        }
        // remember that titles are limited to only max_title_len bytes long.

        if (tl > max_title_len) tl = max_title_len;

        // read in tl bytes of the title itself. add an 'end-of-string' byte.

        sd_file.read(title, tl);
        title[tl] = '\0';
        break;
      }
      else
      if (pb[1] == 'T' && pb[2] == 'T' && pb[3] == '2') {
        // found an id3v2.2 frame! the title's length is in the next 3 bytes,
        // but we read in 4 then ignore the last, which is the text encoding.

        sd_file.read(pb, 4);

        // shift each byte over to get it into its correct 'digits' position.

        unsigned long tl = ((unsigned long) pb[0] << (8 * 2)) +
                           ((unsigned long) pb[1] << (8 * 1)) + pb[2];
        tl--;

        // remember that titles are limited to only max_title_len bytes long.

        if (tl > max_title_len) tl = max_title_len;

        // there's no text encoding, so read in tl bytes of the title itself.

        sd_file.read(title, tl);
        title[tl] = '\0';
        break;
      }
      else
      if (sd_file.position() == v2l) {
        // we reached the end of the id3v2 tag. use the file name as a title.

        strncpy(title, fn, max_name_len);
        break;
      }
    }
  }
  else {
      // there is no id3 tag or the id3 version is not supported.
      title[0]='\0';
  }
  }

*/

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
  if (!sd.begin(SD_SEL, SPI_HALF_SPEED))
    sd.initErrorHalt();
  if (!sd.chdir("/"))
    sd.errorHalt("sd.chdir");
}


// initMP3Player() sets up all of the initialization for the
// MP3 Player Shield. It runs the begin() function, checks
// for errors, applies a patch if found, and sets the volume/
// stero mode.
void initMP3Player()
{
  uint8_t result = MP3player.begin(); // init the mp3 player shield
  if (result != 0) // check result, see readme for error codes.
  {
    // Error checking can go here!
  }
  MP3player.setVolume(volume, volume);
  MP3player.setMonoMode(monoMode);
}
