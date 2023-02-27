#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <Hash.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN D2 
#define DHTTYPE DHT22   
#define MOTORPIN D1
#define LIGHTPIN D8

#define API_KEY "AIzaSyAPiLIcm12Nlam9kXMayBJcle2oRQ_dyMc"
#define DATABASE_URL "https://smart-farm-6d610-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fdbo;
FirebaseAuth auth; 
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

DHT dht(DHTPIN, DHTTYPE);


/*Put your SSID & Password*/
const char* ssid = "CSE Squad";  // Enter SSID here
const char* password = "Triple#:)";  //Enter Password here


float temp = 0.0;
float hum = 0.0; 


const int AirValue = 640;   //you need to replace this value with Value_1
const int WaterValue = 230;  //you need to replace this value with Value_2
const int SensorPin = A0;
int soilMoistureValue = 0;
int soilmoisturepercent=0;
bool motorStatus = false;

void setup(){

  pinMode(MOTORPIN, OUTPUT); 
  pinMode(LIGHTPIN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
 
  delay(100);
  Serial.begin(9600);
  dht.begin();  

  Serial.println("");
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

//set up/ signUp with firebase  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){    
    Serial.println("signUp ok");
    signupOK = true;  
  }else{
  Serial.printf("%s\n", config.signer.signupError.message.c_str());    
  }

config.token_status_callback = tokenStatusCallback;   
Firebase.begin(&config, &auth);
Firebase.reconnectWiFi(true);
}

void loop(){
  
  soilMoistureValue = analogRead(SensorPin);  //put Sensor insert into soil
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Serial.println(soilMoistureValue);  
  temp = dht.readTemperature(); // Gets the values of the temperature
  hum = dht.readHumidity(); // Gets the values of the humidity 
  // Serial.println(temp);
  // Serial.println(hum);
  // Serial.println(soilmoisturepercent);

  

  if(soilmoisturepercent>=70){
    motorStatus = false;
    digitalWrite(MOTORPIN, LOW);
    digitalWrite(BUILTIN_LED,HIGH);
    digitalWrite(LIGHTPIN,HIGH);
  }
  else{
    motorStatus = true;
    digitalWrite(MOTORPIN, HIGH);
    digitalWrite(BUILTIN_LED, LOW);  
    digitalWrite(LIGHTPIN,LOW);      
  }

  if(soilmoisturepercent>100)
    soilmoisturepercent = 100;
  if(soilmoisturepercent<0)
    soilmoisturepercent = 0;    
  


/////Firebase Run

//
if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0)){
  sendDataPrevMillis = millis();    

  //store data in Firebase
  if(Firebase.RTDB.setBool(&fdbo, "/Motor/inOn", motorStatus)){
    Serial.println(); Serial.print(motorStatus);
    Serial.print(" - successfully saved to: " + fdbo.dataPath());
    Serial.println(" (" + fdbo.dataType() + ")");  
  }else{
    Serial.println("Motor Write Failed : " + fdbo.errorReason());    
  }  

  if(Firebase.RTDB.setFloat(&fdbo, "/Sensor/humudity", hum)){
    Serial.println(); Serial.print(hum);
    Serial.print(" - successfully saved to: " + fdbo.dataPath());
    Serial.println(" (" + fdbo.dataType() + ")");     
  }
  else{
    Serial.println("Humidity FAILED: " + fdbo.errorReason());
  }

  if(Firebase.RTDB.setFloat(&fdbo, "/Sensor/temperature", temp)){
    Serial.println(); Serial.print(temp);
    Serial.print(" - successfully saved to: " + fdbo.dataPath());
    Serial.println(" (" + fdbo.dataType() + ")");     
  }
  else{
    Serial.println("Temparature FAILED: " + fdbo.errorReason());
  }

  if(Firebase.RTDB.setFloat(&fdbo, "/Sensor/soil_moisture", soilmoisturepercent)){
    Serial.println(); Serial.print(soilmoisturepercent);
    Serial.print(" - successfully saved to: " + fdbo.dataPath());
    Serial.println(" (" + fdbo.dataType() + ")");     
  }
  else{
    Serial.println("Soil Moisture FAILED: " + fdbo.errorReason());
  }


  //get data from Firebase
  if(Firebase.RTDB.getBool(&fdbo, "/Motor/inOn")){
    if(fdbo.dataType()== "boolean"){
      motorStatus = fdbo.boolData(); 
      Serial.println("Successful read from " + fdbo.dataPath() + ": "+ motorStatus +"(" + fdbo.dataType() + ")");   
    }else{
      Serial.println("Motor Read Failed : " + fdbo.errorReason());  
    } 
    
  }
  
} 

  
  delay(2000);
}


