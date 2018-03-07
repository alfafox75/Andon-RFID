/*
  Upload Data to WebService:
  Support Devices: LoRa Shield + Arduino 
*/

/*  pin    4 buzzer
 *  pin   22 RL1
 *  pin   24 RL2 
 *  pin   26 RL3 
 *  pin   28 RL4 
 *  
 *  pin   30 LED1 verde 
 *  pin   32 LED2 giallo
 *  pin   34 LED3 rosso
 *  
 *  pin   36 tastiera giallo
 *  pin   38 tastiera roso
 */

#include <SPI.h>
#include <RH_RF95.h>
#include <String.h>

RH_RF95 rf95;


byte bGlobalErr;
char node_id[3] = {1,0,0}; //LoRa End Node ID 100
char client_id[14] = {'a','b','c','d','e','f','g','h','i','l','m','n','o','p'}; //LoRa End Node ID 100
char user_id[14] = {'1','2','3','4','5','6','7','8','9','0','1','2','3','4'};
char state_id[3] ={'0','1','2'};
String stringOne;
float frequency = 868.0;
unsigned int count = 1;

void setup()
{
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    
    digitalWrite(3, HIGH);
    tone(4,5000,400);
    delay(1000);
    tone(4,250,400);
    Serial.begin(9600);
    if (!rf95.init())
        Serial.println("init failed");
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    
    Serial.println("LoRa End Node --"); 
    Serial.print("LoRa End Node ID: ");

    for(int i = 0;i < 3; i++)
    {
        Serial.print(node_id[i],HEX);
    }
    Serial.println();
}


//Get Sensor Data


uint16_t calcByte(uint16_t crc, uint8_t b)
{
    uint32_t i;
    crc = crc ^ (uint32_t)b << 8;
    
    for (int i = 0; i < 8; i++)
    {
        if ((crc & 0x8000) == 0x8000)
            crc = crc << 1 ^ 0x1021;
        else
            crc = crc << 1;
    }
    return crc & 0xffff;
}

uint16_t CRC16(uint8_t *pBuffer,uint32_t length)
{
    uint16_t wCRC16=0;
    uint32_t i;
    if (( pBuffer==0 )||( length==0 ))
    {
      return 0;
    }
    for (int i = 0; i < length; i++)
    { 
      wCRC16 = calcByte(wCRC16, pBuffer[i]);
    }
    return wCRC16;
}

void loop()
{
    Serial.print("###########    ");
    Serial.print("COUNT=");
    Serial.print(count);
    Serial.println("    ###########");
    count++;

    //read signals
    char data[50] = {0} ;
    int dataLength = 34; // Payload Length
 
    for (int i = 0; i < 3; i++)  {
      data[i] = node_id[i]; 
    }
    Serial.print("CLIENT_ID: ");
    for (int i = 0; i < 14; i++) {
      data[3+i] = client_id[i];
      Serial.print(char(client_id[i]));
    }
    Serial.println("");

    Serial.print("USER_ID:   ");
    for (int i = 0; i < 14; i++) {
      data[17+i] = user_id[i];
      Serial.print(char(user_id[i]));
    }
    Serial.println("");

    Serial.print("STATE_ID:  ");  
    for (int i = 0; i < 3; i++) {
      data[31+i] = state_id[i];
      Serial.print(char(state_id[i]));
    }
    Serial.println("");
    
    uint16_t crcData = CRC16((unsigned char*)data,dataLength);//get CRC DATA
    //Serial.println(crcData,HEX);
    
    Serial.print("Data to be sent(without CRC): ");
    
    for(int i = 0;i < dataLength; i++)
    {
        Serial.print(data[i],HEX);
        Serial.print(" ");
    }
    Serial.println();
        
    unsigned char sendBuf[50]={0};

    for(int i = 0;i < dataLength;i++)
    {
        sendBuf[i] = data[i] ;
    }
    
    sendBuf[dataLength] = (unsigned char)crcData; // Add CRC to LoRa Data
    sendBuf[dataLength+1] = (unsigned char)(crcData>>8); // Add CRC to LoRa Data

    Serial.print("Data to be sent(with CRC):    ");
    for(int i = 0;i < (dataLength +2); i++)
    {
        Serial.print(sendBuf[i],HEX);
        Serial.print(" ");
    }
    Serial.println();

    rf95.send(sendBuf, dataLength+2);//Send LoRa Data
     
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//Reply data array
    uint8_t len = sizeof(buf);//reply data length

    if (rf95.waitAvailableTimeout(3000))// Check If there is reply in 3 seconds.
    {
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len))//check if reply message is correct
       {
            if(buf[0] == node_id[0] && buf[1] == node_id[2] && buf[2] == node_id[2] ) // Check if reply message has the our node ID
           {
              digitalWrite(3, HIGH);
              //tone(4,4000,200);
              Serial.print("Got Reply from Gateway: ");//print reply
              for (int i = 0; i < 3; i++) {
                Serial.print(buf[i], HEX);
              }
              for (int i = 3; i < 6; i++) {
                Serial.print(char(buf[i]));
              }
              Serial.println("");
              
              delay(2000);
              digitalWrite(3, LOW); 
//              Serial.print("RSSI: ");  // print RSSI
//              Serial.println(rf95.lastRssi(), DEC);        
           }    
        }
        else
        {
           Serial.println("recv failed");//
           rf95.send(sendBuf, strlen((char*)sendBuf));//resend if no reply
        }
    }
    else
    {
        Serial.println("No reply, is LoRa gateway running?");//No signal reply
        tone(4,250,200);
        rf95.send(sendBuf, strlen((char*)sendBuf));//resend data
    }
    delay(60000); // Send sensor data every 30 seconds
    Serial.println("");
}


