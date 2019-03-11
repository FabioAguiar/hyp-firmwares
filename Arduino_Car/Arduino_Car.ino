int rodaA1 = 5;
int rodaA2 = 6;
int rodaB1 = 10;
int rodaB2 = 11;
int velocidade = 255;


void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);  
  pinMode(rodaA1, OUTPUT);
  pinMode(rodaA2, OUTPUT);
  pinMode(rodaB1, OUTPUT);
  pinMode(rodaB2, OUTPUT);      
}

void loop() {
  char comando;
    
  if(Serial1.available() > 0){
    comando = Serial1.read();
    Serial.print(comando);
    if(comando == '0' || comando == '1' || comando == '2' || 
        comando == '3' || comando == '4' || comando == '5' || 
          comando == '6' || comando == '7' || comando == '8' || 
            comando == '9'){
              Serial.println("Entrou: ");
              velocidade = atoi(comando);
              velocidade = map(velocidade, 0, 9, 0, 255);
              Serial.print("Velocidade: ");
              Serial.println(velocidade);
        }
        
  }

  

  switch(comando){
    case 'S':
      parar();
      break;
    case 'F':
      frente();
      break;      
    case 'B':
      tras();
      break;
    case 'L':
      girarEsquerda();
      break;
    case 'R':
      girarDireita();
      break;
    default:
      break;
  }
}

void frente(){
  analogWrite(rodaA1, velocidade);
  analogWrite(rodaA2, LOW);
  analogWrite(rodaB1, velocidade);
  analogWrite(rodaB2, LOW);
  
//  digitalWrite(rodaA1, HIGH);
//  digitalWrite(rodaA2, LOW);
//  digitalWrite(rodaB1, HIGH);
//  digitalWrite(rodaB2, LOW);
}

void tras(){
  digitalWrite(rodaA1, LOW);
  digitalWrite(rodaA2, HIGH);
  digitalWrite(rodaB1, LOW);
  digitalWrite(rodaB2, HIGH);
}

void parar(){
  digitalWrite(rodaA1, LOW);
  digitalWrite(rodaA2, LOW);
  digitalWrite(rodaB1, LOW);
  digitalWrite(rodaB2, LOW);
}

void girarEsquerda(){
  digitalWrite(rodaA1, LOW);
  digitalWrite(rodaA2, HIGH);
  digitalWrite(rodaB1, HIGH);
  digitalWrite(rodaB2, LOW);
}

void girarDireita(){
  digitalWrite(rodaA1, HIGH);
  digitalWrite(rodaA2, LOW);
  digitalWrite(rodaB1, LOW);
  digitalWrite(rodaB2, HIGH);
}
