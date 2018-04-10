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
  #include <String.h>
  #include <Wire.h>

// LoRa Shield
  #include <RH_RF95.h>
  RH_RF95 rf95(5, 2);
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
  int swVerde = 27;
  int swGiallo = 25;
  int swRosso = 23;
  int rl1 =22;
  int rl2 =24;
  int rl3 =26;
  int rl4 =28;
  int t=0;
  
  String UIDrequest = "";
  String UIDassist = "";
  byte bGlobalErr;
  char node_id[3] = {1,0,0}; //LoRa End Node ID 100
  // ID BOX=12345678901238 
  char client_id[14] = {'1','2','3','4','5','6','7','8','9','0','1','2','3','8'}; //LoRa End Node ID 100
  char user_id[14]   = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
  char state_id = '1';
  String stringOne;
  unsigned int count = 1;
  unsigned char sendBuf[50]={0};
  int tColor = 0;
  int timerColor = 0;
  uint16_t crcdata = 0;
  uint16_t recCRCData = 0;
     
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//Reply data array
  uint8_t len = sizeof(buf);//reply data length
    

