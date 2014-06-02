#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#-------------------------------------------------------------------------------
# Name:     air_counter
# Purpose:   エステーが製造しているエアカウンターというガイガーカウンターと接続して得られた計測値
#           を処理して、統計に利用可能な文字列として出力する。
#           ここでは、Arduinoのシリアル通信データにはtera termによるタイムスタンプが付いている
#           ものと仮定する。
# Memo:      2014-05-20時点でのArduino用スケッチAirCounter_Arduino_pro.inoを用いて
#           得られるログを処理の対象としています。
#           対象となる動作モードはmode_lock_free（ロックフリーモード）です。
#
# Author:     Katsuhiro MORISHITA, Kumamoto National College of Technology. @ 2014
#
# Created:     26/05/2014
# Copyright:   (c) morishita 2013
# Licence:     MIT
# History:      2014/05/16   初作成
#-------------------------------------------------------------------------------
import re
import os
import datetime

# tera termのタイムスタンプ関係
# 文字列例：[Thu May 22 14:40:53.873 2014]
listOfWeekName = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT", "N/A"]
dictOfMonthNameAndNum = {"Jan":1, "Feb":2, "Mar":3, "Apr":4, "May":5, "Jun":6, "Jul":7, "Aug":8, "Sep":9, "Oct":10, "Nov":11, "Dec":12}
time_stamp = re.compile("(?P<day_of_week>\w\w\w) (?P<month>\w\w\w) (?P<day_of_month>\d{1,2}) (?P<hour>\d{1,2}):(?P<min>\d{1,2}):(?P<sec>\d{1,2})([.](?P<microsecond>\d+))? (?P<year>\d{4})")


def getTime(oneDataStr):
    """ 文字列から時刻を抽出して、時刻オブジェクトを返す

    Args:
        oneDataStr: 解析したい文字列
    Return:
        datetime.datetime: 正常な処理が行われると、時刻オブジェクトを返す
        None:              文字列が不正
    """
    m = time_stamp.search(oneDataStr)
    if m != None:
        #print(m.groups())
        year = int(m.group("year"))
        month = m.group("month")
        month = dictOfMonthNameAndNum[month]
        day = int(m.group("day_of_month"))
        hour = int(m.group("hour"))
        minute = int(m.group("min"))
        sec = int(m.group("sec"))
        _microsecond   = m.group('microsecond')                                     # このままでは単位が必ずしもμsにはならないので、以下で単位を変換する
        if _microsecond != None:
            microsecond  = int(int(_microsecond) * 10**(6 - len(_microsecond)))     # datetimeオブジェクトを使うとGNSS分野で有効桁が少々足りないのだが・・・実用上は問題ないと思う
        else:
            microsecond  = 0
        return datetime.datetime(year, month, day, hour, minute, sec, microsecond)
    else:
        return None


# ログを読みだして、整理して別のテキストファイルに結果を保存する
p = re.compile("pulse num.,(?P<pulse_num>\d+),amount of radiation,(?P<amount_of_radiation>\d+[.]\d+),uSv/h")

with open("air counter lf mode 2.txt", "r") as fr, open("result.txt", "w") as fw:
    lines = fr.readlines()
    for line in lines:
        #print(line)
        m = p.search(line)
        if m != None:
            t = getTime(line)
            #print(t)
            fw.write(str(t))
            pulse_num = int(m.group("pulse_num"))
            amount_of_radiation = float(m.group("amount_of_radiation"))
            #print("{0},{1}".format(pulse_num, amount_of_radiation))
            fw.write(",{0},{1}".format(pulse_num, amount_of_radiation))
            fw.write(os.linesep)


