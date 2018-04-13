void setup(void) {
  pinMode(swGiallo, INPUT);  
  pinMode(swRosso, INPUT); 
  pinMode(swVerde, INPUT); 
  digitalWrite(swGiallo, HIGH);
  digitalWrite(swRosso, HIGH);
  digitalWrite(swVerde, HIGH);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledGiallo, OUTPUT);
  pinMode(ledRosso, OUTPUT);
  digitalWrite(ledVerde, HIGH);
  digitalWrite(ledGiallo, HIGH);
  digitalWrite(ledRosso, HIGH);
  pinMode(rl1, OUTPUT);
  pinMode(rl2, OUTPUT);
  pinMode(rl3, OUTPUT);
  pinMode(rl4, OUTPUT);
  digitalWrite(rl1, HIGH);
  digitalWrite(rl2, HIGH);
  digitalWrite(rl3, HIGH);
  digitalWrite(rl4, HIGH);
  pinMode(buzPin, OUTPUT);
  tone(buzPin,1000,200);
  delay(2000);

  timer1.setInterval(30000, SendColor);

  Serial.begin(9600);
  Serial.println("Serial OK!");

  nfc.begin();
  Serial.println("NFC OK!");

  if (!rf95.init())
   Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  
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
      if (address == 39) {
        char adrdisplay = "0x27";
        Serial.print(" - i2c Display");
      }
      if (address == 63) {
        char adrdisplay = "0x3F";
        Serial.print(" - i2c Display");
      }
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

  display.init(); 
  display.backlight();
  Serial.println("Display OK!");
  display.clear();
  display.setCursor(0, 1);
  display.print("RFID Assist v. 01.00");
  delay(3000);
  
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  // configure board to read RFID tags
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A Card ...");
  Serial.println("Status Request...");
  SendColor();
}
