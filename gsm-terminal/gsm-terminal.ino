#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(7, 8);

void setup()
{
  gprsSerial.begin(9600);
  Serial.begin(9600);

  Serial.println("Config SIM900...");
  delay(2000);
  Serial.println("Done!...");
  gprsSerial.flush();
  Serial.flush();
  
  

  
 
}


void loop()
{
  if(Serial.available())
  {
    switch (Serial.read())
    {
      //"
      case 34:
      gprsSerial.println("AT+CSQ\r");
      break;
      //A
      case 65:
      gprsSerial.println("AT\r");
      break;
      //B
      case 66:
      gprsSerial.println("AT+CPIN?\r");
      break;
      //C
      case 67:
      gprsSerial.println("AT+CREG?\r");
      break;
      //D
      case 68:
      gprsSerial.println("AT+CGATT?\r");
      break;
      //E
      case 69:
      gprsSerial.println("AT+CIPSHUT\r");
      break;
      //F
      case 70:
      gprsSerial.println("AT+CIPSTATUS\r");
      break;
      //G
      case 71:
      gprsSerial.println("AT+CIPMUX=0\r");
      break;
      //H
      case 72:
      gprsSerial.println("AT+CSTT=\"airtelgprs.com\",\"\", \"\"\r");
      break;
      //I
      case 73:
      gprsSerial.println("AT+CIICR\r");
      break;
      //J
      case 74:
      gprsSerial.println("AT+CIFSR\r");
      break;
      //K
      case 75:
      gprsSerial.println("AT+CIPSTART=\"TCP\", \"r.tlrfindia.com\", \"80\"\r");
      break;
      //L
      case 76:
      gprsSerial.println("AT+CIPSEND\r");
      break;
      //M
      case 77:
      gprsSerial.println("GET http://r.tlrfindia.com/receive.php?id=124\r\n\r\n");
      break;
      //m
      case 109:
      gprsSerial.println("GET http://r.tlrfindia.com/receive.php?id=1\r\n\r\n");
      break;
      //N
      case 78:
      gprsSerial.write(0x1A);
      break;
      //0
      case 79:
      gprsSerial.println("AT+CIPSHUT\r");
      break;
      //P
      case 80:
      gprsSerial.flush();
      break;
      default:
      Serial.println("bnana");
    }
  }
  delay(1000);
  toSerial();
}

void toSerial()
{
  while(gprsSerial.available()!=0)
  {
    Serial.write(gprsSerial.read());
  }
}
