/*
  Upload Data to WebService:
  Support Devices: LG01 
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Ethernet.h>
#include <String.h>
#include <SoftwareSerial.h>
#include "utility/socket.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <SimpleTimer.h>

// Timer
  SimpleTimer timer1; // timer richiesta stato

#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

RH_RF95 rf95(5, 2);

uint16_t crcdata = 0;
uint16_t recCRCData = 0;
float frequency = 868.0;
String dataString = "";

String readString, readString1;
int x=0;
char lf=10;
int T=0;

 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//indirizzo ip dell'Arduino
//IPAddress ip(172, 16, 113, 200);
//IPAddress gateway(172, 16, 113, 1);
//IPAddress subnet(255, 255, 255, 0);
IPAddress ip(192, 168, 2, 67);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress server(10, 162, 128, 65);
int port(8088);
//IPAddress server(192, 168, 2, 97);
//int port(80);
 
EthernetClient client;

int rxPin = 3;
int txPin = 2;

int tConnect = 0;
int timerConnect = 0;

SoftwareSerial bluetooth(rxPin, txPin);

void setup()
{
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(10,OUTPUT);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(10,HIGH);

  timer1.setInterval(60000, WatchDog);
  
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  if (!rf95.init())
      Serial.println("LoRa init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  
  Serial.println("LoRa Gateway 2 Web Service  --");

  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Configurazione DHCP fallita!");
   Ethernet.begin(mac, ip, gateway, gateway, subnet); 
  }
  else
  {
    Serial.println("Configurazione DHCP OK!");
  }
  Serial.println(Ethernet.localIP());
  Serial.println(server);
  Serial.println(port);
  delay(1000);
}

uint16_t calcByte(uint16_t crc, uint8_t b)
{
    uint32_t i;
    crc = crc ^ (uint32_t)b << 8;
  
    for ( i = 0; i < 8; i++)
    {
      if ((crc & 0x8000) == 0x8000)
        crc = crc << 1 ^ 0x1021;
      else
        crc = crc << 1;
    }
    return crc & 0xffff;
}

uint16_t CRC16(uint8_t *pBuffer, uint32_t length)
{
    uint16_t wCRC16 = 0;
    uint32_t i;
    if (( pBuffer == 0 ) || ( length == 0 ))
    {
        return 0;
    }
    for ( i = 0; i < length; i++)
    {
        wCRC16 = calcByte(wCRC16, pBuffer[i]);
    }
    return wCRC16;
}

uint16_t recdata( unsigned char* recbuf, int Length)
{
    crcdata = CRC16(recbuf, Length - 2); //Get CRC code
    recCRCData = recbuf[Length - 1]; //Calculate CRC Data
    recCRCData = recCRCData << 8; //
    recCRCData |= recbuf[Length - 2];
}

void(* Riavvia)(void) = 0;

void loop() {
  timer1.run();
  if (rf95.waitAvailableTimeout(2000))// Listen Data from LoRa Node
    {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
    uint8_t len = sizeof(buf);//data buffer length
    if (rf95.recv(buf, &len))//Check if there is incoming data
      {
        T=0;
        recdata( buf, len);
        Serial.print("Data length: ");
        Serial.println(len);
        Serial.print("Get LoRa Packet: ");
        for (int i = 0; i < len; i++)
        {
            Serial.print(buf[i],HEX);
            Serial.print(" ");
        }
        Serial.println();
        if(crcdata == recCRCData) //Check if CRC is correct
        { 
            if(buf[0] == 1||buf[1] == 0||buf[2] ==0) //Check if the ID match the LoRa Node ID 100
            {
                char data[] = "";//Reply 
                
                String client_id = "";
                String user_id = "";
                String state_id = "";
                
                for (int i = 3; i < 17; i++) {
                  client_id += char(buf[i]);
                }
                Serial.print("Client ID: ");
                Serial.println(client_id);

                if (len >=20) {
                  for (int i = 17; i < 31; i++) {
                    user_id += char(buf[i]);
                  }
                  Serial.print("User   ID: ");
                  Serial.println(user_id);

                  for (int i = 31; i < 32; i++) {
                    state_id += char(buf[i]);
                  }
                  Serial.print("State  ID: ");
                  Serial.println(state_id);

                  data[3] = uploadData(client_id, user_id, state_id);
                  Serial.print("Response: ");
                  for (int i = 0; i < 3; i++) {
                    data[i] = (buf[i]);
                    Serial.print(data[i], HEX);
                  }
                  Serial.print(data[3]);
                  Serial.println();
                  
                  rf95.send(data, 6);// Send Reply to LoRa Node
                  rf95.waitPacketSent();
                }
                else {
                  data[3] = requestState(client_id);
                  Serial.print("Response: ");
                  for (int i = 0; i < 3; i++) {
                    data[i] = (buf[i]);
                    Serial.print(data[i], HEX);
                  }
                  Serial.print(data[3]);
                  Serial.println();
                  
                  rf95.send(data, 6);// Send Reply to LoRa Node
                  rf95.waitPacketSent();
                }
                Serial.println("==================");
                Serial.println();
            }
        } 
        else 
          Serial.println(" CRC Fail");     
     }
     else
     {
          Serial.println("recv failed");
          ;
      }
  
  }
}

char requestState(String client_id) {
  char c;
  char cr;
  String upload_url = "GET /handleRequestBox.php?box=";
//  String upload_url = "GET /TagManager/handleRequestBox.php?box=";
  upload_url += client_id;
  upload_url += "&color=1 HTTP/1.1";
  Serial.print(server);
  Serial.print(":");
  Serial.print(port);
  Serial.print(" - ");
  Serial.println(upload_url);
  Serial.print("Connessione... ");
 
  if (client.connect(server, port))
  {
    Serial.println("- Connesso... ");
    //invio la richiesta al server
    client.println(upload_url);
    client.println("Host: localhost");
    client.println();
  } 
    else {
      Serial.println("connection failed"); //error message if no client connect
      Serial.println("Reboot Arduino");
      Serial.println();
      delay(1000);
      Reset_AVR();
    }
    bool stwr = false;
    while(client.connected() && !client.available()) delay(1); //waits for data
    while (client.connected() || client.available()) { //connected or data available
      c = client.read(); //gets byte from ethernet buffer
//      Serial.print(c);
      if (stwr == true) {
        if (c=='@') {
          break; 
        }
        cr=c;
        
      }
      if (c == '#') {
        stwr = true;
      }
    }
    Serial.println();
    Serial.println("disconnecting.");
    client.stop(); //stop client
    
    return cr;
}

char uploadData(String client_id, String user_id, String state_id) {//Upload Data to ThingSpeak
  char c;
  char cr;
  String upload_url = "GET /handleRequestBox.php?box=";
//  String upload_url = "GET /TagManager/handleRequestBox.php?box=";
  upload_url += client_id;
  upload_url += "&user=";
  upload_url += user_id;
  upload_url += "&act=";
  upload_url += state_id;  
  upload_url += " HTTP/1.1";
  Serial.println(upload_url);
  Serial.print("Connessione... ");
 
  if (client.connect(server, port))
  {
    Serial.println("- Connesso... ");
    //invio la richiesta al server
    client.println(upload_url);
    client.println("Host: localhost");
    client.println();
  } 
    else {
      Serial.println("connection failed"); //error message if no client connect
      Serial.println("Reboot Arduino");
      Serial.println();
      delay(1000);
      Reset_AVR();
    }
    bool stwr = false;
    while(client.connected() && !client.available()) delay(1); //waits for data
    while (client.connected() || client.available()) { //connected or data available
      c = client.read(); //gets byte from ethernet buffer
//      Serial.print(c);
      if (stwr == true) {
        if (c=='@') {
          break; 
        }
        cr=c;
        
      }
      if (c == '#') {
        stwr = true;
      }
    }
    Serial.println();
    Serial.println("disconnecting.");
    
    client.stop(); //stop client
    
    return cr;
}

void WatchDog() {
  T++;
  Serial.println(T);
  if (T==3) {
    Reset_AVR();
  }
}

