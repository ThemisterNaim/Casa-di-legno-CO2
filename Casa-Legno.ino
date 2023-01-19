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
#include <SparkFunCCS811.h>  //link per la libreria: https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library/archive/refs/tags/v2.0.3.zip
#include <Servo.h>
#define CCS811_ADDR 0x5B       //indirizzo I2C predefinito.
CCS811 myCCS811(CCS811_ADDR);  // "nome " per evocare il comando.
Servo myPorta;
//Nomi dei pin
#define servo 12 //Transistror che controlla servo.
#define porta 10  //pin del servo.
#define DC 9  //Transistor che controlla motore.
#define camino 7        //Un semplice led giallo.
#define errore 2         //Led che si attiva quando un cavo del sensore è sconesso.
#define limite 1000 //Limite di particellle di CO2 per milione [ppm].
int maniglia = 0; //Posizione del servo.
bool link=0;
void controllo() {
  Serial.begin(115200);
  pinMode(errore, OUTPUT);
  Wire.begin();
  if (myCCS811.begin() == false) {          //Se sensore non parte...
    Serial.print("Controllare cavi");
    while (myCCS811.begin() == false) digitalWrite(errore, HIGH);  //...accendere Led mentre sensore non parte.
    Serial.println();
    digitalWrite(errore, LOW);
  }
}

void attivazione() {

  pinMode(servo, OUTPUT);
  pinMode(porta, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); //Colore (verde) non gradito
  pinMode(camino, OUTPUT);
  myPorta.attach(porta);
  digitalWrite(servo, HIGH);  //Necessario per dare corrente al servo, in questo modo si posizionerà automaticamente rispetto alla posizione dell'ultimo utilizzo
  myPorta.write(0);
  Serial.println("Sistema attivato");
  while (myCCS811.dataAvailable() == false) {};  //Serve per prevenire la registrazione del falso positivo dato all'avvio
}

void aprire() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(camino, HIGH);
  digitalWrite(servo, HIGH);  //Attiva il transistor che collega il ground al servo
  while (maniglia <= 131) {     //Gira il Servo di 131° in senso orario. ma la porta si apre di circa 70
    myPorta.write(maniglia);
    delay(15);
    maniglia++;
  }

  delay(500);
  digitalWrite(servo, LOW);
  digitalWrite(DC, HIGH);  //Attiva il transistor che collega il motore DC
  Serial.println("Motore attivato");
}

void chiudere() {
  digitalWrite(DC, LOW);  //per prevenire "interferenze" tra il motore DC e il servo

  digitalWrite(servo, HIGH);
  while (maniglia >= 0) {  //Gira il servo di 90° in senso anti orario per chiudere la porta
    myPorta.write(maniglia);
    delay(15);
    maniglia--;
  }

  digitalWrite(servo, LOW);
  Serial.println("Motore disattivato");
  delay(10);
  digitalWrite(camino, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  
}

void setup() {
  if (link ==0){
Serial.println("https://github.com/ThemisterNaim/Casa-di-legno-CO2");
link=1;
  }
  controllo();
  attivazione();
}

void loop() {

  if (myCCS811.dataAvailable()) {  //Se sono stati registrati i dati

    myCCS811.readAlgorithmResults(); //legge i dati registrati
    int co2 = myCCS811.getCO2();  //assegna ad una variabile il valore della CO2
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm");
    Serial.println();

    if (co2 >= limite) aprire();  //se il valore della CO2 è maggiore di 1000
    
    else {
      if (digitalRead(DC) == HIGH) digitalWrite(DC, LOW);  //controllare se il motore è ancora attivato
      chiudere();
    }
    
    Serial.println();
    delay(999);  //Non si deve usare costantemente il bus I2C
  } else {
    if (myCCS811.dataAvailable() == false) setup();
    Serial.println();
  }
}
