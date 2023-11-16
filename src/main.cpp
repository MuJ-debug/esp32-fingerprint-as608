#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#define mySerial Serial2

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id; // 录入指纹占用库的id
void finger_setup(int freq);
void serial_setup(int freq);
void testfingerprint();
uint8_t readnumber(void);


void setup()
{
  serial_setup(9600); // 初始化串口监视器

  finger_setup(57600); // 初始化指纹解锁模块
}

void loop() // run over and over again
{
  Serial.println("Please type in the 1 to test finger");
  id = readnumber();
  if (id == 1)
  {
    testfingerprint();
  }
}

// 初始化串口监视器
void serial_setup(int freq)
{
  Serial.begin(freq);
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor");
}

// 指纹检测模块串口通信初始化函数
void finger_setup(int freq)
{
  // 设置指纹解锁库的串口通信频率
  finger.begin(freq);

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
    {
      delay(1);
    }
  }

  // 像串口监视器回报指纹识别模块信息
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
}

uint8_t readnumber(void)
{
  uint8_t num = 0;

  while (num == 0)
  {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

// 录入指纹
uint8_t getFingerprintEnroll()
{

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

// 录入指纹流程
void ready_getFingerEnroll()
{
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0)
  { // ID #0 not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!getFingerprintEnroll())
    ;
}

// 检测指纹
void testfingerprint()
{
  // 打印准备检测提示字符
  Serial.println("Ready to test a fingerprint!");
  Serial.println("Waiting for valid finger to test");

  // 录入指纹图像
  int p = -1;
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  p = -1;
  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    p = 999;
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    break;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    break;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    break;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    break;
  default:
    Serial.println("Unknown error");
    break;
  }

  if (p == 999)
  {
    // 在指纹库中迅速搜索指纹
    Serial.println("searching......");
    p = finger.fingerFastSearch();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Successful! find the match point");
      break;
    case FINGERPRINT_NOTFOUND:
      Serial.println("no match find, try again");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    }
  }
}