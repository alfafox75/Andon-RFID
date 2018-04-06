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
//  #include <String.h>
  #include <Wire.h>

// LoRa Shield
  #include <RH_RF95.h>
  RH_RF95 rf95;
  float frequency = 868.0;

// RFID/NFC  
  #include <PN532_I2C.h>
  #include <PN532.h>
  PN532_I2C pn532i2c(Wire);
  PN532 nfc(pn532i2c);

//Display
  #include <LiquidCrystal_I2C.h> // libreria di gestione del display lcd
//  LiquidCrystal_I2C display(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
  LiquidCrystal_I2C display(0x3F,20,4);
  
// Definizione variabili
  int ledVerde = 30;
  int ledGiallo = 32;
  int ledRosso = 34;
  int buzPin = 40;
  int swGiallo = 36;
  int swRosso = 38;
  int rl1 =22;
  int rl2 =24;
  int rl3 =26;
  int rl4 =28;
  int t=0;
  
  String UIDrequest = "";
  String UIDassist = "";
  byte bGlobalErr;
  char node_id[3] = {1,0,0}; //LoRa End Node ID 100
  char client_id[14] = {'1','2','3','4','5','6','7','8','9','0','1','2','3','4'}; //LoRa End Node ID 100
  char user_id[14]   = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
  char state_id = '1';
  String stringOne;
  unsigned int count = 1;
  unsigned char sendBuf[50]={0};
   
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//Reply data array
  uint8_t len = sizeof(buf);//reply data length
    
void setup(void) 
{
  pinMode(swGiallo, INPUT_PULLUP);  
  pinMode(swRosso, INPUT_PULLUP); 
  pinMode(ledVerde, OUTPUT);
  digitalWrite(ledVerde, HIGH);
  pinMode(ledGiallo, OUTPUT);
  digitalWrite(ledGiallo, HIGH);
  pinMode(ledRosso, OUTPUT);
  digitalWrite(ledRosso, HIGH);
  pinMode(rl1, OUTPUT);
  digitalWrite(rl1, HIGH);
  pinMode(rl2, OUTPUT);
  digitalWrite(rl2, HIGH);
  pinMode(rl3, OUTPUT);
  digitalWrite(rl3, HIGH);
  pinMode(rl4, OUTPUT);
  digitalWrite(rl4, HIGH);
  pinMode(buzPin, OUTPUT);
  tone(buzPin,1000,200);
  delay(2000);
  digitalWrite(ledGiallo, LOW);
  digitalWrite(ledRosso, LOW);
  digitalWrite(rl1, LOW);
  digitalWrite(rl2, LOW);
  digitalWrite(rl3, LOW);
  digitalWrite(rl4, LOW);

  Serial.begin(9600);
  Serial.println("Serial OK!");

//  display.begin(20, 4);
  display.init(); 
  display.backlight();
  
  Serial.println("Display OK!");
  display.clear();
  display.setCursor(0, 1);
  display.print("RFID Assist v. 01.00");
  delay(3000);
  
  nfc.begin();
  Serial.println("NFC OK!");

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
  Serial.print("CLIENT_ID: ");
    for (int i = 0; i < 14; i++) {
//    data[3+i] = client_id[i];
      Serial.print(char(client_id[i]));
    }
    Serial.println("");

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  // configure board to read RFID tags
  nfc.SAMConfig();
  digitalWrite(rl1, HIGH);
  Serial.println("Waiting for an ISO14443A Card ...");
}

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

void loop(void) {
  t++;
  display.setCursor(0, 1);
  display.print("*** Attesa CARD! ***");
  if (digitalRead(swRosso) == LOW && digitalRead(ledVerde) == 1)
  {
    digitalWrite(ledRosso, HIGH);
    digitalWrite(ledGiallo, LOW);
    state_id='3';
    t=0;
  }
  else if (digitalRead(swGiallo) == LOW && digitalRead(ledVerde) == 1)
  {
    digitalWrite(ledGiallo, HIGH);
    digitalWrite(ledRosso, LOW);
    state_id='2';
    t=0;
  }
  display.setCursor(0, 3);
  String codiceLetto ="";
  String UIDB="";
  byte i;
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  if (t==5 && digitalRead(ledVerde) == 1) {
    digitalWrite(ledRosso, LOW);
    digitalWrite(ledGiallo, LOW);
    t=0;
  }
  else if (t==10) { 
    t=0;
  }
  if (digitalRead(ledRosso) == 1 || digitalRead(ledGiallo) == 1) {     
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (success==0) {
  //    
      return;
    }  
  }
  else {
    delay(50);
    return;
  }
  Serial.print("Found a card! ");
  for (byte i = 0; i < 7; i++) 
    {
      UIDB=String (uid[i],HEX);
      if (UIDB.length()==1) {
        UIDB= "0" + UIDB;
      }
      codiceLetto+= UIDB;
      codiceLetto.toUpperCase();
    }  
  Serial.println(codiceLetto);
  
  if (codiceLetto != UIDrequest) {
    UIDrequest = codiceLetto;
    if (digitalRead(ledVerde) == 1) {
      display.clear();
      Serial.print("Richiesta intervento da: ");
      display.setCursor(0, 0);
      display.print("Richiesta intervento");
      display.setCursor(0, 2);
      display.print(codiceLetto);
      codiceLetto.toCharArray(user_id, 15);
      Serial.println(user_id);
      SendMSG (user_id, state_id);
      relectrl();
      digitalWrite(ledVerde, LOW);
      tone(buzPin,1000,500);
    }
    else {
      digitalWrite(ledRosso, LOW);
      digitalWrite(ledGiallo, LOW);
      digitalWrite(ledVerde, HIGH);
      Serial.print("Intervento eseguito da:");
      UIDrequest = "";
      display.clear();
      display.setCursor(0, 0);
      display.print("Intervento eseguito");
      display.setCursor(0, 2);
      display.print(codiceLetto);
      state_id='1';
      tone(buzPin,1000,500);
      codiceLetto.toCharArray(user_id, 15);
      Serial.println(user_id);
      SendMSG (user_id, state_id );
      relectrl();
    }
    delay(5000);
  }
  else {
    display.clear();
    display.setCursor(1, 1);
    display.print("Errore  Operazione");
    relectrl();
    tone(buzPin,500,500);
    delay(5000);
  }
  relectrl();
  display.clear();
 
}  

void SendMSG (char *user_id, char state_id)
{
    char data[50] = {0} ;
    int dataLength = 34; // Payload Length

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

    Serial.print("USER_ID:   ");
    for (int i = 0; i < 14; i++) {
      data[17+i] = user_id[i];
      Serial.print(char(user_id[i]));
    }
    Serial.println("");

    Serial.print("STATE_ID:  ");  
    data[31] = state_id;
    Serial.println(char(data[31]));
        
    uint16_t crcData = CRC16((unsigned char*)data,dataLength);//get CRC DATA
    
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
    rf95.waitPacketSent();
    if (rf95.waitAvailableTimeout(3000))// Check If there is reply in 3 seconds.
    {
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len))//check if reply message is correct
       {
          Serial.print("Get LoRa Packet: ");
          Serial.println((char*)buf);
    
          if(buf[0] == 1 || buf[1] == 0 || buf[2] == 0 ) // Check if reply message has the our node ID
           {
              uint8_t data[] = "";//Reply
              digitalWrite(3, HIGH);
              //tone(4,4000,200);
              Serial.print("Got Reply from Gateway: ");//print reply
              for (int i = 0; i < 4; i++) {
                Serial.print(buf[i], HEX);
              }
              Serial.println("");
              
              
              Serial.print("RSSI: ");  // print RSSI
              Serial.println(rf95.lastRssi(), DEC);  
              
              display.clear();
              display.setCursor(0, 1);
              display.print("Risposta: ");      
              display.setCursor(10, 1);
              display.print(buf[3], HEX);      
              display.setCursor(0, 2);
              display.print("RSSI: ");  // print RSSI
              display.setCursor(10, 2);
              display.print(rf95.lastRssi(), DEC);  
              delay(2000);

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
}

void relectrl() {
  if (state_id == '3') {
    digitalWrite(rl1, LOW);
    digitalWrite(rl2, LOW);
    digitalWrite(rl3, HIGH);
    digitalWrite(rl4, LOW);
  }
  if (state_id == '2') {
    digitalWrite(rl1, LOW);
    digitalWrite(rl2, HIGH);
    digitalWrite(rl3, LOW);
    digitalWrite(rl4, LOW);
  }
  if (state_id == '1') {
    digitalWrite(rl1, HIGH);
    digitalWrite(rl2, LOW);
    digitalWrite(rl3, LOW);
    digitalWrite(rl4, LOW);
  }
  if (state_id == '4') {
    digitalWrite(rl1, LOW);
    digitalWrite(rl2, LOW);
    digitalWrite(rl3, LOW);
    digitalWrite(rl4, HIGH);
  }
}

