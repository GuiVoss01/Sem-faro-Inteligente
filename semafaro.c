// Pinos dos LEDs
const int carroVermelho = 33;
const int carroAmarelo = 26;
const int carroVerde = 25;

const int pedestreVermelho = 18;
const int pedestreVerde = 19;

// Pino do botão
const int botao = 5;

void setup() {
  // Inicializa os pinos como saída (LEDs)
  pinMode(carroVermelho, OUTPUT);
  pinMode(carroAmarelo, OUTPUT);
  pinMode(carroVerde, OUTPUT);

  pinMode(pedestreVermelho, OUTPUT);
  pinMode(pedestreVerde, OUTPUT);

  // Inicializa o pino do botão como entrada
  pinMode(botao, INPUT_PULLUP); // Usa resistor interno de pull-up
}

void loop() {
  // Estado inicial: Carros verdes, pedestres vermelhos
  digitalWrite(carroVerde, HIGH);
  digitalWrite(pedestreVermelho, HIGH);

  // Aguarda o botão ser pressionado (LOW porque usamos PULLUP)
  while (digitalRead(botao) == HIGH) {
    delay(10); // Evita debounce sem tratamento extra
  }

  // Botão pressionado: espera 5 segundos antes do ciclo
  delay(5000);

  // Etapa 2: Carros amarelos (pedestres ainda vermelhos)
  digitalWrite(carroVerde, LOW);
  digitalWrite(carroAmarelo, HIGH);
  delay(2000);

  // Etapa 3: Carros vermelhos, pedestres verdes
  digitalWrite(carroAmarelo, LOW);
  digitalWrite(carroVermelho, HIGH);
  digitalWrite(pedestreVermelho, LOW);
  digitalWrite(pedestreVerde, HIGH);
  delay(10000); // 10 segundos

  // Volta ao estado inicial
  digitalWrite(carroVermelho, LOW);
  digitalWrite(pedestreVerde, LOW);
}
