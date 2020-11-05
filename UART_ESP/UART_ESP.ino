/**************************************************************
* UART Test code
* This code waits for a 0x04 from Kinetis and replies
* with the following bytes 0x00,0x01,0x02,0x03.
*
**************************************************************/
#define LED D0

boolean ledState = true;

void setup()
{

 Serial.begin(9600,  SERIAL_8N1);
 Serial.swap();                   // TX = D8, RX = D7 
 pinMode(LED, OUTPUT);
 digitalWrite(LED, ledState);
}

void loop()
{
  
 byte serin=0;

  
  if (Serial.available()>0)
  {
    serin=Serial.read();
    ledState = !ledState;
    digitalWrite(LED, ledState);
  }
  
  if(serin==0x04)
  {
    Serial.write(1);
    Serial.write(1);
    Serial.write(1);
    Serial.write(3);
    serin=0;

   
  }

  delay(100);
  

 
}
