#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <RTClib.h>
#include <StringSplitter.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>


#define DEBUG

WiFiClient espClient;
PubSubClient client(espClient);
RTC_DS1307 RTC;
File myFile;
Sd2Card card;
SdVolume volume;
SdFile root;

int tempo = 2000;
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};
int Year, Month, Day, Hour, Minute, Second;
char buff [512];
String inputSerialString;
bool serialPermission = true;

StringSplitter *split;

struct wifi{
  char* ssid;
  char* password;
};

struct server_broker{
  char* serverName;
  char* serverUser;
  char* serverPassword;
  uint16_t serverPort;  
};

struct date_time{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
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
struct server_broker broker;
struct global_helper helper;
struct mqtt_publish pub;
struct mqtt_subscriber subs;
struct split_topic st;
struct date_time dt;

String ler_linha(int num_linha, char* arquivo) {
  String linha = "";
  myFile = SD.open(arquivo);
  if (myFile) {
    for (int i = 0; i < num_linha; i++) {
      while (myFile.read() != '\n') {
        ;
      }
    }
    while (myFile.peek() != '\n') {
      linha += char(myFile.read());
    }
  }
  myFile.close();
  split = new StringSplitter(linha, '|', 2);
  linha = split->getItemAtIndex(1);
  return linha;
}

void setWifi(String ssid, String pass){

  if (wifi_information.ssid != 0) {
    delete [] wifi_information.ssid;
  }
  if (wifi_information.password != 0) {
    delete [] wifi_information.password;
  }

  wifi_information.ssid = new char[ssid.length()];
  ssid.toCharArray(wifi_information.ssid,ssid.length());

  wifi_information.password = new char[pass.length()];
  pass.toCharArray(wifi_information.password,pass.length());  
}

void setInitWifi(){
  String ssid = ler_linha(0, "wifi.txt");
  String pass = ler_linha(1, "wifi.txt");
  setWifi(ssid, pass);
}

void setServerBroker(String serverName, String serverUser, String serverPassword, uint16_t serverPort){
    if (broker.serverName != 0) {
      delete [] broker.serverName;
    }
    if (broker.serverUser != 0) {
      delete [] broker.serverUser;
    }
    if (broker.serverPassword != 0) {
      delete [] broker.serverPassword;
    }

    broker.serverName = new char[serverName.length()];
    serverName.toCharArray(broker.serverName,serverName.length());    

    broker.serverUser = new char[serverUser.length()];
    serverUser.toCharArray(broker.serverUser,serverUser.length());

    broker.serverPassword = new char[serverPassword.length()];
    serverPassword.toCharArray(broker.serverPassword,serverPassword.length());      

    broker.serverPort = serverPort;
}

void setInitServerBroker(){
  String serverName = ler_linha(0, "broker.txt");
  String serverUser = ler_linha(1, "broker.txt");
  String serverPassword = ler_linha(2, "broker.txt");
  String serverPort = ler_linha(3, "broker.txt");

  setServerBroker(serverName, serverUser, serverPassword, strtoul(serverPort.c_str(), NULL, 0));
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

  client.setServer(broker.serverName, broker.serverPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.println("Conectando ao Broker MQTT...");
    #endif
    
    if (client.connect("ESP8266Client", broker.serverUser, broker.serverPassword )) {
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
    //Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(2020, 1, 11, 11, 16, 0));
  }
}

void reconect() {
  //Enquanto estiver desconectado
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Tentando conectar ao servidor MQTT");
    #endif
     
    bool conectado = strlen(broker.serverUser) > 0 ?
                     client.connect("ESP8266Client", broker.serverUser, broker.serverPassword) :
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

void initSDCard(){

  Serial.print("\nInicializando SD card...");

  if ( !SD.begin() ){
    Serial.println("falha na inicialização do cartão!");
  }
  Serial.println("Inicialização do cartão concluída.");

  if (SD.exists("log.txt")) {
    Serial.println("O arquivo log.txt existe no cartão.");
  } else {
    Serial.println("Criando arquivo log.txt...");
    myFile = SD.open("log.txt", FILE_WRITE);
    myFile.close();
  }

  if (SD.exists("wifi.txt")) {
    Serial.println("O arquivo wifi.txt existe no cartão.");
  } else {
    Serial.println("Criando arquivo wifi.txt...");
    myFile = SD.open("wifi.txt", FILE_WRITE);
    if (myFile) {
      myFile.println("ssid|");
      myFile.println("pass|");
      myFile.close();
      Serial.println("Arquivo wifi.txt criado.");
    } else {
      Serial.println("Erro ao abrir wifi.txt");
    }
    myFile.close();
  }

  if (SD.exists("broker.txt")) {
    Serial.println("O arquivo broker.txt existe no cartão.");
  } else {
    Serial.println("Criando arquivo broker.txt...");
    myFile = SD.open("broker.txt", FILE_WRITE);
    if (myFile) {
      myFile.println("serverName|");
      myFile.println("serverUser|");
      myFile.println("serverPassword|");
      myFile.println("serverPort|");
      myFile.close();
      Serial.println("Arquivo broker.txt criado.");
    } else {
      Serial.println("Erro ao abrir broker.txt");
    }
    myFile.close();    
  }

  if (SD.exists("topicos.txt")) {
    Serial.println("O arquivo topicos.txt existe no cartão.");
  } else {
    Serial.println("Criando arquivo topicos.txt...");
    myFile = SD.open("topicos.txt", FILE_WRITE);
    if (myFile) {
      myFile.println("sensor|");
      myFile.println("atuador|");
      myFile.close();
      Serial.println("Arquivo topicos.txt criado.");
    } else {
      Serial.println("Erro ao abrir topicos.txt");
    }
    myFile.close();
  }
  

  if (SD.exists("config.txt")) {
    Serial.println("O arquivo config.txt existe no cartão.");
  } else {
    Serial.println("Criando arquivo config.txt...");
    myFile = SD.open("config.txt", FILE_WRITE);
    if (myFile) {
      myFile.println("teste|");
      myFile.close();
      Serial.println("Arquivo config.txt criado.");
    } else {
      Serial.println("Erro ao abrir config.txt");
    }
    myFile.close();
  }

}

void initConfig(){
  initSDCard();
  cardInformation();  
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

void setDateTime(String dateTime){
  sscanf(dateTime.c_str(), "%d-%d-%d %d:%d:%d", &Year, &Month, &Day, &Hour, &Minute, &Second);
}

void setDateTime(int data,int pos){

  switch(pos){
    case 0:
      dt.year = data;  
      break;
    case 1:
      dt.month = data;    
      break;
    case 2:
      dt.day = data;
      break;
    case 3:
      dt.hour = data;
      break;
    case 4:
      dt.minute = data;
      break;  
    case 5:
      dt.second = data;    
      break;
  }
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
  serialPrintJson(doc, serialPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true);
  //client.publish(topic, String(humidSoil).c_str(), true);
}

void sendHumid(DynamicJsonDocument doc, char* topic){
  float humid = random(15, 20);
  doc["payload"] = humid;
  serialPrintJson(doc, serialPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true);  
  //client.publish(pub.sensor2, String(humid).c_str(), true);
}

void sendTemp(DynamicJsonDocument doc, char* topic){
  float temp = random(25, 31);
  doc["payload"] = temp;
  serialPrintJson(doc, serialPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true);  
  //client.publish(pub.sensor3, String(temp).c_str(), true);
}

void sendLumen(DynamicJsonDocument doc, char* topic){
  float lumen = random(0, 65000);
  doc["payload"] = lumen;
  serialPrintJson(doc, serialPermission);
  serializeJson(doc, buff);
  client.publish(topic, buff, true); 
  //client.publish(pub.sensor4, String(lumen).c_str(), true);
}

void sendpH(DynamicJsonDocument doc, char* topic){
  float pH = random(5, 7);
  doc["payload"] = pH;  
  serialPrintJson(doc, serialPermission);
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

void cardInformation(){

  if (!card.init()) {
    Serial.println("Falha na inicialização.");
    Serial.println("* O cartão foi inserido?");
    return;
  } else {
    Serial.println("As conexões estão corretas e o cartão esta presente.");
  }
  
  // Immprime o tipo do cartão
  Serial.print("\nTipo do cartão: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Desconhecido");
  }

  if (!volume.init(card)) {
    Serial.println("Não foi encontrado nenhum cartão com partição FAT16/FAT32.\n Confira se o cartão foi formatado adequadamente.");
    return;
  }  

  Serial.println("\nArquivos encontrados no cartão (Nome, data e tamanho em bytes): ");
  root.openRoot(volume);
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

void menuSerial(){
  Serial.println("1 - Exibe os dados na porta serial");  
  Serial.println("2 - Para de exibir os dados na porta serial");
  Serial.println("3 - Exibe informações do SD Card");
  Serial.println("4 - Exibe Data e Hora atual");
  Serial.println("5 - Seta Data e Hora");

  Serial.println("9 - Exibe menu");  
}


String serialEvent() {
  char inChar;
  
  while (Serial.available()) {
    inChar = (char)Serial.read();
  }

  return (String)inChar;
}

void refreshDateTime(){
  
}

void switchSerialEvents(){
  String event = serialEvent();
  //Serial.println(event);
  
  if(event == "1"){
  //Exibe os dados na porta serial
    serialPermission = true;
  }else if(event == "2"){
  //Para de exibir os dados na porta serial
    serialPermission = false;
  }else if(event == "3"){
  //Exibe informações do SD Card      
    cardInformation();
    serialPermission = false;
    }else if(event == "5"){
  //Exibe Data e Hora atual do dispositivo
  Serial.println(dateTimeNow());
  serialPermission = false;
  }else if(event == "9"){
    menuSerial();
    serialPermission = false;
  }
}
  
void loop() {
  switchSerialEvents();
  
  if (!client.connected()) {
    reconect();
  }

  if ((millis() - helper.lastMQTTSend) > helper.dispatchInterval){
    sendMQTT();
    helper.lastMQTTSend = millis();
  }

  client.loop();  
}

byte decToBcd(byte val){
 // Convert normal decimal numbers to binary coded decimal
 return ( (val/10*16) + (val%10) );
}

//void freeSpaceDT(){
//  if (dt.year != 0 || dt.month != 0|| dt.day != 0 || dt.hour!= 0 || dt.minute != 0 || dt.second != 0) {
//    delete [] dt.year;
//    delete [] dt.month;
//    delete [] dt.day;
//    delete [] dt.hour;
//    delete [] dt.minute;
//    delete [] dt.second;
//  }
//}

