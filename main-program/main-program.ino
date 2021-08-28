#include <SoftwareSerial.h>                //include a header file contaning objects that provide functionality to communicate with devices attached to arduino (like SIM 900)

#define ADC_BITS  10                       //setting 10 Bits for measuring the analog to digital conveted voltage (values from 0 to 1023 - 2^10)
#define ADC_COUNTS  (1<<ADC_BITS)          //'<<' is bitwise shift operator, so here the ADC_COUNTS takes the vlue of 1024
#define READVCC_CALIBRATION_CONST 1126400L //Constant for converting power supply voltage value in digital format to analog value (units in mV) 

SoftwareSerial gprsSerial(7, 8);     //attach GPRS device(SIM 900) to pin 7 and 8.

double Irms1 = 0;        //Irms from sensor 1
double Irms2 = 0;        //Irms from sensor 2

long time = 0;           //timer variable  
int ctdone = 0;          //show whether CT values are read 
int flag = 0;            //Flag variable for GPRS process
int count = 0;           //Number of data transmission trials made by GPRS device in a 15 min loop
int success = 0;         //Indicate wether data transmission was successfull
int block_id = 0;        //Count number of '15 min blocks of data'  

String sdat = "GET http://app.solarfamily.in/api/v1/feed/readings?device_id=865904027419669&block_id=";   //URL of the Database where data is stored
char cdat[300];        

void setup() 
{
}

void loop() 
{
 
    if((millis()-time) >= 0 && ctdone == 0)
    {
            
            block_id++;                        //Increase the block_id
            sdat += block_id;
            sdat += "&data=";                  //Add it to Block_id part of URL
    
            for (int k = 1; k < 14; k++)       //loop running for 13 min (with an error of 1ms)
            {
                Irms1 = calcIrms(29998, 0);    //Calculate Irms Current from sensor 1 for 30 seconds
                sdat += Irms1;
                sdat += ",";                   //Add ',' after each values
                Irms2 = calcIrms(29998, 1);    //Calculate Irms Current from sensor 2 for 30 seconds
                sdat += Irms2;
                sdat += ",";                   //Add ',' after each values
            }
     
     
     ctdone = 1;
     sdat +="\r\n\r\n";
        
        for( int l = 0; l < sizeof(cdat);  ++l )
        { cdat[l] = (char)0; }
     
     sdat.toCharArray(cdat, 300);
     success = 0;
     count = 0;

     while(count < 2 && success == 0)
      {  
      gprsSerial.begin(9600);
      gprsSerial.flush();
      delay(1000);
      
      gprsSerial.println("AT\r");
      delay(1000);
      flag = search("OK");                       //wave a OKAY flag if the gsm is responding
      delay(10);

      if(flag == 1)                              //if OKAY flag is waved check for GPRS
      {
      gprsSerial.println("AT+CGATT?\r");
      delay(1000);
      flag = search("OK");                       //wave a OKAY flag if GPRS is attached
      }
      
      if(flag == 1)                              //if OKAY flag is waved reset the IP session
      {
      gprsSerial.println("AT+CIPSHUT\r"); 
      delay(2000);
      flag = search("SHUT");                     //wave a OKAY flag after restting IP sessions
      }
      
      if(flag == 1)                              //if OKAY flag is waved check if IP stack is intialised 
      {
      gprsSerial.println("AT+CIPSTATUS\r");
      delay(2000);
      flag = search("OK");                       //wave a INITIALTION DONE flag after initilisation
      }
      
      if(flag == 1)                              //if INITIALISATION FLAG is waved set a single connection
      {
      gprsSerial.println("AT+CIPMUX=0\r");
      delay(1000);
      flag = search("OK");                       //wave a OKAY flag after setting up the connection
      }
      
      if(flag == 1)                              //if OKAY flag is waved start a connection by providing APN  
      {
      gprsSerial.println("AT+CSTT=\"airtelgprs.com\",\"\", \"\"\r");
      delay(2000);
      flag = search("OK");                       //wave a OKAY flag if cneection with service provider is established
      }
      
      if(flag == 1)                              //if OKAY flag is waved bring up the wireless
      {
      gprsSerial.println("AT+CIICR\r");        
      delay(9000);
      flag = search("OK");                       //wave a OKAY flag after the wireless is up
      }
      
      if(flag == 1)                              //if flag is waved get a IP address
      {
      gprsSerial.println("AT+CIFSR\r");
      delay(3000);
      flag = search(".");                        //Wave a IP ADDRESS FLAG (Ex. 100.78.115.17)
      }
      
      if(flag == 1)                              //If IP ADDRESS FLAG is waved establish connection with web server 
      {
      gprsSerial.println("AT+CIPSTART=\"TCP\", \"app.solarfamily.in\", \"80\"\r");
      delay(9000);
      flag = search("CONNECT");                  //wave a CONNECTED flag if connection is established 
      }
      
      if(flag == 1)                              //if CONNECTED flag is waved send the GET request command to initiate a database update
      {
      gprsSerial.println("AT+CIPSEND\r");
      delay(1000);
      gprsSerial.println(cdat);
      delay(1000);
      gprsSerial.write(0x1A);
      delay(9000);                               //DATABASE will wave a RESPONSE flag
      success = 1;                               //Use search function to search a success response from the server
      }
      
      Serial.println("Ending connection");
      gprsSerial.flush();
      gprsSerial.end();
      flag = 0;
      count++;
      delay(2000);
      } 
     
    
    sdat = "GET http://app.solarfamily.in/api/v1/feed/readings?device_id=865904027419669&block_id=";
    }
    
    
    if((millis()-time) >= 900000 && ctdone == 1)    //Reset the Loop after 15 minuites 
    {
    ctdone = 0;                                     
    time = millis();                                //RESET timer
    } 
    
}


double calcIrms(unsigned int timeout, unsigned int _inPinI)
{
   unsigned int inPinI;
   unsigned int NumberofSamples = 0;                   //To count number of samples taken in the specified time period
   double ICAL, offsetI, filteredI, sqI, Irms, sumI;
   int sampleI;
   
   int SupplyVoltage = readVcc();                     //call readVcc() function to calculate supply Voltage
   unsigned long start = millis();                    //Start the timer
   
   inPinI = _inPinI;
   ICAL = 43.47;
   offsetI = ADC_COUNTS>>1;

  
    while ((millis()-start) < timeout)                //Check fo rtime and stop when the time limit is reached
      {
       NumberofSamples++;
      sampleI = analogRead(inPinI);

      offsetI = (offsetI + (sampleI-offsetI)/1024);   // Digital low pass filter extracts the 2.5 V dc offset,  
      filteredI = sampleI - offsetI;                  //  then subtract this - signal is now centered on 0 counts.
      
      sqI = filteredI * filteredI;                    // Root-mean-square method current (square current values and sum it)
      sumI += sqI;
      }

  double I_RATIO = ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms = I_RATIO * sqrt(sumI / NumberofSamples);      //Take the mean of the sum computed above

  sumI = 0;   //Reset accumulators  
  
  return Irms;
}


long readVcc() 
{
  long result;

  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  
  delay(2);                                                //Wait for Vref to Settle
  ADCSRA |= _BV(ADSC);                             
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = READVCC_CALIBRATION_CONST / result;            
  return result;
 }
 

int search(char x[])
{

char ch;
char cha[300];
  
  for( int j = 0; j < sizeof(cha);  ++j )   //Clear the contents of the array
  cha[j] = (char)0;
  
  int i = 0;
  while(gprsSerial.available()!=0)          //Recieve data from gprs device
  {
    ch = gprsSerial.read();  
    cha[i] = ch;
    i++;
  }
 
 Serial.write(cha);
 if(strstr(cha, x) != NULL)                 // Check for the specified word and return the memory location of that word in the string 
 { return 1; }                              //Return 1 if the word is found in the string 
 else
 { return 0; }                              //Return 0 if the word is not found in the string
 
}
