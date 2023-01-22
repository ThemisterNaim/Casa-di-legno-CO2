/*
  |\   |    /\    | |\  /|
  | \  |   /__\   | | \/ |
  |  \ |  /    \  | |    |
  |   \| /      \ | |    |
                   __   ___             ______   ______
  |\  /|    /\    |  \ |   \     /\    /        /        |
  | \/ |   /__\   |__/ |    \   /__\   \______  \______  |
  |    |  /    \  |\   |    /  /    \         \        \ |
  |    | /      \ | \  |___/  /      \ _______/ _______/ |
*/
#include <Wire.h>            //Librerie
#include <SparkFunCCS811.h>  //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include <Servo.h>
#define CCS811_ADDR 0x5B       //Default I2C Address.
CCS811 myCCS811(CCS811_ADDR);  // "nome " per evocare il comando.
Servo myPorta;
//Nomi dei pin
bool NPN;            //Transistror digitale.
#define servo 9      //pin del servo.
#define camino 7     //Un semplice led giallo.
#define errore 2     //Led che si attiva quando un cavo del sensore è sconesso.
#define limite 1000  //Limite di particellle di CO2 per milione [ppm].
int maniglia = 0;    //Posizione del servo.
void controllo() {
  Serial.begin(115200);
  pinMode(errore, OUTPUT);
  Wire.begin();
  if (myCCS811.begin() == false) {  //Se sensore non parte...
    Serial.print("Controllare cavi");
    while (myCCS811.begin() == false) digitalWrite(errore, HIGH);  //...accendere Led mentre sensore non parte.
    Serial.println();
    digitalWrite(errore, LOW);
  }
}

void attivazione() {
  Serial.println("link per codice e per proiezione ortogonale:");
  Serial.println("https://github.com/ThemisterNaim/Casa-di-legno-CO2");
  Serial.println();
  pinMode(servo, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(camino, OUTPUT);
  myPorta.attach(servo);
  NPN = 0;

  myPorta.write(24);
  Serial.println("Sistema attivato");
  while (myCCS811.dataAvailable() == false) {};  //Serve per prevenire la registrazione del falso positivo dato all'avvio
}

void aprire() {
  if (NPN == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(camino, HIGH);
    digitalWrite(NPN, HIGH);   //Attiva il transistor che collega il ground al servo
    while (maniglia <= 160) {  //Gira il Serv
      myPorta.write(maniglia);
      delay(15);
      maniglia++;
    }
    NPN = 1;
    delay(500);
    Serial.println("Porta aperta");
  }
}
void chiudere() {
  if (NPN == 1) {
    while (maniglia >= 24) {  //Gira il servo di 90° in senso anti orario per chiudere la servo
      myPorta.write(maniglia);
      delay(15);
      maniglia--;
    }

    Serial.println("Porta chiusa");
    delay(10);
    digitalWrite(camino, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    NPN = 0;
  }
}

void setup() {
  controllo();
  attivazione();
}

void loop() {

  if (myCCS811.dataAvailable()) {  //Se sono stati registrati i dati

    myCCS811.readAlgorithmResults();  //legge i dati registrati
    int co2 = myCCS811.getCO2();      //assegna ad una variabile il valore della CO2
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm");
    Serial.println();

    if (co2 >= limite) aprire();  //se il valore della CO2 è maggiore di 1000

    else chiudere();

    Serial.println();
    delay(1000);  //Non si deve usare costantemente il bus I2C
  } else {
    if (myCCS811.dataAvailable() == false) setup();
    Serial.println();
  }
}
