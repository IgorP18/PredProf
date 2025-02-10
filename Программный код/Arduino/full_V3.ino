#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define RST_PIN 5
#define SS_PIN 53
#define in1 6
#define in2 7
#define en 8
#define tp 9
#define ep 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

int prevuid = 0;
int uid = 0;
int uid_out = 0;
int count = 0;
unsigned long password = 0;
bool fl_uid = 0;
bool fl_star = 0;
bool fl_enter = 0;
int v = 0;
unsigned long ontime = 0;
int pvr = 55;
long duration, cm;

//надо дописать получение этих переменных с raspberry//
unsigned long true_password = 1231;                  //
int auto_uid = 162;                                  //
int guest_uid = 20;                                  //
//---------------------------------------------------//

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.init();
  lcd.backlight();
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode (in1, OUTPUT);
  pinMode (in2, OUTPUT);
  pinMode (en, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(6, INPUT_PULLUP); //кнопка завершения ввода пароля
  pinMode(7, INPUT_PULLUP); //кнопка пароля 1
  pinMode(8, INPUT_PULLUP); //кнопка пароля 2
  pinMode(9, INPUT_PULLUP); //кнопка пароля 3
  pinMode(3, INPUT_PULLUP); //концевик ворот 1
  pinMode(4, INPUT_PULLUP); //концевик ворот 2
  pinMode(tp, OUTPUT);
  pinMode(ep, INPUT);
}

void loop() {
  getrfid();
  if (fl_uid == 1) {
    fl_uid = 0; // необходимость обработки карты
    fl_star = 1;
    fl_enter = 1;
    lcd.setCursor(0, 0);
    if (uid == auto_uid) {
      lcd.print("access allowed  ");
      gates();
    } else if (uid == guest_uid) {
      lcd.print("access denied   ");
    } else {
      lcd.print("The card has    ");
      lcd.setCursor(0, 1);
      lcd.print("expired         ");
    }
    delay(2000);
    lcd.clear();
    prevuid = 0;
    fl_star = 0;
    fl_enter = 0;
  } else {
    getpassword();
    if (fl_enter == 0) {
      fl_enter = 1;
      lcd.setCursor(0, 0);
      lcd.print("enter password: ");
    }
    if (digitalRead(6) == 0) {
      lcd.clear();
      fl_enter = 0;
      lcd.setCursor(0, 1);
      if (password == true_password) {
        lcd.print("welcome         ");
      } else {
        lcd.print("wrong password  ");
      }
      while (digitalRead(6) == 0);
      delay(2000);
      lcd.clear();
      fl_enter = 0;
      fl_star = 0;
      password = 0;
      count = 0;
    } else {
      if (fl_star == 0) {
        fl_star = 1;
        lcd.setCursor(0, 1);
        for (int i = 0; i < count; i++) {
          lcd.print("*");
        }
      }
      if (millis() / 1000 % 2 == 0) lcd.cursor();
      else lcd.noCursor();
    }
  }
}

void getrfid() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      uid = mfrc522.uid.uidByte[0];
      if (uid != prevuid) {
        Serial.print("Card UID: ");
        Serial.println(uid);
        prevuid = uid;
        uid_out = uid;
        fl_uid = 1;
        delay(100);
      } else {
        delay(100);
      }
    }
  }
}

void gates() {
  v = 1;
  while (v == 1) {
    lcd.setCursor(0, 1);
    lcd.print("opening         ");
    while (digitalRead(3) == 0) {
      analogWrite(en, pvr);
      digitalWrite(in1, 1);
      digitalWrite(in2, 0);
    }
    stop();
    lcd.setCursor(0, 1);
    lcd.print("come on in      ");
    v = 2;
  }
  while (v == 2) {
    dist();
    if (cm < 15)v = 3;
  }
  while (v == 3) {
    dist();
    if (cm > 15)v = 4;
    delay(1000);
    ontime = millis();
  }
  while (v == 4) {
    lcd.setCursor(0, 1);
    lcd.print("closing         ");
    while (digitalRead(4) == 0) {
      dist();
      if (cm > 15) {
        analogWrite(en, pvr);
        digitalWrite(in1, 0);
        digitalWrite(in2, 1);
      } else {
        stop();
      }
    }
    stop();
    fl_uid = 0;
    fl_star = 0;
    fl_enter = 0;
    v = 0;
  }
}

void stop() {
  analogWrite(en, 0);
  digitalWrite(in1, 0);
  digitalWrite(in2, 0);
}

void dist() {
  digitalWrite(tp, LOW);
  delayMicroseconds(2);
  digitalWrite(tp, HIGH);
  delayMicroseconds(10);
  digitalWrite(tp, LOW);
  duration = pulseIn(ep, HIGH);
  cm = (duration / 57) - 2;
  cm = max(cm, 0);
  cm = min(cm, 100);
  Serial.print(cm);
  Serial.println("cm ");
}

void getpassword() {
  if (digitalRead(7) == 0 || digitalRead(8) == 0 || digitalRead(9) == 0) {
    delay(10);
    if (digitalRead(7) == 0 || digitalRead(8) == 0 || digitalRead(9) == 0) {
      if (digitalRead(7) == 0) {
        password = (password * 10) + 1;
      } else if (digitalRead(8) == 0) {
        password = (password * 10) + 2;
      } else if (digitalRead(9) == 0) {
        password = (password * 10) + 3;
      }
      while (digitalRead(7) == 0 || digitalRead(8) == 0 || digitalRead(9) == 0)delay(10);
      count++;
      fl_star = 0;
    }
  }
}
