#include "CTBot.h" 
#include "WiFi.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float h, t;
#define LAMPU_ON_CALLBACK  "lampuON"
#define LAMPU_OFF_CALLBACK "lampuOFF" 
#define MONITOR_RU "MONITOR" 
#define SERV_MAKAN_CALLBACK "servomakan"
#define MON_LAMPU "MonitorLAMP"
Servo servol;
static const int servoPin = 4;
LiquidCrystal_I2C lcd(0x27, 16, 2);
CTBot myBot;
CTBotInlineKeyboard myKbd; 

String ssid = "";   
String pass = "";
String token = "";
uint8_t led = 2;  
uint8_t lampu_status;

void setup() {
  Serial.begin(9600);
  lcd.begin();       
  lcd.backlight(); 
  Serial.println("Starting TelegramBot...");
  lcd.setCursor(0,0);
  lcd.print("  Memulai  bot  ");
  lcd.setCursor(0,1);
  lcd.print("    Telegram    ");
  delay(800);
  lcd.clear();
  
  myBot.wifiConnect(ssid, pass);
  Serial.println("Menghubungkan WiFi ke SSID");
  lcd.setCursor(0,0);
  lcd.print(" Menghubungkan ");
  lcd.setCursor(0,1);
  lcd.print("      WiFi      ");
  delay(800);
  lcd.clear();
  
  myBot.setTelegramToken(token);
  Serial.println("Menghubungkan ke telegram");
  lcd.setCursor(0,0);
  lcd.println("  Menghubungkan ");
  lcd.setCursor(0,1);
  lcd.print("  Telegram bot.  ");
  delay(800);
  lcd.clear();
  
  if (myBot.testConnection()) {
    Serial.println("Koneksi Telegram bagus");
    lcd.setCursor(0,0);
    lcd.print(" Koneksi bagus ");
    delay(800);
    lcd.clear();
  

  } else {
    Serial.println("Koneksi telegram buruk");
    lcd.setCursor(0,0);
    lcd.print(" Koneksi buruk ");
    delay(800);
    lcd.clear();
  }
    servol.attach(servoPin);

    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH); 
    lampu_status = 1;
    dht.begin();
  // inline keyboard customization
  myKbd.addButton("Hidupkan lampu", LAMPU_ON_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Matikan lampu", LAMPU_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Monitoring ruangan", MONITOR_RU, CTBotKeyboardButtonQuery);
  myKbd.addButton("Status lampu", MON_LAMPU, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Beri pakan", SERV_MAKAN_CALLBACK, CTBotKeyboardButtonQuery);
}

void loop() 
{

  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {
    // check what kind of message I received
    if (msg.messageType == CTBotMessageText) {
      // received a text message
      if (msg.text.equalsIgnoreCase("/start")) {
        myBot.sendMessage(msg.sender.id, "Perintah yang tersedia :", myKbd);
      }
      else {
        myBot.sendMessage(msg.sender.id, "Selamat datang, untuk menampilkan perintah tekan --> /start");
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      // received a callback query message
      if (msg.callbackQueryData.equals(LAMPU_ON_CALLBACK)) {
        lampu_status = 0;
        myBot.endQuery(msg.callbackQueryID, "Lampu telah dihidupkan", true);
      } else if (msg.callbackQueryData.equals(LAMPU_OFF_CALLBACK)) {
        lampu_status = 1;
        myBot.endQuery(msg.callbackQueryID, "Lampu telah dimatikan", true);
      } else if (msg.callbackQueryData.equals(MON_LAMPU)) {
         if (lampu_status)
          myBot.endQuery(msg.callbackQueryID, "Lampu mati", true);
          else 
          myBot.endQuery(msg.callbackQueryID, "Lampu hidup", true);
       } else if (msg.callbackQueryData.equals(MONITOR_RU)) {
        t = dht.readTemperature();
        h = dht.readHumidity();
        if (isnan(h) || isnan(t)) {
        Serial.println(F("Gagal membaca data dari sensor DHT!"));
        return;
      }
      
      myBot.sendMessage(msg.sender.id, (String)"Kondisi saat ini\nSuhu : " + t + " °C\nKelembaban : " + h + " %Rh");
      myBot.endQuery(msg.callbackQueryID, "Hasil terbaca di chat", true);
        }
         else if (msg.callbackQueryData.equals(SERV_MAKAN_CALLBACK)) {
        servol.write(90);
        delay(1000);
        servol.write(0);
        myBot.endQuery(msg.callbackQueryID, "Makanan diberi!.", true);
      }
    }
    digitalWrite(led, lampu_status);
    }      
   
    t = dht.readTemperature();
    h = dht.readHumidity();
    lcd.setCursor(0,0);     
    lcd.print("Suhu: ");
    lcd.print(t);           
    lcd.print("*C");
    lcd.setCursor(0,1); 
    lcd.print("Kelembaban: ");
    lcd.print(h);           
    lcd.print("%Rh");
    
  if (t > 31) {
    myBot.sendMessage(msg.sender.id, (String)"Suhu Terlalu Panas!\nSuhu : " + t + " °C\nLampu dimatikan."); 
    digitalWrite(led, HIGH);
    lampu_status = 1;
    lcd.setCursor(0,1);
    lcd.print("SUHU PANAS!!!!!!");
    delay(1000);
    
    }

  else if (t < 28) {
    myBot.sendMessage(msg.sender.id, (String)"Suhu Dingin!\nSuhu : " + t + " °C\nLampu dihidupkan."); 
    digitalWrite(led, LOW);
    lampu_status = 0;
    lcd.setCursor(0,1);
    lcd.print("SUHU DINGIN!!!!!"); 
    delay(1000);

  }
  else {
  Serial.print(F("Kelembaban: "));
  Serial.print(h);
  Serial.print(F("%  Suhu: "));
  Serial.print(t);
  Serial.println(F("°C "));
  delay(1000);
}
}
