// Arduino data logger with SD card and DHT11 humidity and temperature sensor
//https://simple-circuit.com/sd-card-arduino-temperature-data-logger/
 
#include <SPI.h>        // Include SPI library (needed for the SD card)
#include <SD.h>         // Include SD library
#include <DHT.h>        // Include DHT sensor library
#include <EtherCard.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define STATIC_ADDR 0  // DHCP MODE , 1 => Static
 
#define DHTPIN 3            // DHT11 data pin is connected to Arduino pin 4
#define DHTTYPE DHT11       // DHT11 sensor is used

#if STATIC_ADDR
static byte myip[] = { 192,168,0,35 };
static byte gwip[] = { 192,168,0,1 };
#endif

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const char website[] PROGMEM = "192.168.0.14";
byte Ethernet::buffer[700];

int ETH_CS_PIN = 53;
int SD_CS_PIN = 45;
static uint32_t timer;
static uint32_t sdTimer;

DHT dht(DHTPIN, DHTTYPE);   // Initialize DHT library
File dataFile;

SemaphoreHandle_t SDAccessLock; 
SemaphoreHandle_t ExecuteSemaphore; 
TaskHandle_t DHT_TaskHandle;
TaskHandle_t ETH_TaskHandle;

void ConnectToSD();
void ConnectToEth();
 
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
    ; // wait for serial port to connect. Needed for native USB port only
  Serial.print("Initializing SD card...");
  
  ConnectToSD();
  ConnectToEth();

  SDAccessLock = xSemaphoreCreateBinary(); 
  ExecuteSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(SDAccessLock);
  xSemaphoreGive(ExecuteSemaphore);
      Serial.println("LOck given setup");

 xTaskCreate(ETHCommTask,"ETH_Task",1024,NULL,1,&DHT_TaskHandle); 
 xTaskCreate(DHTCommTask,"DHT_Task",248,NULL,1,&ETH_TaskHandle); 
  delay(50);
}
 
uint16_t line = 1;
 
void loop() {
}


static void HandleGETResponse (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void ETHCommTask(void *pvParameters)
{ 
  while(1)
  {
    xSemaphoreTake(ExecuteSemaphore,portMAX_DELAY);
        Serial.print("0");
            ether.browseUrl(PSTR("/online"), "OK", website, NULL); // Send OK Status to server
        if(ether.packetLoop(ether.packetReceive())) {// If a request arrives
            Serial.print("ETH Data revieved");
            xSemaphoreTake(SDAccessLock,portMAX_DELAY);
            
            Serial.print("ETH Took Lock");
              char reply[] = "RANDOM SD DATA";
              memcpy_P(ether.tcpOffset(), reply, sizeof reply);
              ether.httpServerReply(sizeof reply - 1);
              
              xSemaphoreGive(SDAccessLock); 
            Serial.println("LOck given eth");
          }
    xSemaphoreGive(ExecuteSemaphore);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void DHTCommTask(void *pvParameters)
{ 
  while(1)
  {
    xSemaphoreTake(ExecuteSemaphore,portMAX_DELAY);
        Serial.print("1"); 
    // Read humidity
    byte RH = dht.readHumidity();
    //Read temperature in degree Celsius
    byte Temp = dht.readTemperature();
    xSemaphoreTake(SDAccessLock,portMAX_DELAY);
    dataFile = SD.open("DHT11Log.txt", FILE_WRITE);
    if(dataFile){
      dataFile.print(":Temp = ");
      dataFile.print(Temp);
      dataFile.print("°C,Vlaga = ");
      dataFile.print(RH);
      dataFile.println("%");
    Serial.println("Printed to SD");
    }
    dataFile.close();
    xSemaphoreGive(SDAccessLock);
    Serial.println("LOck given dht");
    xSemaphoreGive(ExecuteSemaphore);
    vTaskDelay(750 / portTICK_PERIOD_MS); 
  }
}

void ConnectToEth(){
  
  if (ether.begin(sizeof Ethernet::buffer, mymac, ETH_CS_PIN) == 0){
    Serial.println("Failed to access Ethernet controller");
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
  
  byte hisip[] = { 192,168,0,14 };
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
}

void ConnectToSD(){
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println( "ETH Setup starting");
}
