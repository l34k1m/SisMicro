#define PIN_LED_VERMELHO_MOTORISTA 25
#define PIN_LED_AMARELO_MOTORISTA 26
#define PIN_LED_VERDE_MOTORISTA 27

#define PIN_LED_VERMELHO_PEDESTRE 12
#define PIN_LED_VERDE_PEDESTRE 13

#define TEMPO_BASE_FAROL_VERMELHO_MOTORISTA 3000
#define TEMPO_BASE_FAROL_AMARELO_MOTORISTA 3000
#define TEMPO_BASE_FAROL_VERDE_MOTORISTA 3000

#define TEMPO_MUDANCA_MODAL 250  //diferenca quando muda de modal

struct SemaforoMotorista {
  int verde = PIN_LED_VERDE_MOTORISTA;
  int amarelo = PIN_LED_AMARELO_MOTORISTA;
  int vermelho = PIN_LED_VERMELHO_MOTORISTA;
  int tempoVerde = TEMPO_BASE_FAROL_VERDE_MOTORISTA;
  int tempoAmarelo = TEMPO_BASE_FAROL_AMARELO_MOTORISTA;
  int tempoVermelho = TEMPO_BASE_FAROL_VERMELHO_MOTORISTA;
};

void acionaLuz(String textoTela, int pinoApagar, int pinoAcender) { //desliga o primeiro e liga o segundo
  digitalWrite(pinoApagar, LOW);
  digitalWrite(pinoAcender, HIGH);
  //escreveNaTela(1, textoTela);  //, informa na tela o acionado no momento, caso a tela esteja habilitada
}

void acionaLuzPedestre(int pinoApagar, int pinoAcender) {
  digitalWrite(pinoApagar, LOW);
  digitalWrite(pinoAcender, HIGH);
}

void vermelhoPiscantePedestre(int tempoAmarelo) {
  for (int i=0; i<tempoAmarelo; i+=500) {
    digitalWrite(PIN_LED_VERMELHO_PEDESTRE, LOW);
    delay(250);
    digitalWrite(PIN_LED_VERMELHO_PEDESTRE, HIGH);
    delay(250);
  }
}

SemaforoMotorista farolMotorista;

void setup() {
  pinMode(farolMotorista.vermelho, OUTPUT);
  pinMode(farolMotorista.amarelo, OUTPUT);
  pinMode(farolMotorista.verde, OUTPUT);
  pinMode(PIN_LED_VERMELHO_PEDESTRE, OUTPUT);
  pinMode(PIN_LED_VERDE_PEDESTRE, OUTPUT);
}

void loop() {
  farolMotorista.tempoVerde = 3000;
  acionaLuz("Verde", farolMotorista.vermelho, farolMotorista.verde);
  delay(TEMPO_MUDANCA_MODAL); //trata do espaço entre uma mudança do principal com relacao ao pedestre
  //acionaLuzPedestre(PIN_LED_VERDE_PEDESTRE, PIN_LED_VERMELHO_PEDESTRE); //util se nao tivesse o vermelho piscante
  delay(farolMotorista.tempoVerde); //a ideia e substituir esses delays conforme funcoes forem adicionadas
  acionaLuz("Amarelo", farolMotorista.verde, farolMotorista.amarelo);
  delay(farolMotorista.tempoAmarelo);
  acionaLuz("Vermelho", farolMotorista.amarelo, farolMotorista.vermelho);
  delay(TEMPO_MUDANCA_MODAL);
  acionaLuzPedestre(PIN_LED_VERMELHO_PEDESTRE, PIN_LED_VERDE_PEDESTRE);
  delay(farolMotorista.tempoVermelho);
  acionaLuzPedestre(PIN_LED_VERDE_PEDESTRE, PIN_LED_VERMELHO_PEDESTRE);
  vermelhoPiscantePedestre(3000);
}
