#include <Wire.h> // Librería para usar el BUS I2C
#include <Adafruit_ADS1X15.h> // Librería del ADS1115
Adafruit_ADS1115 ads1115; // Constructor del ads115
   
   int N=0; // Contador
     
     //Humedad 
   int Seco = 0;  // Valor en seco
   int Mojado = 0;  // Valor en agua
   
     //Salinidad 
   int AguaNormal = 0; // Agua destilada
   int AguaSalada = 0; // Agua salada
   
   int power_pin = 5; // Alimentación de la sonda

   double gramos = 0; //Gramos sal

// ---------------------------------------------------

void setup()
{
  Serial.begin(9600);
  Serial.println("ON");
  pinMode(power_pin, OUTPUT); // Pin de salida
  ads1115.begin();
  Serial.println("Voltaje ADS1115 +/- 4.096v");
  ads1115.setGain(GAIN_ONE);
  Serial.println("1 bit = 2mV");
}

void loop()
{ 
  int16_t adc0; 
  int16_t humedad; //Inicializa Humedad
  
  int adc1;
  int16_t salinidad; // Inicializa Salinidad

// Calibrado (para que el programa tenga un uso más genérico y no único al sensor que calibramos una vez)
  if(N==0) //N siendo una variable que "cuenta"
  {
  delay(2000);
  digitalWrite(power_pin, HIGH );
  delay(100);

  Serial.println("Comienza la calibración. Atención a las instrucciones:");
  Serial.println(" ");
  delay(1500); 

// Calibrar sensor de Humedad, no solo el nuestro único, sino cualquiera 
//(en el nuestro cuanta más humedad más valor digital, en el de clase cuanta más humedad, menor valor digital
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
  delay(4500);

// Calibrar sensor Salinidad
  Serial.println("Salinidad:");
  Serial.println(" ");
  delay(3500);
  Serial.print("Introduzca el sensor en agua limpia.");
  for(int i=0; i<=6; i++)
    {
       delay(2500);
       Serial.print(".");
    }
  adc1 = ads1115.readADC_SingleEnded(1);
  Serial.println(" ");
  Serial.println(adc1);
  AguaNormal=adc1; 
  Serial.println("Valor guardado.");
  delay(3500);

  Serial.println(" ");
  Serial.print("Introduzca el sensor de salinidad en agua con sal máxima.");
  for(int i=0; i<=6; i++)
    {
       delay(1500);
       Serial.print(".");
    }
  adc1 = ads1115.readADC_SingleEnded(1);
  Serial.println(" ");
  Serial.println(adc1);
  AguaSalada=adc1; 
  Serial.println("Calibrado.");
  delay(3500);

// Mensaje de OK
  Serial.println(" ");
  Serial.println(" Se han calibrado los sensores con éxito. El programa se adaptará a estos...");
  Serial.println(" ");
  N=1;
  digitalWrite( power_pin, LOW );
  }

// Programa como tal (recuerdo que es todo un grandísimo IF, por lo que tenemos que usar ELSE para caso negativo)
  else
  {
  digitalWrite(power_pin, HIGH );
  delay(100);
  
  adc0 = ads1115.readADC_SingleEnded(0);
  adc1 = ads1115.readADC_SingleEnded(1);
  
  humedad = 100*Seco/(Seco-Mojado)-adc0*100/(Seco-Mojado); //% de humedad
  salinidad = 100*AguaNormal/(AguaNormal-AguaSalada)-adc1*100/(AguaNormal-AguaSalada); //% de salinidad

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
  Serial.println(adc1);
  Serial.print(" ");
  Serial.print(salinidad);
  Serial.println("%");
  
  Serial.println("          ");
  delay(500);
  }
}


//variables calculo ahora Temperatura
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
  Serial.println("Lectura Digital Temperatura: ");
  Serial.println(adc_2);
  Serial.println("Temperatura: ");
  Serial.println(temp);
  delay(1000);
