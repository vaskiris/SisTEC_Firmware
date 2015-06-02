#include <Stepper.h>								//biblioteca de funcoes referentes ao controlo do motor Stepper
#include <Wire.h>                                            			//Biblioteca de funções para o controlo de ADCs
#include <Adafruit_ADS1015.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>																		

#define MAX_OUT_CHARS 16                                       			//Numero maximo de caracteres a ser enviado na comunicaçao serie

int calibrationStatus = 0;							//variavel de controlo
int objectiveStatus = 0;							//variavel de controlo
int testType = 0;																					//variavel de controlo
int velocity = 0;
int start = 0;
int aquisition = 0;

const int stepsPerRevolution = 200; 
Stepper myStepper(stepsPerRevolution, 8,9,10,11);   
unsigned long stepCount = 0; 
double deslocamento = 0;

//ADC//

Adafruit_ADS1115 ads;              						//Declaração do ADC
char buffer[MAX_OUT_CHARS + 1];  					        //buffer used to format a line (+1 is for trailing 0)
int16_t results;

//Comunicacao
String comand = "";
String subComand = "";
String iniComand = "";
String iniType = "";

//constantes
char velConvert [1];
char aqConvert [1];
int type;
long tempactual, tempanterior, intervalo;
long tempactualAqui, tempanteriorAqui, intervaloAqui;
int calibStart = 0;
const int driverControl = 7;
int counter = 0;

//Botoes

const int buttonPinUp = 2;
const int buttonPinDown = 4;
const int ledPin =  13;    
int buttonStateUp = 0;
int buttonStateDown = 0;
long buttonTimeIni = 0;
long buttonTimeFinal = 0;
int carPosition = 1;


void setup() {
	// inicializacao do porto serie
	Serial.begin(9600);     	
	//inicializacao do ADC
	ads.begin();	

        pinMode(ledPin, OUTPUT);  
        pinMode(buttonPinUp, INPUT);
        pinMode(buttonPinDown, INPUT);
        
        pinMode(driverControl, OUTPUT);																			
}

void loop(){
  
          //BOTOES
         buttonStateUp = digitalRead(buttonPinUp);
         buttonStateDown = digitalRead(buttonPinDown);

         if (buttonStateUp == HIGH && carPosition == 1){
           Serial.println("Botao Up");
           myStepper.setSpeed(100);
           myStepper.step(-100);
           start = 0;
           carPosition = 2;
           digitalWrite(ledPin, HIGH);
           digitalWrite(driverControl, LOW);
           Serial.println("Car Position :");
           Serial.println(carPosition);
         }
         else {
           digitalWrite(ledPin, LOW);
         }
         
         if (buttonStateDown == HIGH && carPosition == 1){
           Serial.println("Botao Down");
           myStepper.setSpeed(100);
           myStepper.step(100);
           start = 0;
           carPosition = 0;
           stepCount = 0; 
           digitalWrite(ledPin, HIGH);
           digitalWrite(driverControl, LOW);
           Serial.println("Car Position :");
           Serial.println(carPosition);

         }
         else {
          // turn LED off:
          digitalWrite(ledPin, LOW); 
         }
	
	if(Serial.available()>0){
                  comandReader();
        }
		
		if(start == 1){
			tempactual = micros();
                        tempactualAqui = micros();
                        if ((tempactual - tempanterior) > intervalo){
                            myStepper.step(testType);
                            tempanterior = tempactual;
                            stepCount = stepCount + 1;
                        }
                        if ((tempactualAqui - tempanteriorAqui) > intervaloAqui){  
                                sendData();
                                tempanteriorAqui = tempactualAqui;
                        }
                         
                         
		}
                if(calibStart == 1){
                  tempactualAqui = micros();
                  tempactual = micros();
                  if(stepCount < 10000){
                    if ((tempactual - tempanterior) > intervalo && (buttonStateDown == LOW)){
                            myStepper.step(1);
                            tempanterior = tempactual;
                            stepCount = stepCount + 1;
                        }
                        if ((tempactualAqui - tempanteriorAqui) > intervaloAqui && counter <20){  
                                results = -1 * ads.readADC_Differential_0_1();
		                sprintf(buffer,"#C%d",results);
		                Serial.println(buffer);
                                tempanteriorAqui = tempactualAqui;
                                counter ++;
                        }
                    
                  }
                  else{
                    if (((tempactual - tempanterior) > intervalo) && (buttonStateDown == LOW)){
                        myStepper.step(-1);
                        tempanterior = tempactual;
                    }
                  }
                  
                 //ISTO DEVE SER PARA TIRAR
                 if(buttonStateDown == HIGH){
                   myStepper.setSpeed(100);
                   myStepper.step(100);
                   calibStart = 0;
                   carPosition = 0;
                   counter = 0;
                   digitalWrite(ledPin, HIGH);
                   digitalWrite(driverControl, LOW);
                   Serial.println("Car Position :");
                   Serial.println(carPosition);
                 }
                 else {
                   digitalWrite(ledPin, LOW);
                 }
                 
                 if (buttonStateUp == HIGH){
                   myStepper.setSpeed(100);
                   myStepper.step(-100);
                   calibStart = 0;
                   carPosition = 2;
                   counter = 0;
                   digitalWrite(ledPin, HIGH);
                   digitalWrite(driverControl, LOW);
                   Serial.println("Car Position :");
                   Serial.println(carPosition);
                 }
                 else {
                  // turn LED off:
                  digitalWrite(ledPin, LOW); 
                 }
                 
                }
                
	
        
}
//Le e executa comandos recebidos
void comandReader(){
  
                char endComand = Serial.read();
		comand = comand + endComand;
		//compara ultimo caracter, procede se for '%'
		if(endComand == '%'){
                        Serial.println("Comand Reader"); 
			iniComand = comand.substring(0,1);
			iniType = comand.substring(1,2);
                        
                        if (iniComand == "C"){
                          type = 1;
                        }
                        if (iniComand == "E"){
                          type = 2;
                        }
                        if (iniComand == "T"){
                          type = 3;
                        }
                        if (iniComand == "R"){
                          type = 4;
                        }
                        if (iniComand == "U"){
                          type = 5;
                        }
                        if (iniComand == "D"){
                          type = 6;
                        }
			switch (type){
    			        case 1:
    					//rotina de calibracao
    					calibration();
    					calibrationStatus = 1;
                                        intervaloAqui = 1000000/2;
    					break;
    				case 2:
    					//iniciar motor
                                        if( iniType == "1"){
                                        Serial.println("Motor Start"); 
        					if(objectiveStatus == 0){
        						noObjectiveError();
        					}
        					else{
                                                        digitalWrite(driverControl, HIGH);
        						start = 1;
                                                }
    					}
                                        else{
                                                Serial.println("Motor Stoped");
                                                digitalWrite(driverControl, LOW);
    						start = 0;
                                        }
    					break;
    				case 3:
    					//define tipo de teste, 0->compressao, 1->tensao
    					
                                        tempactual, tempanterior, intervalo = 0;
                                        if( iniType == "0" /*&& carPosition != 0*/){
                                                Serial.println("Test Type 0"); 
    						testType = -1;
                                                carPosition = 1;
                                                //Serial.println(carPosition);
    					}
    					if( iniType == "1" /*&& carPosition != 2*/ ){
                                                Serial.println("Test Type 1"); 
    						testType = 1;
                                                carPosition = 1;
                                                //Serial.println(carPosition);
    					}
                                        velConvert[0] = comand[3];
    					velocity = int(velConvert[0]) - '0';
                                        intervalo = 10000/velocity;
                                        Serial.println("Velocity :");
                                        Serial.println(velocity); 
                                        aqConvert[0] = comand[5];
    					aquisition = atoi(aqConvert);
                                        intervaloAqui = 1000000/aquisition;
                                        Serial.println("Aquisition :");
                                        Serial.println(aquisition); 
    					objectiveStatus = 1;
    					break;
                                case 4:
    					//Reset do deslocamento
    					stepCount = 0;
    					break;
                                case 5:
                                        digitalWrite(driverControl, HIGH);
                                        myStepper.setSpeed(100);
    					myStepper.step(50);
                                        digitalWrite(driverControl, LOW);
    					break;
                                case 6:
                                        digitalWrite(driverControl, HIGH);
                                        myStepper.setSpeed(100);
    					myStepper.step(-50);
                                        digitalWrite(driverControl, LOW);
    					break;
    				default:
                                        ;
				}
                          comand = "";
                    }
                    
}

void calibration(){
      Serial.println("Calibration");
      digitalWrite(driverControl, HIGH);
        calibStart = 1;
        intervalo = 1500;
        stepCount = 0;
	/*for(int i = 0; i < 20; i++){
		results = -1 * ads.readADC_Differential_0_1();
		sprintf(buffer,"#C%d",results);
		Serial.println(buffer); 	
	}*/
      comand = "";
}

void noObjectiveError(){
        Serial.println("Error: No Objective"); 
	sprintf(buffer,"#Unconfigured Test");
	Serial.println(buffer); 	
}

void sendData(){
        
        Serial.println("Send Data"); 
	results = -1 * ads.readADC_Differential_0_1();
	sprintf(buffer,"#F%d",results);
	Serial.println(buffer);
	sprintf(buffer,"#D%lu",stepCount); 
	Serial.println(buffer); //envio dos dados do ADC

}
