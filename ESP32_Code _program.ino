#include <WiFi.h>               // Library WiFi bawaan untuk ESP32
#include <LiquidCrystal_I2C.h>
#include <DHT.h>                // Library untuk DHT22
#include <OneWire.h>            // Library untuk komunikasi dengan DS18B20
#include <DallasTemperature.h>  // Library untuk DS18B20

// Konfigurasi WiFi
const char *ssid = "Wifi";   // SSID hotspot atau WiFi
const char *password = "123Nurdin"; // password hotspot WiFi

// IP Address Server XAMPP
const char *host = "192.168.77.148";

// define pin SDA dan SCL
#define SDA_PIN 32
#define SCL_PIN 33

// set LCD 
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// define pin DHT22 connected
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// define pin DS18B20 
const int oneWireBus = 4;

// Setup oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  // Inisialisasi I2C dengan pin SDA dan SCL
  Wire.begin(SDA_PIN, SCL_PIN);
  // Inisialisasi LCD  
  lcd.init(); 
  // Mengaktifkan backlight LCD                   
  lcd.backlight();           
  // Menampilkan nama alat "AquaSense"
  lcd.clear();
  lcd.setCursor(3, 0);  
  lcd.print("AquaSense"); 

  lcd.setCursor(3, 1);  
  lcd.print("Monitoring"); 
  delay(3000);

  // Start the DHT22 sensor
  dht.begin();
  // Start the DS18B20 sensor
  sensors.begin();

  // Inisialisasi WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("");
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Jika koneksi berhasil
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(1000);
  // Baca data dari DHT22
  float h = dht.readHumidity();  // Membaca kelembapan
  float t = dht.readTemperature(); // Membaca suhu dalam derajat Celsius

  // Cek apakah pembacaan berhasil
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed reception"));
    return; 

  // Baca data dari DS18B20
  sensors.requestTemperatures(); // Meminta suhu dari sensor
  float dsTemp = sensors.getTempCByIndex(0); // Mendapatkan suhu dalam Celsius dari sensor DS18B20

  // Mengirim data ke server
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }

  // URL untuk mengirim data
  String url = "/Monitoring-air-kolam/baca-data.php?data=" + String(t) + "&dht_hum=" + String(h) + "&ds_temp=" + String(dsTemp);


  Serial.print("Requesting URL: ");
  Serial.println(url);

  // Mengirimkan request ke server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Membaca dan menampilkan respon dari server
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  // Tampilkan hasil di LCD
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Hum: ");
  lcd.print(h); 
  lcd.print(" %");

  lcd.setCursor(0, 1); 
  lcd.print("Temp: ");
  lcd.print(dsTemp); 
  lcd.print(" C");

  // Tampilkan kelembapan di Serial Monitor
  Serial.print("dhtTemp: ");
  Serial.print(t);
  Serial.println(" C");
  Serial.print("dhtHum: ");
  Serial.print(h);
  Serial.println(" %");

  Serial.print("dstemp: ");
  Serial.print(dsTemp);
  Serial.println(" C");

  // jeda 30 detik
  delay(30000); 
}
