#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#-------------------------------------------------------------------------------
# Name:     communication_to_air_counter
# Purpose:   エステーが製造しているエアカウンターというガイガーカウンターと接続して得られた計測値
#           を処理して、統計に利用可能なファイルを出力する。
#           ここでは、AirCounter_Arduino_pro.inoが書き込まれたArduinoがエアカウンターと接続されており、
#           Arduinoは計測値をシリアルでWindowsやMacなどのコンピュータに送信している状況を想定しています。
# Memo:      2014-05-20時点でのArduino用スケッチAirCounter_Arduino_pro.inoを用いて
#           得られる文字列を処理の対象としています。
#           対象となる動作モードはmode_lock_free（ロックフリーモード）です。
# 動作差確認：Mac OS 10.9, Widows 7, Python 3.4.x （確認予定）
#
# Author:     Katsuhiro MORISHITA, Kumamoto National College of Technology. @ 2014
#
# Created:     26/05/2014
# Copyright:   (c) morishita 2013
# Licence:     MIT
# ref:       [1] https://mbed.org/cookbook/Interfacing-with-Python
#            [2] http://www.s12600.net/psy/python/21-2.html
# History:      2014/06/02   初作成
#-------------------------------------------------------------------------------
import serial
import time
import datetime
import re

serdev = 'COM18' # fix for your env. if your computer is Mac, serdev is like '/dev/tty.serialhogehoge'.
p = re.compile("pulse num.,(?P<pulse_num>\d+),amount of radiation,(?P<amount_of_radiation>\d+[.]\d+),uSv/h")

with open("result.txt", "w") as fw:
    s = serial.Serial(serdev, 9600)
    s.timeout = 0.01                # 単位は秒
    buff = ""
    print("-- processing start --")
    while True:
        r = s.read(1)                # 数字は受信バイト数.
        if len(r) != 0:
            buff += r.decode()
            # 正規表現でサーチ、計測値をファイルに保存した後、合致箇所後端以降をbuffへこぴぺ
            m = p.search(line)
            if m != None:
                print(buff[m.start(): m.end()])
                pulse_num = int(m.group("pulse_num"))
                amount_of_radiation = float(m.group("amount_of_radiation"))
                out_str = datetime.datetime.now()
                out_str += ",{0},{1}".format(pulse_num, amount_of_radiation)
                fw.write(out_str + os.linesep)  # save to a text file
                buff = buff[m.end():]
    if s != None:
        s.close()