void loop(void) {
  timer1.run();
  
  String codiceLetto ="";
  String UIDB="";
  byte i;
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  display.setCursor(0, 1);
  display.print("*** Attesa CARD! ***");
  
  if (!digitalRead(swRosso))   {
    digitalWrite(ledRosso, HIGH);
    Serial.print("Rosso");
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    newstate_id = '3';
    Serial.print(" - State: ");
    Serial.println(char(newstate_id));
    display.setCursor(0, 3);

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
    Serial.print(codiceLetto);
    codiceLetto.toCharArray(user_id, 15);
    Serial.print(" - State: ");
    Serial.println(char(newstate_id));
    Serial.println(success);
    if (success) {
      SendMSG();
    }
  }

  if (!digitalRead(swGiallo)) {
    digitalWrite(ledGiallo, HIGH);
    Serial.print("Giallo");
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    newstate_id = '2';
    Serial.print(" - State: ");
    Serial.println(char(newstate_id));
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
    Serial.print(codiceLetto);
    codiceLetto.toCharArray(user_id, 15);
    Serial.print(" - State: ");
    Serial.println(char(newstate_id));
    Serial.println(success);
    if (success) {
      SendMSG();
    }
  }

  if (!digitalRead(swVerde)) {
    digitalWrite(ledVerde, HIGH);
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    newstate_id = '1';
    Serial.print("Verde");
    Serial.print(" - State: ");
    Serial.println(char(newstate_id));
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
    Serial.print(codiceLetto);
    codiceLetto.toCharArray(user_id, 15);
    Serial.print(" - State: ");
    Serial.println(char(newstate_id));
    Serial.println(success);
    if (success) {
      SendMSG();
    }
  }

  if (success==0) {
    relectrl();
    return;
  } 
 
} 
