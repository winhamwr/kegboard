/**
ID-12 RFID authentication for kegbot. 

Uses and requires the NewSoftSerial library
<http://arduiniana.org/libraries/NewSoftSerial/> from Mikal Hart.

To enable, set KB_ENABLE_RFID to 1 in kegbot_config.h and connect pin9 from 
your ID-12 to D6 (or whatever you have for KB_PIN_SERIAL_RFID_TX) on your 
arduino.

Adapted for kegbot by Wes Winham <winhamwr@gmail.com> from code found at: 
http://www.arduino.cc/playground/Code/ID12

Thanks to:
* BARRAGAN <http://people.interaction-ivrea.it/h.barragan>
* HC Gilje <http://hcgilje.wordpress.com/resources/rfid_id12_tagreader/>
* djmatic
* Martijn The <http://www.martijnthe.nl/>
**/

#include <NewSoftSerial.h>

#include "WProgram.h"
#include "id12Rfid.h"
#include "kegboard.h"
#include "kegboard_config.h"
#include "KegboardPacket.h"

#if KB_ENABLE_BUZZER
#include "buzzer.h"

PROGMEM prog_uint16_t RFID_MELODY[] = {
  MELODY_NOTE(4, 1, 75), MELODY_NOTE(0, NOTE_SILENCE, 25),
  MELODY_NOTE(4, 5, 75 ), MELODY_NOTE(0, NOTE_SILENCE, 25),
  MELODY_NOTE(4, 3, 50), MELODY_NOTE(0, NOTE_SILENCE, 50),

  MELODY_NOTE(0, NOTE_SILENCE, 0)
};
#endif

void rfidWriteAuthPacket(char* device_name, uint8_t* token, int token_len, char status) {
  KegboardPacket packet;
  packet.SetType(KBM_AUTH_TOKEN);
  packet.AddTag(KBM_AUTH_TOKEN_TAG_DEVICE, strlen(device_name), device_name);
  packet.AddTag(KBM_AUTH_TOKEN_TAG_TOKEN, token_len, (char*)token);
  packet.AddTag(KBM_AUTH_TOKEN_TAG_STATUS, 1, &status);
  packet.Print();
}

void setupRfid(NewSoftSerial *rfidSerial) {
	pinMode(KB_PIN_SERIAL_RFID_TX, INPUT);
	pinMode(KB_PIN_SERIAL_RFID_RX, OUTPUT);
	rfidSerial->begin(9600);
}

void handleRfidAuth(NewSoftSerial *rfidSerial) {
    byte i = 0;
	byte val = 0;
	byte code[6];
	byte checksum = 0;
	byte bytesread = 0;
	byte tempbyte = 0;

	if(rfidSerial->available() > 0) {
	  if((val = rfidSerial->read()) == 2) {                  // check for header 
		bytesread = 0; 
		while (bytesread < 12) {                        // read 10 digit code + 2 digit checksum
		  if(rfidSerial->available() > 0) { 
			val = rfidSerial->read();
			if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) { // if header or stop bytes before the 10 digit reading 
			  break;                                    // stop reading
			}

			// Do Ascii/Hex conversion:
			if ((val >= '0') && (val <= '9')) {
			  val = val - '0';
			} else if ((val >= 'A') && (val <= 'F')) {
			  val = 10 + val - 'A';
			}

			// Every two hex-digits, add byte to code:
			if (bytesread & 1 == 1) {
			  // make some space for this hex-digit by
			  // shifting the previous hex-digit with 4 bits to the left:
			  code[bytesread >> 1] = (val | (tempbyte << 4));

			  if (bytesread >> 1 != 5) {                // If we're at the checksum byte,
				checksum ^= code[bytesread >> 1];       // Calculate the checksum... (XOR)
			  };
			} else {
			  tempbyte = val;                           // Store the first hex digit first...
			};

			bytesread++;                                // ready to read next digit
		  } 
		} 


		// Output kegboard auth_token packet
		if (bytesread == 12) {
			if (code[5] != checksum) {
				return;
			}
			rfidWriteAuthPacket(KB_RFID_DEVICENAME, (uint8_t*)&code, 5, 1);
#if KB_ENABLE_BUZZER
			playMelody(RFID_MELODY);
#endif
		}
		bytesread = 0;
	  }
	}
}
