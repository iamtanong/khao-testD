#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"          // Disable brownour problems
#include "soc/rtc_cntl_reg.h" // Disable brownour problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>

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

#define FILE_PHOTO "./photo.jpg"

// WiFi
const char *ssid = "iHame2";
const char *password = "0947475913";

// Google
#define PROJECT_ID "turnkey-girder-443020-d8"
#define CLIENT_EMAIL "bin-logging@turnkey-girder-443020-d8.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQClaHCqCuyZtcYK\nX2Ix2S0Kz6sqlp4qiYaTFtSWATNETxgUTw27J5oXzHb5z2todHapvZ7xNzzaIlPV\neDkjjXa6mTgpODrzz00Urcavpna74VNS2dZX4dzDumBlBGnKwGowESZ0r26cJP5s\nlk8qKrFq70BrUo7ZOgEieDqyxOjEiz8AfsxmfNY5BorAcEcD3Mk3gXZxYl4JYcJM\nP84kwG7Kgbj9EdctYjL3Qq+ocFHWrGF2OpMY3e70CN+1QpauH31qZhjuJTNxvqkm\nO67v8AB5MgTbtBn99nONkXazVcmVTrGhZp8s7ZcSKLQOXn/Y17SX/5dcJETPfrYX\n3puyv/L7AgMBAAECggEASm5BSFsrFY3kxFUgmCLMcHF7aeol2SlSa8zZs3hty961\njKn4GVAcOHpSzV2nTPUfUolXxN6g3N2WkNgNCjEFjLkWuwGpEX4mhtgdsHu9MRUX\nTBBnMduaXl73Mm22dzx3bBLDlb09jjjdXhqTG/vjIyXu/HxFeH1v9kVY0Inn3ky7\nPcru/i7F8mksT66lx2gS0HjuG2wjRp4uzqtWQQhiFumTc96lz1lamdhqrtQmW72l\n3jtbMtV71GqugQlJlgwunqHgAp4FxGuOTFL2BIuHheD99S3FK8dxgi9l9/QtOdkg\ncU9NpYGrSyufq8p9do/jU8K4C3ZeJ5q3HE+OlIYpWQKBgQDRuvzdMbqJfp16+exy\nUJxId2nnUL3q0nrZs1eKHjMRET+ILtDmR4TGO8WfKLgfHfX3c9/eR0fnk0rbweZX\nD/yukpjYCcp99BSXssF8+NAYzqdJ73XupNK5pSri80PeiEPiojlAVT+a/bHtrb0j\nV9yNuSWGje0VPXPWA11tUUQwUwKBgQDJ5j0wYDrcgYqr7jrUkK+7YTw3njdV430p\nAoQkefvDO+Kd27QiFmSwVQMRsdkYulpo/GSdsz7F0TtXHEUuwtg2dmY6naqmrXrA\ndM69DYMMHhQQyX8B2o5CI2Mkf7j7ccDb5rrlzLM3cdkU4fatkZR47+i7xYSpHUmQ\n30x8TY39uQKBgBq1b2JT5OeBolh4322nal+oJWp509XFvDNhLXK/ac1wnuCe5aeN\n0BDWp9IfA2OjEyHSNd4+wZ9yGIRn6weHV6x7qs9IX+suXDj9YLqjjQy7tH4r6p7b\niqJtsEp6pFgAjnScKCJTOOhqCnC0QXPwynuZ6nd6N4kO7GOCxjEXulFVAoGARpdL\nmZhrvCFG3OpEg4G7D6blqQSWkF8jqa0Jir9juVU83LiueKSfu89dbVhjcXPmnIJy\no3jCX+PR/ZkSz1CszA4FnET4H6LUImo0xoDs6tWxalUpJPrHdnRs+5v0j2WsarZ3\nblVt69rQba670t/tq4MmOlDLG130FMUeNu/ez3ECgYEAviMACdo0q5rPirQvc1Lv\nHpQ5qQTeXKJCSGyTHOQZxLwuDoUUEN3AnkZug53ZWx2H4+Luc1dJ0uQX/hEoLTfm\nMpMC8hvQYXRf+d9AtFGeHOv9cDCx2dElKf83wgwMCvODEcrsBxNU38Ea5Gote7fy\ngNRLF7ETPMb5zYAZ+d3i75s=\n-----END PRIVATE KEY-----\n";
const char spreadsheetId[] = "1_xpC2WFXm3_8KbhqXaYG9Crtsd0161moA2gkyeJkNB4";

// NTP server to request epoch time
const char *ntpServer = "pool.ntp.org";
// Variable to save current epoch time
unsigned long epochTime;

// Function pre-declaration
void connectWifi();
void capturePhoto();
unsigned long getTime();
// Token Callback function
void tokenStatusCallback(TokenInfo info);

AsyncWebServer server(80);
boolean takeNewPhoto = false;
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { text-align:center; }
    .vert { margin-bottom: 10%; }
    .hori{ margin-bottom: 0%; }
  </style>
</head>
<body>
  <div id="container">
    <h2>ESP32-CAM Last Photo</h2>
    <p>It might take more than 5 seconds to capture a photo.</p>
    <p>
      <button onclick="rotatePhoto();">ROTATE</button>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
      <button onclick="location.reload();">REFRESH PAGE</button>
    </p>
  </div>
  <div><img src="saved-photo" id="photo" width="70%"></div>
</body>
<script>
  var deg = 0;
  function capturePhoto() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/capture", true);
    xhr.send();
  }
  function rotatePhoto() {
    var img = document.getElementById("photo");
    deg += 90;
    if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }
    else{ document.getElementById("container").className = "hori"; }
    img.style.transform = "rotate(" + deg + "deg)";
  }
  function isOdd(n) { return Math.abs(n % 2) == 1; }
</script>
</html>)rawliteral";

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  connectWifi();
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else
  {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // OV2640 camera module
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound())
  {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  // Camera configuration
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, 0);                 // -2 to 2
  s->set_contrast(s, 0);                   // -2 to 2
  s->set_saturation(s, 0);                 // -2 to 2
  s->set_special_effect(s, 0);             // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);                   // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);                   // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);                    // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);              // 0 = disable , 1 = enable
  s->set_aec2(s, 0);                       // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);                   // -2 to 2
  s->set_aec_value(s, 300);                // 0 to 1200
  s->set_gain_ctrl(s, 1);                  // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);                   // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0); // 0 to 6
  s->set_bpc(s, 0);                        // 0 = disable , 1 = enable
  s->set_wpc(s, 1);                        // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);                    // 0 = disable , 1 = enable
  s->set_lenc(s, 1);                       // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);                    // 0 = disable , 1 = enable
  s->set_vflip(s, 0);                      // 0 = disable , 1 = enable
  s->set_dcw(s, 1);                        // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);                   // 0 = disable , 1 = enable

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    takeNewPhoto = true;
    request->send_P(200, "text/plain", "Taking Photo"); });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, FILE_PHOTO, "image/jpg", false); });

  // Start server
  configTime(0, 0, ntpServer);

  GSheet.printf("ESP Google Sheet client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

  // Set the callback for Google API access token generation status (for debug only)
  GSheet.setTokenCallback(tokenStatusCallback);

  // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
  GSheet.setPrerefreshSeconds(10 * 60);

  // Begin the access token generation for Google API authentication
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

  server.begin();

  pinMode(12, OUTPUT);

  // Serial 2 : for Arduino communicate
  // Serial2.begin(9600, SERIAL_8N1, RXpin2, TXpin2);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    // If not connected to wifi, Try reconnect
    Serial.println("WiFi is not connected. Reconnecting...");
    connectWifi();
  }
  else
  {
    if (takeNewPhoto)
    {
      capturePhoto();
      takeNewPhoto = false;
    }
  }

  delay(100);
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

// Check if photo capture was successful
bool checkPhoto(fs::FS &fs)
{
  File f_pic = fs.open(FILE_PHOTO);
  unsigned int pic_sz = f_pic.size();
  return (pic_sz > 100);
}

// Capture Photo and Save it to SPIFFS
void capturePhoto(void)
{
  camera_fb_t *fb = NULL; // pointer
  bool ok = 0;            // Boolean indicating if the picture has been taken correctly

  do
  {
    // Take a photo with the camera
    Serial.println("Taking a photo...");
    digitalWrite(12, HIGH);

    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println("Camera capture failed");
      return;
    }

    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // Insert the data in the photo file
    if (!file)
    {
      Serial.println("Failed to open file in writing mode");
    }
    else
    {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");

      FirebaseJson response;
      Serial.println("\nAppend spreadsheet values...");
      Serial.println("----------------------------");

      FirebaseJson valueRange;

      valueRange.add("majorDimension", "COLUMNS");
      valueRange.set("values/[0]/[0]", epochTime);
      valueRange.set("values/[1]/[0]", FILE_PHOTO);

      // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/append
      // Append values to the spreadsheet
      bool success = GSheet.values.append(&response, spreadsheetId, "Rubbish-in_log!A1", &valueRange);
      if (success)
      {
        response.toString(Serial, true);
        valueRange.clear();
      }
      else
      {
        Serial.println(GSheet.errorReason());
      }
      Serial.println();
      Serial.println(ESP.getFreeHeap());
    }

    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
    digitalWrite(12, LOW);
  } while (!ok);
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


// ************************* communication *****************************
// #include <Arduino.h>

// #define RXp2 16 // RX pin for Serial2
// #define TXp2 17 // TX pin for Serial2

// void setup() {
//   Serial.begin(115200); // Debug output
//   Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2); // Initialize Serial2
  
// }

// void loop() {
 
//     // Read a string from Arduino Uno
//     String received = Serial2.readString();
//     if (received != ""){
//       Serial.println(received);
//     }
//    delay(1000);
//    Serial2.println("Ayo from ESP32");


//   delay(1000); // Small delay for stability
// }
