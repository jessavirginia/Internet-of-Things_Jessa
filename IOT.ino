#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h> 
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Konfigurasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Inisialisasi fitur Multi WiFi
ESP8266WiFiMulti wifiMulti;

//Konfigurasi Bot Telegram Nia
#define BOT_TOKEN "7686988019:AAHXwNCXTZ-NiHYmpO-ppWu_CQhKhuPTK_E" 
#define CHAT_ID "1272697471" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pin untuk Soil Moisture Sensor
#define SOIL_SENSOR A0 

void setup() {
  Serial.begin(115200);

//proses clear atau reset lcd (ini harus ada, gunanya untuk menghidupkan lcd)
  Wire.begin(D5, D6);  // Gunakan pin D5 sebagai SDA dan D6 sebagai SCL
  lcd.begin(16, 2);     // Mulai LCD dengan 16 kolom dan 2 baris
  lcd.backlight();      // Menghidupkan lampu latar belakang LCD
//end of clear lcd
  
  
  // --- PENGATURAN WIFI UTAMA DAN CADANGAN ---
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("IOT2025", "!oTI2025");     // WiFi Utama
  wifiMulti.addAP("RUN", "!!!!*()UUU");       // WiFi Cadangan

  Serial.print("Menghubungkan ke WiFi");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Terhubung!");
  // ------------------------------------------

  client.setInsecure(); 
  dht.begin();
  
  // Inisialisasi LCD
  lcd.begin(16, 2);  
  lcd.backlight(); 
  lcd.setCursor(0, 0);
  lcd.print("IoT Monitoring");
  lcd.setCursor(0, 1);
  lcd.print("Cabai ESP8266");
  delay(2000);
}

void loop() {
  // 1. Baca Sensor
  float suhu = dht.readTemperature();
  float kelembaban = dht.readHumidity();
  int soilMoisture = analogRead(SOIL_SENSOR); // Membaca nilai asli (0-1024)
  
  // Kalibrasi kelembaban tanah untuk ditampilkan di LCD (0-100%)
  int soilPercent = map(soilMoisture, 1023, 300, 0, 100); 
  if(soilPercent > 100) soilPercent = 100;
  if(soilPercent < 0) soilPercent = 0;

  // 2. Logika Status Tanah (berdasarkan nilai analog)
  String statusTanah = "";
  if (soilMoisture >= 0 && soilMoisture <= 341) {
    statusTanah = "💧 Tanah basah";
  } else if (soilMoisture >= 342 && soilMoisture <= 682) {
    statusTanah = "✅ Kelembapan Tanah Normal";
  } else {
    statusTanah = "⚠️ Tanah terlalu kering dan butuh penyiraman";
  }

  // 3. Menampilkan data ke LCD bagian 1
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu: " + String(suhu) + "C");
  lcd.setCursor(0, 1);
  lcd.print("Tanah: " + String(soilPercent) + "%");
  delay(2000);

  // 4. Menyusun dan Mengirim Pesan Telegram sesuai format Anda
  String pesan = "📊 *Laporan Sensor Terkini*\n\n";
  pesan += "🌡️ Suhu Udara: " + String(suhu) + " °C\n";
  pesan += "🌱 Nilai Tanah: " + String(soilMoisture) + "\n";
  pesan += "💬 Status: " + statusTanah;

  bot.sendMessage(CHAT_ID, pesan, "Markdown");
  
  delay(30000); // Kirim data setiap 30 detik
}