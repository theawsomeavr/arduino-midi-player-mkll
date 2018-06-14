/*
 * MIDIUSB_test.ino
 *
 * Created: 4/6/2015 10:47:08 AM
 * Author: gurbrinder grewal
 * Modified by Arduino LLC (2015)
 */ 

#include "MIDIUSB.h"
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  pinMode(0,INPUT_PULLUP);
   pinMode(1,INPUT_PULLUP);
  Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL_OMNI);  
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void loop() {

  if (MIDI.read())                // Is there a MIDI message incoming ?
    {
     /* Serial.print(MIDI.getType()|MIDI.getChannel(),HEX);
       Serial.print("  ");
      Serial.print(MIDI.getData1(),HEX);
        Serial.print("  ");
      Serial.println(MIDI.getData2(),HEX);*/
        midiEventPacket_t midii = {(MIDI.getType()/0x10), MIDI.getType()|MIDI.getChannel()-1, MIDI.getData1(), MIDI.getData2()};
       MidiUSB.sendMIDI(midii);
      MidiUSB.flush();
        
        
    }
    


  // controlChange(0, 10, 65); // Set the value of controller 10 on channel 0 to 65
}
