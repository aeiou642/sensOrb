#include <Arduino.h>
#include "PN532.h"

PN532 pnInit;

void setup(void){
Serial.begin(115200);
delay(100); // give some time for it to start
pnInit.begin();
Wire.setClock(400000);
}

void loop(void){
// H2P - InJumpForDep, 212, Passive), check for ack
// P2H - InJumpForDep, OK, TG:1, ATR_RES
// H2P - InDataExchange (Tg:1, CMD_1), check for ack
// P2H - In
}