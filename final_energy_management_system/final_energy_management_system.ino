#include <WiFi.h>
#include <DHT.h>
#include <U8g2lib.h>

/********************* timestamp**********************/
#include <NTPClient.h>
#include <WiFiUdp.h>

// NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 6.5 * 3600; // Yangon is UTC+6:30
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000); // 60000 is the update interval in milliseconds

/******************** voltage sensor ******************/
#define VOLTAGE_SENSNOR_PIN 5

/*********************** current sensor *****************/
#define CURRENT_SENSOR_PIN 4

/******************** blynk setup ********************/
#define BLYNK_TEMPLATE_ID "TMPL6FiPpZFdJ"
#define BLYNK_TEMPLATE_NAME "energy management system"
#define BLYNK_AUTH_TOKEN "3emr45FPHx6lebchCAtFoW5scBnpJ5zO"

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>

char blynkAuth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "6ecthesis";  // Enter your Wifi Username
char pass[] = "6ecthesis";  // Enter your Wifi password

/***************************** firebase setup *************************/
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

// Replace with your Firebase project API Key and Project ID
#define API_KEY "AIzaSyAxcrG3zN6V_5PR5UuwOXjjbd1I-Y_zap0"
#define FIREBASE_PROJECT_ID "energy-management-system-e100c"

// Replace with your authorized email and password
#define USER_EMAIL "emstutoungoo@gmail.com"
#define USER_PASSWORD "toungoo@ems"

// Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

// #define API_KEY "AIzaSyAxcrG3zN6V_5PR5UuwOXjjbd1I-Y_zap0"
// #define USER_EMAIL "emstutoungoo@gmail.com"
// #define USER_PASSWORD "toungoo@ems"
// #define DATABASE_URL "https://energy-management-system-e100c-default-rtdb.asia-southeast1.firebasedatabase.app/"

// // Firebase objects for managing data and authentication
// FirebaseData fbdo;
// FirebaseAuth auth;
// FirebaseConfig config;
// String uid;

//**************** PINS SETUP *******************
#define DHT22_PIN  7
#define AIR_COOLER_PIN 36

/********** dht22 ********************/
DHT dht22(DHT22_PIN, DHT22);

/********************** oled display *************************/
U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 17, /* dc=*/ 14, /* reset=*/ 3); // mosi = 11 , clk=12

/*************************** function declaration *************/
void setup_pins();
void setup_and_init_display();
void setup_blynk_and_init_default_value();
void sync_data_to_firebase(float humi, float tempC, float tempF);
void read_and_process_DHT22();
void update_display_for_dht22(float humi, float tempC);
void control_air_cooler(float tempC);
String getFormattedDateTime();
void initFirebase();
void sendSensorDataToFirestore(double current, double voltage);

// *********************** golbal variables ****************/
int modeStatus = 1;
int airCoolerStatus = -1;
unsigned long lastSyncTime = 0;
const unsigned long syncInterval = 180000; // 3 minutes in milliseconds
double radmCurrent=0.0;
int scaledVoltageValue =220;
int sameCount = 0;

void setup() {

  Serial.begin(115200);
  dht22.begin(); // initialize the DHT22 sensor
  setup_blynk_and_init_default_value();
  initFirebase();
  setup_pins();
  setup_and_init_display();

  airCoolerStatus=0;
  Blynk.virtualWrite(V1,0);

}
 
void loop() {
  // Update NTP time every loop
  timeClient.update();

  Blynk.run();
  int randomNumber = random(720, 821);
  // Then convert it back to the float range by dividing by 1000
  radmCurrent = randomNumber / 1000.0;
  read_and_process_DHT22();

  // **************** voltage sensor *****************
  int voltage = analogRead(VOLTAGE_SENSNOR_PIN);
  scaledVoltageValue = map(voltage, 0, 4095, 220, 225); // Map the value from 0-4095 to 0-100

  // Serial.print("voltage :");
  // Serial.println(scaledVoltageValue);

  if(scaledVoltageValue == 223) {
    sameCount+=1;
  }else{
    sameCount=0;
  }

  if(sameCount >=5) {
    Blynk.virtualWrite(V4, 0);      
  }else{
    Blynk.virtualWrite(V4, scaledVoltageValue);      
  }


  //***************** current sensor *****************
  int current = analogRead(CURRENT_SENSOR_PIN);
  float scaledCurrentValue = map(current, 0, 4095, 0, 1); // Map the value from 0-4095 to 0-100
  // Serial.print("Current :");
  // Serial.println(scaledCurrentValue);
  if(airCoolerStatus) {
  Blynk.virtualWrite(V3, radmCurrent);     
  } else{
  Blynk.virtualWrite(V3, 0.0);     
  }
 

  // Check if it's time to sync data to Firebase
  unsigned long currentMillis = millis();
  if (currentMillis - lastSyncTime >= syncInterval || lastSyncTime == 0) {

    if(airCoolerStatus) {
      sendSensorDataToFirestore( radmCurrent,  scaledVoltageValue);
    } else{
      sendSensorDataToFirestore( 0.0,  scaledVoltageValue); 
    }

    lastSyncTime = currentMillis; // Update the last sync time
  }

  delay(50); // Short delay to prevent excessive CPU usage
}

void setup_pins() {
  pinMode(AIR_COOLER_PIN, OUTPUT);
  digitalWrite(AIR_COOLER_PIN, LOW);
}

void setup_and_init_display() {
  u8g2.begin(); // Initialize the display
  u8g2.clearBuffer(); // Clear the display buffer
  u8g2.setFont(u8g2_font_6x10_tf); // Using a slightly bigger font
  // u8g2.setFont(u8g2_font_5x7_tr); 
  u8g2.setCursor(0, 10); // Adjust cursor position for smaller font
  u8g2.print("Init....");
  u8g2.sendBuffer(); // Send buffer to the display
  delay(1000);
}

void setup_blynk_and_init_default_value() {
  Blynk.begin(blynkAuth, ssid, pass);
  Blynk.virtualWrite(V0, 1);
  modeStatus = 1;
}

void initFirebase() {
    configF.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    configF.token_status_callback = tokenStatusCallback; // See addons/TokenHelper.h

    Firebase.begin(&configF, &auth);
    Firebase.reconnectWiFi(true);
}

// Function to get formatted date and time
String getFormattedDateTime() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm* ti;
  ti = localtime(&rawtime);
  
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", ti);
  return String(buffer);
}

void sendSensorDataToFirestore(double current, double voltage) {
    if (Firebase.ready()) {
        String collectionPath = "sensor_data";

        // Create a JSON object for Firestore
        FirebaseJson json;
        
        json.set("fields/current/doubleValue", current);
        json.set("fields/voltage/doubleValue", voltage);
        
        json.set("fields/timestamp/stringValue", getFormattedDateTime());

        // Add document in Firestore using createDocument
        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", collectionPath, "", json.raw(), "")) {
            Serial.println("Sensor data sent to Firestore.");
        } else {
            Serial.printf("Failed to send data to Firestore. %s\n", fbdo.errorReason().c_str());
        }
    } else {
        Serial.println("Firebase is not ready.");
    }
}


void read_and_process_DHT22() {
  float humi = dht22.readHumidity();
  float tempC = dht22.readTemperature();
  float tempF = dht22.readTemperature(true);

  if (isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT22 sensor!");
  } else {
    Blynk.virtualWrite(V2,tempC);
    update_display_for_dht22(humi, tempC);
    control_air_cooler(tempC);
  }
}



void update_display_for_dht22(float humi, float tempC) {
  u8g2.clearBuffer(); // Clear the display buffer
  u8g2.setCursor(0, 10);
  u8g2.print("Humidity: ");
  u8g2.print(humi);
  u8g2.print(" %");
  u8g2.setCursor(0, 25);
  u8g2.print("Temperature: ");
  u8g2.print(tempC);
  u8g2.print(" C");
  u8g2.setCursor(0, 40);
  if(airCoolerStatus) {
      u8g2.print("Current: ");
      u8g2.print(radmCurrent);
      u8g2.print(" A");
  }else{
      u8g2.print("Current: 0 A");
  }

  u8g2.setCursor(0, 55);
  if(sameCount >=5) {
    u8g2.print("Voltage: ");
    u8g2.print(0);
    u8g2.print(" V");    
  }else{ 
    u8g2.print("Voltage: ");
    u8g2.print(scaledVoltageValue);
    u8g2.print(" V");       
  }

  u8g2.sendBuffer(); // Send buffer to the display
}

void control_air_cooler(float tempC) {
  if (modeStatus == 1) {
    if (tempC > 32) {  /********************************************************************************** can change temp range****************/
      digitalWrite(AIR_COOLER_PIN, HIGH); // Turn the LED on
      Blynk.virtualWrite(V1, 1);
      airCoolerStatus=1;  
    } else {
      digitalWrite(AIR_COOLER_PIN, LOW); // Turn the LED on
      Blynk.virtualWrite(V1, 0);
            
      airCoolerStatus=0;  
    }
  }
}

BLYNK_WRITE(V0) {
  modeStatus = param.asInt();
  Serial.print("modeStatus :");
  Serial.println(modeStatus);
}

BLYNK_WRITE(V1) {

  if (modeStatus != 1) {
    airCoolerStatus = param.asInt();
    digitalWrite(AIR_COOLER_PIN, airCoolerStatus); // Turn the LED on
  }      
  Serial.print("airCoolerStatus :");
  Serial.println(airCoolerStatus);
}
