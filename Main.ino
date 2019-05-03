#include "content.h"

SemaforoMotorista farolMotorista;
SemaforoPedestre farolPedestre;
TelaOLED tela;
SR04 ultrassom;
SDCard microSD;
MPU6050 acelerometro;

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  configuraFarois();
  long tempoInicial = millis();
  microSD.iniciar();
  //long tempoFinal = millis();
  Serial.print("Tempo de init microSD: ");
  Serial.println(millis() - tempoInicial);
  //acelerometro.init();  //PL REACTIVATE
}

void loop() {
  //Serial.println(ultrassom.distancia());  //PL REACTIVATE
  //acelerometro.read();  //PL REACTIVATE
  farolMotorista.alternaLuz(farolMotorista.vermelho, farolMotorista.verde);
  tela.exibe(1, farolMotorista.msgVerde); //primeiro parâmetro informa ajuste de fonte
  delay(1000);  //esse delay é colocado aqui para excluir um certo segundo antes da contagem
  tela.contagem(((int)farolMotorista.tempoVerde/1000)-1); //substitui a contagem comum de luzVerde
  delay(TEMPO_MUDANCA_MODAL); //trata do espaço entre uma mudança do principal com relacao ao pedestre
  farolPedestre.alternaLuz(farolPedestre.verde, farolPedestre.vermelho);
  //delay(farolMotorista.tempoVerde); //a ideia e substituir esses delays conforme funcoes forem adicionadas
  farolMotorista.alternaLuz(farolMotorista.verde, farolMotorista.amarelo);
  tela.exibe(1, farolMotorista.msgAmarelo);
  delay(farolMotorista.tempoAmarelo);
  farolMotorista.alternaLuz(farolMotorista.amarelo, farolMotorista.vermelho);
  tela.exibe(1, farolMotorista.msgVermelho);
  delay(TEMPO_MUDANCA_MODAL);
  farolPedestre.alternaLuz(farolPedestre.vermelho, farolPedestre.verde);
  long tempoInicial = millis(); //comeca contagem de demora de analise para descontar no resto de tempo
  analiseTrafego(); //gera um coeficiente que pode adicionar até 100% do tempo padrao
  long tempoFinal = millis();
  int desconto = tempoFinal-tempoInicial;
  delay(farolPedestre.tempoVermelho+(farolPedestre.tempoVermelho*farolPedestre.coeficienteFluxo)-desconto); //até 2x o original
  //é tempo original + de 0 a 1x o tempo original, e o desconto da análise
  
  farolPedestre.alternaLuz(farolPedestre.verde, farolPedestre.vermelho);
  farolPedestre.vermelhoPiscante(3000);
}

void configuraFarois() {
  pinMode(farolMotorista.vermelho, OUTPUT);
  pinMode(farolMotorista.amarelo, OUTPUT);
  pinMode(farolMotorista.verde, OUTPUT);
  pinMode(farolPedestre.vermelho, OUTPUT);
  pinMode(farolPedestre.verde, OUTPUT);
}

void analiseTrafego() { //essa logica basicamente trata de incrementar em até 1x o tempo do pedestre
  int numeroRodadasPedestres = 5;
  for (int i=0; i < numeroRodadasPedestres; i++) {  //voltar depois ao sr04 para dar um jeito nele
    ultrassom.distancia();
    if(ultrassom.distanciaAtual < 10) {
      farolPedestre.coeficienteFluxo++;
    }
    delay(TEMPO_AMOSTRAS_PASSAGEM_PEDESTRE);
  }
  farolPedestre.coeficienteFluxo = farolPedestre.coeficienteFluxo/numeroRodadasPedestres;
  //vai dar de 0 a 1...
 }
