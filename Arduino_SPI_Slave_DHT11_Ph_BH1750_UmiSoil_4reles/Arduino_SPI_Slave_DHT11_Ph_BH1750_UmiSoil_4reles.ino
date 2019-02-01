#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <SPI.h>

#define DEBUG

#define HUMIDSOLOPIN A0
#define DHTTYPE DHT11
int DHTPIN = 2;
int pHPin = A1; 

int humidadeSoloValor;
float humidadeValor;
float temeperaturaValor;
uint16_t lumenValor;
int pHValor = 0; 

//Auxiliares para sensor de PH
unsigned long int avgValue; 
float b;
int buf[10],temp;

int sensor = 1; // Variável que vai comutar o apanhado de dados do sensor

/*
 * 1 - Sensor de Humidade do Solo
 * 2 - Sensor de Temperatura - DHT11
 * 3 - Sensor de Humidade do ar - DHT11
 * 4 - Sensor de lumens
 * 5 - Sensor de pH
 */

DHT dht(DHTPIN, DHTTYPE);
BH1750 medidorLuz;


// Auxiliares para protocolo SPI
char buff [100];
volatile byte index;
volatile bool receivedone;
String comandoAtuador;

/* Pinos dos Atuadores */
int atuador1 = 4;
int atuador2 = 5;
int atuador3 = 6;
int atuador4 = 7;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  medidorLuz.begin();

  SPCR |= bit(SPE);         /* Habilita SPI */
  pinMode(MISO, OUTPUT);    /* Faz o pino MISO ser OUTPUT */
  index = 0;
  receivedone = false;
  SPI.attachInterrupt();    /* Interrupção SPI */

  pinMode(atuador1, OUTPUT);
  pinMode(atuador2, OUTPUT);
  pinMode(atuador3, OUTPUT);
  pinMode(atuador4, OUTPUT);

  digitalWrite(atuador1, HIGH);  
  digitalWrite(atuador2, HIGH);  
  digitalWrite(atuador3, HIGH);  
  digitalWrite(atuador4, HIGH);  
  delay(1000);
}

float leituraSensorPH()                                                                                                                                                                                                                                                             {
  for(int i=0;i<10;i++) { 
    buf[i]=analogRead(pHPin);
    delay(10);
  }
  for(int i=0;i<9;i++){
    for(int j=i+1;j<10;j++){
      if(buf[i]>buf[j]){
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++){
    avgValue+=buf[i];    
  }

  float pHVol=(float)avgValue*5.0/1024/6;
  float phValue = -5.70 * pHVol + 21.34;
 
  return phValue;
}

void loop() {

  switch(sensor){
    case 1:
      humidadeSoloValor = analogRead(HUMIDSOLOPIN);
      Serial.print("grow/sensor/umidsoil/01/");
      Serial.println(humidadeSoloValor);
      sensor++;
      break;
    case 2:
        humidadeValor = dht.readHumidity();
        if (isnan(humidadeValor)) {
          #ifdef DEBUG
            Serial.println("Falha na leitura do dht11...");
          #endif
        }else{
          #ifdef DEBUG
            Serial.print("grow/sensor/humidade/01/");
            Serial.println(humidadeValor);
          #endif
        }    
      sensor++;
      break;
    case 3:
        temeperaturaValor = dht.readTemperature();
        if (isnan(temeperaturaValor)) {
          #ifdef DEBUG
            Serial.println("Falha na leitura do dht11...");
          #endif
        }else{
          #ifdef DEBUG
            Serial.print("grow/sensor/temperatura/01/");
            Serial.println(temeperaturaValor);
          #endif
        }
      sensor++;
      break;
    case 4:
      lumenValor = medidorLuz.readLightLevel();
      Serial.print("grow/sensor/lumen/01/");
      Serial.println(lumenValor);
      sensor++;
      break;   
    case 5:
      pHValor = leituraSensorPH();
      Serial.print("grow/sensor/pH/01/");
      Serial.println(pHValor);
      sensor = 1;
      break;         
  }

  delay(2000);
  
  if (receivedone){
    /* Check and print received buffer if any */
    buff[index] = 0;
    index = 0;
    comandoAtuador += buff;
    if(comandoAtuador.substring(0,17) == "Atuador 01 Ativar"){
      //Serial.println("Atuador 1 Ativado...");
      digitalWrite(atuador1, LOW);
    }else if(comandoAtuador.substring(0,20) == "Atuador 01 Desativar"){
      //Serial.println("Atuador 1 Desativado...");      
      digitalWrite(atuador1, HIGH);
    }else if(comandoAtuador.substring(0,17) == "Atuador 02 Ativar"){
      //Serial.println("Atuador 2 Ativado...");
      digitalWrite(atuador2, LOW);
    }else if(comandoAtuador.substring(0,20) == "Atuador 02 Desativar"){
      //Serial.println("Atuador 2 Desativado...");      
      digitalWrite(atuador2, HIGH);
    }else if(comandoAtuador.substring(0,17) == "Atuador 03 Ativar"){
      //Serial.println("Atuador 3 Ativado...");
      digitalWrite(atuador3, LOW);
    }else if(comandoAtuador.substring(0,20) == "Atuador 03 Desativar"){
      //Serial.println("Atuador 3 Desativado...");      
      digitalWrite(atuador3, HIGH);
    }else if(comandoAtuador.substring(0,17) == "Atuador 04 Ativar"){
      //Serial.println("Atuador 4 Ativado...");
      digitalWrite(atuador4, LOW);
    }else if(comandoAtuador.substring(0,20) == "Atuador 04 Desativar"){
      //Serial.println("Atuador 4 Desativado...");      
      digitalWrite(atuador4, HIGH);
    }
    comandoAtuador = "";
    receivedone = false;
  }
  delay(500);
}

// Rotina de interrupão SPI
ISR (SPI_STC_vect){
  uint8_t oldsrg = SREG;
  cli();
  char c = SPDR;
  if (index <sizeof buff)
  {
    buff [index++] = c;
    if (c == '\n'){     /* Check for newline character as end of msg */
     receivedone = true;
    }
  }
  SREG = oldsrg;
}
