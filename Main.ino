#include "content.h"

SemaforoMotorista farolMotorista;
SemaforoPedestre farolPedestre;
TelaOLED tela;

void setup() {
  Serial.begin(115200);
  pinMode(farolMotorista.vermelho, OUTPUT);
  pinMode(farolMotorista.amarelo, OUTPUT);
  pinMode(farolMotorista.verde, OUTPUT);
  pinMode(farolPedestre.vermelho, OUTPUT);
  pinMode(farolPedestre.verde, OUTPUT);
}

void loop() {
  //farolMotorista.tempoVerde = 3000; //motivo do porque do uso de struct
  farolMotorista.alternaLuz("Verde", farolMotorista.vermelho, farolMotorista.verde);
  delay(TEMPO_MUDANCA_MODAL); //trata do espaço entre uma mudança do principal com relacao ao pedestre
  //acionaLuzPedestre(farolPedestre.verde, farolPedestre.vermelho); //util se nao tivesse o vermelho piscante
  delay(farolMotorista.tempoVerde); //a ideia e substituir esses delays conforme funcoes forem adicionadas
  farolMotorista.alternaLuz("Amarelo", farolMotorista.verde, farolMotorista.amarelo);
  delay(farolMotorista.tempoAmarelo);
  farolMotorista.alternaLuz("Vermelho", farolMotorista.amarelo, farolMotorista.vermelho);
  delay(TEMPO_MUDANCA_MODAL);
  farolPedestre.alternaLuz(farolPedestre.vermelho, farolPedestre.verde);
  delay(farolMotorista.tempoVermelho);
  farolPedestre.alternaLuz(farolPedestre.verde, farolPedestre.vermelho);
  farolPedestre.vermelhoPiscante(3000);
  tela.contagem();
}
