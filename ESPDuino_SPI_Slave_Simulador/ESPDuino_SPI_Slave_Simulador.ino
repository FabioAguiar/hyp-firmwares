#include <SPI.h>
#include <StringSplitter.h>

char buff [100];
volatile byte index;
volatile boolean process;
int sensor = 0;

int atuador1 = 4;
int atuador2 = 5;
int atuador3 = 6;
int atuador4 = 7;

void initSPIConfig(){
   pinMode(MISO, OUTPUT); // have to send on master in so it set as output
   SPCR |= _BV(SPE); // turn on SPI in slave mode
   index = 0; // buffer empty
   process = false;
   SPI.attachInterrupt(); // turn on interrupt  
}

void setActuatorPort(){
   pinMode(atuador1, OUTPUT);
   pinMode(atuador2, OUTPUT);
   pinMode(atuador3, OUTPUT);
   pinMode(atuador4, OUTPUT);    
}

void setup() {
   Serial.begin (115200);
   initSPIConfig();
   setActuatorPort();
}

ISR (SPI_STC_vect) { // SPI interrupt routine 
   byte c = SPDR; // read byte from SPI Data Register
   if (index < sizeof buff) {
      buff [index++] = c; // save data in the next index in the array buff
      if (c == '\r') //check for the end of the word
      process = true;
   }
}

void readHumidSoil(){
  Serial.println("grow/sensor/umidsoil/01/850");
}

void readHumidity(){
  Serial.println("grow/sensor/humidade/01/42");
}

void readTemp(){
  Serial.println("grow/sensor/temperatura/01/26");
}

void readLumen(){
  Serial.println("grow/sensor/lumen/01/250");
}

void readPh(){
  Serial.println("grow/sensor/pH/01/6.3");
}

void selectSensor(){

  switch(sensor){
    case 0:
      readHumidSoil();
      break;
    case 1:
      readHumidity();
      break;
    case 2:
      readTemp();
      break;
    case 3:
      readLumen();
      break;
    case 4:
      readPh();
      break;
  }

  delay(3000);
  sensor++;
  if(sensor == 5){
    sensor = 0;
  }
}

void setActuator(){
  StringSplitter *splitter;
   if (process) {
      process = false; //reseta o processo
      index= 0;
      
      splitter = new StringSplitter(buff, '/', 4);
      digitalWrite(splitter->getItemAtIndex(2).toInt(), splitter->getItemAtIndex(3).toInt());
   }
   delay(500);
}

void loop() {
  selectSensor();
  setActuator();
}

