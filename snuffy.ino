// A simple CO2 meter using the Adafruit SCD30 breakout and Wemos Lolin32 ESP32 OLED
// https://randomnerdtutorials.com/esp32-built-in-oled-ssd1306/

#include <Adafruit_SCD30.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// define your AP in mywifi.h
#include "mywifi.h"

const char *ssid = MY_SSID;
const char *password = MY_PASSWORD;

WebServer server(80);


#define NUM_DATA 200

double ymax = 1.0;   // small number
double ymin = 88888; // large number

int CO2[NUM_DATA];
int temperature[NUM_DATA];
byte humidity[NUM_DATA];
int num_data = 0;
int first_data = 0;

int lCO2 = 0;
int ltemperature = 0;
byte lhumidity = 0;


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SCD30  scd30;


#define TEMPSIZE 600
void handleRoot() {
  char temp[TEMPSIZE];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, TEMPSIZE,
"<html>\
<head>\
<meta http-equiv='refresh' content='5'/>\
<title>snuffy</title>\
<style>\
body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
</style>\
</head>\
<body>\
<h1>snuffy</h1>\
<p>Uptime: %02d:%02d:%02d</p>\
<p>%4d PPM %2dC %02d%%</p>\
<img src=\"/co2.svg\" />\
</body>\
</html>",
           hr, min % 60, sec % 60, lCO2, ltemperature, lhumidity, int(ymax), int(ymin)
          );
  server.send(200, "text/html", temp);
}

#define MSG_SIZE 64
void getReading() {
  char message[MSG_SIZE];

  snprintf(message, MSG_SIZE, "{\"co2\":%d, \"temperature\":%d, \"humidity\":%d}", lCO2, ltemperature, lhumidity);
  server.send(200, "application/json", message);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}


void setup(void) {
  Serial.begin(115200);
  // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32
  //
  Wire.begin(5, 4);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    // for(;;); // Don't proceed, loop forever
  }
  delay(2000);
  display.display();
  delay(500); // Pause for half second

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setRotation(0);

  // Wait for connection
  int timeout = 20;
  while (WiFi.status() != WL_CONNECTED) {
    if (--timeout <= 0) break;
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/co2.svg", drawGraph);
  server.on("/co2", getReading);
  server.on("/inline", []() {
    server.send(200, "text/plain", "well howdy");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("SCD30 CO2 meter");
  delay(1000);

  
  // Try to initialize!
  if (!scd30.begin()) {
    Serial.println("Failed to find SCD30 chip");
    while (1) { delay(10); }
  }
  Serial.println("SCD30 Found!");


  if (!scd30.setMeasurementInterval(2)){
    Serial.println("Failed to set measurement interval");
    while(1) {delay(10);}
  }
  Serial.print("Measurement Interval: "); 
  Serial.print(scd30.getMeasurementInterval()); 
  Serial.println(" seconds");
}


unsigned int readings = 0;
#define SKIPS 4

void loop() {
  while (scd30.dataReady()) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);

    Serial.println("\nData available!");

    if (!scd30.read()){
      Serial.println("Error reading sensor data");
      display.println("READ ERR");
      display.display();
      break;
    }

    Serial.println(first_data);
    Serial.println(num_data);
    int data_index = (first_data + num_data) % NUM_DATA;
    lCO2 = CO2[data_index] = scd30.CO2;
    ltemperature = temperature[data_index] = scd30.temperature;
    lhumidity = humidity[data_index] = scd30.relative_humidity;

    if ((++readings % SKIPS) == 0) {
      // stash the data for the graph
      //
      if (num_data < NUM_DATA) {
        num_data++;
      }
      else if (++first_data >= NUM_DATA) {
        first_data = 0;
      }
    }
    
    Serial.print("CO2: ");
    Serial.print(scd30.CO2, 3);
    Serial.print(" PPM ");
    Serial.print(scd30.temperature, 3);
    Serial.println(" C ");
    Serial.println("");
    Serial.print(scd30.relative_humidity, 0);
    Serial.print("%");

    display.println("snuffy");
    display.print(scd30.CO2, 0);
    display.println(" PPM");
    display.print(scd30.temperature, 0);
    display.println(" C");
    display.print(scd30.relative_humidity, 0);
    display.print("%");
    
    display.display();
  }

  server.handleClient();
  
  delay(10);
}

#define GRAPH_WIDTH 400
#define GRAPH_HEIGHT 160
#define Y_HEIGHT (GRAPH_HEIGHT-10)
#define GRAPH_X_STEP (1 + (GRAPH_WIDTH / NUM_DATA))

void drawGraph() {
  if (num_data > 2) {
    for (int i=0; i < num_data; i++) {
      if (CO2[i] > ymax) ymax = CO2[i];
      if (CO2[i] < ymin) ymin = CO2[i];
    }
  }
  double yscale = ((double) Y_HEIGHT) / (ymax-ymin);

  Serial.println(yscale);
  String out = "";
  char temps[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"160\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";

  if (num_data > 2) {
    int y = CO2[first_data] * yscale;
    for (int i = 0; i < num_data; i++) {
      int x = i * GRAPH_X_STEP;
      int y2 = (CO2[(first_data+i) % NUM_DATA] - ymin) * yscale;
      // Serial.print(x); Serial.print(','); Serial.println(y2);
      sprintf(temps, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, Y_HEIGHT - y, x + GRAPH_X_STEP, Y_HEIGHT - y2);
      out += temps;
      y = y2;
    }
  }
  
  out += "</g>\n";
  out += "<style> .Rrrrr {\
      font: 16px serif;\
      fill: red;\
    } </style>";
    sprintf(temps, "<text x=\"360\" y=\"14\" class=\"Rrrrr\">%d</text>\n", int(ymax));
    out += temps;
    sprintf(temps, "<text x=\"360\" y=\"144\" class=\"Rrrrr\">%d</text>\n", int(ymin));
    out += temps;
  out += "</svg>\n";

  server.send(200, "image/svg+xml", out);
}
