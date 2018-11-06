#include "BitLed.h"

void setup()
{
    BitLedOpen();
    
    BitScroll(tmp, 5);
    delay(1000);

    char * str = "0123456789ABCDEF";
    BitScroll(str, strlen(str));
    BitLedExit();
}

void loop()
{
    // put your main code here, to run repeatedly:

}
