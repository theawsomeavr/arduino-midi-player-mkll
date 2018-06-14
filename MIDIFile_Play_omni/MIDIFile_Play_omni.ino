// Test playing a succession of MIDI files from the SD card.
// Example program to demonstrate the use of the MIDFile library
// Just for fun light up a LED in time to the music.
//
// Hardware required:
//  SD card interface - change SD_SELECT for SPI comms
//  3 LEDs (optional) - to display current status and beat. 
//            Change pin definitions for specific hardware setup - defined below.
uint16_t count1;
int dd;
//int simple_midi=0;
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
//LiquidCrystal lcd(0x3F,16,2);
enum lcd_state  { LSBegin, LSSelect, LSShowFile, LSGotFile };
enum midi_state { MSBegin, MSLoad, MSOpen, MSProcess, MSClose };
enum seq_state  { LCDSeq, MIDISeq };
#include <SdFat.h>
#define comuputa 1
#include <MD_MIDIFile.h>
#define  FNAME_SIZE    30        // 8.3 + '\0' character file names
#define PLAYLIST_FILE "PLAYLIST.TXT"  // file of file names
#define MIDI_EXT    ".mid"
#define USE_MIDI  1
uint16_t  plCount = 0;
void timeron(){
   TCCR1A =0;
  TCCR1B=0;
  TCCR1B|=(1<<CS12);
  
  TIMSK1|=(1<<TOIE1); 
}
void timeroff(){
  lcd.noAutoscroll();
  TCCR1A =0;
  TCCR1B=0;
}
#if USE_MIDI // set up for direct MIDI serial output

#define DEBUG(x)
#define DEBUGX(x)
#define SERIAL_RATE 57600

#else // don't use MIDI to allow printing debug statements

#define DEBUG(x)  Serial.print(x)
#define DEBUGX(x) Serial.print(x, HEX)
#define SERIAL_RATE 57600
int a;
#endif // USE_MIDI
  uint16_t  count = 0;// count of files
  char      fname[FNAME_SIZE];
char nombre[FNAME_SIZE];
// SD chip select pin for SPI comms.
// Arduino Ethernet shield, pin 4.
// Default SD chip select is the SPI SS pin (10).
// Other hardware will be different as documented for that hardware.
#define  SD_SELECT  10
SdFat SD;
// LED definitions for user indicators
#define READY_LED     A0 // when finished
#define SMF_ERROR_LED A2 // SMF error
#define SD_ERROR_LED  A4 // SD error
#define BEAT_LED      A2 // toggles to the 'beat'
int button[3]={A5,A3,A1};
#define WAIT_DELAY  2000  // ms

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))



// The files in the tune list should be located on the SD card 
// or an error will occur opening the file and the next in the 
// list will be opened (skips errors).
  static lcd_state s = LSBegin;
  static uint8_t  plIndex = 0;
//  static char fname[FNAME_SIZE];
  static SdFile plFile;

void check(void)
// create a play list file on the SD card with the names of the files.
// This will then be used in the menu.
{
   //if(count!=4){
   // play list file
  SdFile    mFile;    // MIDI file


  // open/create the play list file
 
//    LCDErrMessage("PL create fail", true);

  SD.vwd()->rewind();
  while (mFile.openNext(SD.vwd(), O_READ))
  {
    mFile.getName(fname, FNAME_SIZE);


    if (mFile.isFile())
    {
      if (strcmp(MIDI_EXT, &fname[strlen(fname)-strlen(MIDI_EXT)]) == 0)
      // only include files with MIDI extension
      {
        //plFile.write(fname,FNAME_SIZE);
     /*   DEBUG("\nFile ");
    DEBUG();
    DEBUG(" ");*/
    
     mFile.getName(nombre, FNAME_SIZE);
//tuneList[count]={nombre};
   // DEBUG(tuneList[8]);
   if(count1==0){
    lcd.clear();
     lcd.setCursor(0,1);
    // lcd.autoscroll();
    lcd.print(nombre);
   // lcd.noAutoscroll();
      lcd.setCursor(0,0);  // use the next file name and play it
    lcd.print("File: ");
    lcd.setCursor(10,0);
    lcd.print(count);
      lcd.setCursor(11,0);
    lcd.print("/");
     
    
   }
        count1++;
        //delay(500);
      }
       lcd.setCursor(13,0);
      lcd.print(count1-1);
     // xx++;
    }
    mFile.close();
  }
  //DEBUG("\nList completed");

  // close the play list file
 
  //timeron();
delay(500);
}
  


//SdFat SD;
MD_MIDIFile SMF;
//int d;
//int f;

void midiCallback(midi_event *pev)
// Called by the MIDIFile library when a file event needs to be processed
// thru the midi communications interface.
// This callback is set up in the setup() function.
{

  for (uint8_t i=0; i<pev->size; i++)
  {
    if(dd==1){
      lcd.noAutoscroll();
    lcd.clear();
    lcd.print(pev->data[0]| pev->channel);
    lcd.setCursor(4,0); 
    lcd.print(pev->data[1]);
       lcd.setCursor(8,0); 
    lcd.print(pev->data[2]);
    }
   if(pev->data[0] >= 0xA0){
Serial.write(pev->data[0] | pev->channel);
    Serial.write(&pev->data[1], pev->size-1);
  }
 if(pev->data[0] <= 0x9f&&pev->data[0] >= 0x90){
  if(pev->data[2]>=1){
  Serial.write(0x90 | pev->channel);
    Serial.write(pev->data[1]);
      Serial.write(127);
  }
  
  else{
     Serial.write(0x80 | pev->channel);
    Serial.write(pev->data[1]);
      Serial.write(0); 
  }
  }
  if(pev->data[0] <= 0x8f&&pev->data[0] >= 0x80){

  Serial.write(0x80 | pev->channel);
    Serial.write(pev->data[1]);
      Serial.write(0);

  
    
  }
}}


void sysexCallback(sysex_event *pev)
// Called by the MIDIFile library when a system Exclusive (sysex) file event needs 
// to be processed through the midi communications interface. Most sysex events cannot 
// really be processed, so we just ignore it here.
// This callback is set up in the setup() function.
{
  DEBUG("\nS T");
  DEBUG(pev->track);
  DEBUG(": Data ");
  for (uint8_t i=0; i<pev->size; i++)
  {
    DEBUGX(pev->data[i]);
  DEBUG(' ');
  }
}

void midiSilence(void)
// Turn everything off on every channel.
// Some midi files are badly behaved and leave notes hanging, so between songs turn
// off all the notes and sound
{
  midi_event  ev;

  // All sound off
  // When All Sound Off is received all oscillators will turn off, and their volume
  // envelopes are set to zero as soon as possible.
  ev.size = 0;
  ev.data[ev.size++] = 0xb0;
  ev.data[ev.size++] = 120;
  ev.data[ev.size++] = 0;

  for (ev.channel = 0; ev.channel < 16; ev.channel++)
    midiCallback(&ev);
}

void setup(void)
{
  
pinMode(9,INPUT_PULLUP);
if(digitalRead(9)==0){
  dd=1;
}
//  #ifdef computa
  
    Serial.begin(31250);
lcd.begin(16,2);

  
    if (!SD.begin(SD_SELECT, SPI_FULL_SPEED))
  {
   lcd.print("SD init fail!");
   // digitalWrite(SD_ERROR_LED, HIGH);
    while (true) ;
  }
//  plCount = createPlaylistFile();
 /// DEBUG(plCount);
  // Initialize MIDIFile
  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setSysexHandler(sysexCallback);
// Serial.begin(SERIAL_RATE);

 
//Serial.print(mFile.read());
  //return(count);


  // Print a message to the LCD.
  //lcd.backlight();
  // Set up LED 
  pinMode(button[0],INPUT_PULLUP);
  pinMode(button[1],INPUT_PULLUP);
  pinMode(button[2],INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  pinMode(READY_LED, OUTPUT);
  pinMode(SD_ERROR_LED, OUTPUT);
  pinMode(SMF_ERROR_LED, OUTPUT);
check();
  
count1--;
  DEBUG("\n[MidiFile Play List]");

  // Initialize SD


 // digitalWrite(READY_LED, HIGH);
}
ISR(TIMER1_OVF_vect){
     lcd.setCursor(16,0); // setea el cursor fuera del conteo del display
  lcd.autoscroll();    // Setea el display para scroll automatico
  lcd.print(" "); 
  TCNT1=44702;
}
void tickMetronome(void)
// flash a LED to the beat
{
  static uint32_t lastBeatTime = 0;
  static boolean  inBeat = false;
  uint16_t  beatTime;

  beatTime = 60000/SMF.getTempo();    // msec/beat = ((60sec/min)*(1000 ms/sec))/(beats/min)
  if (!inBeat)
  {
    if ((millis() - lastBeatTime) >= beatTime)
    {
      lastBeatTime = millis();
      digitalWrite(BEAT_LED, HIGH);
      inBeat = true;
    }
  }
  else
  {
    if ((millis() - lastBeatTime) >= 100) // keep the flash on for 100ms only
    {
      digitalWrite(BEAT_LED, LOW);
      inBeat = false;
    }
  }

}
int i;
int v;
int chan;
void loop(void)
{  
      
           // imprime un character en blanco

      
      count=0;
  //char *tuneList[count1];
 // delay(500);
  if(digitalRead(8)==0){
   chan=1-chan;
   delay(200);
  }
  if(chan==1){
    timeron();
  }
  else{
    timeroff(); 
  }
  if(digitalRead(button[0])==0&&i<count1){
  
  }
  //if(v==0){
    // create a play list file on the SD card with the names of the files.
// This will then be used in the menu.
if(digitalRead(button[0])==0){
 
 i++;
 if(i>count1){
 i=0; 
 }
  //if(count!=4){
   // play list file
  SdFile    mFile;    // MIDI file


  // open/create the play list file
 
//    LCDErrMessage("PL create fail", true);

  SD.vwd()->rewind();
  while (mFile.openNext(SD.vwd(), O_READ))
  {
    mFile.getName(fname, FNAME_SIZE);


    if (mFile.isFile())
    {
      if (strcmp(MIDI_EXT, &fname[strlen(fname)-strlen(MIDI_EXT)]) == 0&&count<i+1)
      // only include files with MIDI extension
      {
        //plFile.write(fname,FNAME_SIZE);
     /*   DEBUG("\nFile ");
    DEBUG();
    DEBUG(" ");*/
    
     mFile.getName(nombre, FNAME_SIZE);
//tuneList[count]={nombre};
   // DEBUG(tuneList[8]);
   if(count==i){
    lcd.clear();
     lcd.setCursor(0,1);
    lcd.print(nombre);
      lcd.setCursor(0,0);  // use the next file name and play it
    lcd.print("File: ");
    lcd.setCursor(10,0);
    lcd.print(count);
      lcd.setCursor(12,0);
    lcd.print("/");
          lcd.setCursor(13,0);
    lcd.print(count1);
        
        }
        count++;
        //delay(500);
      }
     // xx++;
    }
    mFile.close();
  }
  //DEBUG("\nList completed");

  // close the play list file

  
delay(250);
}
  // /* DEBUG("\nFile: ");
    //DEBUG(nombre);
    int  err;
	

	  // reset LEDs
	  digitalWrite(READY_LED, LOW);
	  digitalWrite(SD_ERROR_LED, LOW);

if(digitalRead(button[2])==0/*&&i>0*/){
 
 i--;
  if(i<0){
 i=count1; 
 }
  //if(count!=4){
    // play list file
  SdFile    mFile;    // MIDI file


  // open/create the play list file
 
//    LCDErrMessage("PL create fail", true);

  SD.vwd()->rewind();
  while (mFile.openNext(SD.vwd(), O_READ))
  {
    mFile.getName(fname, FNAME_SIZE);


    if (mFile.isFile())
    {
      if (strcmp(MIDI_EXT, &fname[strlen(fname)-strlen(MIDI_EXT)]) == 0&&count<i+1)
      // only include files with MIDI extension
      {
        //plFile.write(fname,FNAME_SIZE);
     /*   DEBUG("\nFile ");
    DEBUG();
    DEBUG(" ");*/
    
     mFile.getName(nombre, FNAME_SIZE);
//tuneList[count]={nombre};
   // DEBUG(tuneList[8]);
   
   if(count==i){
    lcd.clear();
     lcd.setCursor(0,1);
    lcd.print(nombre);
      lcd.setCursor(0,0);  // use the next file name and play it
    lcd.print("File: ");
    lcd.setCursor(10,0);
    lcd.print(count);
      lcd.setCursor(12,0);
    lcd.print("/");
          lcd.setCursor(13,0);
    lcd.print(count1);
  
   }
        count++;
        
        //delay(500);
      }

      
     // xx++;
    }
    mFile.close();
  }
  //DEBUG("\nList completed");

  // close the play list file
  //plFile.close();
  
delay(250);
}
 //Serial.print(tuneList[i]);

    if(digitalRead(button[1])==0){

        lcd.setCursor(15,0);
    lcd.print("p");
	  SMF.setFilename(nombre);
 if(dd==0){
  
 
   timeron();
 }
	  err = SMF.load();
	  if (err != -1)
	  {
    lcd.clear();
		lcd.print("SMF load Error ");
lcd.setCursor(0,1);   
		lcd.print(err);
		
	//	delay(WAIT_DELAY);
	  }
	  else
	  {
    delay(500);
		// play the file
		while (!SMF.isEOF())
		{
    tickMetronome();
			if (SMF.getNextEvent())
     if(digitalRead(button[1])==0){
     //digitalWrite(A3,0);
   timeroff();
      SMF.close();
    midiSilence();
     }
      if(digitalRead(button[0])==0&&digitalRead(button[2])==1){
     //digitalWrite(A3,0);
     SMF.setTempoAdjust(SMF.getTempoAdjust()+10);
      delay(500);
     }
       if(digitalRead(button[2])==0&&digitalRead(button[0])==1){
     //digitalWrite(A3,0);
     SMF.setTempoAdjust(SMF.getTempoAdjust()-10);
      delay(500);
     }
       if(digitalRead(button[2])==0&&digitalRead(button[0])==0){
     //digitalWrite(A3,0);
     SMF.setTempoAdjust(0);
      delay(500);
     }
			
		}

		// done with this one
   timeroff();
		SMF.close();
		midiSilence();

		// signal finish LED with a dignified pause
		digitalWrite(READY_LED, HIGH);
		delay(WAIT_DELAY);
	  
	}
    }
}
