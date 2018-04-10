void loop(void) {
  t++;
  
  if (tColor != millis() / 1000) {
    tColor = millis()/1000;
    timerColor++;
    if (timerColor == 30) {
      Serial.println("Status Request...");
      SendColor();
      timerColor = 0;
    }
  }
  
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
