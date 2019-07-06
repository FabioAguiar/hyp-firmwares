#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include<SPI.h>

#define INTERVALO_ENVIO       2000
#define DEBUG

//informações da rede WIFI
const char* ssid = "SUPERNET_FABYUU";  //SSID da rede WIFI
const char* password =  "luci6666";   //senha da rede wifi
//const char* ssid = "Ap-F12";  //SSID da rede WIFI
//const char* password =  "08774576";   //senha da rede wifi


//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "m14.cloudmqtt.com";   //server
const char* mqttUser = "cxhovvsp";              //user
const char* mqttPassword = "v-nT9GcH_-Cr";      //password
const int mqttPort = 14167;                     //port
//tópico que serão assinados
const char* mqttTopicSubA1 = "grow/A1";
const char* mqttTopicSubA2 = "grow/A2";
const char* mqttTopicSubA3 = "grow/A3";
const char* mqttTopicSubA4 = "grow/A4";

 
int ultimoEnvioMQTT = 0;

WiFiClient espClient;
PubSubClient client(espClient);

char buff[100];

char atuador1Ativar[]="Atuador 01 Ativar\n";
char atuador1Desativar[]="Atuador 01 Desativar\n";
char atuador2Ativar[]="Atuador 02 Ativar\n";
char atuador2Desativar[]="Atuador 02 Desativar\n";
char atuador3Ativar[]="Atuador 03 Ativar\n";
char atuador3Desativar[]="Atuador 03 Desativar\n";
char atuador4Ativar[]="Atuador 04 Ativar\n";
char atuador4Desativar[]="Atuador 04 Desativar\n";

int tempo = 2000;





void setup() {
 Serial.begin(9600); /* begin serial with 9600 baud */
 SPI.begin();  /* begin SPI */

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG
    Serial.println("Conectando ao WiFi..");
    #endif
  }
  #ifdef DEBUG
  Serial.println("Conectado na rede WiFi");
  #endif
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.println("Conectando ao Broker MQTT...");
    #endif
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      #ifdef DEBUG
      Serial.println("Conectado");  
      #endif
 
    } else {
      #ifdef DEBUG 
      Serial.print("falha estado  ");
      Serial.print(client.state());
      #endif
      delay(2000);
 
    }
  }
 
  //subscreve no tópico
  client.subscribe(mqttTopicSubA1); 
  client.subscribe(mqttTopicSubA2); 
  client.subscribe(mqttTopicSubA3); 
  client.subscribe(mqttTopicSubA4);   
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);
  String strTopic = topic;
  
  #ifdef DEBUG
  Serial.print("Mensagem chegou do tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem:");
  Serial.print(strMSG);
  Serial.println();
  Serial.println("-----------------------");
  #endif

  if(strTopic == "grow/A1"){
    if(strMSG == "1"){
       for(int i=0; i<sizeof atuador1Ativar; i++){  
          SPI.transfer(atuador1Ativar[i]);
       }      
    }else{
       for(int i=0; i<sizeof atuador1Desativar; i++){ 
          SPI.transfer(atuador1Desativar[i]);
       }
    }
  }else if(strTopic == "grow/A2"){
    if(strMSG == "1"){
       for(int i=0; i<sizeof atuador2Ativar; i++){  
          SPI.transfer(atuador2Ativar[i]);
       }      
    }else{
       for(int i=0; i<sizeof atuador2Desativar; i++){ 
          SPI.transfer(atuador2Desativar[i]);
       }
    }
  }else if(strTopic == "grow/A3"){
    if(strMSG == "1"){
       for(int i=0; i<sizeof atuador3Ativar; i++){  
          SPI.transfer(atuador3Ativar[i]);
       }      
    }else{
       for(int i=0; i<sizeof atuador3Desativar; i++){ 
          SPI.transfer(atuador3Desativar[i]);
       }
    }
  }else if(strTopic == "grow/A4"){
    if(strMSG == "1"){
       for(int i=0; i<sizeof atuador4Ativar; i++){  
          SPI.transfer(atuador4Ativar[i]);
       }      
    }else{
       for(int i=0; i<sizeof atuador4Desativar; i++){ 
          SPI.transfer(atuador4Desativar[i]);
       }
    }
  }

  delay(tempo);
}
 
//função pra reconectar ao servido MQTT
void reconect() {
  //Enquanto estiver desconectado
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Tentando conectar ao servidor MQTT");
    #endif
     
    bool conectado = strlen(mqttUser) > 0 ?
                     client.connect("ESP8266Client", mqttUser, mqttPassword) :
                     client.connect("ESP8266Client");
 
    if(conectado) {
      #ifdef DEBUG
      Serial.println("Conectado!");
      #endif
      //subscreve no tópico com nivel de qualidade QoS 1
      client.subscribe(mqttTopicSubA1, 1); 
      client.subscribe(mqttTopicSubA2, 1);
      client.subscribe(mqttTopicSubA3, 1);
      client.subscribe(mqttTopicSubA4, 1);      
    } else {
      #ifdef DEBUG
      Serial.println("Falha durante a conexão.Code: ");
      Serial.println( String(client.state()).c_str());
      Serial.println("Tentando novamente em 10 s");
      #endif
      //Aguarda 10 segundos 
      delay(10000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconect();
  }

  //envia a cada X segundos
  if ((millis() - ultimoEnvioMQTT) > INTERVALO_ENVIO){
    char s[100];
    char c;
    int cont = 0;
    //Leitura de bytes (caracteres ) porta da porta RX
    Serial.flush();
     while (Serial.available() > 0)  {
        c = Serial.read();
        s[cont] = c; //concatena os caracteres em um array de char
        if(s[cont] == '\n'){ // Quando encontrar o caracter \n, adiciona o caracter \0 ao final do array
          s[cont] = '\0';
          break;
        }
        cont++;
        delay(3);
        ultimoEnvioMQTT = millis();      
     }
      Serial.println(s);//Impressão do array de char
      Serial.flush();
      enviaMQTT(s);
  }
  
  client.loop();
}

void enviaMQTT(char msgMQTT[]){
  String msg_aux = msgMQTT;
  char msgMQTT_path[50];

  
  if(msg_aux.substring(0,21) == "grow/sensor/umidsoil/"){
    msg_aux = msg_aux.substring(24);
    
    float valorMsg = msg_aux.toFloat();
    valorMsg = 1023 - msg_aux.toFloat();

    strncpy(msgMQTT_path, msgMQTT, 24);
    msgMQTT_path[24] = '\0'; 
    
    sprintf(msgMQTT,"%f",valorMsg); 
    client.publish(msgMQTT_path, msgMQTT);
  }else if(msg_aux.substring(0,21) == "grow/sensor/humidade/"){
    msg_aux = msg_aux.substring(24);
    
    float valorMsg = msg_aux.toFloat();

    strncpy(msgMQTT_path, msgMQTT, 24);
    msgMQTT_path[24] = '\0'; 
    
    sprintf(msgMQTT,"%f",valorMsg); 
    client.publish(msgMQTT_path, msgMQTT);
  }else if(msg_aux.substring(0,24) == "grow/sensor/temperatura/"){
    msg_aux = msg_aux.substring(27);
    
    float valorMsg = msg_aux.toFloat();

    strncpy(msgMQTT_path, msgMQTT, 27);
    msgMQTT_path[27] = '\0'; 
    
    sprintf(msgMQTT,"%f",valorMsg); 
    client.publish(msgMQTT_path, msgMQTT);    
  }else if(msg_aux.substring(0,18) == "grow/sensor/lumen/"){
    msg_aux = msg_aux.substring(21);
    
    float valorMsg = msg_aux.toFloat();

    strncpy(msgMQTT_path, msgMQTT, 21);
    msgMQTT_path[21] = '\0'; 
            
    sprintf(msgMQTT,"%f",valorMsg); 
    client.publish(msgMQTT_path, msgMQTT);      
  }else if(msg_aux.substring(0,15) == "grow/sensor/pH/"){
    msg_aux = msg_aux.substring(18);
    
    float valorMsg = msg_aux.toFloat();

    strncpy(msgMQTT_path, msgMQTT, 18);
    msgMQTT_path[18] = '\0'; 
    
    sprintf(msgMQTT,"%f",valorMsg); 
    client.publish(msgMQTT_path, msgMQTT);          
  }
}

