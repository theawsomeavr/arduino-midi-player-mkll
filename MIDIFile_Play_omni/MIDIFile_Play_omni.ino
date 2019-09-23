#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
#include <SdFat.h>
#include <MD_MIDIFile.h>
#define  FNAME_SIZE    40         // 8.3 + '\0' character file names
#define MIDI_EXT    ".mid"
#define USE_MIDI  1
void timeron() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << TOIE1);
}
void timeroff() {
  lcd.noAutoscroll();
  TCCR1A = 0;
  TCCR1B = 0;
}
#define baudrate 31250
char fname[FNAME_SIZE];
bool dd, chan;
#define  SD_SELECT  10
SdFat SD;
// LED definitions for user indicators
#define READY_LED     A0 // when finished
#define SMF_ERROR_LED A2 // SMF error
#define SD_ERROR_LED  A4 // SD error
#define BEAT_LED      A2 // toggles to the 'beat'
int button[3] = {A5, A3, A1};
SdFile file;
SdFile dirFile;

// Number of files found.
uint16_t n = 0;
const uint8_t nMax = 120;
int8_t num;
uint16_t dirIndex[nMax];
//------------------------------------------------------------------------------
//mFile.getName(fname, FNAME_SIZE);
void init_files() {
  dirFile.open("/", O_READ);
  while (n < nMax && file.openNext(&dirFile, O_READ)) {
    file.getName(fname, FNAME_SIZE);
    if (!file.isSubDir() && !file.isHidden() && (strcmp(MIDI_EXT, &fname[strlen(fname) - strlen(MIDI_EXT)]) == 0)) {
      dirIndex[n] = file.dirIndex();
      n++;
    }
    file.close();
  }
  n--;
}
MD_MIDIFile SMF;
void midiCallback(midi_event *pev)
{
  Serial.write(pev->data[0] | pev->channel);
  for (uint8_t i = 1; i < pev->size; i++)Serial.write(pev->data[i]);
}


void sysexCallback(sysex_event *pev) {}

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
  pinMode(9, INPUT_PULLUP);
  pinMode(BEAT_LED, 1);
  if (digitalRead(9) == 0) {
    dd = 1;
  }
  Serial.begin(baudrate);
  lcd.begin(16, 2);
  while (!SD.begin(SD_SELECT, SPI_FULL_SPEED))
  {
    lcd.print("SD init fail!");
    PORTC ^= _BV(PC2);
    delay(500);
    lcd.clear();
    PORTC ^= _BV(PC2);
  }
  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setSysexHandler(sysexCallback);
  pinMode(button[0], INPUT_PULLUP);
  pinMode(button[1], INPUT_PULLUP);
  pinMode(button[2], INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  init_files();
  file.open(&dirFile, dirIndex[num], O_READ);
  file.getName(fname, FNAME_SIZE);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(fname);
  lcd.setCursor(0, 0); // use the next file name and play it
  lcd.print("File: ");
  lcd.setCursor(10, 0);
  lcd.print(num + 1);
  lcd.setCursor(12, 0);
  lcd.print("/");
  lcd.setCursor(13, 0);
  lcd.print(n + 1);
  file.close();
}
ISR(TIMER1_OVF_vect) {
  lcd.setCursor(16, 0); // setea el cursor fuera del conteo del display
  lcd.autoscroll();    // Setea el display para scroll automatico
  lcd.print(" ");
  TCNT1 = 44702;
}
void tickMetronome(void)
// flash a LED to the beat
{
  static uint32_t lastBeatTime = 0;
  static boolean  inBeat = false;
  uint16_t  beatTime;

  beatTime = 60000 / SMF.getTempo();  // msec/beat = ((60sec/min)*(1000 ms/sec))/(beats/min)
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
void loop(void)
{
  if (digitalRead(8) == 0) {
    chan = !chan;
    delay(200);
  }
  if (chan) {
    timeron();
  }
  else {
    timeroff();
  }
  if (digitalRead(button[0]) == 0) {
    num++;
    if (num > n) {
      num = 0;
    }
    file.open(&dirFile, dirIndex[num], O_READ);
    file.getName(fname, FNAME_SIZE);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(fname);
    lcd.setCursor(0, 0); // use the next file name and play it
    lcd.print("File: ");
    lcd.setCursor(10, 0);
    lcd.print(num + 1);
    lcd.setCursor(12, 0);
    lcd.print("/");
    lcd.setCursor(13, 0);
    lcd.print(n + 1);
    file.close();
    delay(250);
  }
  int  err;
  if (digitalRead(button[2]) == 0) {
    num--;
    if (num < 0) {
      num = n;
    }
    file.open(&dirFile, dirIndex[num], O_READ);
    file.getName(fname, FNAME_SIZE);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(fname);
    lcd.setCursor(0, 0); // use the next file name and play it
    lcd.print("File: ");
    lcd.setCursor(10, 0);
    lcd.print(num + 1);
    lcd.setCursor(12, 0);
    lcd.print("/");
    lcd.setCursor(13, 0);
    lcd.print(n + 1);
    file.close();
    delay(250);
  }
  if (digitalRead(button[1]) == 0) {
    lcd.setCursor(15, 0);
    lcd.print("p");
    if (dd == 1) {
      lcd.clear();
    }
    if (dd == 0) {
      timeron();
    }
    err = SMF.load_FileNum(dirIndex[num]);
    if (err != -1)
    {
      lcd.clear();
      lcd.print("SMF load Error ");
      lcd.setCursor(0, 1);
      lcd.print(err);
    }
    else
    {
      delay(500);
      while (!SMF.isEOF())
      {
        tickMetronome();
        if (SMF.getNextEvent())
          if (digitalRead(button[1]) == 0) {
            timeroff();
            SMF.close();
            midiSilence();
            delay(500);
          }
        if (digitalRead(button[0]) == 0 && digitalRead(button[2]) == 1) {
          SMF.setTempoAdjust(SMF.getTempoAdjust() + 10);
          delay(500);
        }
        if (digitalRead(button[2]) == 0 && digitalRead(button[0]) == 1) {
          SMF.setTempoAdjust(SMF.getTempoAdjust() - 10);
          delay(500);
        }
        if (digitalRead(button[2]) == 0 && digitalRead(button[0]) == 0) {
          SMF.setTempoAdjust(0);
          delay(500);
        }
      }
      timeroff();
      SMF.close();
      midiSilence();
    }
  }
}
