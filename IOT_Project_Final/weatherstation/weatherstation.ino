#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

// Wi-Fi credentials
// const char* ssid = "realmeGT2";
// const char* password = "noideahs";

const char* ssid = "12345";
const char* password = "shubham123";

// Firebase credentials
#define FIREBASE_HOST "weatherstation-aef29-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "xLtXUIb8VDKyW7dVsC8K2ArCQONhPQ8U8SDmXvDZ" 

// Define sensor pins
#define DHTPIN 4           
#define DHTTYPE DHT22
#define SOIL_SENSOR_PIN 34
#define RAIN_SENSOR_PIN 35 

// Initialize sensors
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
FirebaseData firebaseData;

// Firebase config and auth objects
FirebaseConfig config;
FirebaseAuth auth;

// Wi-Fi connection attempt counter
int WiFiReconnectAttempts = 0;
const int maxReconnectAttempts = 5;

// Serial counter for unique IDs
unsigned long serialID = 1;  

void setup() {
  Serial.begin(115200);
  
  analogReadResolution(10);
  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT sensor initialized");

  // Initialize BMP180 sensor
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1); 
  } else {
    Serial.println("BMP180 sensor initialized");
  }

  // Connect to Wi-Fi with retry logic
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");
  
  while (WiFi.status() != WL_CONNECTED && WiFiReconnectAttempts < maxReconnectAttempts) {
    Serial.print(".");
    delay(1000);
    WiFiReconnectAttempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi");
    return;
  }

  // Configure Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;  
  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase initialized");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi not connected. Reconnecting...");
    WiFi.begin(ssid, password);
    delay(5000); 
    return;
  }

  // Read data from sensors
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float pressure = bmp.readPressure() / 100.0; 
  float altitude = bmp.readAltitude();
  int soilMoisture = analogRead(SOIL_SENSOR_PIN);
  int raindrop = analogRead(RAIN_SENSOR_PIN);

  // Print readings to Serial Monitor
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Altitude: ");
  Serial.println(altitude);
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  Serial.print("Raindrop: ");
  Serial.println(raindrop);

  // Check if readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Generate a unique ID for each data entry (serial ID)
  String serialID_str = String(serialID);

  // Create JSON data structure to store in Firebase
  FirebaseJson json;
  json.set("temperature", temperature);
  json.set("humidity", humidity);
  json.set("pressure", pressure);
  json.set("altitude", altitude);
  json.set("soil_moisture", soilMoisture);
  json.set("raindrop", raindrop);

  // Attempt to send data to Firebase with unique serial ID
 // String path = "/data1/" + serialID_str; // Store data under unique path with serial ID
  // String path = "/data2/" + serialID_str; 
  // String path = "/data3/" + serialID_str; // cold
  // String path = "/data4/" + serialID_str; // Nagapond
  //  String path = "/data5/" + serialID_str; // Soil moisture 
  //  String path = "/data6/" + serialID_str; // hot 
      String path = "/test/" + serialID_str; // test

  Serial.println("Attempting to send data to Firebase...");
  if (Firebase.setJSON(firebaseData, path, json)) {
    Serial.println("Data sent successfully");
  } else {
    Serial.print("Error sending data: ");
    Serial.println(firebaseData.errorReason());
  }

  // Increment the serial ID for the next data entry
  serialID++;
  if(serialID>10)serialID=0;

  // Delay before the next reading
  delay(10000); // Update every 10 seconds
}
