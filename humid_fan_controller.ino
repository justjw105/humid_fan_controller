//Libraries
#include <DHT.h>;

//Constants
#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
// pin constants
const int fan_pin = 6, manual_on_pin = 4, humid_set_pin = A5, speed_set_pin = A1;
// delay constants
const int temp_delay = 20, on_delay = 3000, button_delay = 4;


//Variables
float hum;  //Stores humidity value
float temp; //Stores temperature value
int fan_pin_value = 0; //Store state of fan_pin
int temp_counter = 0; // To handle delay of temp/humid reading
int manual_counter = 0;
int button_counter = 0;
int buttonState = 0;
int humid_max = 0;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
  pinMode(fan_pin, OUTPUT);
  pinMode(manual_on_pin, INPUT);
  analogWrite(fan_pin, fan_pin_value);

}

void loop() {
  delay(100);// Do a base delay of .1 second
  // Convert POT setting to a floating percentage
  float humid_max_float = ((float)analogRead(humid_set_pin) / 1024) * 100.0;
  
  int speed_value = (analogRead(speed_set_pin)/8)+ 128;
  //Imply conversion to an int
  humid_max =  humid_max_float;
  //Update all the counters
  //The counters are used to increase the delay for each item.
  // We dont want to read the temp every .1 second, but we do want to watch for a button press that quickly
  if(manual_counter > 0 ){
    manual_counter = manual_counter - 1;
  }
  if(button_counter > 0){
    button_counter = button_counter - 1;
  }
  if(temp_counter > 0){
    temp_counter = temp_counter - 1;
  } else {
    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    temp_counter = temp_delay;
    Serial.print("humid Read:");
    Serial.println(hum);
  }
  //If we are not currently in manual override
  if (manual_counter == 0){
    if(hum > humid_max){
      updatePWMSpeed(speed_value);
    } else if ((hum < humid_max - 5)){
      updatePWMSpeed(0);;
    }
  }

  buttonState = digitalRead(manual_on_pin);

  // We want to delay once pressed to make sure the press is not accidenatlly replicated
  if(buttonState == HIGH && button_counter == 0){
    // if we are already being overridden, then hitting the button again cancels it out 
    if(manual_counter > 0) {
      manual_counter = 0;
    } else {
      // else we start the override
      manual_counter = on_delay;
      if(fan_pin_value > 0){
        updatePWMSpeed(0);
      } else {
        updatePWMSpeed(speed_value);
      }
    }
    
    button_counter = button_delay;
  }
  //if active, make sure the speed gets updated if the pin analog reading changes
  if (fan_pin_value != 0) {
    updatePWMSpeed(speed_value);
  }
}

void updatePWMSpeed(int newValue){
  if(newValue != fan_pin_value) {
    Serial.print("speed Value Set:");
    Serial.println(newValue);
    fan_pin_value = newValue;
    analogWrite(fan_pin, fan_pin_value);
  }
}
