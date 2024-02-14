#include <avr/pgmspace.h>
#include "meldetext.h"
char *buffer1 = malloc(40);
char *buffer2 = malloc(40);
String Zeile1 = "                                    ";
String Zeile2 = "                                    ";
boolean first = true;
int Counttext = 0;
int viewMeldung = 0;
int viewMeldungletzte = 255;
int letzteMeldungen[20]; //Letzte Meldungen
String  letzteVariablen[20];
uint8_t Variablencount = 0;
uint8_t Variablen[20];
int Meldetexte = 0;
uint8_t VARIABLE = 0;
uint8_t LETZTEMELDUNG = 0;
uint8_t TAKTLOESCHEN = 0;
boolean LETZTEMELDUNGblock = false;
boolean LETZTEMELDUNGblockfirst = false;
boolean pushvar = false;
boolean forceupdate = false;
int value = 0;
void setup() {
  Serial.begin(115200);
  //attachInterrupt(digitalPinToInterrupt(2), TAKT, FALLING); // Bei TAKT Eingang TAKT ausführen
  //Testtakte
  TAKT( 0b01100111, 0b01000000);
  TAKT( 0b01100111, 0b01000000);
  TAKT( 0b01100111, 0b01000000);
  TAKT( 0b00000001, 0b00100000);
 
}
//Unendliche Wiederholung zur Abarbeitung von neuen Variablen und texten an Display. (ca. 1Mhz)
void loop() {
 
  if ( (LETZTEMELDUNGblockfirst || viewMeldung != viewMeldungletzte || forceupdate || pushvar)&&!LETZTEMELDUNGblock) {//(!LETZTEMELDUNGblock && LETZTEMELDUNGblockfirst) ||
    forceupdate = 0;
 
    if (LETZTEMELDUNGblockfirst)
      LETZTEMELDUNGblockfirst = false;
    strcpy_P(buffer1, (char*)pgm_read_word(&(string_table[letzteMeldungen[viewMeldung] * 2]))); // Meldetexte zweizeilig Den Nummern zuordnen
    int z = 0;
    Zeile1 = String(buffer1);
    //Serial.println(viewMeldung);
    //Serial.println(letzteVariablen[viewMeldung]);
    while (Zeile1.indexOf('*') != -1 || letzteVariablen[viewMeldung].charAt(z) != -1)
    {
      if (letzteVariablen[viewMeldung].charAt(z) != '\0')
        Zeile1.setCharAt(Zeile1.indexOf('*'), letzteVariablen[viewMeldung].charAt(z++));
      else break;
    };
 
    strcpy_P(buffer2, (char*)pgm_read_word(&(string_table[(letzteMeldungen[viewMeldung] * 2) + 1]))); // Meldetexte zweizeilig Den Nummern zuordnen
    Zeile2 = String(buffer2);
 
    while (Zeile2.indexOf('*') != -1 || letzteVariablen[viewMeldung].charAt(z) != -1)
    {
      if (letzteVariablen[viewMeldung].charAt(z) != '\0')
        Zeile2.setCharAt(Zeile2.indexOf('*'), letzteVariablen[viewMeldung].charAt(z++));
      else break;
    };
 
    pushvar = false;
   // Serial.println(Zeile1);
   // Serial.println(Zeile2);
    viewMeldungletzte = viewMeldung;
 
    //Dummy Aktualisiere Display
  }
  //TESTfälle
  // delay(1000);
  value++;
}
/////Bei Takteingang die Eingangsbits einlesen
void TAKT(uint8_t test1, uint8_t test2) {
  //Bei Flanke Taktsignal ausführen
  uint16_t myPORTC = test1;        //Lese Byte für 8 Eingänge für PORTC erste 8Bit
  uint16_t myPORTD = test2;        //Lese Byte für 8 Eingänge für PORTD 9te Bit, Variable,TAKTLOESCHEN,LetzteMeldung
  uint16_t Meldetextnummer = myPORTC + ((myPORTD & 128) << 1); //Ein Wort aus den 2 Bytes erstellen und das 9te Bit anhängen=Meldetextnummer
  VARIABLE = ((uint8_t)test2 & 64); // nicht null wenn Eingang gesetzt
  LETZTEMELDUNG = ((uint8_t)test2 & 32);
  TAKTLOESCHEN  = ((uint8_t)test2 & 16);
 
  ////LÖSCHEN Eingang abfragen
  if (TAKTLOESCHEN != 0) {
    Meldetexte = 0;
  }
  ///Variablenbit gesetzt?
  if (VARIABLE == 0 ) { //Ohne Variable
    if (Meldetexte < 19)
    {
      //if LETZTEMELDUNG
      Meldetexte++;
      letzteMeldungen[Meldetexte] = Meldetextnummer;
      if (Variablencount > 0) { //Variable vorhanden?
        for (int i = 0; i < Variablencount; i++) {
          letzteVariablen[Meldetexte] += Variablen[i];
        }       Variablencount = 0;
      }
    }
    if (TAKTLOESCHEN != 0)
      forceupdate = true;
 
 
    viewMeldung = Meldetexte;
    if (LETZTEMELDUNG == 0) { LETZTEMELDUNGblock = true; LETZTEMELDUNGblockfirst = true;}else LETZTEMELDUNGblock = false;}
    else Var_Function(myPORTC);//mit Variableneingang Nibbles aus byte einlesen
 
}
//Bei VariablenBit am Eingang Worte für Digits einlesen und Speichern
void Var_Function(uint16_t myPORTC) {
  uint8_t nibble1 =  ((uint8_t)myPORTC & 0b0000000000001111);
  uint8_t nibble2 =  ((uint8_t)myPORTC & 0b0000000011110000) >> 4;
 
  if (nibble1 == 10 || nibble2 == 10) {
    pushvar = true;
 
    Variablen[Variablencount++] = nibble1;
    Variablen[Variablencount++] = nibble2;
    letzteVariablen[viewMeldung] = "";
    for (int i = 0; i < Variablencount; i++) {
      letzteVariablen[viewMeldung] += Variablen[i];
    }       Variablencount = 0;
 
 
  } else {
    Variablen[Variablencount++] = nibble1;
    Variablen[Variablencount++] = nibble2;
  }
}
