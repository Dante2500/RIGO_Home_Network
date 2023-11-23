#ifndef RAK3272_LIB_P2P
#define RAK32_LIB_P2P

#include <Arduino.h>


class RAK3272
{
    public:
        RAK3272(uint32_t Freq, byte SF, byte BW, byte CR, uint16_t PREAMB_LEN, byte TX_PW);
        void inicialize();
        void check();
        void sendCommand(String command, bool impresion);
        void sendMessage(String message);
        //String sendCommand_Hearval(String command, String response, bool impresion);
        void sendCommand_Hearval(String command, String *response, byte size_response,  bool impresion);
        String receiveMessage();

    private:
        u_int32_t _Freq;
        byte _SF;
        byte _BW;
        byte _CR;
        uint16_t _PREAMB_LEN;
        byte _TX_PW;


};

#endif