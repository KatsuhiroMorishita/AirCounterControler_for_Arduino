/********************************************************
 * Name: AirCounter_Arduino_pro
 * Author: Katsuhiro MORISHITA
 * platform: Arduino UNO
 * purpose: エステーが販売しているエアカウンターというガイガーカウンタ
 *          を使った計測を行う。
 *          観測中に受信したバイナリデータを全てシリアルでPCに出力します。
 *          proはbasicに対して、文字列解析機能が増強されています。
 *          観測だけ行って、観測値が欲しい場合はこちらが便利です。
 *          Windowsならtera termなどのソフトウェアを利用して記録できます。
 *          記録後に、適当なスクリプトで処理するか、processingでグラフ化するか、
 *          サーバに自動アップロード&公開するかはあなたの自由です。
 * Thank you: 本プログラムは広島高専、広島商船高専、北九州高専、
 *            熊本高専、その他多数の高専における研究者の協力の基
 *            作成されました。
 * license: MIT
 * port map:
 *    D6 connect to VBAT port of air counter, remove battery into air-counter
 *    D7 connect to SW port of air counter
 *    D8 connect to Tx port of air counter
 *    D9 connect to Rx port of air counter
 *    D10 port is not available for softserial with AltSoftSerial
 * History: 
 *  2014-05-19 created.
 *  2014-05-20 debug...
 *  2014-05-22 1) add power supply port function.
 *             2) some bugs fixed.
 ********************************************************/
#include <AltSoftSerial.h>             // see URL: 

// serial and other
AltSoftSerial mySerial;                // RX 8, TX 9
long baudrate = 9600;
const int pin_power = 6;
const int pin_sw = 7;

// measurement mode
const char mode_lock = 0x00;
const char mode_lock_free = 0x01;          // 一定の時間の間、観測を続けてその時間内に観測されたパルス数と放射線量を出力
const char mode_communication = 0x02;      // 時間と放射線パルス数を出
char mode = mode_lock_free;

const char measurement_setting_ack_size[3] = {0, 8, 8};
const unsigned int measurement_term = 120u; // 計測時間, mode == mode_lock_freeで有効. エアカウンターEXだと30秒、エアカウンターは120秒。


/**************************************************/
// timeout check class
class TimeOut
{
  private:
    long timeout_time;
  
  public:
    // set timeout time width
    void set_timeout(long timeout)
    {
      this->timeout_time = millis() + timeout;
    }
    // timeout check, true: timeout
    boolean is_timeout()
    {
      if(millis() > this->timeout_time)
        return true;
      else
        return false;
    }
    // constructer
    TimeOut()
    {
      this->timeout_time = 0l;
    }
};


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
  digitalWrite(pin_power, HIGH);
  delay(500);
  Serial.println("-- power on --");
  digitalWrite(pin_sw, HIGH);
  delay(2000);
  digitalWrite(pin_sw, LOW);
  delay(500);
}

// power off a air-counter
void power_off()
{
  Serial.println("-- power off --");
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
  TimeOut to;
  to.set_timeout(10000l);
  int count = 0;
  while(count < 4 && to.is_timeout() == false)
  {
    if (_serial->available())
    {
      Serial.println(_serial->read());
      count += 1;
    }
  }
  if(to.is_timeout())
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
  TimeOut to;
  to.set_timeout(10000l);
  int count = 0;
  while(count < 4 && to.is_timeout() == false)
  {
    if (_serial->available())
    {
      Serial.println(_serial->read());
      count += 1;
    }
  }
  if(to.is_timeout())
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
  
  clear_softserial_buffer(_serial);
  
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
  TimeOut to;
  to.set_timeout(10000l);
  int count = 0;
  while(count < measurement_setting_ack_size[mode] && to.is_timeout() == false)
  {
    if (_serial->available())
    {
      Serial.println(_serial->read());
      count += 1;
    }
  }
  if(to.is_timeout())
    Serial.println("-- time out --");
}


String parse(AltSoftSerial *_serial)
{
  TimeOut to;
  String ans = "";
  
  if(mode == mode_lock_free)
  {
    int _size = 16;
    int buff[_size];
    int i = 0;
    
    while(_serial->available() == false);  // 最初の1 byteはいつ来るかわからないのでポーリングで待つ 
    to.set_timeout(1500l);
    
    // pass 'Z'
    while(to.is_timeout() == false)
    {
      if (_serial->available())
      {
        int c = _serial->read();
        to.set_timeout(1500l);
        if(c == 'Z')
          Serial.print("Z");
        else
        {
          buff[0] = c;
          i += 1;
          break;
        }
      }
    }
    Serial.println("");
    
    // read count num. and amount of radiation
    to.set_timeout(10l);
    while(to.is_timeout() == false)
    {
      if (_serial->available())
      {
        buff[i++] = _serial->read();
        to.set_timeout(10l);
        if(i > _size)
          break;
      }
    }
    // 計算。ビットシフトだとうまく行かなかった。なぜ？
    long pulse_num = (long)buff[0] * 4096l + (long)buff[1] * 256l + (long)buff[2] * 16l + (long)buff[3];
    long amount_of_radiation = (long)buff[8] * 4096l + (long)buff[9] * 256l + (long)buff[10] * 16l + (long)buff[11];
    long amount_of_radiation_upper = amount_of_radiation / 100l; // float型をStringに直接変換できなかったのでこうした
    long amount_of_radiation_mod = amount_of_radiation % 100l;
    ans = "pulse num.," + String(pulse_num, DEC) + ",amount of radiation," +
      String(amount_of_radiation_upper, DEC) +
      ".";
    if((amount_of_radiation_mod / 10l) == 0l)
      ans += "0";
    ans += String(amount_of_radiation_mod, DEC) + ",uSv/h";
  }
  else if(mode == mode_communication)
  {
    int _size = 4;
    int buff[_size];
    int i = 0;
    
    while(_serial->available() == false);  // 最初の1 byteはいつ来るかわからないのでポーリングで待つ 
    to.set_timeout(10l);
    
    while(to.is_timeout() == false)
    {
      if (_serial->available())
      {
        buff[i++] = _serial->read();
        to.set_timeout(10l);
        i %= _size;
      }
    }
    int time = buff[0] * 256 + buff[1];
    int pulse_num = buff[2] * 256 + buff[3];
    ans = "time," + String(time, DEC) + ",pulse num," + String(pulse_num, DEC);
  }
  return ans;
}



// setup Arduino
TimeOut to_loop;
void setup()
{
  pinMode(pin_power, OUTPUT);
  pinMode(pin_sw, OUTPUT);
  
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("program start...");
  
  mySerial.begin(9600);
  
  delay(2000);
  Serial.println("-- setup air counter --");
  power_on();
  send_lockbit(mode, &mySerial);
  delay(500);
  read_lockbit(&mySerial);
  power_off();
  delay(1000);
  power_on();
  delay(1000);
  send_measurement_setting(measurement_term, &mySerial);
  //clear_softserial_buffer(&mySerial);
  Serial.println("-- measurement start --");
  to_loop.set_timeout(3000l);
}

// processing forever
void loop()
{
  if (mySerial.available())
  {
    //Serial.println(mySerial.read());  // for debug
    Serial.println(parse(&mySerial));
    to_loop.set_timeout(3000l);
  }
  if (mySerial.overflow())
    Serial.println("-- over flow --");

  // restart measurment
  if(to_loop.is_timeout())
  {
    Serial.println("-- time out in loop() --");
    if(mode == mode_communication)
    {
      power_off();
      delay(1000);
      power_on();
      delay(1000);
    }
    send_measurement_setting(measurement_term, &mySerial);
    to_loop.set_timeout(3000l);
    Serial.println("-- measurement start --");
  }
}
