#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

#define DEBUG
WiFiClient espClient;
PubSubClient client(espClient);
int tempo = 2000;

DHTesp dht;

struct wifi{
  char* ssid;
  char* password;
};

struct broker_mqtt{
  char* mqttServer;
  char* mqttUser;
  char* mqttPassword;
  uint16_t mqttPort;
};

struct global_helper{
  int dispatchInterval;
  char* topicBase;
  int lastMQTTSend;
};

struct mqtt_publish{
  char* sensor1;
  char* sensor2;
  char* sensor3;
};

struct mqtt_subscriber{
  const char* actuator1;
};

struct wifi wifi_information;
struct broker_mqtt server;
struct global_helper helper;
struct mqtt_publish pub;
struct mqtt_subscriber subs;

void setInitWifi(){
  //wifi_information = (wifi) {"DEN_BIOMASSA", "BIO_DEN_2014+"};  
  wifi_information = (wifi) {"SUPERNET_FABYUU", "luci6666"};
  //wifi_information = (wifi) {"Ap-F12", "08774576"};
}

void setWifi(char* ssid, char* password){
  wifi_information = (wifi) {ssid, password};
}

void setInitServerBroker(){
  server = (broker_mqtt) {"m14.cloudmqtt.com", "cxhovvsp", "v-nT9GcH_-Cr", 14167};
}

void setServerBroker(char* mqttServer, char* mqttUser, char* mqttPassword, uint16_t mqttPort){
  server = (broker_mqtt) {mqttServer, mqttUser, mqttPassword, mqttPort};
}

void setInitGlobalHelper(){
  helper = (global_helper) {2000, "grow/", 0};
}

void setGlobalHelper(int dispatchInterval, char* topicBase, int lastMQTTSend){
  helper = (global_helper) {dispatchInterval, topicBase, lastMQTTSend};
}

void setInitMQTTSubscriber(){
  subs = (mqtt_subscriber) {"grow/A1"};
}

void setMQTTSubscriber(char* actuator1){
  subs = (mqtt_subscriber) {actuator1};
}

void setInitMQTTPublish(){
  pub = (mqtt_publish) {"grow/sensor/humidsolo/pressostato/", "grow/sensor/humidade/DHT22/", "grow/sensor/temperatura/DHT22/"};
}

void setMQTTPublish(char* sensor1, char* sensor2, char* sensor3){
  pub = (mqtt_publish) {sensor1, sensor2, sensor3};
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);
  String strTopic = topic;
  String subsActuator1(subs.actuator1);
  pinMode(LED_BUILTIN, OUTPUT);

  
  #ifdef DEBUG
  Serial.print("Mensagem chegou do tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem:");
  Serial.print(strMSG);
  Serial.println();
  Serial.println("-----------------------");
  #endif


  if(strTopic == subsActuator1){
    if(strMSG == "1"){
      digitalWrite(D0, LOW);
    }else{
      digitalWrite(D0, HIGH);
    }
  }
//  else if(strTopic == subsActuator2){
//    if(strMSG == "1"){
//    // ligar atuador 2
//    }else{
//    // desligar atuador 2
//    }
//  }else if(strTopic == subsActuator3){
//    if(strMSG == "1"){
//    // ligar atuador 3
//    }else{
//    // desligar atuador 3
//    }
//  }else if(strTopic == subsActuator4){
//    if(strMSG == "1"){
//    // ligar atuador 4
//    }else{
//    // desligar atuador 4
//    }
//  }

  delay(tempo);
}


void initWifi(){

  WiFi.begin(wifi_information.ssid, wifi_information.password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG
    Serial.println("Conectando ao WiFi..");
    #endif
  }
  #ifdef DEBUG
  Serial.println("Conectado na rede WiFi");
  #endif

  client.setServer(server.mqttServer, server.mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.println("Conectando ao Broker MQTT...");
    #endif
 
    if (client.connect("ESP8266Client", server.mqttUser, server.mqttPassword )) {
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
}

void reconect() {
  //Enquanto estiver desconectado
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Tentando conectar ao servidor MQTT");
    #endif
     
    bool conectado = strlen(server.mqttUser) > 0 ?
                     client.connect("ESP8266Client", server.mqttUser, server.mqttPassword) :
                     client.connect("ESP8266Client");
 
    if(conectado) {
      #ifdef DEBUG
      Serial.println("Conectado!");
      #endif
      //subscreve no tópico com nivel de qualidade QoS 1
      client.subscribe(subs.actuator1, 1); 
//      client.subscribe(subs.actuator2, 1);
//      client.subscribe(subs.actuator3, 1);
//      client.subscribe(subs.actuator4, 1);
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

void initConfig(){
  setInitGlobalHelper();
  setInitWifi();
  setInitServerBroker();
  initWifi();
  setInitMQTTPublish();
  setInitMQTTSubscriber();
  client.subscribe(subs.actuator1); 
//  client.subscribe(subs.actuator2);
//  client.subscribe(subs.actuator3);
//  client.subscribe(subs.actuator4);
}

void setup() {
  Serial.begin(9600);
  initConfig();
  dht.setup(0, DHTesp::DHT22); // Connect DHT sensor to GPIO 0
}

void sendHumidSolo(){
  float umidSoil = random(0, 1);
  //client.publish(pub.sensor1, String(umidSoil).c_str(), true);
}

void sendHumid(){
  delay(dht.getMinimumSamplingPeriod());
  float humidade = dht.getHumidity();
  client.publish(pub.sensor2, String(humidade).c_str(), true);
}

void sendTemp(){
  delay(dht.getMinimumSamplingPeriod());
  float temperatura = dht.getTemperature();
  client.publish(pub.sensor3, String(temperatura).c_str(), true);
}



void sendMQTT(){
  sendHumidSolo();
  sendHumid();
  sendTemp();
}

void loop() {
  if (!client.connected()) {
    reconect();
  }

  if ((millis() - helper.lastMQTTSend) > helper.dispatchInterval){
    sendMQTT();
    helper.lastMQTTSend = millis();
  }

  client.loop();  
}
