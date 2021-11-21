

DynamicJsonDocument doc(4000);

String yandexGetUrl() {
  int limit = 5;
  int offset = random(YA_FOLDER_SIZE - limit);
  int item = random(limit);

  String url = "https://cloud-api.yandex.net/v1/disk/resources?path=" 
  + String(YA_FOLDER) + "&preview_crop=true&fields=_embedded.items.media_type%2C_embedded.items.preview&limit=" 
  + String(limit) + "&offset=" + String(offset) + "&preview_crop=false&preview_size=" + String(YA_IMAGE_SIZE);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(url);
    String data = getImagesChunk(url);
    Serial.println(data);

    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return "";
    }
    return doc["_embedded"]["items"][item]["preview"];
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  return "";
}

String getImagesChunk(String url) {
  WiFiClient client;
  HTTPClient http;

  http.begin(url);
  http.addHeader("Accept", "application/json");
  http.addHeader("Authorization", "OAuth " + String(YA_TOKEN));
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.setTimeout(30000);
  http.setReuse(true);
  //http.setConnectTimeout(10000);

  // Send HTTP POST request
  int httpResponseCode = -1;
  while(httpResponseCode<0){
      //I don't know why, but first requests do not work
      httpResponseCode = http.GET();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      if (httpResponseCode > 0) {
        break;
      }
      delay(1000);
  }

  String payload = http.getString();
  http.end();
  return payload;
}

bool gdownloadImageToFile(String url, String filename) {

  // If it exists then no need to fetch it
  if (SPIFFS.exists(filename) == true) {
    Serial.println("Found " + filename);
    return 0;
  }

  Serial.println("Downloading "  + filename + " from " + url);

  // Check WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");

    http.begin(url);
    http.addHeader("Authorization", "OAuth " + String(YA_TOKEN));
    
    int httpCode = http.GET();
    if (httpCode > 0) {
      fs::File f = SPIFFS.open(filename, "w+");
      if (!f) {
        Serial.println("file open failed");
        return 0;
      }
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // File found at server
      if (httpCode == HTTP_CODE_OK) {
        int total = http.getSize();
        int len = total;
        uint8_t buff[128] = { 0 };
        WiFiClient * stream = http.getStreamPtr();
        while (http.connected() && (len > 0 || len == -1)) {
          size_t size = stream->available();
          if (size) {
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            f.write(buff, c);
            if (len > 0) {
              len -= c;
            }
          }
          yield();
        }
        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
      }
      f.close();
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return 1; // File was fetched from web
}
