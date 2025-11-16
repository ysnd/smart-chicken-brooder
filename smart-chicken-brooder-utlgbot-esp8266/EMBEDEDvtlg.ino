#include <string.h>
#include <Arduino.h>
#include WiFi.h
#include <LiquidCrystal_I2C.h>
// Custom libraries
#include <utlgbotlib.h>
//Inisialisasi Motor Servo
#include <Servo.h>
static const int servoPin = 0;
Servo servo1;
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float h, t;
#define WIFI_SSID ""
#define WIFI_PASS ""

#define MAX_CONN_FAIL 50
#define MAX_LENGTH_WIFI_SSID 31
#define MAX_LENGTH_WIFI_PASS 63

#define TLG_TOKEN "" 

#define DEBUG_LEVEL_UTLGBOT 0

#define PIN_RELAY 16
LiquidCrystal_I2C lcd(0x27, 14, 12); //Alamat I2C 

const char TEKS_MULAI[] =
  "Hallo bangsatt, saya robot ciken auau.\n"
  "\n"
  "Klik /Menu untuk cara menggunakan saya.";

char buff[100];
boolean state_t, state_h;

const char TEKS_MENU[] =
  "Perintah yang tersedia:\n"
  "\n"
  "/Mulai - Memulai bot.\n"
  "\n"
  "/Menu - Menampilkan perintah yang tersedia.\n"
  "\n"
  "/HidupkanLampu - Menghidupkan Lampu.\n"
  "\n"
  "/MatikanLampu - Mematikan Lampu.\n"
  "\n"
  "/StatusLampu - Melihat status lampu saat ini.\n"
  "\n"
  "/Monitoring - Melihat suhu & kelembaban saat ini.\n"
  "\n"
  "/BeriPakan - Memberikan pakan.\n"
  "\n";
  

void wifi_init_stat(void);
bool wifi_handle_connection(void);


// Create Bot object
uTLGBot Bot(TLG_TOKEN);

// RELAY status
uint8_t relay_status;

void setup(void)
{
 //Untuk memulai motor Servo
  servo1.attach(servoPin);
  lcd.begin();       //Mulai LCD26
  Bot.set_debug(DEBUG_LEVEL_UTLGBOT);
  Serial.begin(9600);
  digitalWrite(PIN_RELAY, HIGH);
  pinMode(PIN_RELAY, OUTPUT);
  relay_status = 1;

  wifi_init_stat();

  Serial.println("Menunggu koneksi WIFI.");
  lcd.setCursor(0,0);     //Koordinat text47
  lcd.print("Menunggu koneksi");
  lcd.setCursor(0,0);
  lcd.print("    wifi");
  delay(500);
  lcd.clear();
  while (!wifi_handle_connection())
  {
    Serial.print(".");
    delay(500);
  }
  dht.begin();
  Bot.getMe();
}

void loop()
{
  if (!wifi_handle_connection())
  {
    // Wait 100ms and check again
    delay(100);
    return;
  }
  t = dht.readTemperature();  
  h = dht.readHumidity();
  lcd.setCursor(0,0);     //Koordinat text47
  lcd.print("Suhu: ");
  lcd.print(t);           //Tampilkan suhu49
  lcd.print("*C");
  lcd.setCursor(0,1); 
   lcd.print("Kelembaban: ");
  lcd.print(h);           //Tampilkan suhu49
    lcd.print("%Rh");
  // Check for Bot received messages
  
  while (Bot.getUpdates())
  {
    Serial.println("Pesan diterima:");
    Serial.println(Bot.received_msg.text);
    Serial.println(Bot.received_msg.chat.id);

    if (strncmp(Bot.received_msg.text, "/Mulai", strlen("/Mulai")) == 0)
    {
      Bot.sendMessage(Bot.received_msg.chat.id, TEKS_MULAI);
    }
    else if (strncmp(Bot.received_msg.text, "/start", strlen("/start")) == 0)
    {
      Bot.sendMessage(Bot.received_msg.chat.id, TEKS_MULAI);
    }

    else if (strncmp(Bot.received_msg.text, "/Menu", strlen("/Menu")) == 0)
    {
      Bot.sendMessage(Bot.received_msg.chat.id, TEKS_MENU);
    }

    else if (strncmp(Bot.received_msg.text, "/HidupkanLampu", strlen("/HidupkanLampu")) == 0)
    {
      relay_status = 0;
      Serial.println("Perintah menghidupkan lampu diterima.");
      Serial.println("Menghidupkan lampu.");

      Bot.sendMessage(Bot.received_msg.chat.id, "Lampu telah hidup.");
      lcd.setCursor(0,0);     //Koordinat text47
      lcd.print("Lampu Hidup!.");
    }

    else if (strncmp(Bot.received_msg.text, "/MatikanLampu", strlen("/MatikanLampu")) == 0)
    {
      relay_status = 1;
      // Show command reception through Serial
      Serial.println("Perintah mematikan lampu diterima.");
      Serial.println("Mematikan lampu.");

      // Send a Telegram message to notify that the RELAY has been turned off
      Bot.sendMessage(Bot.received_msg.chat.id, "Lampu telah mati.");
       lcd.setCursor(0,0);     //Koordinat text47
      lcd.print("Lampu Mati!.");
    }

    // If /RELAYstatus command was received
    else if (strncmp(Bot.received_msg.text, "/StatusLampu", strlen("/StatusLampu")) == 0)
    {
      // Send a Telegram message to notify actual RELAY status
      if (relay_status)
        Bot.sendMessage(Bot.received_msg.chat.id, "Lampu mati.");
      else
        Bot.sendMessage(Bot.received_msg.chat.id, "Lampu hidup.");
    }
    //servo pakan
    else if (strncmp(Bot.received_msg.text, "/BeriPakan", strlen("/BeriPakan")) == 0) {
        Bot.sendMessage(Bot.received_msg.chat.id, "Pakan telah diberi", "");
        servo1.write(180);
        delay(1000);
        servo1.write(0);
        delay(1000);
         lcd.setCursor(0,0);     //Koordinat text47
      lcd.print("makanan diberi!.");
    }     
    else if (strncmp(Bot.received_msg.text, "/Monitoring", strlen("/Monitoring")) == 0)
    {
      t = dht.readTemperature();
      h = dht.readHumidity();
      if (isnan(h) || isnan(t)) {
        Serial.println(F("Gagal membaca data dari sensor DHT!"));
        return;
      }
      String msg = "Suhu :";
      msg += t;
      msg += " °C\n";
      msg += "Kelembaban :";
      msg += h;
      msg += " %Rh\n";

      msg.toCharArray(buff, 100);
      Bot.sendMessage(Bot.received_msg.chat.id, buff);
    }

    digitalWrite(PIN_RELAY, relay_status);
    // Feed the Watchdog
    yield();
  }

  h = dht.readHumidity();
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Gagal membaca data dari sensor DHT!"));
    return;
  }

  if (t > 28 && state_t == 0) {
    state_t = 1;
    String msg = "Status Suhu :";
    msg += t;
    msg += " °C\n";
    msg += "Hati Hati Panas.";

    msg.toCharArray(buff, 100);
    Bot.sendMessage("735823292", buff);
  }
  else if (t <= 37) {
    state_t = 0;

  }

  if (h < 60 && state_h == 0) {
    state_h = 1;
    String msg = "Status Kelembaban :";
    msg += h;
    msg += " %Rh\n";
    msg += "Hati Hati Kering.";

    msg.toCharArray(buff, 100);
    Bot.sendMessage("735823292", buff);
  }
  else if (h >= 60) {
    state_h = 0;
  }
  Serial.print(F("Kelembaban: "));
  Serial.print(h);
  Serial.print(F("%  Temperatur: "));
  Serial.print(t);
  Serial.println(F("°C "));
  delay(1000);
}


// Init WiFi interface
void wifi_init_stat(void)
{
  Serial.println("Menginisialisasi adapter TCP-IP...");
  Serial.print("Menghubungkan WiFi ke SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println("Adapter TCP-IP berhasil diinisialisasi.");
}


bool wifi_handle_connection(void)
{
  static bool wifi_connected = false;

  // Device is not connected
  if (WiFi.status() != WL_CONNECTED)
  {
    // Was connected
    if (wifi_connected)
    {
      Serial.println("WiFi terputus.");
      wifi_connected = false;
    }

    return false;
  }
  // Device connected
  else
  {
    // Wasn't connected
    if (!wifi_connected)
    {
      Serial.println("");
      Serial.println("WiFi tersambung");
      Serial.print("Alamat IP : ");
      Serial.println(WiFi.localIP());

      wifi_connected = true;
    }

    return true;
  }
}
