#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <RTClib.h>
#include <StringSplitter.h>
#include <ArduinoJson.h>

#define DEBUG

WiFiClient espClient;
PubSubClient client(espClient);
RTC_DS1307 RTC;

int tempo = 2000;
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};
char buff [512];
bool btPermission = true;

StringSplitter *splitter;

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
  char* sensor4;
  char* sensor5;
};

struct mqtt_subscriber{
  const char* actuator1;
  const char* actuator2;
  const char* actuator3;
  const char* actuator4;    
  //char* dateMillisTime;
};

struct split_topic{
  StringSplitter *splitTopicSensor1;
  StringSplitter *splitTopicSensor2;
  StringSplitter *splitTopicSensor3;
  StringSplitter *splitTopicSensor4;
  StringSplitter *splitTopicSensor5;
  StringSplitter *splitTopicActuator1;
  StringSplitter *splitTopicActuator2;
  StringSplitter *splitTopicActuator3;
  StringSplitter *splitTopicActuator4;    
};

struct wifi wifi_information;
struct broker_mqtt server;
struct global_helper helper;
struct mqtt_publish pub;
struct mqtt_subscriber subs;
struct split_topic st;

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
  subs = (mqtt_subscriber) {"grow/A1", "grow/A2", "grow/A3", "grow/A4"};
  st.splitTopicActuator1 = new StringSplitter(subs.actuator1, '/', 1);
  st.splitTopicActuator2 = new StringSplitter(subs.actuator2, '/', 1);
  st.splitTopicActuator3 = new StringSplitter(subs.actuator3, '/', 1);
  st.splitTopicActuator4 = new StringSplitter(subs.actuator4, '/', 1);
}

void setMQTTSubscriber(char* actuator1, char* actuator2, char* actuator3, char* actuator4){
  subs = (mqtt_subscriber) {actuator1, actuator2, actuator3, actuator4};
}

void setInitMQTTPublish(){
  pub = (mqtt_publish) {"grow/sensor/d1/s1/FC28", "grow/sensor/d1/s2/DHT11", "grow/sensor/d1/s3/DHT11", "grow/sensor/d1/s4/BH1750", "grow/sensor/d1/s5/pH"};
  st.splitTopicSensor1 = new StringSplitter(pub.sensor1, '/', 5);
  st.splitTopicSensor2 = new StringSplitter(pub.sensor2, '/', 5);
  st.splitTopicSensor3 = new StringSplitter(pub.sensor3, '/', 5);
  st.splitTopicSensor4 = new StringSplitter(pub.sensor4, '/', 5);
  st.splitTopicSensor5 = new StringSplitter(pub.sensor5, '/', 5);
}

void setMQTTPublish(char* sensor1, char* sensor2, char* sensor3, char* sensor4){
  pub = (mqtt_publish) {sensor1, sensor2, sensor3, sensor4};
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);
  String strTopic = topic;
  String subsActuator1(subs.actuator1);
  String subsActuator2(subs.actuator2);
  String subsActuator3(subs.actuator3);
  String subsActuator4(subs.actuator4);
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
      digitalWrite(LED_BUILTIN, LOW);
    }else{
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }else if(strTopic == subsActuator2){
    if(strMSG == "1"){
    // ligar atuador 2
    }else{
    // desligar atuador 2
    }
  }else if(strTopic == subsActuator3){
    if(strMSG == "1"){
    // ligar atuador 3
    }else{
    // desligar atuador 3
    }
  }else if(strTopic == subsActuator4){
    if(strMSG == "1"){
    // ligar atuador 4
    }else{
    // desligar atuador 4
    }
  }

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


void initWire(){
    Wire.begin();  
}

void initRTC(){
    RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    //RTC.adjust(DateTime(2020, 1, 7, 14, 46, 0));
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
      client.subscribe(subs.actuator2, 1);
      client.subscribe(subs.actuator3, 1);
      client.subscribe(subs.actuator4, 1);
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
  client.subscribe(subs.actuator2);
  client.subscribe(subs.actuator3);
  client.subscribe(subs.actuator4);
  initWire();
  initRTC();
}

void setup() {
  Serial.begin(9600);
  initConfig();
}

String dateTimeNow(){
    DateTime now = RTC.now(); 
    String dtn;
    dtn += now.year();
    dtn += '-';
    dtn += now.month();
    dtn += '-';
    dtn += now.day();
    dtn += ' ';
    dtn +=now.hour();
    dtn += ':';
    dtn +=now.minute();
    dtn += ':';
    dtn +=now.second();
    return dtn;
}

void serialPrintJson(DynamicJsonDocument doc, bool permission){
  if(permission){
    serializeJson(doc, Serial);
    Serial.println("");    
  }
}

DynamicJsonDocument docJSON(StringSplitter* topicSensor){
  DynamicJsonDocument doc(1024);
  doc["s_id"] = topicSensor->getItemAtIndex(3);
  doc["sensor"] = topicSensor->getItemAtIndex(4);
  doc["d_id"] = topicSensor->getItemAtIndex(2);
  doc["dateTime"] = dateTimeNow();

  return doc;
}

void sendHumidSoil(DynamicJsonDocument doc, char* topic){
  float humidSoil = random(0, 1024);
  doc["payload"] = humidSoil;
  serialPrintJson(doc, btPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true);
  //client.publish(topic, String(humidSoil).c_str(), true);
}

void sendHumid(DynamicJsonDocument doc, char* topic){
  float humid = random(15, 20);
  doc["payload"] = humid;
  serialPrintJson(doc, btPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true);  
  //client.publish(pub.sensor2, String(humid).c_str(), true);
}

void sendTemp(DynamicJsonDocument doc, char* topic){
  float temp = random(25, 31);
  doc["payload"] = temp;
  serialPrintJson(doc, btPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true);  
  //client.publish(pub.sensor3, String(temp).c_str(), true);
}

void sendLumen(DynamicJsonDocument doc, char* topic){
  float lumen = random(0, 65000);
  doc["payload"] = lumen;
  serialPrintJson(doc, btPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true); 
  //client.publish(pub.sensor4, String(lumen).c_str(), true);
}

void sendpH(DynamicJsonDocument doc, char* topic){
  float pH = random(5, 7);
  doc["payload"] = pH;  
  serialPrintJson(doc, btPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true); 
  //client.publish(pub.sensor5, String(pH).c_str(), true);
}

void sendSensor1(StringSplitter* topicSplit){
  sendHumidSoil(docJSON(topicSplit), pub.sensor1);
}

void sendSensor2(StringSplitter* topicSplit){
  sendHumid(docJSON(topicSplit), pub.sensor2);
}

void sendSensor3(StringSplitter* topicSplit){
  sendTemp(docJSON(topicSplit), pub.sensor3);
}

void sendSensor4(StringSplitter* topicSplit){
  sendLumen(docJSON(topicSplit), pub.sensor4);
}

void sendSensor5(StringSplitter* topicSplit){
  sendpH(docJSON(topicSplit), pub.sensor5);
}

void sendMQTT(){
  sendSensor1(st.splitTopicSensor1);
  sendSensor2(st.splitTopicSensor2);
  sendSensor3(st.splitTopicSensor3);
  sendSensor4(st.splitTopicSensor4);
  sendSensor5(st.splitTopicSensor5);
}

void btRead(){
  if (Serial.available()){
    char data_received; 
    data_received = Serial.read();
    if (data_received == '1'){
      btPermission = true;
    }else if (data_received == '0'){
      btPermission = false;
    }
  }  
}


void loop() {

  btRead();
  
  if (!client.connected()) {
    reconect();
  }

  if ((millis() - helper.lastMQTTSend) > helper.dispatchInterval){
    sendMQTT();
    helper.lastMQTTSend = millis();
  }

  client.loop();  
}
