/*
  XE THONG MINH - Arduino Uno R3 + Shield L293D V1 + HC-05 + HC-SR04 + SG90 + IR
  Cac che do: Bluetooth (X), dieu khien remote IR (I), tu dong ne vat can (A).

  Thu vien can cai dat:
    - Adafruit Motor Shield library V1 (AFMotor)
    - Arduino-IRremote 4.x
    - Servo va SoftwareSerial co san trong Arduino IDE

  Dau day:
    Dong co trai M3, dong co phai M4
    Servo SG90 -> SERVO1 / D10
    HC-SR04: Trig A0, Echo A1
    HC-05: TX -> A2, RX <- A3 (nen chia ap 5V xuong 3.3V)
    Mat thu IR: OUT -> D2

  Bluetooth: X/I/A doi che do; F tien, B lui, L/R re, S dung, 0..9 toc do.
  Remote: phim 1/2/3 doi che do; mui ten de lai; OK dung.
*/

#include <AFMotor.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <IRremote.hpp>
#include <ctype.h>

// Chan ket noi
#define PIN_TRIG   A0
#define PIN_ECHO   A1
#define PIN_BT_RX  A2
#define PIN_BT_TX  A3
#define PIN_IR     2
#define PIN_SERVO  10

// Ma command cua remote IR (co the can thay doi theo remote)
#define IR_MODE_BT   0x45
#define IR_MODE_IR   0x46
#define IR_MODE_AUTO 0x47
#define IR_UP        0x18
#define IR_DOWN      0x52
#define IR_LEFT      0x08
#define IR_RIGHT     0x5A
#define IR_OK        0x1C

const int KHOANG_CACH_NE_CM = 20;
const int KHOANG_CACH_DUNG_CM = 15;
const int TOC_DO_TU_DONG = 170;
const unsigned long THOI_GIAN_CHO_IR_MS = 250;
const unsigned long THOI_GIAN_DO_MS = 70;

AF_DCMotor dongCoTrai(3);
AF_DCMotor dongCoPhai(4);
Servo servoQuet;
SoftwareSerial bluetooth(PIN_BT_RX, PIN_BT_TX);

enum CheDo { CHE_DO_BLUETOOTH, CHE_DO_IR, CHE_DO_TU_DONG };
enum BuocTuDong {
  TU_DONG_DO_DUONG,
  TU_DONG_LUI,
  TU_DONG_QUAY_TRAI,
  TU_DONG_DO_TRAI,
  TU_DONG_DO_PHAI,
  TU_DONG_CHO_SERVO_VE_GIUA,
  TU_DONG_RE
};

CheDo cheDoHienTai = CHE_DO_BLUETOOTH;
BuocTuDong buocTuDong = TU_DONG_DO_DUONG;
int tocDoTay = 200;
char lenhDiChuyen = 'S';
unsigned long mocThoiGianBuoc = 0;
unsigned long mocDoKhoangCach = 0;
unsigned long mocNhanIR = 0;
int khoangCachTrai = 0;
int khoangCachPhai = 0;
int thoiGianReMs = 450;

void datTocDo(int tocDo) {
  tocDo = constrain(tocDo, 0, 255);
  dongCoTrai.setSpeed(tocDo);
  dongCoPhai.setSpeed(tocDo);
}

void dungXe() {
  dongCoTrai.run(RELEASE);
  dongCoPhai.run(RELEASE);
}

void chayTien(int tocDo) {
  datTocDo(tocDo);
  dongCoTrai.run(FORWARD);
  dongCoPhai.run(FORWARD);
}

void chayLui(int tocDo) {
  datTocDo(tocDo);
  dongCoTrai.run(BACKWARD);
  dongCoPhai.run(BACKWARD);
}

void quayTrai(int tocDo) {
  datTocDo(tocDo);
  dongCoTrai.run(BACKWARD);
  dongCoPhai.run(FORWARD);
}

void quayPhai(int tocDo) {
  datTocDo(tocDo);
  dongCoTrai.run(FORWARD);
  dongCoPhai.run(BACKWARD);
}

// Tra ve centimet; 0 co nghia la khong nhan duoc phan hoi (loi/mat tin hieu).
int docKhoangCachCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long thoiGianEcho = pulseIn(PIN_ECHO, HIGH, 25000UL);
  if (thoiGianEcho == 0) return 0;
  return (int)(thoiGianEcho / 58UL);
}

void chuyenCheDo(CheDo cheDoMoi) {
  cheDoHienTai = cheDoMoi;
  lenhDiChuyen = 'S';
  buocTuDong = TU_DONG_DO_DUONG;
  dungXe();
  servoQuet.write(90);

  const __FlashStringHelper* tenCheDo = F("KHONG RO");
  if (cheDoMoi == CHE_DO_BLUETOOTH) tenCheDo = F("BLUETOOTH");
  else if (cheDoMoi == CHE_DO_IR) tenCheDo = F("REMOTE IR");
  else if (cheDoMoi == CHE_DO_TU_DONG) tenCheDo = F("TU DONG");

  Serial.print(F("Che do: "));
  Serial.println(tenCheDo);
  bluetooth.print(F("Che do: "));
  bluetooth.println(tenCheDo);
}

void dieuKhienTay(char lenh) {
  lenhDiChuyen = lenh;
  switch (lenh) {
    case 'F': chayTien(tocDoTay); break;
    case 'B': chayLui(tocDoTay); break;
    case 'L': quayTrai(tocDoTay); break;
    case 'R': quayPhai(tocDoTay); break;
    default: lenhDiChuyen = 'S'; dungXe(); break;
  }
}

void xuLyBluetooth() {
  while (bluetooth.available()) {
    int kyTuNhan = bluetooth.read();
    if (kyTuNhan < 0) continue;
    char lenh = (char)toupper((unsigned char)kyTuNhan);

    if (lenh == 'X') { chuyenCheDo(CHE_DO_BLUETOOTH); continue; }
    if (lenh == 'I') { chuyenCheDo(CHE_DO_IR); continue; }
    if (lenh == 'A') { chuyenCheDo(CHE_DO_TU_DONG); continue; }

    // Cho phep lenh S dung khan cap ca khi xe dang tu dong.
    if (lenh == 'S' && cheDoHienTai == CHE_DO_TU_DONG) {
      chuyenCheDo(CHE_DO_BLUETOOTH);
      continue;
    }
    if (cheDoHienTai != CHE_DO_BLUETOOTH) continue;

    if (lenh >= '0' && lenh <= '9') {
      // 0 dung xe; muc 1..9 dieu chinh toc do tu 120 den 255.
      if (lenh == '0') {
        tocDoTay = 0;
        dieuKhienTay('S');
      } else {
        tocDoTay = map(lenh - '0', 1, 9, 120, 255);
        if (lenhDiChuyen != 'S') dieuKhienTay(lenhDiChuyen);
      }
    } else if (lenh == 'F' || lenh == 'B' || lenh == 'L' || lenh == 'R' || lenh == 'S') {
      dieuKhienTay(lenh);
    }
  }
}

void xuLyIR() {
  if (!IrReceiver.decode()) {
    if (cheDoHienTai == CHE_DO_IR && lenhDiChuyen != 'S' &&
        millis() - mocNhanIR > THOI_GIAN_CHO_IR_MS) {
      dieuKhienTay('S');
    }
    return;
  }

  bool laLapLai = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) != 0;
  uint8_t maLenh = IrReceiver.decodedIRData.command;
  IrReceiver.resume();

  if (laLapLai) {
    mocNhanIR = millis();
    return;
  }

  Serial.print(F("Ma IR: 0x"));
  Serial.println(maLenh, HEX);

  if (maLenh == IR_MODE_BT) { chuyenCheDo(CHE_DO_BLUETOOTH); return; }
  if (maLenh == IR_MODE_IR) { chuyenCheDo(CHE_DO_IR); return; }
  if (maLenh == IR_MODE_AUTO) { chuyenCheDo(CHE_DO_TU_DONG); return; }

  if (cheDoHienTai == CHE_DO_TU_DONG && maLenh == IR_OK) {
    chuyenCheDo(CHE_DO_IR);
    return;
  }
  if (cheDoHienTai != CHE_DO_IR) return;

  mocNhanIR = millis();
  switch (maLenh) {
    case IR_UP: dieuKhienTay('F'); break;
    case IR_DOWN: dieuKhienTay('B'); break;
    case IR_LEFT: dieuKhienTay('L'); break;
    case IR_RIGHT: dieuKhienTay('R'); break;
    case IR_OK: dieuKhienTay('S'); break;
  }
}

void kiemTraAnToanLaiTay() {
  if (cheDoHienTai != CHE_DO_BLUETOOTH && cheDoHienTai != CHE_DO_IR) return;
  if (lenhDiChuyen != 'F' || millis() - mocDoKhoangCach < THOI_GIAN_DO_MS) return;
  mocDoKhoangCach = millis();

  int khoangCach = docKhoangCachCm();
  // Mat tin hieu cam bien cung dung xe de an toan, khong coi la duong trong.
  if (khoangCach == 0 || khoangCach <= KHOANG_CACH_DUNG_CM) {
    dieuKhienTay('S');
    Serial.println(F("Canh bao: vat can hoac mat tin hieu, da dung xe."));
    bluetooth.println(F("Canh bao: vat can/mat tin hieu, da dung xe."));
  }
}

bool hetThoiGianBuoc(unsigned long thoiLuongMs) {
  return millis() - mocThoiGianBuoc >= thoiLuongMs;
}

void batDauBuoc(BuocTuDong buocMoi) {
  buocTuDong = buocMoi;
  mocThoiGianBuoc = millis();
}

// May trang thai khong dung delay dai; Bluetooth va IR van duoc xu ly lien tuc.
void xuLyTuDong() {
  unsigned long hienTai = millis();

  switch (buocTuDong) {
    case TU_DONG_DO_DUONG: {
      if (hienTai - mocDoKhoangCach < THOI_GIAN_DO_MS) return;
      mocDoKhoangCach = hienTai;
      int khoangCach = docKhoangCachCm();
      if (khoangCach == 0) {
        dungXe(); // Cam bien khong tra loi: dung an toan
        Serial.println(F("Khong co phan hoi tu HC-SR04; xe da dung."));
      } else if (khoangCach <= KHOANG_CACH_NE_CM) {
        dungXe();
        chayLui(TOC_DO_TU_DONG);
        batDauBuoc(TU_DONG_LUI);
      } else {
        chayTien(TOC_DO_TU_DONG);
      }
      break;
    }

    case TU_DONG_LUI:
      if (hetThoiGianBuoc(300)) {
        dungXe();
        servoQuet.write(160);
        batDauBuoc(TU_DONG_QUAY_TRAI);
      }
      break;

    case TU_DONG_QUAY_TRAI:
      if (hetThoiGianBuoc(450)) {
        khoangCachTrai = docKhoangCachCm();
        servoQuet.write(20);
        batDauBuoc(TU_DONG_DO_TRAI);
      }
      break;

    case TU_DONG_DO_TRAI:
      if (hetThoiGianBuoc(600)) {
        khoangCachPhai = docKhoangCachCm();
        servoQuet.write(90);
        batDauBuoc(TU_DONG_CHO_SERVO_VE_GIUA);
      }
      break;

    case TU_DONG_CHO_SERVO_VE_GIUA:
      if (hetThoiGianBuoc(300)) {
        Serial.print(F("Trai: ")); Serial.print(khoangCachTrai);
        Serial.print(F(" cm | Phai: ")); Serial.println(khoangCachPhai);

        // Mat tin hieu khi quet cung duoc xem nhu huong bi chan.
        bool traiBiChan = khoangCachTrai == 0 || khoangCachTrai < 15;
        bool phaiBiChan = khoangCachPhai == 0 || khoangCachPhai < 15;
        if (traiBiChan && phaiBiChan) {
          thoiGianReMs = 900;
          quayPhai(TOC_DO_TU_DONG);
        } else if (!traiBiChan && (phaiBiChan || khoangCachTrai > khoangCachPhai)) {
          thoiGianReMs = 450;
          quayTrai(TOC_DO_TU_DONG);
        } else {
          thoiGianReMs = 450;
          quayPhai(TOC_DO_TU_DONG);
        }
        batDauBuoc(TU_DONG_RE);
      }
      break;

    case TU_DONG_RE:
      if (hetThoiGianBuoc((unsigned long)thoiGianReMs)) {
        dungXe();
        batDauBuoc(TU_DONG_DO_DUONG);
        mocDoKhoangCach = 0;
      }
      break;
  }
}

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  servoQuet.attach(PIN_SERVO);
  servoQuet.write(90);
  IrReceiver.begin(PIN_IR, DISABLE_LED_FEEDBACK);

  datTocDo(tocDoTay);
  dungXe();
  Serial.println(F("Xe san sang. Che do: BLUETOOTH"));
}

void loop() {
  xuLyBluetooth();
  xuLyIR();

  if (cheDoHienTai == CHE_DO_TU_DONG) {
    xuLyTuDong();
  } else {
    kiemTraAnToanLaiTay();
  }
}
