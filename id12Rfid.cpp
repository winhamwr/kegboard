#include <NewSoftSerial.h>

#include "WProgram.h"
#include "id12Rfid.h"
#include "kegboard_config.h"

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
	  Serial.println("RFID Detected");
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

		// Output to Serial:

		if (bytesread == 12) {                          // if 12 digit read is complete
		  Serial.print("5-byte code: ");
		  for (i=0; i<5; i++) {
			if (code[i] < 16) Serial.print("0");
			Serial.print(code[i], HEX);
			Serial.print(" ");
		  }
		  Serial.println();

		  Serial.print("Checksum: ");
		  Serial.print(code[5], HEX);
		  Serial.println(code[5] == checksum ? " -- passed." : " -- error.");
		  Serial.println();
		}

		bytesread = 0;
	  }
	}
}
