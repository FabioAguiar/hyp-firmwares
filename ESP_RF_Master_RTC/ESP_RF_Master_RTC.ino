// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

#include <SPI.h> //INCLUSÃO DE BIBLIOTECA
#include <nRF24L01.h> //INCLUSÃO DE BIBLIOTECA
#include <RF24.h> //INCLUSÃO DE BIBLIOTECA

RF24 radio(D3, D8); //CRIA UMA INSTÂNCIA UTILIZANDO OS PINOS (CE, CSN)
RTC_DS1307 rtc;


char daysOfTheWeek[7][12] = {"Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const byte address[6] = "00002"; //CRIA UM ENDEREÇO PARA ENVIO DOS
//DADOS (O TRANSMISSOR E O RECEPTOR DEVEM SER CONFIGURADOS COM O MESMO ENDEREÇO)

void setup() {
  radio.begin(); //INICIALIZA A COMUNICAÇÃO SEM FIO
  radio.openWritingPipe(address); //DEFINE O ENDEREÇO PARA ENVIO DE DADOS AO RECEPTOR
  radio.setPALevel(RF24_PA_HIGH); //DEFINE O NÍVEL DO AMPLIFICADOR DE POTÊNCIA
  radio.stopListening(); //DEFINE O MÓDULO COMO TRANSMISSOR (NÃO RECEBE DADOS)

  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.isrunning()) {
    rtc.adjust(DateTime(2019, 10, 12, 21, 0, 0));   // <----------------------SET TIME AND DATE: YYYY,MM,DD,HH,MM,SS
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    rtc.adjust(DateTime(2019, 10, 12, 21, 0, 0));   // <----------------------SET TIME AND DATE: YYYY,MM,DD,HH,MM,SS
  }
  delay(100);
  
}

void loop() {
  DateTime now = rtc.now();
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
    
  delay(3000); //Print date and time every 3 sec
  
  const char text[] = "CONSEGUI CARAI!!!!!"; //VARIÁVEL RECEBE A MENSAGEM A SER TRANSMITIDA
  radio.write(&text, sizeof(text)); //ENVIA AO RECEPTOR A MENSAGEM
  delay(1000); //INTERVALO DE 1 SEGUNDO



}
