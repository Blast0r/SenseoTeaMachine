
#define     THERMISTOR_PIN     3       // Pin between the thermistor and 
                                       // series resistor.
#define     BUTTON1_PIN        2       // Heizung/Hauptschalter
#define     BUTTON2_PIN        3       //Pumpen

#define     PUMP_PIN           7       //Digital Pin, über den die Pumpe angesteuert werden kann
#define     BOILER_PIN         8       //Digital Pin zum Ansprechen des Boilers
#define     LED_PIN            6       //Digital Pin für Zustands LED

#define     MAX_TEMP           70      //Abschalt-Temperatur in °C des Boilsers.. wir wollen das Sieden nicht riskieren

#define     SERIES_RESISTOR    9875    // Series resistor value in ohms.

#define     ADC_SAMPLES        5       // Number of ADC samples to average
                                       // when taking a reading.
#define     BUTTON_TH          100

#define     A     0.000857108116       //Steinhart-Hart coefficients
#define     B     0.000268916344
#define     C     0.000000095860


#define PUMP_TIME 21000 //zeit in ms um eine Tasse voll zu bekommen... überprüfen...

#define TempUnit "Celsius"


boolean heating;
boolean heat;
boolean doPump; // wir wollen gleich pumpen, 
boolean pumping;
boolean temp_reached;
int button1_pressed;
unsigned long startTime;
boolean LED;
boolean LED_ON;
long lastButtonPress1; // zeit, wann der button das letzte mal gedrückt wurde?
long lastButtonPress2;
long tempMillis;
long timeRound;

void setup() {
  Serial.begin(115200);
  analogReference(DEFAULT);
  pinMode(BUTTON1_PIN,INPUT_PULLUP);
  pinMode(BUTTON2_PIN,INPUT_PULLUP);
  pinMode(PUMP_PIN,OUTPUT);
  pinMode(BOILER_PIN,OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(PUMP_PIN,LOW); //sicherheitshalber erstmal auf LOW schieben.
  digitalWrite(BOILER_PIN,LOW);
  digitalWrite(LED_PIN, LOW);
  lastButtonPress1 = 0;
  lastButtonPress2 = 0;
}



void loop() {
  
  float temp = kelvinToCelsius(readTemp()); //Temperatur überprüfen
  if (temp >= MAX_TEMP){
   temp_reached = true; 
   }
   else if (temp <= MAX_TEMP - 0.25){
    temp_reached = false;
    }
  
  if (temp >= MAX_TEMP) LED = true;
  if (temp <= MAX_TEMP-0.25) LED = false;
  
  if (digitalRead(BUTTON1_PIN)==LOW){
    tempMillis = millis();
    if (tempMillis - BUTTON_TH > lastButtonPress1) { 
      if (!temp_reached){
          if (heat){
            heat = false;
           }
           else{
           heat = true;
           //Serial.print(F("hihi: ")); Serial.println(heating);
           }
          
      }
    }
    lastButtonPress1 = tempMillis;
  }
  
  if (digitalRead(BUTTON2_PIN)==LOW){
    tempMillis = millis();
    if(tempMillis-BUTTON_TH > lastButtonPress2) {
      if (doPump) {
        doPump = false;
        heat = false;
      }
      else{
        doPump = true;
        }
    }
    lastButtonPress2= tempMillis;
  }
    
  if (temp_reached && heating || !heat){
    heating = false;
    //heat = false;
    digitalWrite(BOILER_PIN,LOW);
  }
  
  if (heat && !(temp_reached) && !heating){
    heating = true;
    digitalWrite(BOILER_PIN,HIGH); 
  }


  if(doPump && !pumping && temp_reached) { //wenn wir pumpen sollen, dann wird die pumpe aktiviert, die startZeit gemerkt und die Pumpe eingeschaltet
   // if(doPump && !pumping) { // for debug.. so kann die pumpe schon vor erreichen der Tempereatur eingestellt werden
    pumping=true;
    digitalWrite(PUMP_PIN,HIGH);
    startTime=millis();
   }
   else if (!doPump && pumping){
       pumping = false;
       digitalWrite(PUMP_PIN,LOW);
   }
     
   
   if (((pumping) && (millis() >= (startTime + PUMP_TIME)))) {
     digitalWrite(PUMP_PIN,LOW);
     doPump=false;
     pumping = false;
     heat=false;
     }
     
   if(LED && !LED_ON){
    digitalWrite(LED_PIN,HIGH);
    LED_ON = true;
    }
   if(!LED && LED_ON){
     digitalWrite(LED_PIN,LOW);
     LED_ON = false;
    }
   
    
    
  Serial.print(temp); Serial.print("C,P"); Serial.print(pumping); Serial.print(",H");Serial.print(heating); 
  Serial.print(F(" T")); Serial.println(temp_reached);
  //delay(100);
  //long tempTime2 = micros();
  //Serial.println(tempTime2-timeRound);
  //timeRound = tempTime2;
}


double readResistance() {
  float reading = 0;
  for (int i = 0; i < ADC_SAMPLES; ++i) {
    reading += analogRead(THERMISTOR_PIN);
  }
  reading /= (float)ADC_SAMPLES;
  reading = (1023 / reading) - 1;
  return SERIES_RESISTOR / reading;
}

float kelvinToCelsius(float kelvin) {
  return kelvin - 273.15;
}

float readTemp() {
  float R = readResistance();
  float kelvin = 1.0/(A + B*log(R) + C*pow(log(R), 3.0));
  return kelvin;
}
