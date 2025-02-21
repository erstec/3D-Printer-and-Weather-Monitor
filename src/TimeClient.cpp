/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
Modified by David Payne for use in the Scrolling Marquee
*/

#include "TimeClient.h"
#include <Timezone.h>    // https://github.com/JChristensen/Timezone

TimeClient::TimeClient(float utcOffset) {
  myUtcOffset = utcOffset;
}

void TimeClient::updateTime() {
  WiFiClient client;
  
  if (!client.connect(ntpServerName, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: www.google.com\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while(!client.available() && repeatCounter < 10) {
    delay(1000);
    Serial.println(".");
    repeatCounter++;
  }

  String line;

  int size = 0;
  client.setNoDelay(false);
  while(client.connected()) {
    while((size = client.available()) > 0) {
      line = client.readStringUntil('\n');
      line.toUpperCase();
      // example:
      // date: Thu, 19 Nov 2015 20:25:40 GMT
      if (line.startsWith("DATE: ")) {
        Serial.println(line);
        Serial.println(line.substring(23, 25) + ":" + line.substring(26, 28) + ":" +line.substring(29, 31));
        int parsedHours = line.substring(23, 25).toInt();
        int parsedMinutes = line.substring(26, 28).toInt();
        int parsedSeconds = line.substring(29, 31).toInt();
        Serial.println(String(parsedHours) + ":" + String(parsedMinutes) + ":" + String(parsedSeconds));

        int parsedDay = line.substring(11, 13).toInt();
        String parsedMonthStr = line.substring(14, 17);
        int parsedMonth = 1;
        if (parsedMonthStr == "JAN") {
          parsedMonth = 1;
        } else if (parsedMonthStr == "FEB") {
          parsedMonth = 2;
        } else if (parsedMonthStr == "MAR") {
          parsedMonth = 3;
        } else if (parsedMonthStr == "APR") {
          parsedMonth = 4;
        } else if (parsedMonthStr == "MAY") {
          parsedMonth = 5;
        } else if (parsedMonthStr == "JUN") {
          parsedMonth = 6;
        } else if (parsedMonthStr == "JUL") {
          parsedMonth = 7;
        } else if (parsedMonthStr == "AUG") {
          parsedMonth = 8;
        } else if (parsedMonthStr == "SEP") {
          parsedMonth = 9;
        } else if (parsedMonthStr == "OCT") {
          parsedMonth = 10;
        } else if (parsedMonthStr == "NOV") {
          parsedMonth = 11;
        } else if (parsedMonthStr == "DEC") {
          parsedMonth = 12;
        }
        int parsedYear = line.substring(18, 22).toInt();

        Serial.println(String(parsedYear) + "-" + String(parsedMonth) + "-" + String(parsedDay));

        localEpoc = (parsedHours * 60 * 60 + parsedMinutes * 60 + parsedSeconds);
        Serial.println(localEpoc);

        tmElements_t tm;
        tm.Month = parsedMonth;
        tm.Day = parsedDay;
        tm.Year = CalendarYrToTm(parsedYear);
        tm.Hour = parsedHours;
        tm.Minute = parsedMinutes;
        tm.Second = parsedSeconds;

        unixEpoc = makeTime(tm);
        Serial.println(unixEpoc);
        
        localMillisAtUpdate = millis();
        client.stop();
      }
    }
  }

}

void TimeClient::setUtcOffset(float utcOffset) {
	myUtcOffset = utcOffset;
}

String TimeClient::getHours() {
    if (localEpoc == 0) {
      return "--";
    }
    int hours = ((getCurrentEpochWithUtcOffset()  % 86400L) / 3600) % 24;
    if (hours < 10) {
      return "0" + String(hours);
    }
    return String(hours); // print the hour (86400 equals secs per day)

}
String TimeClient::getMinutes() {
    if (localEpoc == 0) {
      return "--";
    }
    int minutes = ((getCurrentEpochWithUtcOffset() % 3600) / 60);
    if (minutes < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      return "0" + String(minutes);
    }
    return String(minutes);
}
String TimeClient::getSeconds() {
    if (localEpoc == 0) {
      return "--";
    }
    int seconds = getCurrentEpochWithUtcOffset() % 60;
    if ( seconds < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      return "0" + String(seconds);
    }
    return String(seconds);
}

String TimeClient::getAmPmHours() {
	int hours = getHours().toInt();
	if (hours >= 13) {
		hours = hours - 12;
	}
	if (hours == 0) {
		hours = 12;
	}
	return String(hours);
}

String TimeClient::getAmPm() {
	int hours = getHours().toInt();
	String ampmValue = "AM";
	if (hours >= 12) {
		ampmValue = "PM";
	}
	return ampmValue;
}

String TimeClient::getYear() {
    if (localEpoc == 0) {
      return "--";
    }
    int _year = year(getCurrentUnixEpoch() + (myUtcOffset * 3600));
    return String(_year);
}

String TimeClient::getMonth() {
    if (localEpoc == 0) {
      return "--";
    }
    int _month = month(getCurrentUnixEpoch() + (myUtcOffset * 3600));
    if (_month < 10) {
      return "0" + String(_month);
    }
    return String(_month);
}

String TimeClient::getDay() {
    if (localEpoc == 0) {
      return "--";
    }
    int _day = day(getCurrentUnixEpoch() + (myUtcOffset * 3600));
    if (_day < 10) {
      return "0" + String(_day);
    }
    return String(_day);
}

String TimeClient::getFormattedDate() {
  return getYear() + "-" + getMonth() + "-" + getDay();
}

String TimeClient::getFormattedTime() {
  return getHours() + ":" + getMinutes() + ":" + getSeconds();
}

String TimeClient::getAmPmFormattedTime() {
	return getAmPmHours() + ":" + getMinutes() + " " + getAmPm();
}

long TimeClient::getCurrentEpoch() {
  return localEpoc + ((millis() - localMillisAtUpdate) / 1000);
}

long TimeClient::getCurrentEpochWithUtcOffset() {
  return (long)round(getCurrentEpoch() + 3600 * myUtcOffset + 86400L) % 86400L;
}

long TimeClient::getCurrentUnixEpoch() {
  return unixEpoc + ((millis() - localMillisAtUpdate) / 1000);
}
