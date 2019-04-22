#include <U8g2lib.h> //chama a biblioteca U8G2, que trabalha com displays (incluindo suporte ao controlador SSD1306, contido na tela OLED)
#include <Wire.h> //biblioteca para trabalhar com protocolo I2C

//TELA: SDA->D21, SCL->D22
//alimentação pela 3.3V (ou talvez por 5V também)
//https://techtutorialsx.com/2017/12/02/esp32-arduino-interacting-with-a-ssd1306-oled-display/

U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0); //rotação, SDA (D21), SCL (D22), reset //criação de objeto para uso da tela OLED //1 = buffer pequeno, 2 = buffer grande, F = buffer completo

void escreveNaTela(int tamanhoFonte, const String& conteudo) {
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
  escreveNaTela(2, F("3"));
  delayMicroseconds(1000000);
  escreveNaTela(2, F("2"));
  delayMicroseconds(1000000);
  escreveNaTela(2, F("1"));
  delayMicroseconds(1000000);
}


void acionaLuz(String textoTela, int pinoApagar, int pinoAcender) {
  digitalWrite(pinoApagar, LOW);
  digitalWrite(pinoAcender, HIGH);
  escreveNaTela(1, textoTela);
  contagem();
  delayMicroseconds(1000000);
}



void setup() {
  u8g2.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_VERDE, HIGH);
  escreveNaTela(1, F("Verde"));
  contagem();
  delayMicroseconds(1000000);
  
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, HIGH);
  escreveNaTela(1, F("Amarelo"));
  contagem();
  delayMicroseconds(1000000);

  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
  escreveNaTela(1, F("Vermelho"));
  contagem();
  delayMicroseconds(1000000);
  */
}
