#include <HTTPClient.h>
#include <Update.h>

#define WIFI_SSID "ProGames"
#define WIFI_PASS "ProGames2023Hit"

#define WIFI_SSID2 "MiFi"
#define WIFI_PASS2 "HasloWifi123"


#define USE_SERIAL Serial

String payload;

void GetFile(int num);


// perform the actual update from a given stream
void performUpdate(Stream &updateSource, size_t updateSize) {
   if (Update.begin(updateSize)) {
      Serial.println("updateSize : " + String(updateSize));
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) {
         Serial.println("Written : " + String(written) + " successfully");
      }
      else {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) {
         Serial.println("OTA done!");
         if (Update.isFinished()) {
            Serial.println("Update successfully completed. Rebooting.");
         }
         else {
            Serial.println("Update not finished? Something went wrong!");
         }
      }
      else {
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }

   }
   else
   {
      Serial.println("Not enough space to begin OTA");
   }
}

static const String FW_UPDATE_FILE = "/update.bin";

// check given FS for valid update and perform update if available
void updateFromFS(fs::FS &fs) {
   File updateBin = fs.open(FW_UPDATE_FILE);
   if (updateBin) {
      if(updateBin.isDirectory()){
         Serial.println("Error, update is not a file");
         updateBin.close();
         return;
      }

      size_t updateSize = updateBin.size();
      USE_SERIAL.printf("\n updateSize=%d", updateSize);

      if (updateSize > 0) {
         Serial.println("\n Try to start update");
         performUpdate(updateBin, updateSize);
      }
      else {
         USE_SERIAL.printf("Error, file is empty");
      }

      updateBin.close();
    
      // whe finished remove the binary from sd card to indicate end of the process
      int ret = fs.remove(FW_UPDATE_FILE);
      USE_SERIAL.printf("fs.remove = %d", ret);
      ret = fs.remove(FW_UPDATE_FILE);
      USE_SERIAL.printf("fs.remove = %d", ret);
   }
   else {
      USE_SERIAL.printf("Could not load update from sd root");
   }
}

void UpdateFromMMC(void) {
    updateFromFS(SD_MMC);
}

int StartWifiClient(void)
{
    Serial.println("StartWifiClient");
    int wifiStatus;
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("Connecting Wifi...");
    for(int i=0;i<100;i++) 
    {
        wifiStatus = WiFi.status();
        if (wifiStatus == WL_CONNECTED) {
            Serial.print("\n IP address: ");
            Serial.println(WiFi.localIP());
            break;
        }
        delay(100);
        printf("\n WiFi=%d", wifiStatus);
    }
    

    if (wifiStatus != WL_CONNECTED) {
      WiFi.begin(WIFI_SSID2, WIFI_PASS2);
      Serial.println("Connecting Wifi2...");

      for(int i=0;i<100;i++) 
      {
        wifiStatus = WiFi.status();
        if (wifiStatus == WL_CONNECTED) {
            Serial.print("\n IP address: ");
            Serial.println(WiFi.localIP());
            break;
        }
        delay(100);
        printf("\n WiFi=%d", wifiStatus);
      }           
    }

    printf("\n Wifi STATE = %d", wifiStatus);
    
    return wifiStatus;
}

bool CheckSdMmc(void)
{
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    Serial.print("Total bytes: ");
    Serial.println(SD_MMC.totalBytes());

    Serial.print("Used bytes: ");
    Serial.println(SD_MMC.usedBytes());

    return true;
}


bool DownloadFirmwareUpdate(void)
{
    HTTPClient http;
    bool result = false;

    String url = "http://pgm.aksell.pl/pgm.ino.bin";

    if(true==SD_MMC.remove(FW_UPDATE_FILE)) USE_SERIAL.printf("old file removed");
    if(true==SD_MMC.remove(FW_UPDATE_FILE)) USE_SERIAL.printf("old file removed");
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            File file = SD_MMC.open(FW_UPDATE_FILE, FILE_WRITE);

            USE_SERIAL.printf("\n SD_MMC file %s open=%d", FW_UPDATE_FILE, file);

            // get length of document (is -1 when Server sends no Content-Length header)
            int len = http.getSize();
            USE_SERIAL.printf("\n getSize = %d", len);

            // create buffer for read
            uint8_t buff[128] = {0};

            // get tcp stream
            WiFiClient *stream = http.getStreamPtr();

            // read all data from server
            while (http.connected() && (len > 0 || len == -1))
            {
                // get available data size
                size_t size = stream->available();

                if (size)
                {
                    //USE_SERIAL.printf("x");
                    // read up to 128 byte
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    // Serial.print("c=");
                    // Serial.println(c);
                    file.write(buff, c);

                    if (len > 0)
                    {
                        len -= c;
                    }
                }
                delay(1);
            }

            printf("[HTTP] connection closed or file end.\n");
            file.close();
            result = true;
        }
    }
    else
    {
        printf("[HTTP] GET... res=%d, error: %s\n", httpCode, http.errorToString(httpCode).c_str());
    }

    http.end();

    printf("\n http.end() Update Downloaded");

    return result;

    //UpdateFirmware();
}
uint16_t Mp3FileList[1000];
int Mp3FileCnt = 0;

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

bool StartMp3Update(void)
{
    bool ret = false;   
    HTTPClient http;

    CheckSdMmc();

    createDir(SD_MMC, "/mp3");

  for (int i=0;i<3;i++) {

    Serial.print("Request File List");
    String url = "http://pgm.aksell.pl/mp3";
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(url);
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
          WiFiClient *client = http.getStreamPtr();
          String currentLine;
          
          while (http.connected()) {
          if (client->available()) {
            currentLine = client->readStringUntil(':');
            int idx = currentLine.indexOf(".mp3");
            if(idx > 0) {
              Serial.printf("\n  idx = %d ",idx);
              idx += 6;
              int ending = currentLine.lastIndexOf(".mp3");
              if(ending > 0) {
                Serial.printf(" ending = %d ",ending);
                String sub = currentLine.substring(idx, ending);
                Serial.printf(" sub = %s ",sub);
                Mp3FileList[Mp3FileCnt] = sub.toInt();
                Mp3FileCnt++;
                Serial.printf(" fileCnt = %d ",Mp3FileCnt);
              }
            }
            //Serial.println(currentLine);
            currentLine = "";
        }
      }
     }
        ret = true;
        return ret;
    }
    else
    {
        printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
    return ret;
}

int UpdateProgress = 0;

void RunMp3Update(void)
{
    if (UpdateProgress < Mp3FileCnt)
    {
      GetFile(Mp3FileList[UpdateProgress]);
      UpdateProgress++;
      delay(1);
    }   
    else delay(100);
}

void GetFile(int num)
{
    HTTPClient http;
    int localFileSize = 0;
    int remoteFileSize;

    printf("[HTTP] begin...\n");

    // configure server and url
    String url = "http://pgm.aksell.pl/mp3";
    String FileName = String(num) + ".mp3";

    if (1)
    {

        url += "/";
        url += FileName;

        http.begin(url);

        printf("[HTTP] GET...\n");
        Serial.print(url);
        // start connection and send HTTP header
        int httpCode = http.GET();
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                Serial.print("FileName=");
                Serial.println(FileName);
                String path = "/mp3/" + FileName;
                printf("\n SD path=%s", path);
                File file = SD_MMC.open(path);
                localFileSize = file.size();
                printf("\n local file size=%d", file.size());
                file.close();

                
                // get length of document (is -1 when Server sends no Content-Length header)
                int len = http.getSize();
                remoteFileSize = len;
                printf("\n remoteFileSize=%d", remoteFileSize);

                if(remoteFileSize != localFileSize) 
                {
                file = SD_MMC.open(path, FILE_WRITE);
                printf("\n http.getSize size= %d\n", len);
                // create buffer for read
                uint8_t buff[128] = {0};

                // get tcp stream
                WiFiClient *stream = http.getStreamPtr();

                // read all data from server
                while (http.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = stream->available();

                    if (size)
                    {
                        // read up to 128 byte
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        
                        file.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }
                }
                else printf("\n FileSize the same, skip");

                printf("\n [HTTP] connection closed or file end.\n");
                file.close();
            }
        }
        else
        {
            printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
    else
    {
        printf("\n SKIP %s", FileName);
    }
}
