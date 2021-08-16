//major credit for this script is given to DPV TECHNOLOGY's tutorial on how to send data in the form of a GET request to thingspeak.com.
#include <SoftwareSerial.h>
#include "DHT.h"

//pin configurations can be modified based on how the circuit is wired and if there are additional sensors
#define RX 2 //Yes I know that these look swapped, but RX pin of Arduino is connected to TX pin of ESP8266 
#define TX 3
#define DHTPIN 7 
#define DHTTYPE DHT22 //AM2302 should have DHTTYPE of DHT22
int SOILSENSPIN = A0;

String AP = "SSID";       // WIFI NETWORK NAME. Change this to your network name
String PASS = "PASSWORD"; // WIFI NETWORK PASSWORD. Change this to your network password
String HOST = "dweet.io"; 
//String HOST = "requestbin.net"; 
int countTrueCommand;
int countTimeCommand;
int PORT=80;
boolean found = false;
int valSoilSensor = 1; //random number, no need to change
float valDhtTemp = 1.9; //random number, no need to change
float valDhtHumid = 1.8; //random number, no need to change
String thingName="thingName";//Change to your thing name (you can name it anything you want, but choose a name not likely to be used) Need a new thing name for each Arduino ESP8266 device that is being configured
SoftwareSerial esp8266(RX,TX);
DHT dht(DHTPIN, DHTTYPE);
 
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  dht.begin();
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK"); // sets mode for esp8266 to connect and retrieve data from a server
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK"); // command to connect to wifi
}

void loop() {
 
 valSoilSensor = getSoilSensorData();
 valDhtTemp = getDhtTemp();
 valDhtHumid = getDhtHum();
 String paramString= "soilMoisture=" + String(valSoilSensor)+ "&" +"airTemp="+String(valDhtTemp)+"&"+ "airHum="+ String(valDhtHumid); //edit this string based on names, values, and the number of datapoints you trying to collect.
 String getData= "GET /dweet/for/"+ thingName +"?"+ paramString +" HTTP/1.1\r\nHost: "+ HOST +":80\r\n\r\n"; //if using a different host, edit this string, specifically the GET directory. \r\n is used to print to the start of a newline
// String postData="POST /r/h87l56pu HTTP/1.1\r\nHost: requestbin.net\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: "+ String(paramString.length())+"\r\n\r\n"+paramString;
 sendCommand("AT+CIPMUX=1",5,"OK"); 
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK"); //connect to server via HTTP default port 80
// sendCommand("AT+CIPSERVER=1,80",10,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),10,">");//send AT Command to make GET request to dweet server. getData.length()+4 specifies that we are sending the amount of characters plus 4 in the getData String
// sendCommand("AT+CIPSEND=0," +String(postData.length()),10,">");
 esp8266.println(getData);delay(4500);countTrueCommand++; 
// esp8266.println(postData);delay(4500);countTrueCommand++;

 sendCommand("AT+CIPCLOSE=0",5,"OK"); // closes connection to the server
}

// functions used for the arduino uno to retrieve data from the sensors
int getSoilSensorData(){
  int soilSensorVal=analogRead(SOILSENSPIN);
  int percentageMoisture=map(soilSensorVal,240,595,100,0);// converts the initial readings (ranging from 240 to 595) to percent readings more recognizable by people
  return percentageMoisture; 
}
float getDhtTemp(){
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  float far=(1.8*t)+32; //Celsius to Fahrenheit conversion
  return far;
  }
float getDhtHum(){
  float h = dht.readHumidity(); //read as a percent by default
  return h;
  }


//function used to send AT Commands (means of communication between Arduino and ESP8266) as well as visualize those commands and their outcomes in the serial monitor of the IDE 
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
 
    countTimeCommand++;
  }
 
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
 
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
 
  found = false;
 }
