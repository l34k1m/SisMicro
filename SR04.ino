//Leitura de distância com o sensor HC-SR04
//Tensão de 5V
#include <Ultrasonic.h>

#define INTERVALO_LEITURA 250 //tempo em ms

long distancia;

//conexão dos pinos para o sensor ultrasonico
#define PIN_TRIGGER   34
#define PIN_ECHO      35

Ultrasonic ultrasonic(PIN_TRIGGER, PIN_ECHO);   //Inicializa o sensor nos pinos definidos acima

int getDistance() {   //faz a leitura das informacoes do sensor (em cm)
    int distanciaCM;
    long microsec = ultrasonic.timing();    // pode ser um float ex: 20,42 cm se declarar a var float
    distanciaCM = ultrasonic.convert(microsec, Ultrasonic::CM);
    Serial.println(distanciaCM);
    return distanciaCM;
}
 
// Esta função "setup" roda uma vez quando a placa e ligada ou resetada
 void setup() {
 Serial.begin(115200); //Habilita Comunicação Serial a uma taxa de 9600 bauds.
 }
 
// Função que se repete infinitamente quando a placa é ligada
void loop() {
  getDistance();
  delay(INTERVALO_LEITURA);
}
