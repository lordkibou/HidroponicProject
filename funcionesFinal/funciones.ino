//A0 Humedad, A1 PH, A2 Temperatura, A3 Iluminación, ADC Salinidad
//#define power_pin 5 //Pin alimentación Salinidad
#include <Adafruit_ADS1X15.h> // Librería del ADS1115
#include <Wire.h> // Librería para usar el BUS I2C
#include <math.h>
//tension del sensor pH 3,3 a 5,5v
#define channelValue 0
#define Offset +0.12 //Definimos el Offset
#define samplingInterval 20
#define printInterval 800
#define ArrayLength 40 
int pHArray[ArrayLength]; //Array para toma de valores
int pHArrayIndex = 0; //Indice de incremento lista
int N=0;
//Temperatura:
int16_t adc_2;
int16_t adc0;
int16_t adc;
  int16_t humedad; //Inicializa Humedad
  int16_t salinidad; 
int temperaturaArray[20]; //Array de valores a hacer media de temperatura

//Humedad:
int Seco = 0;  // Valor en seco
int Mojado = 0;  // Valor en agua
   
//Salinidad:
int AguaNormal = 0; // Agua destilada
int AguaSalada = 0; // Agua salada
int power_pin = 5; // Alimentación de la sonda

double gramos = 0; //Gramos sal


//Construir ADS1115
  
  Adafruit_ADS1115 ads1115;


//---------------------------------------------
void setup() {

   
  //Inicializamos el Monitor Serie
  Serial.begin(9600);
  delay(1000);
  Serial.println("ON");
  pinMode(power_pin, OUTPUT); // Pin de salida
  Serial.println("Voltaje ADS1115 +/- 4.096v");
  Serial.println("1 bit = 2mV");
  Serial.println("Inicializando el medidor de pH");
  delay(1000);
  Serial.println("Inicializando el sensor de Temperatura");

  //Inicializamos el ADS1115
   ads1115.begin(); //Salinidad ADC 
   ads1115.begin(0x48); //Sensores ADS1115

  //Configuramos la ganancia del ADS1115
  ads1115.setGain(GAIN_ONE);
}

//Funcion completa para sal y humedad
void salyhumedad(){
  if(N==0) //N siendo una variable que "cuenta"
  {
  delay(2000);
  digitalWrite(power_pin, HIGH);
  delay(100);

  Serial.println("Comienza la calibración. Atención a las instrucciones:");
  Serial.println(" ");
  delay(1500); 

// Calibrar sensor de Humedad, no solo el nuestro único, sino cualquiera 
//(en el nuestro cuanta más humedad más valor digital, en el de clase cuanta más humedad, menor valor digital)
  Serial.println("Humedad:");
  delay(2000);
  Serial.println(" ");
  Serial.print("Calibrando sensor de Humedad en ambiente seco...");
  for(int i=0; i<=6; i++)
    {
       delay(2500);
       Serial.print("_");
    }
  adc0 = ads1115.readADC_SingleEnded(0);
  Serial.println(" ");
  Serial.println(adc0);
  Seco = adc0; 
  Serial.println("Valor guardado.");
  delay(3500);

  Serial.println(" ");
  Serial.print("Ahora moje el sensor en un recipiente con agua.");
  for(int i=0; i<=6; i++)
    {
       delay(2500);
       Serial.print(".");
    }
  adc0 = ads1115.readADC_SingleEnded(0);
  Serial.println(" ");
  Serial.println(adc0);
  Mojado=adc0; 
  Serial.println("Calibrado.");
  Serial.println(" ");
  delay(6000);

// Calibrar sensor Salinidad
  Serial.println("Salinidad:");
  Serial.println(" ");
  delay(3500);
  Serial.print("Introduzca el sensor en agua limpia.");
  for(int i=0; i<=6; i++)
    {
       delay(2000);
       Serial.print(".");
    }
  digitalWrite( power_pin, HIGH );
  delay(500);
  adc = analogRead(A0); //adc del sparkfun
  digitalWrite( power_pin, LOW );
  Serial.println(" ");
  Serial.println(adc);
  AguaNormal=adc; 
  Serial.println("Valor guardado.");
  delay(3500);

  Serial.println(" ");
  Serial.print("Introduzca el sensor de salinidad en agua con sal máxima.");
  for(int i=0; i<=6; i++)
    {
       delay(2000);
       Serial.print(".");
    }
  digitalWrite( power_pin, HIGH );
  adc = analogRead(A0);
  delay(500);
  digitalWrite( power_pin, LOW );
  Serial.println(" ");
  Serial.println(adc);
  AguaSalada=adc; 
  Serial.println("Calibrado.");
  delay(3500);

// Mensaje de OK
  Serial.println(" ");
  Serial.println(" Se han calibrado los sensores con éxito. El programa se adaptará a estos...");
  Serial.println(" ");
  N=1;
  digitalWrite(power_pin, LOW);
  }

//Imprimir Humedad y Salinidad

digitalWrite(power_pin, HIGH);
  delay(200);
  
  adc0 = ads1115.readADC_SingleEnded(0);
  adc = analogRead(A0);
  digitalWrite(power_pin, LOW);
  humedad = 100*Seco/(Seco-Mojado)-adc0*100/(Seco-Mojado); //% de humedad
  salinidad = 100*AguaNormal/(AguaNormal-AguaSalada)-adc*100/(AguaNormal-AguaSalada); //% de salinidad

 if(humedad>=100)
    humedad=100; //que no sobrepase el 100%
   
  if(humedad<=0)
    humedad=0;  //que no baje del 0%

  if(salinidad>=100)
    salinidad=100; //que no sobrepase el 100%
   
  if(salinidad<=0)
    salinidad=0;  //que no baje del 0%

 digitalWrite( power_pin, LOW );
 
// Lo que aparecerá en el Monitor Serie:

  Serial.println(" ");
  Serial.print("Humedad: ");
  Serial.println(adc0);
  Serial.print(" ");
  Serial.print(humedad);
  Serial.println("%");

  Serial.print("Salinidad: ");
  Serial.println(adc);
  Serial.print(" ");
  Serial.print(salinidad);
  Serial.println("%");
  //Serial.print("Gramos de Sal: ");
  //Serial.println(gramos);
  delay(500);

}

//Función sensor de temperatura
void temperatura(){
   double m = (34.6*(pow(10,-3)));
  double b = (0.789);
  
  //Captura una muestra del ADS1115
  int16_t adc_2 = ads1115.readADC_SingleEnded(2);

  for(int i=0;i<=4;i++){ //Toma de 5 valores
  temperaturaArray[i]=ads1115.readADC_SingleEnded(2);
  delay(200);
  }//for()

  adc_2 = averageSample(&temperaturaArray[0],5);//Valor del average sample a partir de la lista
  
  //formula dependiendo de la lectura digital del sensor de temperatura
  double temp = ((4.096/(pow(2,15)))*adc_2-b)*(1/m);
  Serial.print("Sensor de Temperatura: ");
  Serial.println(adc_2);
  Serial.print(" ");
  Serial.print(temp);
  Serial.println(" ºC");
  delay(2000);
  for(int i=0;i<=5;i++){
  Serial.println(".");
  delay(500);
  }
}

//Funcion ph
void phFuncion(){
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  for(int i=0;i<=2000;i++){
  if (millis() - samplingTime > samplingInterval){
    pHArray[pHArrayIndex++]= ads1115.readADC_SingleEnded(1) ; //Guardamos valores para el array
    if (pHArrayIndex == ArrayLength){
      pHArrayIndex=0;
      voltage=(4.096/(pow(2,15)-1))*averageSample(&pHArray[0],40);
      pHValue = 3.5 * voltage + Offset;
      samplingTime = millis();
    }
  }
  //Mostramos en pantalla finalmente
 if (millis() - printTime > printInterval){
   Serial.print("Voltaje: ");
   Serial.print(voltage, 2);
   Serial.println(" Valor pH: ");
   Serial.print(pHValue, 2);
    printTime = millis();
 }
  }//for()

  for(int i=0;i<=5;i++){
  Serial.println(".");
  delay(100);
  }
}

//Funcion Puntero para varios valores y hacer media
double averageSample(int *p,int cuantos){
  double x=0;
  for(int i=0;i<=cuantos-1;i++){
    x+=p[i];
  }
  return (x/cuantos);
}

//Iluminosidad:
int  val = 0;

// Real----->lightReading()
void lightReading(int pin){ //Funcion de intervalos
   int val = ads1115.readADC_SingleEnded(pin);
   double valF;
   if(val<3){
        val=3;
      }
      if(val<=21 && val>=3){
       Serial.println("Sensor Tapado/obstruido");
      }
      if(val<=450 && val>21){
       Serial.println("Sombreado/Nubes");
      }
      if(val>450 && val<2100){
        Serial.println("Soleado");
       }
      if(val>=2100){
        Serial.println("Demasiada luz para el huerto");
      }
      Serial.println(val);
      valF=(4.096*10/32767)*val;
      Serial.println(valF);
       delay(2000);
     }

void loop(void) {

// Calibrado (para que el programa tenga un uso más genérico y no único al sensor que calibramos una vez)
  salyhumedad();

//variables calculo ahora Temperatura
temperatura();

//pH
phFuncion();

  lightReading(3); //Función sensor iluminosidad,PIN 3 INT
}
