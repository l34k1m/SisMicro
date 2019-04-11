//Programa: semáforo WiPoste
//Autor: Mikael e equipe WiPoste


#include <dht.h> //biblioteca para o DHT11, sensor de temperatura e pressão
#include <DS1307RTC.h> //biblioteca para trabalhar com DS1307 (relógio RTC)
#include <ESP8266.h> //biblioteca feita pela própria equipe para trabalhar com a placa de Wi-Fi ESP8266-12F
#include <Time.h> //usada para fazer operações de millis, micros, de acordo com a necessidade de tempo a ser mostrado
#include <U8g2lib.h> //chama a biblioteca U8G2, que trabalha com displays (incluindo suporte ao controlador SSD1306, contido na tela OLED)
#include <Ultrasonic.h> //biblioteca para trabalhar com o HC-SR04, sensor de ultrassom
#include <Wire.h> //biblioteca para trabalhar com protocolo I2C

/*
Sobre o IR:
	erro: não piscava o verdePrincipal;
	ficava pouco tempo no amareloPrincipal;
	parava no vermelhoPrincipal;
	>> deve ter sido conflito de hardware! Usei lib mais recente e não resolveu.
*/

//Velocidades de comunicação. Tais velocidades são medidas em bauds (podem ir até 115200)
#define VELOCIDADE_SERIAL 9600
#define VELOCIDADE_SERIAL_ESP 9600

//Lógica de sensores
//HC-SR04
#define TRIGGER_SR04 7 //pino do emissor de som do HC-SR04 //era 12
#define ECHO_SR04 8 //pino receptor de som do HC-SR04 //era 13
#define TIMEOUT_SR04 1500 //750 = 12.5cm, 1500 = 25cm, 3000 = 50cm, 30000 = 500cm, como aparece na biblioteca
#define DISTANCIA_MAXIMA_ULTRASSOM 10 //distancia maxima para um objeto ser considerado valido e aparecer no resumo de avaliacoes
//Fotorresistor
#define SENSOR_LUZ A0 //possui resistor de 10kOhm //porta analógica para fotorresistor; Recebe valor entre 0 e 1023
#define LIMIAR_LUZ_BAIXA 256 //valor que, abaixo dele, o semáforo considera que há pouca luz
#define LIMIAR_LUZ_ALTA 768 //valor que, acima dele, o semáforo considera que há muita luz, e aumenta a intensidade dos semáforos
#define VALOR_PWM_FAROL_BAIXO 12 //valor para modulação de pulso (PWM), usado para regular iluminação dos LED's em iluminação padrão (luz média)
//DHT
#define SENSOR_DHT A1 //Pino DATA do Sensor de umidade/temperatura ligado na porta Analogica A1
//MPU 
#define ENDERECO_MPU 0x69 //endereço I2C da MPU //era 0x68

//Lógica de semáforo
//Principal
#define VERDE 9 //possui resistor de 220 Ohm; Pino do LED do semáforo principal
#define AMARELO 10 //possui resistor de 220 Ohm; Pino do LED do semáforo principal
#define VERMELHO 11 //possui resistor de 220 Ohm //pino do LED do semáforo principal
#define VERDE_PEDESTRE 6 //possui resistor de 220 Ohm; Pino do LED do semáforo de pedestres
#define VERMELHO_PEDESTRE 5 //possui resistor de 220 Ohm; pino do LED do semáforo de pedestres
#define TEMPO_AVALIACAO 200 //tempo para cada leitura de pedestre
#define DELAY_MUDANCA_FAROL 200 //atraso entre um farol e outro, no mesmo semáforo
#define DELAY_FAROIS_PEDESTRE_MOTORISTA 200 //atraso entre um farol de motorista e um de pedestre
#define DELAY_PISCA_VERMELHO_PEDESTRE_ACESO 250 //tempo aceso para cada piscada do farol vermelho do pedestre
#define DELAY_PISCA_VERMELHO_PEDESTRE_APAGADO 250 //tempo apagado para cada piscada do farol vermelho do pedestre
#define TEMPO_INICIAL_FAROL_ABERTO 8000 //tempo do farol verde principal (vai para variavel)
#define TEMPO_INICIAL_FAROL_AMARELO 4000 //tempo do farol amarelo principal (vai para variavel)
#define TEMPO_ADICIONAL_FAROL_FECHADO 2000 //tempo do farol vermelho principal (vai para variavel). Já tem 2s para avaliar pedestres e 2s para piscar o vermelho do pedestre

//Informações de senha para conexão a um roteador
// ::Celular
//#define SSID "peregrine-AP"
//#define PWD "connects"
// ::UFABC
#define SSID "UFABC"
#define PWD "85265"

//Informações para criação de rede
//#define SSID_AP "WiPoste"
//#define PWD_AP "2017rede" //usando WPS, tem que ter no mínimo 8 caracteres
//#define CANAL_AP 6 //canal seguindo 802.11n, que vai de 1 a 13 (acho)
//#define ENCRIPT_AP 3 //nível de seguranção da rede; 0 = OPEN 1 = WEP; 2 = WPA; 3 = WPA2 (acho)

//IP de servidor web para conexão como cliente
// ::Celular
//#define IP "192.168.43.57"
// ::UFABC
#define IP "172.31.121.41"

//URL dentro do servidor a se conectar
//#define URL "/wiposte/post.php" //padrão
#define URL "/cgi-bin/post.cgi"

//Mensagens de cronometro	> são passadas a uma função para serem exibidas em cada uso de cronometro
#define MSG_TEMPO_INICIO_ESP8266 "ms p/ iniciar Wi-Fi."
#define MSG_TEMPO_RECOLHA_DADOS_SENSORES "ms p/ recolher dados."
#define MSG_TEMPO_INFORMAR_VALORES_SENSORES "ms p/ inf. dados dos sensores."
#define MSG_TEMPO_TRATAR_REQUISICAO_ESP "ms p/ gerir reqs. da ESP."
#define MSG_TEMPO_MOSTRAR_TEMPO_RESTANTE "ms p/ mostrar tempo restante."
//#define MSG_TEMPO_EXECUCAO_ESP "ms p/ atender req. ESP"

//Mensagens de operacao
#define MSG_DELAY_CAPTURA_DADOS_SENSORES "ms p/ capturar dados de sensores."
#define MSG_FIM_RODADA "Fim de rodada.\n\n\n"

//Tempos de funcoes internas de janela de tempo
#define TEMPO_LIMITE_ESP 1200 //tempo estimado para que o Arduino confirme requisição, envie resposta e confirme a requisição
#define TEMPO_VERMELHO_PEDESTRE_PISCANTE 2000 //pisca-pisca tradicional do farol vermelho de pedestre

//Tamanho de string para reserve. Reserve evita realocações (consequentemente evita fragmentação) e otimiza desempenho
#define TAMANHO_MEDIO_STRING_RESUMO_SENSORES 240 //tamanho médio da string resumoDados, para passar em reserve()
#define TAMANHO_MEDIO_STRING_FOTORRESISTOR 8 //tamanho médio da string texto, para passar em reserve()
#define TAMANHO_MEDIO_STRING_DHT 10 //tamanho médio da string texto, para passar em reserve()
#define TAMANHO_MEDIO_STRING_TEMP_MPU 4 //tamanho médio da string texto, para passar em reserve()
#define TAMANHO_MEDIO_STRING_VALORES_MPU 72 //tamanho médio da string texto, para passar em reserve()


//Declaração de funções  //o uso de inline força o copiar-colar da função em cada chamada (aumenta desempenho e tamanho de arquivo)
inline void iniciarPinos(); //inicia os pinos do Arduino para trabalhar com sensores e LED's'
inline void iniciarComunicacaoWire(); //inicia a identificação de dispositivos I2C

inline void armazenarValorLDR(); //recolhe valor entre 0 e 1023 vindo da porta analógica do fotorresistor
inline void armazenarValorDHT(); //recolhe umidade vinda do DHT11
inline void prepararAcessoMPU(); //ajusta usando Wire o enderecamento e etc...
inline void armazenarValoresMPU(); //recolhe dados atuais da MPU
inline void armazenarValoresSensores(); //agrupa funções de armazenamento de dados de sensores

inline void relatarLuminosidade(); //imprime informações sobre luz
inline void relatarUmidade(dht); //imprime informações vindas do DHT
inline void relatarValoresMPU(); //imprime valores da MPU
inline void relatarDadosSensores(); //agrupa funções de exibição de informações de sensores

inline void ligarFarol(int); //escreve em HIGH na porta para acender o LED informado
inline void desligarFarol(int); //escreve em LOW na porta para apagar o LED informado

inline void mostrarNaOLED(const String&); //mostra uma String na tela de OLED
inline void mostrarNaOLED(int, const String&); //mostra mostra uma String na tela de OLED, e permite informar um tamanho de letra pré-determinado
inline void ajustarExibicaoDigitoDuplo(int); //evita a falta de 0 à esquerda em números menores que 10
inline void mostrarTempoRestanteFarolVerde(); //mostra na tela OLED o tempo restante do farol verde
inline void mostrarHora(); //mostra a hora. Ainda não é funcional, falta um DS1307 ou similar para usar
inline void mostrarTemperatura(); //mostra a temperatura na tela OLED

inline void avaliarFluxoPedestres(); //durante x segundos, recolhe n vezes o valor do sensor de ultrassom, e salva quantas vezes algo ficou na distância considerada como pessoa presente
inline void piscarVermelhoPedestre(); //faz a piscagem do LED de pedestre, de acordo com o tempo informado em TEMPO_VERMELHO_PEDESTRE_PISCANTE, revezando entre aceso e apagado por cada x ms

//inline void informarPedestreForaFaixa();

//Globais

String resumoDados; //utilizada para formar conteúdo a ser enviado via ESP numa requisição
int rodada; //número de vezes que loop() foi executado
//long tempoEtapa; //provavelmente sera retirado

struct Acelerometro { // struct é uma forma arcaica vinda do C para associar valores, similar a Orientação a Objetos
	int x, y, z; //valores de acelerômetro
};
Acelerometro accel; //"objeto" composto internamente pelas três variáveis de tipo inteiro

struct Giroscopio {
	int x, y, z;
};
Giroscopio gyro; //"objeto" giroscópio

struct Temporizador {
	long inicio; //tempo em ms no comeco do uso
	long total; //tempo final, que pode ser mostrado como tempo total de uma operacao

	void iniciar() {
		inicio = millis(); //pega como inicio o tempo em ms do sistema
	}

	void terminar() {
		total = millis() - inicio; //subtrai tempo em ms inicial do tempo atual em ms, resultando no total
	}

	void relatarExecucao(const String& mensagem) { //recebe uma mensagem a ser mostrada junto com o tempo total
		Serial.print(total);
		Serial.println(mensagem);
	}
};
Temporizador cronometro; //"objeto" para ser usada para medir tempo de processamento
Temporizador cronometro2; //tem estrategias já precisando de um Temporizador dentro do outro, já foi feito esse cronometro para facilitar o uso rapido

//para usar um temporizador cronometro: cronometro.iniciar(); ...OPERACAO...; cronometro.terminar(); cronometro.relatarExecucao("mensagem");

struct JanelaTempo {
	long limite; //tempo maximo baseado na soma de millis() atual com tempo de duracao
	long tempoAtual; //recebe millis() do momento exato
	long tempoLimiteOperacaoInterna; //tempo maximo de demora para execucao do conteudo do laco
	long tempoRestante; //variavel a ser calculada a cada vez que precisar avaliar a validade do tempo

	void iniciar(int tempoAdicional) { //apenas inicia a contagem atribuindo tempo de sistema em ms a uma variavel
		limite = millis() + tempoAdicional;
		tempoLimiteOperacaoInterna = 0;
	}

	void iniciar(int tempoAdicional, int tempoLimiteLaco) { //existe polimorfismo (perceba que já há um método com o mesmo nome)
		limite = millis() + tempoAdicional;
		tempoLimiteOperacaoInterna = tempoLimiteLaco;
	}

	bool validade() { //verifica se o tempo inicial em ms somado ao tempo previsto para janela ainda é maior que o tempo atual de sistema
		tempoRestante = limite - millis();
		//Serial.print(tempoRestante);
		//Serial.println(F(" ms rest."));
		if ((tempoRestante - tempoLimiteOperacaoInterna) > 0) {
			return true;
		} else {
			return false;
		}
	}

	void corrigirTempo() { //pode ser usado tanto com janela while quanto com if
		delay(limite - millis()); //tempo adicional para resultar no tempo ideal de execucao do laco
	}
};
JanelaTempo janelaTempo; //"objeto" para fazer contagem para janelas de tempo //se tiver uma contagem dentro da outra, o jeito é fazer mais dessa struct

//para usar uma JanelaTempo janelaTempo: janelaTempo.iniciar(tempoAdicional / tempoAdicional, tempoLimiteLaco); while/if janelaTempo.validade(); ...OPERACAO...; janelaTempo

U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0); //criação de objeto para uso da tela OLED //1 = buffer pequeno, 2 = buffer grande, F = buffer completo
dht DHT; //objeto do tipo DHT11, que vai conter temperatura e umidade; Pinos da dir. para esq.: 1=5V, 2=dados, 3=sem uso, 4=GND
ESP8266 esp(2,3); //criação de objeto para uso com a ESP8266 (RX, TX)
Ultrasonic ultrasonic(TRIGGER_SR04, ECHO_SR04, TIMEOUT_SR04); //criação de objeto para uso do ultrassom

unsigned short int registroFotorresistor; //salva valor entre 0-1023
int tempMPU; //variavel para armazenar temperatura, atualmente vinda da MPU-6050
String textoTempMPU; //serve para exibir temperatura na tela

//Lógica do semáforo
// *** Seria uma boa ideia fazer um objeto semáforo para cada semáforo em operação, e colocar esse tanto de coisa dentro de cada um
unsigned int farolAberto = TEMPO_INICIAL_FAROL_ABERTO; //tempo do LED principal verde em ms; ~1000-250000
unsigned short int farolAtencao = TEMPO_INICIAL_FAROL_AMARELO; //tempo do LED principal amarelo em ms; unsigned short //é bom que seja múltiplo de 500, por causa do tempo piscando
unsigned short int farolFechado; //será usado para o tempo total calculado após contagem de pedestres
unsigned short int adicionalFarolFechado = TEMPO_ADICIONAL_FAROL_FECHADO; //tempo adicional do LED principal vermelho em ms, serve como base para ajuste
unsigned short int avaliacoesRodada = 10; //recomendado: ~1-100 //que tal fazer uma integral?
unsigned int totalPedestres;
unsigned short int totalPedestresRodada; //como avaliacoes conta apenas amostras, ~0-avaliacoesRodada
//long tempoAtual;

void setup() {
	Serial.begin(VELOCIDADE_SERIAL); //inicia servico de comunicação com a serial usada para enviar para o computador
	esp.serial.begin(VELOCIDADE_SERIAL_ESP); //importante para uso de esp
	iniciarPinos(); //faz os pinMode, selecionando como INPUT ou OUTPUT, etc
	iniciarComunicacaoWire(); //é usado pela MPU. Eu ainda precisaria aprender mais sobre, só desmembrei para ficar menos confuso
	textoTempMPU.reserve(TAMANHO_MEDIO_STRING_TEMP_MPU);
	resumoDados.reserve(TAMANHO_MEDIO_STRING_RESUMO_SENSORES); //reservando x bytes para a global resumoDados, provavelmente evitando fragmentacao e etc

	u8g2.begin(); //inicia a tela OLED
	u8g2.clearDisplay(); //limpa display OLED
	
	mostrarNaOLED(F("Inicio..."));
	
	cronometro.iniciar(); //esse está com objetivo de saber o tempo gasto na inicialização da ESP8266
	esp.resetar();
	esp.iniciarClienteBasico(F(SSID), F(PWD));
	//esp.iniciarCliente(F(SSID), F(PWD));
	//esp.iniciarPontoAcesso(F(SSID_AP), F(PWD_AP), CANAL_AP, ENCRIPT_AP);
	esp.mostrarIP();
	cronometro.terminar();
	cronometro.relatarExecucao(F(MSG_TEMPO_INICIO_ESP8266));
	//Serial.print(cronometro.total);
	//Serial.println();
	//esp.tempoSetup = cronometro.total;

	mostrarNaOLED(F("Iniciado."));
}

void loop() {
	//Antes tinha os parametros iniciais da rodada aqui, mas isso interferia no tempo final. Tudo foi realocado para opera num delay pequeno

	//Captura de valores e semáforo
	
	/*
	Captura de dados dos sensores
	*/
	
	ligarFarol(VERMELHO_PEDESTRE);
	//delay(DELAY_FAROIS_PEDESTRE_MOTORISTA); //deixar mais suave para as pessoas a mudança dos faróis. Substituido por uma janelaTempo
	janelaTempo.iniciar(DELAY_FAROIS_PEDESTRE_MOTORISTA); //substitui o delay da linha acima por uma janela de tempo, mais aproveitável
	cronometro.iniciar();
	if(janelaTempo.validade()) { //esta aqui para aproveitar o tempo que seria para o mero delay acima
		//Iniciar os parâmetros iniciais da rodada
		rodada++; //incrementa numero da rodada
		resumoDados = ""; //essa variavel é limpa no início. Vai receber a concatenacao de conteudo a ser enviado pela ESP
		Serial.println("Rodada " + String(rodada)); //mostra numero de rodada no monitor serial
		mostrarNaOLED("Rodada " + String(rodada)); //mostra numero de rodada na tela OLED
		//delay(1000);

		//Ler dados
		cronometro2.iniciar();
		armazenarValoresSensores(); //armazenar valor de DHT, MPU, fotorresistor...
		cronometro2.terminar();
		cronometro2.relatarExecucao(F(MSG_TEMPO_RECOLHA_DADOS_SENSORES));

		//Preparar dados para envio (tem tambem a funcao de mostrar dados, basta liberar as linhas comentadas)
		cronometro2.iniciar();
		relatarDadosSensores();
		cronometro2.terminar();
		cronometro2.relatarExecucao(F(MSG_TEMPO_INFORMAR_VALORES_SENSORES));
		resumoDados += F("&");
		resumoDados += F("pedestres=");
		resumoDados += String(totalPedestres);

		Serial.println(F("Farol VERDE."));
		mostrarNaOLED(F("Verde"));
	}
	janelaTempo.corrigirTempo();
	cronometro.terminar();
	cronometro.relatarExecucao(F(MSG_DELAY_CAPTURA_DADOS_SENSORES));
	Serial.println();

	/*
	Verde;
	vermelho do pedestre.
	*/

	ligarFarol(VERDE); //intensidade de luz é o valor do fotorresistor

	//delay(farolAberto); //substituido por janela de tempo para ESP8266
	janelaTempo.iniciar(farolAberto); //farolAberto é um parâmetro importante, que se incorpora ao limite
	cronometro.iniciar();
	mostrarTempoRestanteFarolVerde(); //tinha um trecho beta de tratar requisições da ESP dentro	
	cronometro.terminar();
	cronometro.relatarExecucao(F(MSG_TEMPO_TRATAR_REQUISICAO_ESP));

	desligarFarol(VERDE); //digitalWrite LOW
	u8g2.clearDisplay(); //limpa o display, somente
	Serial.println();

	//delay(DELAY_MUDANCA_FAROL); //foi trocado por uma janela de tempo
	
	/*
	Amarelo.
	*/

	janelaTempo.iniciar(DELAY_MUDANCA_FAROL);
	if(janelaTempo.validade()) {
		Serial.println(F("Farol AMARELO."));
		mostrarNaOLED(F("Amarelo"));
		ligarFarol(AMARELO);
	}
	janelaTempo.corrigirTempo();

	//delay(farolAtencao); //tinha uma janela de tempo para ESP, mas foi atualizado para outra janela de tempo, que faz a ESP enviar os dados sozinha
	//Janela de tempo para permitir envio de dados para servidor
	janelaTempo.iniciar(farolAtencao);
	cronometro.iniciar();
	if(janelaTempo.validade()) {
		esp.conectarComoCliente(F(IP));
		esp.geraRequisicao(F(URL), resumoDados);
		esp.fecharConexao();
	}
	cronometro.terminar();
	cronometro.relatarExecucao(F("ms p/ enviar dadosSensores para servidor"));
	janelaTempo.corrigirTempo(); //lembrando que o conteúdo do if leva cerca de 2.6s para ser concluído na configuração atual

	desligarFarol(VERMELHO_PEDESTRE);
	desligarFarol(AMARELO);
	u8g2.clearDisplay();
	Serial.println();

	/*
	Vermelho;
	verde do pedestre.
	*/

	ligarFarol(VERMELHO);

	//delay(DELAY_MUDANCA_FAROL); //substituído por uma janela de tempo
	janelaTempo.iniciar(DELAY_MUDANCA_FAROL);
	if(janelaTempo.validade()) {
		Serial.println(F("Farol VERMELHO."));
		mostrarNaOLED(F("Vermelho"));
	}
	janelaTempo.corrigirTempo();

	delay(DELAY_FAROIS_PEDESTRE_MOTORISTA); //esta aqui para deixar mais suave e seguro para o pedestre o inicio do seu farol verde
	ligarFarol(VERDE_PEDESTRE); //PEDESTRE

	avaliarFluxoPedestres(); //precisa ficar mais flexivel, e mais poderoso
	farolFechado = ((adicionalFarolFechado/avaliacoesRodada)*totalPedestresRodada) + adicionalFarolFechado;

	delay(farolFechado); //tinha uma janela de tempo em versões anteriores
	desligarFarol(VERDE_PEDESTRE);
	piscarVermelhoPedestre(); //função que reveza a cada 500ms o vermelho de pedestre entre aceso e apagado, por x segundos (vide função para encontrar o tempo de operação)

	totalPedestresRodada = 0; //precisa limpar para cada loop, visto que virou global

	desligarFarol(VERMELHO);
	u8g2.clearDisplay();

	delay(DELAY_MUDANCA_FAROL);
	delay(DELAY_FAROIS_PEDESTRE_MOTORISTA); //para deixar mais suave e seguro para o pedestre o inicio do seu farol vermelho, partindo do fim do farol vermelho para motoristas
	Serial.println(F(MSG_FIM_RODADA));
}

//FUNÇÕES ADICIONAIS
void iniciarPinos() {
	pinMode(SENSOR_LUZ, INPUT); 
	pinMode(VERDE, OUTPUT); 
	pinMode(AMARELO, OUTPUT); 
	pinMode(VERMELHO, OUTPUT);
	pinMode(VERDE_PEDESTRE, OUTPUT);
	pinMode(VERMELHO_PEDESTRE, OUTPUT);
}

void iniciarComunicacaoWire() {
	Wire.begin(); //descobrir todo o motivo dessas linhas do Wire no setup!!!
	Wire.beginTransmission(ENDERECO_MPU);
	Wire.write(0x6B);
	//Inicializa o MPU-6050
	Wire.write(0);
	Wire.endTransmission(true);
}

//Funções responsáveis por salvar os dados de sensores
void armazenarValorLDR() {
	registroFotorresistor = analogRead(SENSOR_LUZ);
}

void armazenarValorDHT() {
	DHT.read11(SENSOR_DHT); //Lê as informações do sensor DHT11
}

void prepararAcessoMPU() {
	Wire.beginTransmission(ENDERECO_MPU);
	Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
	Wire.endTransmission(false);
}

void armazenarValoresMPU() {
	Wire.requestFrom(ENDERECO_MPU, 14, true); //importante estudar essa chamada!!!
	//Armazena os valores da MPU nas variaveis correspondentes
	accel.x = Wire.read()<<8|Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
	accel.y = Wire.read()<<8|Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
	accel.z = Wire.read()<<8|Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
	tempMPU = Wire.read()<<8|Wire.read(); //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
	gyro.x = Wire.read()<<8|Wire.read(); //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
	gyro.y = Wire.read()<<8|Wire.read(); //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
	gyro.z = Wire.read()<<8|Wire.read(); //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void armazenarValoresSensores() {
	armazenarValorLDR();
	armazenarValorDHT();
	prepararAcessoMPU(); //Ajusta endereçamento e inicia possibilidade de comunicação com MPU
	armazenarValoresMPU();
}

//Funções responsáveis por mostrar os dados de sensores no Serial
void relatarLuminosidade() { //inclui esboço de JSON
	String texto;
	texto.reserve(TAMANHO_MEDIO_STRING_FOTORRESISTOR);
	//texto += F("?"); //seria útil se fosse GET...
	texto += F("luz=");
	texto += registroFotorresistor;
	/*
	if (registroFotorresistor < LIMIAR_LUZ_BAIXA) {
		texto += F("POUCA LUZ.\n");
		//Serial.print();
		//fazer uso de semaforo amarelo (falta programar)
	} else if (registroFotorresistor > LIMIAR_LUZ_ALTA) {
		texto += F("MUITA LUZ.\n");
	} else {
		//funcionamento padrao
	}
	*/
	//Serial.println(texto);
	resumoDados += texto;
}

void relatarUmidade(dht DHT) {
	String texto;
	texto.reserve(TAMANHO_MEDIO_STRING_DHT);
	texto += F("&");
	texto += F("umidade=");
	texto += DHT.humidity; //é percentual
	resumoDados += texto;
	//Serial.println(texto);
}

void relatarValoresMPU() {
	String texto;
	texto.reserve(TAMANHO_MEDIO_STRING_VALORES_MPU);
	texto += F("&");
	texto += F("acelX=");
	texto += String(accel.x);
	texto += F("&");
	texto += F("acelY=");
	texto += String(accel.y);
	texto += F("&");
	texto += F("acelZ=");
	texto += String(accel.z);
	texto += F("&");
	texto += F("giroX=");
	texto += String(gyro.x);
	texto += F("&");
	texto += F("giroY=");
	texto += String(gyro.y);
	texto += F("&");
	texto += F("gyroZ=");
	texto += String(gyro.z);
	texto += F("&");
	texto += F("temp=");
	textoTempMPU = String(tempMPU/340.00+36.53); //descobrir a lógica desses números do final
	texto += textoTempMPU;
	//Serial.println(texto);
	resumoDados += texto;
}

void relatarDadosSensores() {
	relatarLuminosidade(); //pode mostrar os valores via serial, descomentando a linha de print
	relatarUmidade(DHT); //pode mostrar os valores via serial, descomentando a linha de print
	relatarValoresMPU(); //pode mostrar os valores via serial, descomentando a linha de print
}

void ligarFarol(int pinoFarol) {
	if (registroFotorresistor > LIMIAR_LUZ_BAIXA && registroFotorresistor < LIMIAR_LUZ_ALTA) {
		analogWrite(pinoFarol, VALOR_PWM_FAROL_BAIXO); //escrita que se aproveita de PWM para ajuste de iluminacao; simula porta analógica
	} else {
		digitalWrite(pinoFarol, HIGH); //escrita comum para ligar
	}
}

void desligarFarol(int pinoFarol) {
	digitalWrite(pinoFarol, LOW); //escrita no pino para desligá-lo após um pedido de ligação
}

void mostrarNaOLED(const String& conteudo) { //não deu para usar passagem por referência, deve ser por causa do F()
	//u8g2.clearDisplay();
	u8g2.firstPage();
	do {
		u8g2.setFont(u8g2_font_crox3c_tf); //trocar por alguma fonte maior
		u8g2.setCursor(0, 30);
		u8g2.print(conteudo);
		mostrarHora();
		mostrarTemperatura(); //tem setCursor e print dentro
	} while( u8g2.nextPage() );
}

void mostrarNaOLED(int tamanhoFonte, const String& conteudo) {
	//u8g2.clearDisplay();
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
				fonte = u8g2_font_fur25_tn; //uma fonte maior, para numeros
				posicaoCursorX = 32, posicaoCursorY = 60;
			break;
		}
		u8g2.setCursor(posicaoCursorX, posicaoCursorY); //ajusta x e y na tela para iniciar o texto
		u8g2.print(conteudo); //imprime efetivamente a String informada
		mostrarHora();
		mostrarTemperatura(); //tem setCursor e print dentro
	} while( u8g2.nextPage() );
}

void ajustarExibicaoDigitoDuplo(int tempoRestante) {
	String valorTela;
	if (tempoRestante < 10) { //se sim, acrescenta um 0 à string informada na impressão
		valorTela += F("0");
		valorTela += String(tempoRestante);
	} else {
		valorTela = tempoRestante;
	}
	mostrarNaOLED(2, valorTela);
}

void mostrarTempoRestanteFarolVerde() {
	int contador;
	Temporizador tempoExibicao;
	for (contador = (farolAberto / 1000); contador > 0; contador--) { //faz a contagem de cada 1s para mostrar tempo restante na tela
		tempoExibicao.iniciar();
		ajustarExibicaoDigitoDuplo(contador);
		tempoExibicao.terminar();
		tempoExibicao.relatarExecucao(F(MSG_TEMPO_MOSTRAR_TEMPO_RESTANTE));
		tempoExibicao.terminar(); //terminando de novo para nao considerar o tempo de relatar execucao

		if (tempoExibicao.total > 0) {
			delay(1000 - tempoExibicao.total); //garante que a rodada de mostrar valor na tela vai durar 1s
		}
	}
}

void mostrarHora() {
	tmElements_t tm;
	String hora;
	hora.reserve(5);
	if (RTC.read(tm)) {
    hora += tm.Hour;
    hora += F(":");
	if (String(tm.Minute).length() < 2) {
		hora += F("0");
	}
    hora += tm.Minute;
	} else {
		hora = F("hh:mm");
	}
	u8g2.setFont(u8g2_font_7x14_tf); //ajusta fonte
	u8g2.setCursor(0, 10);
	u8g2.print(hora); //ainda é texto fixo, em breve será alterado
}

void mostrarTemperatura() {
	u8g2.setFont(u8g2_font_7x14_tf); //ajusta fonte
	u8g2.setCursor(80, 10);
	u8g2.print(textoTempMPU + F("'\C"));
}

void avaliarFluxoPedestres() { //essa funcao tem duas operacoes de captacao de dados (por string e por variaveis globais/locais), para possibilitar modularizacao
	unsigned short int contadorAvaliacoes = 0; //~0--avaliacoesRodada
	float distanciaPedestre; //~0-25 (cm)
	Serial.print(F("sr04(amostras(\n"));
	for (contadorAvaliacoes = 0; contadorAvaliacoes < avaliacoesRodada; contadorAvaliacoes++) {
		distanciaPedestre = ultrasonic.Ranging(CM);
		if (distanciaPedestre > DISTANCIA_MAXIMA_ULTRASSOM) { //distancia em cm //poderia ser feita uma avaliacao de pedestres fora do semaforo apropriado
			Serial.print(F("dist:out"));
		} else {
			String distancia = String(distanciaPedestre);
			Serial.print(F("dist:"));
			Serial.print(distancia);
			totalPedestresRodada++;
			Serial.print(F("cm"));
		}
		(contadorAvaliacoes < (avaliacoesRodada -1)) ? Serial.print(F(",\n")) : Serial.print(F("\n)\n")); //'notacao rapida' para if -> (condicao, V ou F) ? conteudo_if : conteudo_else;
		//Serial.println(totalPedestres);
		delay(TEMPO_AVALIACAO); //tem que ver o valor do #define
	}
	//Serial.print(F("\t]\n};\n"));
	totalPedestres += totalPedestresRodada;
	Serial.print(F("Total de pedestres: "));
	Serial.println(totalPedestres);
	Serial.println(F("Fim aval. pedestres."));
}

void piscarVermelhoPedestre() {
	int contador;
	for (contador=0; contador < TEMPO_VERMELHO_PEDESTRE_PISCANTE; contador+=500) {
		delay(DELAY_PISCA_VERMELHO_PEDESTRE_ACESO);
		desligarFarol(VERMELHO_PEDESTRE);
		delay(DELAY_PISCA_VERMELHO_PEDESTRE_APAGADO);
		ligarFarol(VERMELHO_PEDESTRE);
	}
}