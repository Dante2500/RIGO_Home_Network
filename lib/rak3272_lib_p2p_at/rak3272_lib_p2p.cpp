#include "rak3272_lib_p2p.h"

RAK3272::RAK3272(uint32_t Freq, byte SF, byte BW, byte CR, uint16_t PREAMB_LEN, byte TX_PW){
    _Freq = Freq;
    _SF = SF;
    _BW = BW;
    _CR = CR;
    _PREAMB_LEN = PREAMB_LEN;
    _TX_PW = TX_PW;
}

void RAK3272::inicialize(){
    //buffer para armar el comando con los parametros dados
    char command[38];
    //respuesta por el RAK3272: la primer String es el value, y la 2 string es el status code
    String P2P_response[2];
    //Respuesta esperada: no se incluye el \n por ser el caracter terminador
    String P2P_expected[2] = {"AT+NWM=0\r","OK\r"};
    
    //Asegurarse que el nodo está en configuración P2P
    sendCommand_Hearval("AT+NWM=?",P2P_response, 2, true);


    // SI no esta en P2P, inicializarLO.
    if(P2P_response[0].compareTo(P2P_expected[0]) !=0){
        Serial.print("Corrigiendo...");
        Serial2.print(P2P_expected[0]+'\n');
    }
    Serial2.flush();
    //cargar los parametros del modulo
    sprintf(command, "AT+P2P=%li:%d:%d:%d:%i:%d",_Freq,_SF, _BW, _CR,_PREAMB_LEN,_TX_PW);
    sendCommand(String(command), true);
}

void RAK3272::check(){
    String message="";
    Serial2.print("AT\r\n");
    Serial2.flush();
    while(Serial2.available()){
        message=Serial2.readStringUntil('\n');
        Serial.print(message);
    }
}

void RAK3272::sendCommand(String command, bool impresion){
    Serial2.print(command+"\r\n");
    Serial2.flush();
    delay(250);
    if (impresion == true){
        while(Serial2.available()){
            Serial.println(Serial2.readStringUntil('\n'));
        }
    }
}

void RAK3272::sendMessage(String payload){
    sendCommand("AT+PSEND="+payload, true);
}



void RAK3272::sendCommand_Hearval(String command, String *response, byte size_response,  bool impresion){
    
    Serial2.print(command+"\r\n");
    Serial2.flush();
    delay(250);
    byte i=0;
    while(Serial2.available()){
        if(i<size_response){
            *response= Serial2.readStringUntil('\n');
            response++;
            i++;
        }
    }
    response = response-i;

    if(impresion == true){
        for(i=0;i<size_response;i++){
            Serial.println(*response);
            response++;
        }
    }
    response = response-i;
}

String RAK3272::receiveMessage(){
    String Recepted;
    Serial2.flush();
    delay(250);
    while(Serial2.available()){
        Recepted= Serial2.readStringUntil('\n');
    }
    
    return Recepted;

}
