// Arduino data logger with SD card and DHT11 humidity and temperature sensor
//https://simple-circuit.com/sd-card-arduino-temperature-data-logger/
 
#include <SPI.h>        
#include <SD.h>         
#include <DHT.h>       
#include <EtherCard.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define STATIC_ADDR 0  // DHCP MODE , 1 => Static
#define DEBUG_MODE 0

#define DHTPIN 25   
#define DHTTYPE DHT11 

static int RESPONSE_TYPE_DATA = 0;
static int RESPONSE_TYPE_HTML = 1;
static int RESPONSE_TYPE_NOT_FOUND = 2;
static int RESPONSE_TYPE_GET_SD_CARD = 3;

#if STATIC_ADDR
static byte myip[] = { 192,168,0,35 };
static byte gwip[] = { 192,168,0,1 };
#endif

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const char website[] PROGMEM = "cc-projecttest.ddns.net";
byte Ethernet::buffer[700];

int ETH_CS_PIN = 53;
int SD_CS_PIN = 45;

static uint32_t timer;
static uint32_t sdTimer;

int HttpOkResponse = 0;
String DHT_DATA = "";
int dhtdata[2] = {0,0};

DHT dht(DHTPIN, DHTTYPE);
File dataFile;

SemaphoreHandle_t SDAccessLock; 
SemaphoreHandle_t ExecuteSemaphore; 
TaskHandle_t DHT_TaskHandle;
TaskHandle_t ETH_TaskHandle;

void ConnectToSD();
void ConnectToEth();

void setup() {
    Serial.begin(9600);
    while (!Serial)
      ; 
    Serial.print("Initializing SD card...");
    
    ConnectToSD();
    ConnectToEth();
    
    SDAccessLock = xSemaphoreCreateBinary(); 
    ExecuteSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(SDAccessLock);
    xSemaphoreGive(ExecuteSemaphore);
    Serial.println("Mutex locks given out");
    
    xTaskCreate(ETHCommTask,"ETH_Task",1024,NULL,1,&ETH_TaskHandle); 
    xTaskCreate(DHTCommTask,"DHT_Task",512,NULL,1,&DHT_TaskHandle); 
    delay(50);
}

void HandleAndParse(int responseType){
  if(responseType == RESPONSE_TYPE_DATA){
    String responseHolder = "HTTP/1.0 200 OK\r\nContent-Type: application/json\r\n\r\n{\"temp\":" + (String)dhtdata[0] + ", \"humid\":" + (String)dhtdata[1] + "}";
    char payload[responseHolder.length()];
    for(int i = 0; i < responseHolder.length(); i++){
      payload[i] = responseHolder[i];
    }
    memcpy(ether.tcpOffset(), payload, sizeof payload);
    ether.httpServerReply(sizeof payload);
  } else if(responseType == RESPONSE_TYPE_HTML) {
    String responseHolder = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>DHT Sensors</title></head><body><h3>Sensor states : </h3><br><h5>Temperature : " +  (String)dhtdata[0] +"</h5><br><h5>Humidity : " + (String)dhtdata[1] +"</h5></body></html>";
    char payload[responseHolder.length()];
    for(int i = 0; i < responseHolder.length(); i++){
      payload[i] = responseHolder[i];
    } memcpy(ether.tcpOffset(), payload, sizeof payload);
    ether.httpServerReply(sizeof payload);
  } else if (responseType == RESPONSE_TYPE_GET_SD_CARD){
    String responseHolder = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>DHT Sensors - SD CARD</title></head><body><h3>Last 20 inputs in the SD Card : </h3>";
    xSemaphoreTake(SDAccessLock,portMAX_DELAY);
    dataFile = SD.open("DHT11Log.txt", FILE_READ);
    int lines = 0;
    String CardLine = "";
    if(dataFile){
      while(dataFile.available()){
        if(lines < 20) {
          responseHolder += "<br><h5>Temperature : " + (String)dataFile.read() ;
          if(dataFile.available()){
            responseHolder += ", Humidity : " + (String) dataFile.read() + + "</h5>";
          } else {
            responseHolder += + "</h5>";
          }
        } else if (lines > 20) break;
        lines++;
      }
    }
    responseHolder += "</body></html>";
    dataFile.close();
    xSemaphoreGive(SDAccessLock); 
       char payload[responseHolder.length()];
    for(int i = 0; i < responseHolder.length(); i++){
      payload[i] = responseHolder[i];
    } memcpy(ether.tcpOffset(), payload, sizeof payload);
    ether.httpServerReply(sizeof payload);
  }else {
    String responseHolder = "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><head><title>DHT Sensors</title></head><body><h5> Please  verify your endpoint</h5></body></html>";
    char payload[responseHolder.length()];
    for(int i = 0; i < responseHolder.length(); i++){
      payload[i] = responseHolder[i];
    } memcpy(ether.tcpOffset(), payload, sizeof payload);
    ether.httpServerReply(sizeof payload);
  }
}
 
void loop() {
}

void ETHCommTask(void *pvParameters){ 
  while(1) {
    xSemaphoreTake(ExecuteSemaphore,portMAX_DELAY);
    word len = ether.packetReceive();
    word pos = ether.packetLoop(len);     
    String dataString = "";
    if (pos > 0){  
      char* req = (char *) Ethernet::buffer + pos;        
      dataString = String(req);  
      if (len > 0) {                        
        Serial.println(dataString);
        if (dataString.indexOf("/data") > 0)  {
          HandleAndParse(RESPONSE_TYPE_DATA);
        } else if (dataString.indexOf("/web-view") > 0){
          HandleAndParse(RESPONSE_TYPE_HTML);
        } else if (dataString.indexOf("/sd-card") > 0){
          HandleAndParse(RESPONSE_TYPE_GET_SD_CARD);
        } else {
          HandleAndParse(RESPONSE_TYPE_NOT_FOUND);
        }
      }     
    };       
    xSemaphoreGive(ExecuteSemaphore);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void DHTCommTask(void *pvParameters)
{ 
  while(1) {
    xSemaphoreTake(ExecuteSemaphore,portMAX_DELAY);
    
    byte RH = dht.readHumidity();
    byte Temp = dht.readTemperature();
    dhtdata[0] = RH;
    dhtdata[1] = Temp;
    
    xSemaphoreTake(SDAccessLock,portMAX_DELAY);
    dataFile = SD.open("DHT11Log.txt", FILE_WRITE);
    
    if(dataFile){
      DHT_DATA = "";
      dataFile.print(":Temp = ");
      dataFile.print(Temp);
      dataFile.print("°C,Vlaga = ");
      dataFile.print(RH);
      dataFile.println("%");
      DHT_DATA = (String)"?temp=" + (String)Temp + (String)"&humid=" + (String)RH;
    }
    dataFile.close();
    xSemaphoreGive(SDAccessLock); 
    xSemaphoreGive(ExecuteSemaphore);
    
    vTaskDelay(150 / portTICK_PERIOD_MS); 
  }
}

void ConnectToEth(){
  
  if (ether.begin(sizeof Ethernet::buffer, mymac, ETH_CS_PIN) == 0){
    Serial.println("Failed to access Ethernet controller - buggy");
  } else{
     Serial.println("Ethernet OK");
  }
   
  #if STATIC_ADDR
    ether.staticSetup(myip, gwip);
    Serial.println( "Ethernet OK");
  #else
    if (!ether.dhcpSetup())
      Serial.println("DHCP failed");
    Serial.println("DHCP success");
  #endif

//  if (!ether.dnsLookup(website)){
//    Serial.println("DNS failed, setting it manually");
//  }

  // Using local adresses while debugging
  #if DEBUG_MODE
  byte hisip[] = { 192,168,0,27 };
  ether.copyIp(ether.hisip, hisip);
  ether.hisport = 8000;
  
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
  ether.printIp("SRV: ", ether.hisip);
    Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
    }
    Serial.println( ""); 
  #endif
}

void ConnectToSD(){
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println( "ETH Setup starting");
}
