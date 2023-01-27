#include "Arduino.h"

// long int tz = -28800 // - (8 * 60 * 60)

class TimeData {
  public:
    String day = "...";
    String timeOnly = " 00:00";
    String monthOnly = "...";
    String yearOnly = "00";
    String dayOnly = "00";
    String ampm = "AM";
    long int date = 0;
    long int month = 0;
    long int currYear = 0;
    // Function to convert unix time to
    // Human readable format
    void parseUnixTime(long int seconds, long int tz) {
      // Number of days in month
      // in normal year
      int daysOfMonth[] = {31, 28, 31, 30, 31, 30,
                          31, 31, 30, 31, 30, 31};

      long int sec = seconds + tz;
      long int daysTillNow, extraTime, extraDays,
          index, hours, minutes, flag = 0;

      // Calculate total days unix time T
      daysTillNow = sec / (24 * 60 * 60);
      extraTime = sec % (24 * 60 * 60);
      currYear = 1970;

      // Calculating current year
      while (true) {
      if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0)) {
        if (daysTillNow < 366) {
          break;
        }
        daysTillNow -= 366;
      }
      else {
        if (daysTillNow < 365) {
          break;
        }
        daysTillNow -= 365;
      }
      currYear += 1;
      }
      // Updating extradays because it
      // will give days till previous day
      // and we have include current day
      extraDays = daysTillNow + 1;

      if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0))
      flag = 1;

      // Calculating MONTH and DATE
      month = 0, index = 0;
      if (flag == 1) {
      while (true) {
        if (index == 1) {
          if (extraDays - 29 < 0)
            break;
          month += 1;
          extraDays -= 29;
        }
        else {
          if (extraDays - daysOfMonth[index] < 0) {
            break;
          }
          month += 1;
          extraDays -= daysOfMonth[index];
        }
        index += 1;
      }
      }
      else {
      while (true) {
        if (extraDays - daysOfMonth[index] < 0) {
          break;
        }
        month += 1;
        extraDays -= daysOfMonth[index];
        index += 1;
      }
      }

      // Current Month
      if (extraDays > 0) {
      month += 1;
      date = extraDays;
      }
      else {
      if (month == 2 && flag == 1)
        date = 29;
      else {
        date = daysOfMonth[month - 1];
      }
      }

      // Calculating HH:MM:SS
      hours = extraTime / 3600;
      minutes = (extraTime % 3600) / 60;
      // secondss = (extraTime % 3600) % 60;
      int hour = hours;
      if (hour > 12) {
        ampm = "PM";
      } else {
        ampm = "AM";
      }

      int currentHours = hour > 12 ? hour - 12 : hour;
      char timeBuffer[6];
      sprintf(timeBuffer, " %02d:%02d", currentHours, int(minutes));
      timeOnly = String(timeBuffer);
      Serial.println(timeOnly);
      Serial.println(currYear);

      if (month == 1) { monthOnly = "ENE"; }
      if (month == 2) { monthOnly = "FEB"; }
      if (month == 3) { monthOnly = "MAR"; }
      if (month == 4) { monthOnly = "ABR"; }
      if (month == 5) { monthOnly = "MAY"; }
      if (month == 6) { monthOnly = "JUN"; }
      if (month == 7) { monthOnly = "JUL"; }
      if (month == 8) { monthOnly = "AGO"; }
      if (month == 9) { monthOnly = "SEP"; }
      if (month == 10) { monthOnly = "OCT"; }
      if (month == 11) { monthOnly = "NOV"; }
      if (month == 12) { monthOnly = "DIC"; }

      char dayBuffer[6];
      sprintf(dayBuffer, "%02d", int(date));
      dayOnly = String(dayBuffer);

      String year = String(currYear);
      yearOnly = year.substring(2, 4);

      dayOfTheWeek(date, month, currYear);
      Serial.println(day);
    }

  private:
    void dayOfTheWeek(int _day, int _month, int _year) {
      String names[7]{"DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"};
      int total = 0;
      for (int i = 1970; i < _year; ++i) {
        if (isLeap(i)) {
          total += 366;
        }
        else {
          total += 365;
        }
      }
      for (int i = 1; i < _month; ++i) {
        switch (i) {
        case 1:
          total += 31;
          break;
        case 2:
          total += (isLeap(_year) ? 29 : 28);
          break;
        case 3:
          total += 31;
          break;
        case 4:
          total += 30;
          break;
        case 5:
          total += 31;
          break;
        case 6:
          total += 30;
          break;
        case 7:
          total += 31;
          break;
        case 8:
          total += 31;
          break;
        case 9:
          total += 30;
          break;
        case 10:
          total += 31;
          break;
        case 11:
          total += 30;
          break;
        }
      }
      day = names[(4 + total + _day - 1) % 7];
    }
    bool isLeap(int _year) {
      if (_year % 400 == 0)
        return true;
      if (_year % 100 == 0)
        return false;
      if (_year % 4 == 0)
        return true;
      return false;
    }
};
