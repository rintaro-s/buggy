/*
参考：
https://zenn.dev/arsaga/articles/e651ce6a4e75e4   サーボ
https://qiita.com/maominionbsk54/items/e5fbdc52f51b11abbea3   超音波
https://akizukidenshi.com/goodsaffix/TB67H450FNG_datasheet_ja_20190401.pdf モタドラ
なお、PWMの信号は4番のVREFピンに入れるらしい byまっつー
*/


#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>
#include <NewPing.h>  // 超音波センサーのライブラリ

// Wi-Fi SSIDとパスワード
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// サーボモーターの設定
Servo servo;
const int servoPin = 13;
int servoPos = 90;  // 初期位置は90度(中央)

// モータードライバー(TB67H450FNG)の設定
const int in1Pin = 27; 
const int in2Pin = 26; 
const int pwmPin = 25; // PWMピン

// 超音波センサーの設定
#define TRIGGER_PIN 33
#define ECHO_PIN 32
#define MAX_DISTANCE 200  // 最大距離(cm)
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// 距離制限
const int distanceThreshold = 10;

// Webサーバーのインスタンス
AsyncWebServer server(80);

// サーボモーターを制御する関数
void controlServo(String direction) {
  if (direction == "left1") {
    servoPos = 60;
  } else if (direction == "left2") {
    servoPos = 30;
  } else if (direction == "right1") {
    servoPos = 120;
  } else if (direction == "right2") {
    servoPos = 150;
  } else {
    servoPos = 90;  // 中央
  }
  servo.write(servoPos);
}

// DCモーターの制御関数
void controlMotor(String direction, String speed) {
  int motorSpeed = (speed == "Fast") ? 255 : (speed == "Slow") ? 150 : 0;

  if (direction == "Front") {
    int distance = sonar.ping_cm();  // 物体との距離を測定
    if (distance == 0 || distance < distanceThreshold) {
      Serial.println("Object too close! Can't move forward.");
      motorSpeed = 0;  // 前進禁止
    } else {
      digitalWrite(in1Pin, HIGH);
      digitalWrite(in2Pin, LOW);  // 正転
    }
  } else if (direction == "Back") {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);  // 逆転
  } else {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);  // ストップ
  }

  // PWM出力
  ledcWrite(pwmPin, motorSpeed); // PWM出力を設定
}

void setup() {
  Serial.begin(115200);
  
  // Wi-Fi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // サーボモーターの初期化
  servo.attach(servoPin);
  controlServo("center");

  // DCモーターの初期化
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  ledcSetup(0, 5000, 8); // PWM設定 (チャンネル0, 周波数5kHz, 分解能8ビット)
  ledcAttachPin(pwmPin, 0); // PWMピンをチャンネル0にアタッチ
  ledcWrite(0, 0);  // 初期速度は0
  
  // Webサーバールートハンドラー
  server.on("/servo", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("direction")) {
      String direction = request->getParam("direction")->value();
      controlServo(direction);
      request->send(200, "text/plain", "Servo direction: " + direction);
    } else {
      request->send(400, "text/plain", "Parameter 'direction' is missing");
    }
  });

  server.on("/motor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("direction") && request->hasParam("speed")) {
      String direction = request->getParam("direction")->value();
      String speed = request->getParam("speed")->value();
      controlMotor(direction, speed);
      request->send(200, "text/plain", "Motor direction: " + direction + ", speed: " + speed);
    } else {
      request->send(400, "text/plain", "Parameters 'direction' or 'speed' are missing");
    }
  });

  // サーバー開始
  server.begin();
}

void loop() {
  // 必要に応じてメインループに処理を追加
}
