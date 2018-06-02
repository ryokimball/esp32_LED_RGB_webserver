#include <WiFi.h>

// wifi credentials
const char* ssid     = "PUBLICWIFI";
const char* password = "SUPERSECRET";

// Pins connected to R,G, & B respectively 
const uint8_t ledPins[3] = {27,12,13};

// used for the speed of the patterns
int delay_ms = 5;


const uint8_t ledChannels[3] = {1,2,3};
const int freq = 5000;
const int resolution = 8;



struct RGB {
  byte R,G,B;
};
class LedRGB {
  private:
  uint8_t ledRChannel,ledGChannel,ledBChannel;
  public:
  void All(RGB rgb){
    ledcWrite(ledRChannel, rgb.R);
    ledcWrite(ledGChannel, rgb.G);
    ledcWrite(ledBChannel, rgb.B);
  }
  void All(uint8_t val){
    All(RGB {val,val,val});
  }
  void R(uint8_t val){
    ledcWrite(ledRChannel, val);
  }
  void G(uint8_t val){
    ledcWrite(ledGChannel, val);
  }
  void B(uint8_t val){
    ledcWrite(ledBChannel, val);
  }
  LedRGB(const uint8_t ledChannels[3],const uint8_t ledPins[3],const int freq, const int res){
    for(uint8_t i = 0; i < 3; ++i){
      ledcAttachPin(ledPins[i], ledChannels[i]);
      ledcSetup(ledChannels[i], freq, res);
    }
    ledRChannel = ledChannels[0];
    ledGChannel = ledChannels[1];
    ledBChannel = ledChannels[2];
    All(0);
  };
};


LedRGB rgb = LedRGB(ledChannels,ledPins,freq,resolution);

TaskHandle_t xHandle;

char linebuf[80];
int charcount=0;


WiFiServer server(80);

void rgbPatternOne(void * param){
  while(true){
    Serial.println("Running Pattern One");
    for(uint8_t i = 0; i < 255; ++i){
      rgb.All(i);
      delay(delay_ms);
    }
    for(uint8_t i = 255; i > 0; --i){
      rgb.G(i);
      rgb.B(i);
      delay(delay_ms);
    }
    for(uint8_t i = 0; i < 255; ++i){
      rgb.G(i);
      delay(delay_ms);
    }
    for(uint8_t i = 255; i > 0; --i){
      rgb.R(i);
      delay(delay_ms);
    }
    for(uint8_t i = 0; i < 255; ++i){
      rgb.B(i);
      delay(delay_ms);
    }
    for(uint8_t i = 255; i > 0; --i){
      rgb.G(i);
      delay(delay_ms);
    }
    for(uint8_t i = 0; i < 255; ++i){
      rgb.R(i);
      delay(delay_ms);
    }
    for(uint8_t i = 0; i < 255; ++i){
      rgb.G(i);
      delay(delay_ms);
    }
    for(uint8_t i = 255; i > 0; --i){
      rgb.All(i);
      delay(10);
    }
  }
}

void rgbPatternTwo(void * param){
  while(true){
    Serial.println("Running Pattern Two");
    rgb.All(255);
    delay(delay_ms*20);  
    rgb.All(RGB {255,0,0});
    delay(delay_ms*20);  
    rgb.All(RGB {127,127,0});
    delay(delay_ms*20);  
    rgb.All(RGB {0,255,0});
    delay(delay_ms*20);  
    rgb.All(RGB {0,127,127});
    delay(delay_ms*20);  
    rgb.All(RGB {0,0,255});
    delay(delay_ms*20);  
    rgb.All(RGB {127,0,127});
    delay(delay_ms*20);  
  }
}

void singleColor(const char *s){
  int x = 0;
  RGB inRGB {0,0,0};
  for(int i = 0;i < 6; ++i) {
    char c = *s;
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10;
    }
    else if(c >= 'a' && c <= 'f') {
      x *= 16;
      x += (c - 'A' - 32) + 10;
    }
    else break;
    ++s;
    if(i%2!=0){
      Serial.println("val: " + x);
      if(i < 2) inRGB.R = x;
      else if (i < 4) inRGB.G = x;
      else {
        inRGB.B = x;
        break;
      }
      x = 0;
    }
  }
  rgb.All(inRGB); 
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while(!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  // attempt to connect to Wifi network:
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
  
}

void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        //read char by char HTTP request
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML><html><head>");
          client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<title>ESP32 Thing</title></head>");
          client.println("<h1>ESP32 - Web Server</h1>");
          client.println("<p>RGB <a href=\"ptrn1\"><button>Pattern 1 (fading)</button></a></p>");
          client.println("<p>RGB <a href=\"ptrn2\"><button>Pattern 2 (switching)</button></a></p>");
          client.println("<div id='colorpicker'> Color picker: <input type='color' id='colorpicker'> </input> </div>");
          client.println("<script type='text/javascript'>");
          client.println("document.querySelector('#colorpicker').addEventListener('change',function(ev){window.location.replace('/rgb?val='+ev.target.value.substring(1));});");
          client.println("</script>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          if (strstr(linebuf,"GET /ptrn1") > 0){
            if(xHandle != NULL){
              Serial.println("killing current pattern...");
              vTaskDelete(xHandle);
            }
            Serial.println("starting pattern 1");
            xTaskCreate(  rgbPatternOne,
                          "rgbPatternOne",
                          10000,
                          NULL,
                          1,
                          &xHandle);
          }
          else if (strstr(linebuf,"GET /ptrn2") > 0){
            if(xHandle != NULL){
              Serial.println("killing current pattern...");
              vTaskDelete(xHandle);
            }
            Serial.println("starting pattern 2");
            xTaskCreate(  rgbPatternTwo,
                          "rgbPatternTwo",
                          10000,
                          NULL,
                          1,
                          &xHandle);
          }
          else if (strstr(linebuf,"GET /rgb") > 0){
            if(xHandle != NULL){
              Serial.println("killing current pattern...");
              vTaskDelete(xHandle);
            }
            Serial.println("FULL");
            Serial.println(linebuf);
            const char* strRGB = String(linebuf).substring(String(linebuf).indexOf("val=")+4).c_str();
            Serial.println("SUBSTRING: ");
            Serial.println(strRGB);
            singleColor(strRGB);
          }

        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}


