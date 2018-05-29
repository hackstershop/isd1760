

#include <ISD1700.h>
ISD1700 chip(10); // Initialize chipcorder with
// SS at Arduino's digital pin 10

int apc = 0;
int vol = 5; //volume 0=MAX, 7=min
int startAddr = 0x10;
int endAddr = 0x1EF;
int inInt = 0;
int SRZERO = 0;
long INTVAL = 0; //　インターバル用変数

int RDY_LED_PIN = 2;    //LEDピン番号
int PTT_IN_PIN = 4;     // MIC側PTT
int PTT_OUT_PIN = 5;     // RIG側PTT
int REPT_BT = 6;        // REPT再生ボタン
int REC_PIN = 7;        // 録音ボタン
int ERS_PIN = 8;        // 消去ボタン
int PLAY_ONE_BT = 9;    // 1回再生ボタン
int INTVAL_PIN = 14;    //インターバルアナログピン

int RDY_DETECT_STAT = 0; //上記のステータス
int PTT_IN_STAT =  1; //上記のステータス
int PTT_OUT_STAT = 0; //上記のステータス
int REPT_BT_STA = 1; //プルアップPTT
int REPT_FLAG = 0;
long wait_n = 0;
long wait_n_max = 35000;
int REC_STAT = 0;
int PLAY_ONE_BT_STA = 0;
int  ONE_FLAG = 0;

////////////////////////////////　初回セットアップ ////////////////////////////////

void setup()
{
  pinMode (RDY_LED_PIN, OUTPUT);      // LED表示
  pinMode(PTT_IN_PIN, INPUT_PULLUP);  // MIC側PTT
  pinMode (PTT_OUT_PIN, OUTPUT);      // RIG側PTT
  pinMode(REPT_BT, INPUT_PULLUP);     // リピート再生用ボタン
  pinMode(REC_PIN, INPUT_PULLUP);     // 録音ボタン
  pinMode(ERS_PIN, INPUT_PULLUP);     // 録音ボタン
  pinMode(PLAY_ONE_BT, INPUT_PULLUP);     // リピート再生用ボタン

  apc = apc | vol; //D0, D1, D2
  //apc = apc | 0x8; //D3 comment to disable output monitor during record
  apc = apc | 0x50; // D4& D6 select MIC REC
  //apc = apc | 0x00; // D4& D6 select AnaIn REC
  //apc = apc | 0x10; // D4& D6 select MIC + AnaIn REC
  apc = apc | 0x80; // D7 AUX ON, comment enable AUD
  apc = apc | 0x100; // D8 SPK OFF, comment enable SPK
  //apc = apc | 0x200; // D9 Analog OUT OFF, comment enable Analog OUT
  //apc = apc | 0x400; // D10 vAlert OFF, comment enable vAlert
  apc = apc | 0x800; // D11 EOM ON, comment disable EOM

  Serial.begin(9600);
  Serial.println("Sketch is starting up");

  chip.pu(); //  初期化
  delay(500);
  //chip.wr_apc2(0x445); // APコマンド設定、データシート参照 // D7電圧出力
  chip.wr_apc2(0x4C0); // APコマンド設定、データシート参照 // D7電圧出力
  delay(500);
}

void loop()
{
  char c;


  ////////////////////////////////　インターバル ////////////////////////////////

  INTVAL = analogRead(INTVAL_PIN);
  wait_n_max = (INTVAL * 100);

  ////////////////////////////////　録音 ////////////////////////////////

  if (digitalRead(REC_PIN) == 0 && REC_STAT == 0) {
    REC_STAT = 1;
    chip.rec();
    digitalWrite(RDY_LED_PIN, HIGH);
    delay(1000);
  }
  if (digitalRead(REC_PIN) == 0 && REC_STAT == 1) {
    REC_STAT = 0;
    chip.stop();
    digitalWrite(RDY_LED_PIN, LOW);
    delay(1000);
  }

  ////////////////////////////////　消去 ////////////////////////////////

  if (digitalRead(ERS_PIN) == 0) {
    chip.erase();
    delay(100);
    digitalWrite(RDY_LED_PIN, HIGH);
    delay(100);
    digitalWrite(RDY_LED_PIN, LOW);
    chip.erase();
    delay(100);
    digitalWrite(RDY_LED_PIN, HIGH);
    delay(100);
    digitalWrite(RDY_LED_PIN, LOW);
    chip.erase();
    delay(100);
    digitalWrite(RDY_LED_PIN, HIGH);
    delay(100);
    digitalWrite(RDY_LED_PIN, LOW);
    chip.erase();
    delay(100);
  }

  ////////////////////////////////　通常交信PTT ////////////////////////////////

  PTT_IN_STAT = digitalRead(PTT_IN_PIN);         // PTT押されたことを検出
  if (PTT_IN_STAT == 0 && REPT_FLAG == 0) {
    digitalWrite(RDY_LED_PIN, HIGH);
    digitalWrite(PTT_OUT_PIN, HIGH);
  } else {
    if (REPT_FLAG == 0 && REC_STAT == 0 && ONE_FLAG == 0) {
      digitalWrite(RDY_LED_PIN, LOW);
      digitalWrite(PTT_OUT_PIN, LOW);
    }
  }

  ////////////////////////////////　1回再生 ////////////////////////////////

  PLAY_ONE_BT_STA = digitalRead(PLAY_ONE_BT);

  SRZERO = chip.rd_status() & 4;
  //  if (PLAY_ONE_BT_STA == 0 && REPT_FLAG == 0 && SRZERO == 0) {
  if (PLAY_ONE_BT_STA == 0 && REPT_FLAG == 0 && SRZERO == 0) {
    chip.play();
    ONE_FLAG = 1;
    delay(300);
  }
  SRZERO = chip.rd_status() & 4;                                // リピート再生中に送信
  //   if (ONE_FLAG = 1 && REPT_FLAG == 0 && SRZERO == 4 ) { // リピート初回
  if (ONE_FLAG = 1 && REPT_FLAG == 0 && SRZERO == 4 ) { // リピート初回
    digitalWrite(RDY_LED_PIN, HIGH);
    digitalWrite(PTT_OUT_PIN, HIGH);
  }
  if (ONE_FLAG == 1 && SRZERO == 0 && REC_STAT == 0) {
    ONE_FLAG = 0;
    digitalWrite(RDY_LED_PIN, LOW);
    digitalWrite(PTT_OUT_PIN, LOW);
  }
  if (PTT_IN_STAT == 0 && ONE_FLAG == 1) {                  //1回再生中キャンセル　by　PTT
    ONE_FLAG = 0;
    chip.stop();
    delay(500);
  }
  PLAY_ONE_BT_STA = digitalRead(PLAY_ONE_BT);               //1回再生中キャンセル　by　1回再生ボタン
  if (PLAY_ONE_BT_STA == 0 && ONE_FLAG == 1) {
    ONE_FLAG = 0;
    chip.stop();
    delay(500);
  }
  REPT_BT_STA = digitalRead(REPT_BT);                        //1回再生中キャンセル　by　1回再生ボタン 
  if (REPT_BT_STA == 0 && ONE_FLAG == 1) {
    ONE_FLAG = 0;
    chip.stop();
    delay(500);
  }

  ////////////////////////////////　リピート ////////////////////////////////

  REPT_BT_STA = digitalRead(REPT_BT);                         // リピートボタン押されたか
  SRZERO = chip.rd_status() & 4;

  if (REPT_BT_STA == 0 && REPT_FLAG == 0 && SRZERO == 0) {  // リピート初回
    REPT_FLAG = 1;
    chip.play();
    delay(300);
  }
  SRZERO = chip.rd_status() & 4;                                // リピート2回以降
  if (REPT_FLAG == 1 && SRZERO == 0) {                      //リピート中に再生終了
    //   if (wait_n == wait_n_max) {
    if (wait_n >= wait_n_max) {
      chip.play();
      wait_n = 0;
    } else {
      wait_n = wait_n + 1;
      if (millis() % 1000 > 500 ) {                            // リピートの間、LED点滅
        digitalWrite(RDY_LED_PIN, HIGH);
      } else {
        if ( REC_STAT == 0) {
          digitalWrite(RDY_LED_PIN, LOW);
        }
      }
    }
  }
  if (PTT_IN_STAT == 0 && REPT_FLAG == 1) {                  //リピートキャンセル　by　PTT
    REPT_FLAG = 0;
    wait_n = 0;
    chip.stop();
    delay(500);
  }
  REPT_BT_STA = digitalRead(REPT_BT);                       //リピートキャンセル　by　リピートボタン
  if (REPT_BT_STA == 0 && REPT_FLAG == 1) {
    REPT_FLAG = 0;
    wait_n = 0;
    chip.stop();
    delay(500);
  }
  PLAY_ONE_BT_STA = digitalRead(PLAY_ONE_BT);               //リピートキャンセル　by　1回再生ボタン
  if (PLAY_ONE_BT_STA == 0 && REPT_FLAG == 1) {
    REPT_FLAG = 0;
    wait_n = 0;
    chip.stop();
    delay(500);
  }
  SRZERO = chip.rd_status() & 4;                                // リピート再生中に送信
  if (REPT_FLAG == 1 && SRZERO == 4 && REC_STAT == 0) {  // リピート初回
    digitalWrite(RDY_LED_PIN, HIGH);
    digitalWrite(PTT_OUT_PIN, HIGH);
  } else {
    if (REC_STAT == 0 && ONE_FLAG == 0) {
      digitalWrite(RDY_LED_PIN, LOW);
      digitalWrite(PTT_OUT_PIN, LOW);
    }
  }

  ////////////////////////////////　シリアル制御用 ////////////////////////////////

chip.rec();
delay(1800);
chip.stop();
delay(200);
chip.play();
delay(1800);
chip.stop();
chip.g_erase();
delay(200);

} 

int SerialIn() {
  inInt = 0;

  while (Serial.available() <= 0)
  {
    delay(300);
  }
  while (Serial.available()) {
    // get the new byte:
    char c = Serial.read();
    // add it to the inputString:
    inInt = (inInt * 10) + (c - 48);
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (c == '\n') {
      //stringComplete = true;
      Serial.print("stringComplete ");
    }
  }
}

