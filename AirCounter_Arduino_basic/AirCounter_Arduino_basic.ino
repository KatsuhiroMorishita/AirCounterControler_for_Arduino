/********************************************************
 * Name: AirCounter_Arduino_basic
 * Author: Katsuhiro MORISHITA
 * platform: Arduino UNO
 * purpose: エステーが販売しているエアカウンターというガイガーカウンタ
 *          を使った計測を行う。
 *          観測中に受信したバイナリデータを全てシリアルでPCに出力します。
 * Thank you: 本プログラムは広島高専、広島商船高専、北九州高専、
 *            熊本高専、その他多数の高専における研究者の協力の基
 *            作成されました。
 * license: MIT
 * port map:
 *    D7 connect to SW port of air counter
 *    D8 connect to Tx port of air counter
 *    D9 connect to Rx port of air counter
 *    D10 port is not available for softserial with AltSoftSerial
 *    TEST1 port of air-counter is shorted to GND.
 * memo:    you can use incorporated battery in air-counter.
 * History: 
 *  2014-05-19 created.
 ********************************************************/
#include <AltSoftSerial.h>             // see URL: 

AltSoftSerial mySerial;                // RX 8, TX 9
long baudrate = 9600;
const int pin_power = 7;

const char mode_lock = 0x00;
const char mode_lock_free = 0x01;          // 一定の時間の間、観測を続けてその時間内に観測された放射線量で占領
const char mode_communication = 0x02;
char mode = mode_communication;

const unsigned int measurement_term = 30u; // 計測時間, mode == mode_lock_freeで有効

//HardwareSerial hoge;


/**************************************************/
// timeout set
long timeout_time = 0l;
void set_timeout(long timeout)
{
  timeout_time = millis() + timeout;
}

// timeout check, true: timeout
boolean is_timeout()
{
  if(millis() > timeout_time)
    return true;
  else
    return false;
}

// clear and reset softserial
void clear_softserial_buffer(AltSoftSerial *_serial)
{
  _serial->begin(baudrate);
  while (_serial->available())
    (void)_serial->read();
}


// power on a air-counter
void power_on()
{
  Serial.println("-- power on --");
  digitalWrite(pin_power, HIGH);
  delay(2000);
  digitalWrite(pin_power, LOW);
}

// power off a air-counter
void power_off()
{
  Serial.println("-- power off --");
  digitalWrite(pin_power, HIGH);
  delay(5000);
  digitalWrite(pin_power, LOW);
}

// send lock bit to a air-counter
void send_lockbit(char code, AltSoftSerial *_serial)
{
  char c1, c2;
  
  c1 = (code >> 4) & 0x0f;
  c2 = code & 0x0f;
  
  Serial.println("-- send lockbit setting --");
  clear_softserial_buffer(_serial);
  _serial->write(0x86);
  _serial->write(0x79);
  _serial->write(c1);
  _serial->write(c2);
  _serial->write(c1);
  _serial->write(c2);
  
  // ack
  Serial.println("-- read lockbit setting ack --");
  set_timeout(10000l);
  int count = 0;
  while(count < 4 && is_timeout() == false)
  {
    if (_serial->available())
    {
      Serial.println(_serial->read());
      count += 1;
    }
  }
  if(is_timeout())
    Serial.println("-- time out --");
  
}

// read lock bit from a air-counter
void read_lockbit(AltSoftSerial *_serial)
{
  Serial.println("-- read lockbit setting --");
  clear_softserial_buffer(_serial);
  _serial->write(0x87);
  _serial->write(0x78);
  
  // ack
  Serial.println("-- read lockbit setting ack --");
  set_timeout(10000l);
  int count = 0;
  while(count < 4 && is_timeout() == false)
  {
    if (_serial->available())
    {
      Serial.println(_serial->read());
      count += 1;
    }
  }
  if(is_timeout())
    Serial.println("-- time out --");
  
}

// send measurement setting to air-counter
void send_measurement_setting(unsigned int term, AltSoftSerial *_serial)
{
  char c1, c2, c3, c4;
  
  c1 = (term >> 12) & 0x0f;
  c2 = (term >>  8) & 0x0f;
  c3 = (term >>  4) & 0x0f;
  c4 = (term >>  0) & 0x0f;
  
  Serial.println("-- measurement setting --");
  Serial.println((int)c1);
  Serial.println((int)c2);
  Serial.println((int)c3);
  Serial.println((int)c4);
  Serial.println("--");
  _serial->write(0x81);
  _serial->write(0x7E);
  _serial->write(c1);
  _serial->write(c2);
  _serial->write(c3);
  _serial->write(c4);
  _serial->write(c1);
  _serial->write(c2);
  _serial->write(c3);
  _serial->write(c4);
  Serial.println("-- measurement setting ack --");
  set_timeout(10000l);
  int count = 0;
  while(count < 8 && is_timeout() == false)
  {
    if (_serial->available())
    {
      Serial.println(_serial->read());
      count += 1;
    }
  }
  if(is_timeout())
    Serial.println("-- time out --");
}

// setup Arduino
void setup()
{
  pinMode(pin_power, OUTPUT);
  
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("program start...");
  
  mySerial.begin(9600);
  
  delay(2000);
  Serial.println("-- setup air counter --");
  power_on();
  delay(2000);
  send_lockbit(mode, &mySerial);
  delay(500);
  read_lockbit(&mySerial);
  power_off();
  delay(5000);
  power_on();
  delay(2000);
  send_measurement_setting(measurement_term, &mySerial);
  clear_softserial_buffer(&mySerial);
  Serial.println("-- measurement start --");
  /**/
}

// processing forever
void loop()
{
  if (mySerial.available())
  {
    Serial.println(mySerial.read());
    set_timeout(3000l);
  }
  if (mySerial.overflow())
    Serial.println("-- over flow --");

  if(is_timeout())
  {
    Serial.println("-- time out in loop() --");
    if(mode == mode_communication)
    {
      power_off();
      delay(2000);
      power_on();
      delay(2000);
    }
    send_measurement_setting(measurement_term, &mySerial);
    set_timeout(3000l);
    Serial.println("-- measurement start --");
  }
}
