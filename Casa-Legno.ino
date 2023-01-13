
#include <Wire.h>            //Librerie
#include <SparkFunBME280.h>  //Click here to get the library: http://librarymanager/All#SparkFun_BME280
#include <SparkFunCCS811.h>  //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include <Servo.h>
#define CCS811_ADDR 0x5B       //Default I2C Address
CCS811 myCCS811(CCS811_ADDR);  // "nome " per evocare il comando
Servo myPorta;
//Nomi dei pin
#define chiave 12
#define porta 10
#define npn 9  //funge da porta NOT
#define limite 1000
#define errore 2
int maniglia = 0;

void controllo() {
  Serial.begin(115200);
  Wire.begin();
  if (myCCS811.begin() == false) {
    Serial.print("Controllare cavi");
    while (myCCS811.begin() == false) digitalWrite(errore, HIGH);
    Serial.println();
    digitalWrite(errore, LOW);
  }
}

void attivazione() {

  pinMode(chiave, OUTPUT);
  pinMode(porta, OUTPUT);
  pinMode(npn, OUTPUT);
  pinMode(errore, OUTPUT);
  myPorta.attach(porta);
  digitalWrite(chiave, HIGH);  //Necessario per dare corrente al servo, in questo modo si posizionerà automaticamente rispetto alla posizione dell'ultimo utilizzo
  myPorta.write(0);
  Serial.println("Sistema attivato");
  while (myCCS811.dataAvailable() == false) {};  //Serve per prevenire la registrazione del falso positivo dato all'avvio
}

void aprire() {

  digitalWrite(chiave, HIGH);  //Attiva il transistor che collega il ground al servo
  while (maniglia <= 120) {     //Gira il Servo di 90° in senso orario.
    myPorta.write(maniglia);
    delay(15);
    maniglia++;
  }

  delay(500);
  digitalWrite(chiave, LOW);
  digitalWrite(npn, HIGH);  //Attiva il transistor che collega il motore DC
  Serial.println("Motore attivato");
}

void chiudere() {
  digitalWrite(npn, LOW);  //per prevenire "interferenze" tra il motore DC e il servo
  delay(500);

  digitalWrite(chiave, HIGH);
  while (maniglia >= 0) {  //Gira il servo di 90° in senso anti orario per chiudere la porta
    myPorta.write(maniglia);
    delay(15);
    maniglia--;
  }

  digitalWrite(chiave, LOW);
  Serial.println("Motore disattivato");
}

void setup() {
  controllo();
  attivazione();
}

void loop() {

  if (myCCS811.dataAvailable()) {  //Se sono stati reistrati i dati

    myCCS811.readAlgorithmResults();
    int co2 = myCCS811.getCO2();  //assegna ad una variabile il valore della CO2
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm");
    Serial.println();

    if (co2 >= limite) aprire();
    else {
      if (digitalRead(npn) == HIGH) digitalWrite(npn, LOW);
      chiudere();
    }
    
    Serial.println();
    delay(2580);  //Don't spam the I2C bus
  } else {
    if (myCCS811.dataAvailable() == false) setup();
    Serial.println();
  }
}