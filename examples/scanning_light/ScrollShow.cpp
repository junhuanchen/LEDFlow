#include "BitLed.h"

void setup()
{
    BitLedOpen();
    
    char * str = " !\"#$%%&()*+'-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\^_`abcdefghijklmnopqrstuvwsyz{|}~]";
    BitScroll(str, strlen(str));
    BitLedExit();
}

void loop()
{
    // put your main code here, to run repeatedly:

}
