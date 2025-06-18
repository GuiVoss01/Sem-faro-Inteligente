#include <WiFi.h>
#include <WebServer.h>

// Configuração dos pinos
const int carroVermelho = 5;
const int carroAmarelo = 4;
const int carroVerde = 2;
const int pedestreVermelho = 21;
const int pedestreVerde = 18;
const int botao = 19;
const int sensorUmidade = 32;

// Variáveis do sistema
int umidade = 0;
int porcentagem = 0;
int tempoEspera = 5; // em segundos
bool botaoPressionado = false;
bool sistemaPausado = false;
String ultimoEvento = "Sistema iniciado";

// Controle de tempo sem delay()
unsigned long tempoAnterior = 0;
int etapaSemafaro = 0; // 0=inicial, 1=espera, 2=amarelo, 3=pedestres

// WiFi
const char* ssid = "Visitantes";
const char* password = "";

WebServer server(80);

String getHTML() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="pt-BR">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Semáforo Inteligente</title>
    <style>
      body {
        font-family: 'Arial', sans-serif;
        text-align: center;
        background-color: #f5f5f5;
        margin: 0;
        padding: 20px;
      }
      h1 { color: #2c3e50; }
      .container {
        display: flex;
        justify-content: space-around;
        flex-wrap: wrap;
        margin: 20px 0;
      }
      .semaforo {
        background: #fff;
        border-radius: 10px;
        padding: 15px;
        box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        width: 45%;
        min-width: 250px;
        margin: 10px;
      }
      .led {
        font-size: 24px;
        margin: 10px 0;
        transition: all 0.3s;
      }
      .carro-verde { color: #27ae60; }
      .carro-amarelo { color: #f39c12; }
      .carro-vermelho { color: #e74c3c; }
      .pedestre-verde { color: #27ae60; }
      .pedestre-vermelho { color: #e74c3c; }
      .dados {
        background: #fff;
        border-radius: 10px;
        padding: 20px;
        box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        max-width: 600px;
        margin: 20px auto;
      }
      .status-chuva, .status-botao {
        display: inline-block;
        padding: 5px 10px;
        border-radius: 20px;
        font-weight: bold;
      }
      .chovendo { background: #3498db; color: white; }
      .seco { background: #f1c40f; color: black; }
      .botao-ativo { background: #2ecc71; color: white; }
      .botao-inativo { background: #95a5a6; color: white; }
      #botaoPausa {
        padding: 12px 24px;
        font-size: 16px;
        background-color: %COR_BOTAO%;
        color: white;
        border: none;
        border-radius: 5px;
        cursor: pointer;
        margin: 10px;
      }
      .led-ativo {
        font-weight: bold;
        text-shadow: 0 0 5px rgba(0,0,0,0.2);
      }
      .controle-manual {
        background: #fff;
        border-radius: 10px;
        padding: 15px;
        margin: 20px auto;
        max-width: 600px;
        box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      }
      .botoes-controle {
        display: flex;
        gap: 10px;
        justify-content: center;
        flex-wrap: wrap;
      }
      .botao-controle {
        padding: 10px 15px;
        background: #3498db;
        color: white;
        border: none;
        border-radius: 5px;
        cursor: pointer;
      }
      .botao-vermelho {
        background: #3498db;
      }
    </style>
  </head>
  <body>
    <h1>Semáforo Inteligente</h1>
    
    <div class="container">
      <div class="semaforo">
        <h2>Carros</h2>
        <div class="led carro-verde" id="carro-verde">○ Verde</div>
        <div class="led carro-amarelo" id="carro-amarelo">○ Amarelo</div>
        <div class="led carro-vermelho" id="carro-vermelho">○ Vermelho</div>
      </div>
      
      <div class="semaforo">
        <h2>Pedestres</h2>
        <div class="led pedestre-verde" id="pedestre-verde">○ Verde</div>
        <div class="led pedestre-vermelho" id="pedestre-vermelho">○ Vermelho</div>
      </div>
    </div>
    
    <div class="dados">
      <h2>Dados do Sistema</h2>
      <p><strong>Umidade:</strong> <span id="umidade">0</span>% 
        (<span id="status-chuva" class="status-chuva seco">Seco</span>)</p>
      <p><strong>Tempo de Travessia:</strong> <span id="tempo">0</span> segundos</p>
      <p><strong>Botão Físico:</strong> <span id="status-botao" class="status-botao botao-inativo">NÃO PRESSIONADO</span></p>
      <p><strong>Último Evento:</strong> <span id="evento">Aguardando acionamento...</span></p>
      <p><strong>Status Sistema:</strong> <span id="status-pausa">%STATUS%</span></p>
      
      <button id="botaoPausa" onclick="togglePausa()">%TEXTO_BOTAO%</button>
    </div>

    <div class="controle-manual">
      <h2>Controle Manual</h2>
      <div class="botoes-controle">
        <button class="botao-controle" onclick="acionarTravessia()">Acionar Travessia</button>
        <button class="botao-controle" onclick="controlarSemaforo('carro-verde')">Verde Carros</button>
        <button class="botao-controle" onclick="controlarSemaforo('carro-vermelho')">Vermelho Carros</button>
        <button class="botao-controle" onclick="controlarSemaforo('pedestre-verde')">Verde Pedestres</button>
        <button class="botao-controle botao-vermelho" onclick="controlarSemaforo('pedestre-vermelho')">Vermelho Pedestres</button>
      </div>
    </div>

    <script>
      function atualizarDados() {
        fetch("/dados")
          .then(response => response.json())
          .then(data => {
            // Atualiza LEDs com destaque visual
            updateLed("carro-verde", data.carroVerde, "VERDE");
            updateLed("carro-amarelo", data.carroAmarelo, "AMARELO");
            updateLed("carro-vermelho", data.carroVermelho, "VERMELHO");
            updateLed("pedestre-verde", data.pedestreVerde, "VERDE");
            updateLed("pedestre-vermelho", data.pedestreVermelho, "VERMELHO");

            // Atualiza umidade
            document.getElementById("umidade").textContent = data.porcentagem;
            const statusChuva = document.getElementById("status-chuva");
            statusChuva.textContent = data.porcentagem >= 10 ? "Chovendo" : "Seco";
            statusChuva.className = data.porcentagem >= 10 ? "status-chuva chovendo" : "status-chuva seco";

            // Atualiza botão físico
            const statusBotao = document.getElementById("status-botao");
            statusBotao.textContent = data.botaoPressionado ? "PRESSIONADO AGORA" : "NÃO PRESSIONADO";
            statusBotao.className = data.botaoPressionado ? "status-botao botao-ativo" : "status-botao botao-inativo";

            // Atualiza tempo e evento
            document.getElementById("tempo").textContent = data.tempoEspera;
            document.getElementById("evento").textContent = data.ultimoEvento;
            
            // Atualiza pausa
            document.getElementById("status-pausa").textContent = data.pausado ? "PAUSADO" : "EM EXECUÇÃO";
            document.getElementById("botaoPausa").textContent = data.pausado ? "DESPAUSAR" : "PAUSAR";
            document.getElementById("botaoPausa").style.backgroundColor = data.pausado ? "#27ae60" : "#e74c3c";
          });
      }

      function updateLed(id, estado, texto) {
        const elemento = document.getElementById(id);
        if (estado) {
          elemento.textContent = "● " + texto + " (ATIVO)";
          elemento.classList.add("led-ativo");
        } else {
          elemento.textContent = "○ " + texto.split(" ")[0];
          elemento.classList.remove("led-ativo");
        }
      }

      function togglePausa() {
        fetch("/pausar");
      }

      function acionarTravessia() {
        fetch("/acionar-travessia")
          .then(response => response.text())
          .then(data => console.log(data));
      }

      function controlarSemaforo(acao) {
        fetch("/controle-manual?acao=" + acao)
          .then(response => response.text())
          .then(data => console.log(data));
      }

      setInterval(atualizarDados, 200); // Atualização rápida (200ms)
    </script>
  </body>
  </html>
  )rawliteral";

  html.replace("%COR_BOTAO%", sistemaPausado ? "#27ae60" : "#e74c3c");
  html.replace("%TEXTO_BOTAO%", sistemaPausado ? "DESPAUSAR" : "PAUSAR");
  html.replace("%STATUS%", sistemaPausado ? "PAUSADO" : "EM EXECUÇÃO");
  
  return html;
}

void handleDados() {
  String json = "{";
  json += "\"carroVermelho\":" + String(digitalRead(carroVermelho)) + ",";
  json += "\"carroAmarelo\":" + String(digitalRead(carroAmarelo)) + ",";
  json += "\"carroVerde\":" + String(digitalRead(carroVerde)) + ",";
  json += "\"pedestreVermelho\":" + String(digitalRead(pedestreVermelho)) + ",";
  json += "\"pedestreVerde\":" + String(digitalRead(pedestreVerde)) + ",";
  json += "\"porcentagem\":" + String(porcentagem) + ",";
  json += "\"tempoEspera\":" + String(tempoEspera) + ",";
  json += "\"botaoPressionado\":" + String(botaoPressionado ? "true" : "false") + ",";
  json += "\"ultimoEvento\":\"" + ultimoEvento + "\",";
  json += "\"pausado\":" + String(sistemaPausado ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handlePausar() {
  sistemaPausado = !sistemaPausado;
  ultimoEvento = "Sistema " + String(sistemaPausado ? "pausado" : "despausado") + " às " + String(millis() / 1000) + "s";
  server.send(200, "text/plain", "OK");
}

void handleAcionarTravessia() {
  if (!botaoPressionado && !sistemaPausado) {
    botaoPressionado = true;
    ultimoEvento = "Travessia acionada pelo WEB às " + String(millis() / 1000) + "s";
    
    // Lê sensor
    umidade = analogRead(sensorUmidade);
    porcentagem = map(umidade, 0, 4095, 100, 0);
    tempoEspera = (porcentagem >= 10) ? 2 : 5;
    
    etapaSemafaro = 1;
    tempoAnterior = millis();
  }
  server.send(200, "text/plain", "Travessia acionada!");
}

void handleControleManual() {
  String acao = server.arg("acao");
  
  if (acao == "carro-verde") {
    digitalWrite(carroVerde, HIGH);
    digitalWrite(carroVermelho, LOW);
    digitalWrite(carroAmarelo, LOW);
    ultimoEvento = "Semáforo de carros FORÇADO para VERDE";
  } 
  else if (acao == "carro-vermelho") {
    digitalWrite(carroVerde, LOW);
    digitalWrite(carroVermelho, HIGH);
    digitalWrite(carroAmarelo, LOW);
    ultimoEvento = "Semáforo de carros FORÇADO para VERMELHO";
  }
  else if (acao == "pedestre-verde") {
    digitalWrite(pedestreVerde, HIGH);
    digitalWrite(pedestreVermelho, LOW);
    ultimoEvento = "Semáforo de pedestres FORÇADO para VERDE";
  }
  else if (acao == "pedestre-vermelho") {
    digitalWrite(pedestreVerde, LOW);
    digitalWrite(pedestreVermelho, HIGH);
    ultimoEvento = "Semáforo de pedestres FORÇADO para VERMELHO";
  }

  server.send(200, "text/plain", "Controle aplicado: " + acao);
}

void setup() {
  // Configuração dos pinos
  pinMode(carroVermelho, OUTPUT);
  pinMode(carroAmarelo, OUTPUT);
  pinMode(carroVerde, OUTPUT);
  pinMode(pedestreVermelho, OUTPUT);
  pinMode(pedestreVerde, OUTPUT);
  pinMode(botao, INPUT_PULLUP);
  pinMode(sensorUmidade, INPUT);

  Serial.begin(115200);

  // Conecta ao WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado! IP: " + WiFi.localIP().toString());

  // Rotas do servidor
  server.on("/", []() { server.send(200, "text/html", getHTML()); });
  server.on("/dados", handleDados);
  server.on("/pausar", handlePausar);
  server.on("/acionar-travessia", handleAcionarTravessia);
  server.on("/controle-manual", handleControleManual);
  server.begin();

  // Estado inicial
  digitalWrite(carroVerde, HIGH);
  digitalWrite(pedestreVermelho, HIGH);
}

void loop() {
  server.handleClient();

  if (!sistemaPausado) {
    // Verifica botão físico
    if (digitalRead(botao) == LOW && !botaoPressionado) {
      botaoPressionado = true;
      ultimoEvento = "Botão pressionado às " + String(millis() / 1000) + "s";
      
      // Lê sensor
      umidade = analogRead(sensorUmidade);
      porcentagem = map(umidade, 0, 4095, 100, 0);
      tempoEspera = (porcentagem >= 10) ? 2 : 5;
      
      etapaSemafaro = 1;
      tempoAnterior = millis();
    }

    // Máquina de estados do semáforo
    if (botaoPressionado) {
      switch (etapaSemafaro) {
        case 1: // Espera o tempo ajustado
          if (millis() - tempoAnterior >= tempoEspera * 1000) {
            digitalWrite(carroVerde, LOW);
            digitalWrite(carroAmarelo, HIGH);
            etapaSemafaro = 2;
            tempoAnterior = millis();
            ultimoEvento = "Semáforo amarelo ativado";
          }
          break;

        case 2: // Amarelo (2s)
          if (millis() - tempoAnterior >= 2000) {
            digitalWrite(carroAmarelo, LOW);
            digitalWrite(carroVermelho, HIGH);
            digitalWrite(pedestreVermelho, LOW);
            digitalWrite(pedestreVerde, HIGH);
            etapaSemafaro = 3;
            tempoAnterior = millis();
            ultimoEvento = "Semáforo vermelho/pedestre verde ativado";
          }
          break;

        case 3: // Pedestres verdes (10s)
          if (millis() - tempoAnterior >= 10000) {
            digitalWrite(pedestreVerde, LOW);
            digitalWrite(pedestreVermelho, HIGH);
            digitalWrite(carroVermelho, LOW);
            digitalWrite(carroVerde, HIGH);
            etapaSemafaro = 0;
            botaoPressionado = false;
            ultimoEvento = "Ciclo completo às " + String(millis() / 1000) + "s";
          }
          break;
      }
    }
  }
}
