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

DHT dht(DHTPIN, DHTTYPE);
File dataFile;

SemaphoreHandle_t SDAccessLock; 
SemaphoreHandle_t ExecuteSemaphore; 
TaskHandle_t DHT_TaskHandle;
TaskHandle_t ETH_TaskHandle;

void ConnectToSD();
void ConnectToEth();

static void HandleOnlineGET (byte status, word off, word len) {
    Ethernet::buffer[off+300] = 0;
    String data = (const char*) Ethernet::buffer + off;
    Serial.println(data);
    if (data.indexOf("Status - OK") > 0)  {
      HttpOkResponse = 1;
  }
}

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
    
    xTaskCreate(ETHCommTask,"ETH_Task",1024,NULL,1,&DHT_TaskHandle); 
    xTaskCreate(DHTCommTask,"DHT_Task",1024,NULL,1,&ETH_TaskHandle); 
    delay(50);
}
 
 
void loop() {
}

void ETHCommTask(void *pvParameters){ 
  while(1) {
    // Take exec semaphore
    xSemaphoreTake(ExecuteSemaphore,portMAX_DELAY);
    
    ether.packetLoop(ether.packetReceive());
    if (millis() > timer) {
      timer = millis() + 1000;
      Serial.println();
      if(DHT_DATA != ""){
        // Write data to URL and send via TCP, handle response via callback
        ether.browseUrl(PSTR("/data"), DHT_DATA.c_str(), website, HandleOnlineGET);
      }
    }
       
    xSemaphoreGive(ExecuteSemaphore);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void DHTCommTask(void *pvParameters)
{ 
  while(1) {
    xSemaphoreTake(ExecuteSemaphore,portMAX_DELAY);
    
    byte RH = dht.readHumidity();
    byte Temp = dht.readTemperature();
    
    xSemaphoreTake(SDAccessLock,portMAX_DELAY);
    dataFile = SD.open("DHT11Log.txt", FILE_WRITE);
    
    if(dataFile){
      DHT_DATA = "";
      dataFile.print(":Temp = ");
      dataFile.print(Temp);
      dataFile.print("°C,Vlaga = ");
      dataFile.print(RH);
      dataFile.println("%");
      //write query params for server
      DHT_DATA = (String)"?temp=" + (String)Temp + (String)"&humid=" + (String)RH;
      Serial.println(DHT_DATA);
    }
    
    dataFile.close();
    xSemaphoreGive(SDAccessLock); 
    xSemaphoreGive(ExecuteSemaphore);
    
    vTaskDelay(750 / portTICK_PERIOD_MS); 
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

  if (!ether.dnsLookup(website)){
    Serial.println("DNS failed, setting it manually");
  }

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
