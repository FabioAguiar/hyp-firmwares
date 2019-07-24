#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <StringSplitter.h>
#include <SPI.h>

#define DEBUG
WiFiClient espClient;
PubSubClient client(espClient);

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
  int lastMQTTSend;
};

struct mqtt_subscriber{
  const char* actuator1;
  const char* actuator2;
  const char* actuator3;
  const char* actuator4;    
  //char* dateMillisTime;
};

struct wifi wifi_information;
struct broker_mqtt server;
struct global_helper helper;
struct mqtt_subscriber subs;

void setInitWifi(){
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
  helper = (global_helper) {2000, 0};
}

void setGlobalHelper(int dispatchInterval, int lastMQTTSend){
  helper = (global_helper) {dispatchInterval, lastMQTTSend};
}

void setInitMQTTSubscriber(){
  subs = (mqtt_subscriber) {"grow/a1/4", "grow/a2/5", "grow/a3/6", "grow/a4/7"};
}

void setMQTTSubscriber(char* actuator1, char* actuator2, char* actuator3, char* actuator4){
  subs = (mqtt_subscriber) {actuator1, actuator2, actuator3, actuator4};
}

void callback(char* topic, byte* payload, unsigned int length) {
  pinMode(LED_BUILTIN, OUTPUT); 

  payload[length] = '\0';
  String topicMsg = String(topic) + '/' + String((char*)payload) + '\r';

  char c;
  digitalWrite(SS, LOW);
  for (const char * p = topicMsg.c_str() ; c = *p; p++){
    SPI.transfer (c);
    Serial.print(c);
  }
  digitalWrite(SS, HIGH);
  delay(2000);

  if(int((char*)payload)){
    digitalWrite(LED_BUILTIN, LOW);
  }else{
    digitalWrite(LED_BUILTIN, HIGH);    
  }
  
  
  #ifdef DEBUG
    Serial.print("Mensagem chegou do tópico: ");
    Serial.println(topic);
    Serial.print("Mensagem:");
    Serial.print(String((char*)payload));
    Serial.println();
    Serial.println("-----------------------");
  #endif

  delay(helper.dispatchInterval);
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
  setInitMQTTSubscriber();
  client.subscribe(subs.actuator1); 
  client.subscribe(subs.actuator2);
  client.subscribe(subs.actuator3);
  client.subscribe(subs.actuator4);
}

void setup() {
  Serial.begin(115200);
  digitalWrite(SS, HIGH); // Desabilita Slave Select
  SPI.begin ();
  SPI.setClockDivider(SPI_CLOCK_DIV8);// Divide clock por 8
  
  initConfig();
}

void sendMQTT(char msgMQTT[]){
  StringSplitter *splitter = new StringSplitter(msgMQTT, '/', 5);
  String topic = splitter->getItemAtIndex(0) + '/' + splitter->getItemAtIndex(1) + '/' + splitter->getItemAtIndex(2) + '/' + splitter->getItemAtIndex(3) + '/';
  float msg = splitter->getItemAtIndex(4).toFloat();  
  
  if(splitter->getItemAtIndex(2).equals("umidsoil")){
      msg = 1023 - msg;
  }

  client.publish(String(topic).c_str(), String(msg).c_str(), true);
}

void readSerialPort(){
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
    }
    //Serial.println(s);//Impressão do array de char
    Serial.flush();
    sendMQTT(s);
}

void loop() {
  if (!client.connected()) {
    reconect();
  }

  if ((millis() - helper.lastMQTTSend) > helper.dispatchInterval){
    readSerialPort();
    helper.lastMQTTSend = millis();
  }

   client.loop();
}
