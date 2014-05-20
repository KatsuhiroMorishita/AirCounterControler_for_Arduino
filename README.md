AirCounterControler_for_Arduino
===============================

This is a Arduino code for to control a Air-Counter. Air-Counter is a kind of geiger-counter, reasonable.
The information of "Air-Counter" is published in http://www.st-c.co.jp/air-counter/.

To use, you need "AltSoftSerial" lib.
visit and download it from follow URL.
https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html


follow is comment of AirCounter_Arduino_pro.
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
 *    D7 connect to SW port of air counter
 *    D8 connect to Tx port of air counter
 *    D9 connect to Rx port of air counter
 *    D10 port is not available for softserial with AltSoftSerial
 * History: 
 *  2014-05-19 created.
 *  2014-05-20 debug...
 ********************************************************/
