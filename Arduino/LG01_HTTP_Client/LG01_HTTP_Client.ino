/*
  Upload Data to WebService:
  Support Devices: LG01
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
//#include <Bridge.h>
#include <Process.h>
#define BAUDRATE 115200
RH_RF95 rf95;

uint16_t crcdata = 0;
uint16_t recCRCData = 0;
float frequency = 868.0;
String dataString = "";

void setup()
{
  Bridge.begin(BAUDRATE);
  Console.begin();
  while(!Console);
  if (!rf95.init())
    Console.println("init failed");
  ;
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);

  Console.println("LoRa Gateway 2 Web Service  --");

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
  if (rf95.waitAvailableTimeout(2000))// Listen Data from LoRa Node
  {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
    uint8_t len = sizeof(buf);//data buffer length
    if (rf95.recv(buf, &len))//Check if there is incoming data
    {
      recdata( buf, len);
      Console.print("Get LoRa Packet: ");
      for (int i = 0; i < len; i++)
      {
        Console.print(buf[i], HEX);
        Console.print(" ");
      }
      Console.println();
      if (crcdata == recCRCData) //Check if CRC is correct
      {
        if (buf[0] == 1 || buf[1] == 0 || buf[2] == 0) //Check if the ID match the LoRa Node ID 100
        {
          uint8_t data[] = "";//Reply

          Console.print("Response: ");
          for (int i = 0; i < 3; i++) {
            data[i] = (buf[i]);
            Console.print(data[i], HEX);
          }

          
          data[3] = (buf[31]);
          Console.print(char(data[3]));
        
          Console.println();

          rf95.send(data, 4);// Send Reply to LoRa Node
          rf95.waitPacketSent();

          String client_id = "";
          String user_id = "";
          String state_id = "";

          for (int i = 3; i < 17; i++) {
            client_id += char(buf[i]);
          }
          Console.print("Client ID: ");
          Console.println(client_id);

          for (int i = 17; i < 31; i++) {
            user_id += char(buf[i]);
          }
          Console.print("User   ID: ");
          Console.println(user_id);

          state_id += char(buf[31]);
          Console.print("State  ID: ");
          Console.println(state_id);

          uploadData(client_id, user_id, state_id);
        }
      }
      else
        Console.println(" CRC Fail");
    }
    else
    {
      Console.println("recv failed");
      ;
    }
  }

}

void uploadData(String client_id, String user_id, String state_id) {
  // form the string for the URL parameter, be careful about the required "
  String upload_url = "http://192.168.2.97/TagManager/handleRequestBox.php?box=";
  upload_url += client_id;
  upload_url += "&user=";
  upload_url += user_id;
  upload_url += "&act=";
  upload_url += state_id;  

  Console.println(upload_url);
  Console.println("Call Linux Command to Send Data");
  Process p;    // Create a process and call it "p", this process will execute a Linux curl command
  p.begin("curl");
  p.addParameter("-k");
  p.addParameter(upload_url);
  p.run();    // Run the process and wait for its termination

  Console.print("Feedback from Linux: ");
  // If there's output from Linux,
  // send it out the Console:
  while (p.available() > 0)
  {
    char c = p.read();
    Console.write(c);
  }
  Console.println("");
  Console.println("Call to server completed");
  Console.println("####################################");
  Console.println("");

}

