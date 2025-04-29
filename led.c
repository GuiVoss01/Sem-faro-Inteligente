//Definindo os pinos e seus respectivos nomes.
#define sinalVmCarro 32  //Sinal Vermelho para carros               
#define sinalAmCarro 25    //Sinal Vermelho para carros               
#define sinalVdCarro 33
#define sinalVmPedestre 21
#define sinalVdPedestre 22
#define botao 18

//Variável para o laço "for"
int contador;
int contagem = 10;                      

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //Definição se os pinos são saídas ou entradas

  pinMode(sinalVmCarro, OUTPUT);           
  pinMode(sinalAmCarro, OUTPUT);            
  pinMode(sinalVdCarro, OUTPUT);
  pinMode(sinalVmPedestre, OUTPUT);
  pinMode(sinalVdPedestre, OUTPUT);
  pinMode(botao, INPUT);

  //Desliga tudo para começar

  digitalWrite(sinalVmCarro, LOW);       
  digitalWrite(sinalAmCarro, LOW);
  digitalWrite(sinalVdCarro, 0);
  digitalWrite(sinalVmPedestre, LOW);
  digitalWrite(sinalVdPedestre, LOW);
  Serial.println("LIBERADO PARA O MOTORISTA");
}

void loop() {
  Serial.println("valor: "+ digitalRead(botao));
  //Farol verde dos veiculos sempre estará aceso enquanto o botao não for pressionado
  while(digitalRead(botao)==1){
      digitalWrite(sinalVdCarro, 1);
      digitalWrite(sinalVmPedestre, 1);  

  //sequencia do semaforo
  if(digitalRead(botao)==0){
    delay(2000);
    digitalWrite(sinalVdCarro, LOW);
    digitalWrite(sinalVmPedestre, HIGH);
    digitalWrite(sinalVmCarro, LOW);
    digitalWrite(sinalAmCarro, HIGH);
    
    delay(2000);
    digitalWrite(sinalAmCarro, 0);
    digitalWrite(sinalVmCarro, 1);
    digitalWrite(sinalVdPedestre,1);
    digitalWrite(sinalVmPedestre, LOW);
    delay(5000);

    // Sinal do semaforo dos pedestres começa a piscar
    for (contador = 1; contador<=10; contador++){
      digitalWrite(sinalVdPedestre,1);   
      delay(500);
      digitalWrite(sinalVdPedestre,0);
      delay((500));
      
      contagem--;
      Serial.println((String) "ATENÇÃO PEDESTRE, O SINAL VAI FECHAR EM " + contagem + " SEGUNDOS"); 
    }

    //Volta ao estado inicial do sistema
    digitalWrite(sinalVdCarro, LOW);
    digitalWrite(sinalVmPedestre, HIGH);
    digitalWrite(sinalVmCarro,0);
  }
  }

}

