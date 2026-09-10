// ===== OXÍMETRO DE PULSO CASEIRO =====
// Instrumentação Biomédica I - Trabalho 1

const int LED_VERMELHO = 9;   // 660nm
const int LED_INFRA = 10;     // 940nm
const int SENSOR = A0;        // Fotodiodo/LDR

// Buffer para média móvel (filtro passa-baixa simples)
const int TAMANHO_MEDIA = 10;
int bufferVermelho[TAMANHO_MEDIA];
int bufferInfra[TAMANHO_MEDIA];
int indice = 0;

// Variáveis para detectar picos (batimento)
float ultimoValor = 0;
float valorAnterior = 0;
unsigned long ultimoPico = 0;
unsigned long tempoAtual = 0;
float bpm = 0;

// Variáveis AC/DC para cálculo do SpO2
float maxVermelho = 0, minVermelho = 4095;
float maxInfra = 0, minInfra = 4095;
unsigned long ultimaAtualizacaoSpO2 = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_INFRA, OUTPUT);
  
  // Zera os buffers
  for (int i = 0; i < TAMANHO_MEDIA; i++) {
    bufferVermelho[i] = 0;
    bufferInfra[i] = 0;
  }
}

void loop() {
  // --- Lê LED Vermelho ---
  digitalWrite(LED_VERMELHO, HIGH);
  digitalWrite(LED_INFRA, LOW);
  delayMicroseconds(200); // tempo de estabilização
  int leituraVermelha = analogRead(SENSOR);
  digitalWrite(LED_VERMELHO, LOW);

  // --- Lê LED Infravermelho ---
  digitalWrite(LED_INFRA, HIGH);
  delayMicroseconds(200);
  int leituraInfra = analogRead(SENSOR);
  digitalWrite(LED_INFRA, LOW);

  // --- Aplica média móvel (filtro passa-baixa) ---
  bufferVermelho[indice] = leituraVermelha;
  bufferInfra[indice] = leituraInfra;
  indice = (indice + 1) % TAMANHO_MEDIA;

  float mediaVermelho = calcularMedia(bufferVermelho);
  float mediaInfra = calcularMedia(bufferInfra);

  // --- Detecção de pico para BPM ---
  detectarBatimento(mediaInfra);

  // --- Atualiza min/max para cálculo do SpO2 (a cada 3 segundos) ---
  if (mediaVermelho > maxVermelho) maxVermelho = mediaVermelho;
  if (mediaVermelho < minVermelho) minVermelho = mediaVermelho;
  if (mediaInfra > maxInfra) maxInfra = mediaInfra;
  if (mediaInfra < minInfra) minInfra = mediaInfra;

  tempoAtual = millis();
  if (tempoAtual - ultimaAtualizacaoSpO2 > 3000) {
    float spo2 = calcularSpO2();
    Serial.print("BPM: ");
    Serial.print(bpm, 0);
    Serial.print(" | SpO2: ");
    Serial.print(spo2, 0);
    Serial.println("%");

    // Reseta min/max pra próxima janela
    maxVermelho = 0; minVermelho = 4095;
    maxInfra = 0; minInfra = 4095;
    ultimaAtualizacaoSpO2 = tempoAtual;
  }

  // --- Plot no Serial Plotter (sinal bruto filtrado) ---
  Serial.print(mediaVermelho);
  Serial.print(",");
  Serial.println(mediaInfra);

  delay(10); // ~100Hz de amostragem
}

// Média móvel: suaviza ruído de alta frequência (tremor, interferência elétrica)
float calcularMedia(int buffer[]) {
  long soma = 0;
  for (int i = 0; i < TAMANHO_MEDIA; i++) {
    soma += buffer[i];
  }
  return soma / (float)TAMANHO_MEDIA;
}

// Detecta o pico do sinal infravermelho = 1 batimento cardíaco
void detectarBatimento(float valor) {
  valorAnterior = ultimoValor;
  ultimoValor = valor;

  // Pico = quando o sinal sobe e depois começa a descer
  if (valorAnterior > ultimoValor && valorAnterior > 600) { // 600 = limiar, ajustar conforme sensor
    unsigned long agora = millis();
    if (agora - ultimoPico > 300) { // evita contar ruído como 2 batimentos
      unsigned long intervalo = agora - ultimoPico;
      bpm = 60000.0 / intervalo;
      ultimoPico = agora;
    }
  }
}

// Calcula SpO2 pela fórmula empírica padrão
float calcularSpO2() {
  float acVermelho = maxVermelho - minVermelho;
  float dcVermelho = (maxVermelho + minVermelho) / 2.0;
  float acInfra = maxInfra - minInfra;
  float dcInfra = (maxInfra + minInfra) / 2.0;

  float R = (acVermelho / dcVermelho) / (acInfra / dcInfra);
  float spo2 = 110 - 25 * R; // fórmula empírica (varia por calibração)

  if (spo2 > 100) spo2 = 100;
  if (spo2 < 0) spo2 = 0;
  return spo2;
}
