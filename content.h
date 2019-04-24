#define PIN_LED_VERMELHO_MOTORISTA 25
#define PIN_LED_AMARELO_MOTORISTA 26
#define PIN_LED_VERDE_MOTORISTA 27

//referencia de pinos para OLED
//#define PIN_DATA_OLED 21
//#define PIN_CLOCK_OLED 22

#define PIN_LED_VERMELHO_PEDESTRE 12
#define PIN_LED_VERDE_PEDESTRE 13

#define TEMPO_BASE_FAROL_VERMELHO_MOTORISTA 3000
#define TEMPO_BASE_FAROL_AMARELO_MOTORISTA 3000
#define TEMPO_BASE_FAROL_VERDE_MOTORISTA 3000

#define MSG_LED_VERMELHO_MOTORISTA "Vermelho"
#define MSG_LED_AMARELO_MOTORISTA "Amarelo"
#define MSG_LED_VERDE_MOTORISTA "Verde"

//pensando em fazer uns defines de selecionar melhor modo de exibir texto

//referencia para tempos de pedestre
//#define TEMPO_BASE_FAROL_VERDE_PEDESTRE 3000
//#define TEMPO_BASE_FAROL_VERMELHO_PEDESTRE 3000

#define TEMPO_MUDANCA_MODAL 250  //diferenca quando muda de modal (carro ou pernas)


//parte OLED
#include <U8g2lib.h> //chama a biblioteca U8G2, que trabalha com displays (incluindo suporte ao controlador SSD1306, contido na tela OLED)
#include <Wire.h> //biblioteca para trabalhar com protocolo I2C

//TELA: SDA->D21, SCL->D22
//alimentação pela 3.3V (ou talvez por 5V também)
//https://techtutorialsx.com/2017/12/02/esp32-arduino-interacting-with-a-ssd1306-oled-display/

U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0);  //criação de objeto para uso da tela OLED
//rotação, SDA (D21), SCL (D22), reset
//1 = buffer pequeno, 2 = buffer grande, F = buffer completo

//funcoes para os LEDs
struct SemaforoMotorista {
  int verde = PIN_LED_VERDE_MOTORISTA;
  int amarelo = PIN_LED_AMARELO_MOTORISTA;
  int vermelho = PIN_LED_VERMELHO_MOTORISTA;
  int tempoVerde = TEMPO_BASE_FAROL_VERDE_MOTORISTA;
  int tempoAmarelo = TEMPO_BASE_FAROL_AMARELO_MOTORISTA;
  int tempoVermelho = TEMPO_BASE_FAROL_VERMELHO_MOTORISTA;
  String msgVerde = MSG_LED_VERDE_MOTORISTA;
  String msgAmarelo = MSG_LED_AMARELO_MOTORISTA;
  String msgVermelho = MSG_LED_VERMELHO_MOTORISTA;

  void alternaLuz(String textoTela, int pinoApagar, int pinoAcender) { //desliga o primeiro e liga o segundo
    digitalWrite(pinoApagar, LOW);
    digitalWrite(pinoAcender, HIGH);
    //exibe(1, textoTela);  //, informa na tela o acionado no momento, caso a tela esteja habilitada
  }
};

/*
void acionaLuzMotorista(String textoTela, int pinoApagar, int pinoAcender) { //desliga o primeiro e liga o segundo
  digitalWrite(pinoApagar, LOW);
  digitalWrite(pinoAcender, HIGH);
  //exibe(1, textoTela);  //, informa na tela o acionado no momento, caso a tela esteja habilitada
}
*/

struct SemaforoPedestre {
  int verde = PIN_LED_VERDE_PEDESTRE;
  int vermelho = PIN_LED_VERMELHO_PEDESTRE;
  //int tempoVerde = TEMPO_BASE_FAROL_VERDE_PEDESTRE;
  //int tempoVermelho = TEMPO_BASE_FAROL_VERMELHO_PEDESTRE;

  void alternaLuz(int pinoApagar, int pinoAcender) {
    digitalWrite(pinoApagar, LOW);
    digitalWrite(pinoAcender, HIGH);
  }

  void vermelhoPiscante(int tempoAmarelo) {
    for (int i=0; i<tempoAmarelo; i+=500) {
      digitalWrite(PIN_LED_VERMELHO_PEDESTRE, LOW);
      delay(250);
      digitalWrite(PIN_LED_VERMELHO_PEDESTRE, HIGH);
      delay(250);
    }
  }
};

//funcoes para a tela
struct TelaOLED {
  
  void exibe(int tamanhoFonte, const String& conteudo) {
    String fonte;
    short int posicaoCursorX, posicaoCursorY;
    u8g2.firstPage();
    do {
      switch(tamanhoFonte) { //verifica valor da variável, efetuando operações de acordo com o valor
        case 1: //seria a execução para valor 1, por exemplo
          u8g2.setFont(u8g2_font_crox3c_tf); //uma fonte menor
          posicaoCursorX = 0, posicaoCursorY = 17;
        break;
        case 2:
          u8g2.setFont(u8g2_font_fur25_tn);
          posicaoCursorX = 46, posicaoCursorY = 60;
        break;
        default:
          u8g2.setFont(u8g2_font_fur25_tn); //uma fonte maior, para numeros
          posicaoCursorX = 32, posicaoCursorY = 60;
        break;
      }
      u8g2.setCursor(posicaoCursorX, posicaoCursorY); //ajusta x e y na tela para iniciar o texto
      u8g2.print(conteudo); //imprime efetivamente a String informada
    } while( u8g2.nextPage() );
  }

  void contagem() {
    exibe(2, F("3"));
    delayMicroseconds(1000000);
    exibe(2, F("2"));
    delayMicroseconds(1000000);
    exibe(2, F("1"));
    delayMicroseconds(1000000);
  }

  /*
  void acionaLuz(String textoTela, int pinoApagar, int pinoAcender) {
    digitalWrite(pinoApagar, LOW);
    digitalWrite(pinoAcender, HIGH);
    exibe(1, textoTela);
    contagem();
    delayMicroseconds(1000000);
  }
  */
};
/*
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
*/
//END
