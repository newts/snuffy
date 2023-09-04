// A simple CO2 meter using the Adafruit SCD30 breakout and Wemos Lolin32 ESP32 OLED
// https://randomnerdtutorials.com/esp32-built-in-oled-ssd1306/

#include <Adafruit_SCD30.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SCD30  scd30;

void setup(void) {
  Serial.begin(115200);
  //while (!Serial) 
  delay(1000);     // will pause Zero, Leonardo, etc until serial console opens
  
  // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32
  //
  Wire.begin(5, 4);

  Serial.println("SCD30 CO2 meter");
  delay(1000);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  delay(2000);
  
#if 1
  // Try to initialize!
  if (!scd30.begin()) {
    Serial.println("Failed to find SCD30 chip");
    while (1) { delay(10); }
  }
  Serial.println("SCD30 Found!");
#endif


  if (!scd30.setMeasurementInterval(2)){
    Serial.println("Failed to set measurement interval");
    while(1) {delay(10);}
  }
  Serial.print("Measurement Interval: "); 
  Serial.print(scd30.getMeasurementInterval()); 
  Serial.println(" seconds");
  
  display.display();
  delay(500); // Pause for half second

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setRotation(0);
}


void loop() {
  if (scd30.dataReady()) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);

    Serial.println("Data available!");

    if (!scd30.read()){
      Serial.println("Error reading sensor data");
      display.println("READ ERR");
      display.display();
      return;
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

  delay(100);
}
