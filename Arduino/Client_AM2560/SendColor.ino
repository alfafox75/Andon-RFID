void SendColor()
{
  char data[50] = {0} ;
  int dataLength = 17; // Payload Length
  uint16_t recCRCData = 0;
  
  Serial.print("NODE_ID:   ");  
  for (int i = 0; i < 3; i++)  {
    data[i] = node_id[i]; 
    Serial.print(node_id[i],HEX);
  }
  Serial.println("");

  Serial.print("CLIENT_ID: ");
  for (int i = 0; i < 14; i++) {
    data[3+i] = client_id[i];
    Serial.print(char(client_id[i]));
  }
  Serial.println("");  
  uint16_t crcData = CRC16((unsigned char*)data,dataLength);//get CRC DATA
    Serial.print("Data length: "); 
    Serial.println(dataLength +2);
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
    rf95.waitPacketSent(2000); 
    if (rf95.waitAvailableTimeout(3000))// Check If there is reply in 10 seconds.
    {
        len = 4;
        if (rf95.recv(buf, &len))//check if reply message is correct
       {
            if(buf[0] == 1 ||buf[1] == 0 ||buf[2] == 0 ) // Check if reply message has the our node ID
           {
              digitalWrite(3, HIGH);
              //tone(4,4000,200);
              Serial.print("Got Reply from Gateway: ");//print reply
              for (int i = 0; i < 3; i++) {
                Serial.print(buf[i], HEX);
              }
              Serial.println(char(buf[3]));
              if (char(buf[3])=='1') {state_id = '1';}
              else if (char(buf[3])=='2') {state_id = '2';}
              else if (char(buf[3])=='3') {state_id = '3';}
              
              delay(2000);
              
              Serial.print("RSSI: ");  // print RSSI
              Serial.println(rf95.lastRssi(), DEC);        
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
        rf95.send(sendBuf, strlen((char*)sendBuf));//resend data
    }
    
    Serial.println("");
    relectrl();
}
