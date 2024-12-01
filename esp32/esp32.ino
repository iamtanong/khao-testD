#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"          // Disable brownout problems
#include "soc/rtc_cntl_reg.h" // Disable brownout problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <HTTPClient.h>
#include "Base64.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

// Save image Reference: https://randomnerdtutorials.com/esp32-cam-take-photo-display-web-server/
// Reference: https://github.com/kosaladeshan/IOT-Group-Project/blob/main/esp32/esp32.ino
// Spreadsheet logging: https://randomnerdtutorials.com/esp32-datalogging-google-sheets
// Drive: https://www.niraltek.com/blog/how-to-take-photos-and-upload-it-to-google-drive-using-esp32-cam/

// #define RXpin2 16
// #define TXpin2 17

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FLASH_LED 33      // Built-in LED
#define CAM_STATUS_LED 12 // Status LED

// WiFi
const char *ssid = "tanong_eexe";
const char *password = "123456788";

// Google Sheet
const char *sheetDomain = "script.google.com";
String sheetScript = "/macros/s/AKfycbx86sz94WELWgqFyJaE2OX-7tQaS2VHRrTr8548_aHJ1dXGVwNV9LnHMTaiUZzD58cZ/exec";

// Google Drive
const char *driveDomain = "script.google.com";
String driveScript = "/macros/s/AKfycbzk08OI1yarr6mZUDQcqf_IhE9X0_wTOF7-fAr75kDJQTYdQQFNW6PX_AHy-NfutIBHzQ/exec";
String driveFilename = "filename=ESP32-CAM.jpg";
String mimeType = "&mimetype=image/jpeg";
String driveImage = "&data=";

// Gemini Ai
const char *geminiDomain = "generativelanguage.googleapis.com";
String geminiEndpoint = "/v1beta/models/gemini-1.5-flash:generateContent";
#define GEMINI_API_KEY "AIzaSyCDSya9IDLuavyxr7C3TrooK3GIEJKo2kk"

// NTP server to request epoch time
const char *ntpServer = "pool.ntp.org";
// Variable to save current epoch time
unsigned long epochTime;

// Function pre-declaration
void connectWifi();
void cameraInit();
camera_fb_t *capturePhoto();
void sheet_logging(String binType, bool isFull);
void postDrive(camera_fb_t *fb);
String postGemini(String base64String);
unsigned long getTime();
void tokenStatusCallback(TokenInfo info); // Token Callback function for google Sheet
String urlencode(String str);
String encodeToBase64(camera_fb_t *fb);

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  connectWifi();

  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Initialize OV2640 camera
  cameraInit();

  // Serial 2 : for Arduino communicate
  // Serial2.begin(9600, SERIAL_8N1, RXpin2, TXpin2);
}

void loop()
{
  // if (WiFi.status() != WL_CONNECTED)
  // {
  //   // If not connected to wifi, Try reconnect
  //   Serial.println("WiFi is not connected. Reconnecting...");
  //   connectWifi();
  // }
  // else
  // {
  /*
   * Work Flow
   * 1. Rubbish in
   * 2. Vibration sensor detect (if not detected, push button | if raindrop detected, rubbish_type = WET) || Arduino part
   * 3. Capture photo || ESP32 part
   * 4. Send Photo to Gemini Ai (response as type of rubbish) || ESP32 part
   * 5. Bin rotate to the correct position (PLASTIC, PAPER, WET) || Arduino part
   * 6. Rubbish out (open gate) || Arduino part
   * 7. Check bin height || Arduino part
   * 8. Google sheet log || ESP32 part
   * 9. Send photo to Google Drive || ESP32 part
   * 10. Repeat
   * PS. 8 and 9 can be done at the same time during 5 and 6
   */
  ArduinoOTA.handle();
  camera_fb_t *fb = capturePhoto();
  String base64 = encodeToBase64(fb);
  // postDrive(fb);
  // sheet_logging(String("PLASTIC_TEST"), true);
  // newPostGemini(base64);
  postGemini(base64);
  // }

  delay(10000);
}

void connectWifi()
{
  WiFi.mode(WIFI_OFF);
  delay(100);

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wifi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
    Serial.println("Attemping to connect...");
  }

  Serial.print("Connect to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void cameraInit()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA; // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  pinMode(FLASH_LED, OUTPUT);
  pinMode(CAM_STATUS_LED, OUTPUT);

  // sensor_t * s = esp_camera_sensor_get();
  // s->set_brightness(s, 0);     // -2 to 2
  // s->set_contrast(s, 0);       // -2 to 2
  // s->set_saturation(s, 0);     // -2 to 2
  // s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  // s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  // s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  // s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  // s->set_exposure_ctrl(s, 0);  // 0 = disable , 1 = enable
  // s->set_aec2(s, 1);           // 0 = disable , 1 = enable
  // s->set_ae_level(s, 0);       // -2 to 2
  // s->set_aec_value(s, 1200);    // 0 to 1200
  // s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  // s->set_agc_gain(s, 0);       // 0 to 30
  // s->set_gainceiling(s, (gainceiling_t)6);  // 0 to 6
  // s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  // s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  // s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  // s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  // s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  // s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  // s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  // s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}

camera_fb_t *capturePhoto()
{
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return NULL;
  }

  Serial.println("Camera capture success");
  return fb;
}

void sheet_logging(String binType, bool isFull)
{
  WiFiClientSecure client;
  client.setInsecure();
  if (client.connect(sheetDomain, 443))
  {
    String isBinFull = "";
    if (isFull)
    {
      isBinFull = "true";
    }
    else
    {
      isBinFull = "false";
    }
    String url = sheetScript + "?binType=" + binType + "&isFull=" + isBinFull;
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + String(sheetDomain));
    client.println("Connection: close");
    client.println();
    Serial.println("Waiting for response.");
    long int StartTime = millis();
    while (!client.available())
    {
      Serial.print(".");
      delay(100);
      if ((StartTime + 30000) < millis()) // wait for 30 seconds
      {
        Serial.println();
        Serial.println("No response.");
        // If you have no response, maybe need a greater value of waitingTime
        break;
      }
    }
    Serial.println("Url: " + url);
    while (client.available())
    {
      Serial.print(char(client.read()));
    }
  }
  else
  {
    Serial.println("Connected to " + String(sheetDomain) + " failed.");
  }
}

void postDrive(camera_fb_t *fb)
{
  WiFiClientSecure client;
  client.setInsecure();

  if (client.connect(driveDomain, 443))
  {
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "";
    for (int i = 0; i < fb->len; i++)
    {
      base64_encode(output, (input++), 3);
      if (i % 3 == 0)
        imageFile += urlencode(String(output));
    }
    String Data = driveFilename + mimeType + driveImage;

    esp_camera_fb_return(fb);

    Serial.println("Send a captured image to Google Drive.");

    client.println("POST " + driveScript + " HTTP/1.1");
    client.println("Host: " + String(driveDomain));
    client.println("Content-Length: " + String(Data.length() + imageFile.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();

    client.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index + 1000)
    {
      client.print(imageFile.substring(Index, Index + 1000));
    }

    Serial.println("Waiting for response.");
    long int StartTime = millis();
    while (!client.available())
    {
      Serial.print(".");
      delay(100);
      if ((StartTime + 30000) < millis()) // wait for 30 seconds
      {
        Serial.println();
        Serial.println("No response.");
        // If you have no response, maybe need a greater value of waitingTime
        break;
      }
    }
    Serial.println();
    while (client.available())
    {
      Serial.print(char(client.read()));
    }
  }
  else
  {
    Serial.println("Connected to " + String(driveDomain) + " failed.");
  }
  client.stop();
}

String postGemini(String base64String)
{
  HTTPClient http;
  String url = "https://" + String(geminiDomain) + geminiEndpoint + "?key=" + GEMINI_API_KEY;

  String jsonPayload = "{";
  jsonPayload.concat("\"contents\": [{");
  jsonPayload.concat("\"parts\": [");
  jsonPayload.concat("{ \"text\": \"What is the material that the object in this image made of? Plastic or Paper. Ignore the curcuit, sensor and the black paper in the backgroud, give the result shortly, one word, Paper or Plastic\" },");
  jsonPayload.concat("{ \"inline_data\": {");
  jsonPayload.concat("\"mime_type\": \"image/jpeg\",");
  jsonPayload.concat("\"data\": \"" + base64String + "\"");
  jsonPayload.concat("} }");
  jsonPayload.concat("]");
  jsonPayload.concat("}]}");

  http.begin(url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(jsonPayload);
  if (httpCode > 0)
  {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      Serial.println("Response Payload: ");
      // Serial.println(payload);
      Serial.println("Data sent to Gemini successfully!");
      //
      StaticJsonDocument<1024> doc; // Adjust size based on payload size
      DeserializationError error = deserializeJson(doc, payload);

      if (error)
      {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return "";
      }

      // Navigate the JSON structure to get the text value
      String material = doc["candidates"][0]["content"]["parts"][0]["text"];
      material.trim(); // Remove any leading/trailing whitespace or newline
      Serial.println("Extracted Material: " + material);
      return material;
    }
    else
    {
      Serial.printf("Unexpected response code: %d\n", httpCode);
      String response = http.getString();
      Serial.println("Response Body: ");
      Serial.println(response);
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return "";
}

unsigned long getTime()
{
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return 0;
  }
  time(&now);
  return now;
}

void tokenStatusCallback(TokenInfo info)
{
  if (info.status == token_status_error)
  {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  }
  else
  {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += '+';
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      // encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}

String encodeToBase64(camera_fb_t *fb)
{
  // Calculate Base64 length
  int base64Length = base64_enc_len(fb->len);
  char *base64Buffer = new char[base64Length + 1];

  // Encode to Base64
  base64_encode(base64Buffer, (char *)fb->buf, fb->len);

  // Convert to String
  String base64String = String(base64Buffer);

  // Free the buffer
  delete[] base64Buffer;

  return base64String;
}
