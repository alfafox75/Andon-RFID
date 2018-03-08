/*
  Upload Data to WebService:
  Support Devices: LG01 
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Ethernet.h>
#include <String.h>

RH_RF95 rf95(5, 2);

uint16_t crcdata = 0;
uint16_t recCRCData = 0;
float frequency = 868.0;
String dataString = "";


 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//indirizzo ip dell'Arduino
IPAddress ip(192, 168, 137, 200);
IPAddress server(10, 162, 128, 57);
 
EthernetClient client;

void setup()
{
  Serial.begin(9600);
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
    Ethernet.begin(mac, ip);
  }
  else
  {
    Serial.println("Configurazione DHCP OK!");
    Serial.println(Ethernet.localIP());
  }
  
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

void loop()
{
    //Serial.println(millis());
    if (rf95.waitAvailableTimeout(2000))// Listen Data from LoRa Node
    {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
        uint8_t len = sizeof(buf);//data buffer length
        if (rf95.recv(buf, &len))//Check if there is incoming data
        {
            recdata( buf, len);
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
                    uint8_t data[] = "";//Reply 
                    
                    Serial.print("Response: ");
                    for (int i = 0; i < 3; i++) {
                      data[i] = (buf[i]);
                      Serial.print(data[i], HEX);
                    }

                    for (int i = 3; i < 6; i++) {
                      data[i] = (buf[i+28]);
                      Serial.print(char(data[i]));
                    }

                    Serial.println();
                    
                    rf95.send(data, 6);// Send Reply to LoRa Node
                    rf95.waitPacketSent();
                    
                    String client_id = "";
                    String user_id = "";
                    String state_id = "";
                    
                    for (int i = 3; i < 17; i++) {
                      client_id += char(buf[i]);
                    }
                    Serial.print("Client ID: ");
                    Serial.println(client_id);

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

                    uploadData(client_id, user_id, state_id);
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

void uploadData(String client_id, String user_id, String state_id) {//Upload Data to ThingSpeak
  // form the string for the API header parameter:


  // form the string for the URL parameter, be careful about the required "
  String upload_url = "GET /arduino/test.php?device=";
  upload_url += client_id;
  upload_url += "&user=";
  upload_url += user_id;
  upload_url += "&state=";
  upload_url += state_id;
  upload_url += " HTTP/1.1";
  Serial.println(upload_url);
  Serial.print("Connessione... ");
 
  if (client.connect(server, 80))
  {
    Serial.print("- Connesso... ");
    //invio la richiesta al server
    client.println(upload_url);
    client.println("Host: localhost");
    client.println("Connection: close");
    client.println();
    //chiudo la connessione
    client.stop();
    Serial.println("- Invio eseguito");
    
  }
  else
  {
    Serial.println("Errore Connessione");
  }
}

