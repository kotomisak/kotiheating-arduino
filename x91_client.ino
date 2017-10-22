
ESP8266WiFiMulti WiFiMulti;
#define USE_SERIAL Serial

Task clientTask(10000, TASK_FOREVER, &connectClient, &scheduler, false);


void connectClient(){
	if (WiFi.status() == WL_CONNECTED) {
		updateTimetableFromServer();
    delay(1000);
		sendStatusToServer();
	}
}

void updateTimetableFromServer() {
		HTTPClient http;
		USE_SERIAL.print("updateTimetableFromServer [HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
        http.begin("http://192.168.1.56:8080/api/kotinode/heating/schedule/raw"); //HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
}

void sendStatusToServer() {
      HTTPClient http;
      USE_SERIAL.print("sendStatusToServer [HTTP] begin...\n");
      http.begin("http://192.168.1.56:8080/api/kotinode/heating/status");
      http.addHeader("Content-Type", "application/json");
      http.addHeader("key", DEVICE_PASSWORD); 
          /*
          // or
          http.begin("http://192.168.1.12/test.html");
          http.setAuthorization("user", "password");

          // or
          http.begin("http://192.168.1.12/test.html");
          http.setAuthorization("dXNlcjpwYXN3b3Jk");
         */
        String payload = "{";
        payload += "\"temperature\":12,";
        payload +=  "\"deviceDateTime\": \"" + deviceDateTime() + "\",";
        payload += "\"timetable\":[";
        for (uint8_t day = 0; day < 7; day++) {
          payload += '[';
          for (uint8_t hour = 0; hour < 24; hour++) {
            payload += logicAutoTemp[day][hour];
            if (hour != 23) {
              payload += ',';
            }
          }
          payload += ']';
          if (day != 6) {
            payload += ',';
          }
        }
        payload += "]}\n";

      USE_SERIAL.println(payload);
      //int httpCode = http.POST("{\"temperature\":\"11\"}");
      int httpCode = http.POST(payload);
      if(httpCode > 0) {
      USE_SERIAL.printf("sendStatusToServer [HTTP] POST... code: %d\n", httpCode);
      if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("sendStatusToServer [HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
            String payload = http.getString();
            USE_SERIAL.println(payload);
        }
      http.end();
}

String deviceDateTime(){
uint32_t wallTime = ntpWallTime();

  String ret = "";
  if (wallTime == TIME_UNKNOWN) {
    ret+="UNKNOWN";
  } else {
    uint8_t dayOfWeek = ((wallTime / 86400) + 4) % 7;
    uint8_t hour = (wallTime % 86400) / (60 * 60);
    char buff[20];
    String hourStr = itoa(hour,buff,10);
    uint8_t minute = (wallTime % 3600) / 60;
    String minuteStr = itoa(minute,buff,10);

    ret+="[hour:"+hourStr+"]";
    ret+="[minute:"+minuteStr+"]";

    switch (dayOfWeek) {
      case 0: ret+="[day:SU]"; break;
      case 1: ret+="[day:MO]"; break;
      case 2: ret+="[day:TU]"; break;
      case 3: ret+="[day:WE]"; break;
      case 4: ret+="[day:TH]"; break;
      case 5: ret+="[day:FR]"; break;
      case 6: ret+="[day:SU]"; break;
    }
    
    
  }
  return ret;
}

