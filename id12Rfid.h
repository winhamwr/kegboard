#ifndef ID12RFID_H
#define ID12RFID_H

#include <NewSoftSerial.h>

void setupRfid(NewSoftSerial*);
void handleRfidAuth(NewSoftSerial*);

#endif // ID12RIFD_H
