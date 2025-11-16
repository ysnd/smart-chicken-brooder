#include "CTBot.h" 
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float h, t;
#define LIGHT_ON_CALLBACK  "lightON"  // callback data sent when "lampu ON" button is pressed
#define LIGHT_OFF_CALLBACK "lightOFF" // callback data sent when "lampu OFF" button is pressed
#define MONITOR_RU "MONITOR" // callback data sent when "monitor ruangan" button is pressed
#define SERV_MAKAN_CALLBACK "servomakan" // callback data sent when "beri pakan" button is pressed
#define MON_LIGHT "MonitorLAMP" // callback data sent when "status lampu" button is pressed
Servo servol;
static const int servoPin = 0;
LiquidCrystal_I2C lcd(0x27, 14, 12);
CTBot myBot;
CTBotInlineKeyboard myKbd;  // custom inline keyboard object helper

String ssid = "";     // REPLACE mySSID WITH YOUR WIFI SSID
String pass = ""; // REPLACE myPassword YOUR WIFI PASSWORD, IF ANY
String token = "";   // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
uint8_t led = 16;  
uint8_t lampu_status;

void setup() {
  // initialize the Serial
  Serial.begin(9600);
  lcd.begin();        //Memulai lcd     
  lcd.backlight();    //menghidupkan backlight lcd
  Serial.println("Starting TelegramBot...");
  lcd.setCursor(0,0);
  lcd.print("  Memulai  bot  ");
  lcd.setCursor(0,1);
  lcd.print("    Telegram    ");
  delay(800);
  lcd.clear();
  
  // connect the ESP8266 to the desired access point
  myBot.wifiConnect(ssid, pass);
  Serial.print("Menghubungkan WiFi ke SSID");
  lcd.setCursor(0,0);
  lcd.print(" Menghubungkan ");
  lcd.setCursor(0,1);
  lcd.print("      WiFi      ");
  delay(800);
  lcd.clear();
  
  // set the telegram bot token
  myBot.setTelegramToken(token);
  Serial.println("Adapter TCP-IP berhasil");
  lcd.setCursor(0,0);
  lcd.println(" Adapter  TCP-IP ");
  lcd.setCursor(0,1);
  lcd.print(" diinisialisasi.");
  delay(800);
  lcd.clear();
  
  //tes koneksi dengan hotspot
  if (myBot.testConnection()) {
    Serial.println("Koneksi bagus");
    lcd.setCursor(0,0);
    lcd.print(" Koneksi  bagus ");
    delay(800);
    lcd.clear();
  

  } else {
    Serial.println("Koneksi buruk");
    lcd.setCursor(0,0);
    lcd.print(" Koneksi  buruk ");
    delay(800);
    lcd.clear();
  }
    servol.attach(servoPin);
  // set the pin connected to the LED to act as output pin
    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH); // turn off the led (inverted logic!)
    lampu_status = 1;
    dht.begin();

  // inline keyboard customization
  // add a query button to the first row of the inline keyboard
  myKbd.addButton("Hidupkan lampu", LIGHT_ON_CALLBACK, CTBotKeyboardButtonQuery);
  // add another query button to the first row of the inline keyboard
  myKbd.addButton("Matikan lampu", LIGHT_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  // add a new empty button row
  myKbd.addRow();
  myKbd.addButton("Monitoring ruangan", MONITOR_RU, CTBotKeyboardButtonQuery);
  myKbd.addButton("Status lampu", MON_LIGHT, CTBotKeyboardButtonQuery);
  // add a URL button to the second row of the inline keyboard
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
        // the user is asking to show the inline keyboard --> show it
        myBot.sendMessage(msg.sender.id, "Perintah yang tersedia :", myKbd);
      }
      else {
        // the user write anithing else --> show a hint message
        myBot.sendMessage(msg.sender.id, "Selamat datang, untuk memulai silahkan klik -> /start'");
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      // received a callback query message
      if (msg.callbackQueryData.equals(LIGHT_ON_CALLBACK)) {
        // pushed "LIGHT ON" button...
        lampu_status = 0;
        // terminate the callback with an alert message
        myBot.endQuery(msg.callbackQueryID, "Lampu telah dihidupkan", true);
      } else if (msg.callbackQueryData.equals(LIGHT_OFF_CALLBACK)) {
        // pushed "LIGHT OFF" button...
        lampu_status = 1;
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "Lampu telah dimatikan", true);
      } else if (msg.callbackQueryData.equals(MON_LIGHT)) {
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
        servol.write(180);
        delay(1000);
        servol.write(0);
        delay(1000);
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
    
  //Jika suhu lebih dari 30
  if (t > 34) {
    myBot.sendMessage(msg.sender.id, (String)"Suhu Terlalu Panas!\nSuhu : " + t + " °C"); 
    digitalWrite(led, HIGH);
    lampu_status = 1;
    lcd.setCursor(0,1);
    lcd.print("SUHU PANAS!!!!!!");
    delay(1000);
    
    }

  else if (t < 25) {
    myBot.sendMessage(msg.sender.id, (String)"Suhu Dingin!\nSuhu : " + t + " °C"); 
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
