
#include <Arduino.h>

uint32_t mp_hal_ticks_us(void);
__attribute__((always_inline)) static inline uint32_t mp_hal_ticks_cpu(void)
{
    uint32_t ccount;
    __asm__ __volatile__("rsr %0,ccount" : "=a"(ccount));
    return ccount;
}

void IRAM_ATTR esp_neopixel_write(uint8_t pin, uint8_t *pixels, uint32_t numBytes, uint8_t timing)
{
    uint8_t *p, *end, pix, mask;
    uint32_t t, time0, time1, period, c, startTime, pinMask;

    pinMask = 1 << pin;
    p = pixels;
    end = p + numBytes;
    pix = *p++;
    mask = 0x80;
    startTime = 0;

    uint32_t fcpu = ets_get_cpu_frequency() * 1000000;

    if (timing == 1)
    {
        // 800 KHz
        time0 = (fcpu * 0.35) / 1000000;  // 0.35us
        time1 = (fcpu * 0.8) / 1000000;   // 0.8us
        period = (fcpu * 1.25) / 1000000; // 1.25us per bit
    }
    else
    {
        // 400 KHz
        time0 = (fcpu * 0.5) / 1000000;  // 0.35us
        time1 = (fcpu * 1.2) / 1000000;  // 0.8us
        period = (fcpu * 2.5) / 1000000; // 1.25us per bit
    }

    uint32_t irq_state = portENTER_CRITICAL_NESTED();
    for (t = time0;; t = time0)
    {
        if (pix & mask)
            t = time1; // Bit high duration
        while (((c = mp_hal_ticks_cpu()) - startTime) < period)
            ;                                       // Wait for bit start
        GPIO_REG_WRITE(GPIO_OUT_W1TS_REG, pinMask); // Set high
        startTime = c;                              // Save start time
        while (((c = mp_hal_ticks_cpu()) - startTime) < t)
            ;                                       // Wait high duration
        GPIO_REG_WRITE(GPIO_OUT_W1TC_REG, pinMask); // Set low
        if (!(mask >>= 1))
        { // Next bit/byte
            if (p >= end)
                break;
            pix = *p++;
            mask = 0x80;
        }
    }
    while ((mp_hal_ticks_cpu() - startTime) < period)
        ; // Wait for last bit
    portEXIT_CRITICAL_NESTED(irq_state);
}

#define LedPower 2
#define LedCtrl  4

#define BitRed 0
#define BitGreen 1
#define BitBlue 2

#define BitFillArea(Led, Size, View, Color) for(uint32_t x = 0, t = View; x < Size; x++) memset(&Led[x * 3], 0, 3), Led[x * 3 + Color[x / 5]] = (t & 1) * 5, t >>= 1

#define BitFillNext(Next, View) View <<= 5, View |= Next

#define BitFillView(Led, Pos, View) memcpy(Led + Pos*5, View, 5)

const uint8_t WordLib[][5] = {
    {0x00, 0x1F, 0x11, 0x1F, 0x00},
    {0x00, 0x12, 0x1F, 0x10, 0x00},
    {0x00, 0x1D, 0x15, 0x17, 0x00},
    {0x00, 0x15, 0x15, 0x1F, 0x00},
    {0x00, 0x07, 0x04, 0x1F, 0x00},
    {0x00, 0x17, 0x15, 0x1D, 0x00},
    {0x00, 0x1F, 0x15, 0x1D, 0x00},
    {0x00, 0x01, 0x1D, 0x03, 0x00},
    {0x00, 0x1F, 0x15, 0x1F, 0x00},
    {0x00, 0x17, 0x15, 0x1F, 0x00},
    {0},{0},{0},{0},{0},{0},{0},
    {0x00, 0x1F, 0x05, 0x1F, 0x00},
    {0x00, 0x1F, 0x15, 0x0A, 0x00},
    {0x00, 0x0E, 0x11, 0x11, 0x00},
    {0x00, 0x1F, 0x11, 0x0E, 0x00},
    {0x00, 0x1F, 0x15, 0x15, 0x00},
    {0x00, 0x1F, 0x05, 0x05, 0x00},
};

inline void BitShow(uint8_t word[], uint8_t Color[5] = {0})
{
    uint8_t tmp[25 * 3] = { };
    uint32_t View = 0;
    
    for(int i = 0; i < 5; i++)
    {
        BitFillNext(word[i], View);

        BitFillArea(tmp, 25, View, Color);
    }
    esp_neopixel_write(LedCtrl, tmp, sizeof(tmp), 1);
}

inline void __BitShow(uint8_t buf[], uint8_t len)
{
    uint8_t tmp[25 * 3] = { };
    uint32_t View = 0;
    uint8_t Color[5] = {0}, Temp = 0;

    for(int i = 0; i < len; i++)
    {
        if(0 == (i % 5)) Temp = (Temp + 1) % 3;

        for(int i = 4; i > 0; i--) Color[i] = Color[i - 1];

        Color[0] = Temp;
        
        BitFillNext(buf[i], View);

        BitFillArea(tmp, 25, View, Color);

        esp_neopixel_write(LedCtrl, tmp, sizeof(tmp), 1);

        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}

inline void BitScroll(char data[], uint datalen)
{
    uint8_t result[(datalen + 2) * 5] = { 0 };
    for(int i = 1; i < datalen + 1; i++) BitFillView(result, i, WordLib[data[i - 1] - '0']);
    __BitShow(result, sizeof(result));
}

inline void BitScroll(uint8_t data[], uint datalen)
{
    __BitShow(data, datalen);
}

inline void BitLedOpen()
{
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    pinMode(4, OUTPUT);

}

inline void BitLedExit()
{
    digitalWrite(2, LOW);
}
