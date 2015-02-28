/* barometer.c */
#include <spi.h>
#include <usart.h>
#include <flash.h>
#include <tasks.h>
#include <stdio.h>
#include <stm32f4xx.h>
#include <clock.h>
#include <stdint.h>
#include <stm32f4xx_usart.h>

   enum
    {
      BMP183_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CHIPID             = 0xD0,
      BMP183_REGISTER_VERSION            = 0xD1,
      BMP183_REGISTER_SOFTRESET          = 0xE0,
      BMP183_REGISTER_CONTROL            = 0xF4,
      BMP183_REGISTER_TEMPDATA           = 0xF6,
      BMP183_REGISTER_PRESSUREDATA       = 0xF6,
      BMP183_REGISTER_READTEMPCMD        = 0x2E,
      BMP183_REGISTER_READPRESSURECMD    = 0x34
    };
    
#define ASSERT_CS() GPIO_ResetBits(GPIOA, GPIO_Pin_4); wait(1)
#define DEASSERT_CS() GPIO_SetBits(GPIOA, GPIO_Pin_4); wait(1)

    
static int16_t ac1, ac2, ac3, b1, b2, mb, mc, md; /* Calibration constants */
static uint16_t ac4, ac5, ac6;

#define PRESSURE_THRESHOLD 50
#define TEMPERATURE_THRESHOLD 2
#define SAMPLE_SIZE 20

#define HISTORY_SIZE 100
typedef struct
{
    uint32_t t;
    uint32_t p;
    uint32_t timestamp;
} history_t;

static uint8_t history_full = 0;
static history_t * history_current = 0;
static history_t history_buffer[HISTORY_SIZE];


static void barometer_read(uint32_t *, uint32_t *);

static void calculateAverages(uint32_t * baseAverageT, uint32_t * baseAverageP, uint32_t * currentAverageT, uint32_t * currentAverageP)
{

}

static uint16_t read16(uint8_t address)
{
    uint8_t txBuffer[3] = {0, 0, 0};
    uint8_t rxBuffer[3] = {0, 0, 0};
    char sprintfBuffer[42];
    uint16_t ret;
    
    txBuffer[0] = address;
    ASSERT_CS();
    SPI1_Transfer(txBuffer, rxBuffer, 3);
    DEASSERT_CS();
    ret = rxBuffer[2]; /* LSB */
    ret |= (rxBuffer[1] << 8); /* MSB */
    
    
    return ret; 
}

void BAROMETER_Initialize()
{
    uint8_t printfBuffer[128];
    
    history_full = 0;
    history_current = history_buffer;
    
    ac1 = read16(BMP183_REGISTER_CAL_AC1);
    ac2 = read16(BMP183_REGISTER_CAL_AC2);
    ac3 = read16(BMP183_REGISTER_CAL_AC3);
    ac4 = read16(BMP183_REGISTER_CAL_AC4);
    ac5 = read16(BMP183_REGISTER_CAL_AC5);
    ac6 = read16(BMP183_REGISTER_CAL_AC6);

    b1 = read16(BMP183_REGISTER_CAL_B1);
    b2 = read16(BMP183_REGISTER_CAL_B2);

    mb = read16(BMP183_REGISTER_CAL_MB);
    mc = read16(BMP183_REGISTER_CAL_MC);
    md = read16(BMP183_REGISTER_CAL_MD);
    
    sprintf(printfBuffer, "AC1:%d\r\nAC2:%d\r\nAC3:%d\r\nAC4:%d\r\nAC5:%d\r\nAC6:%d\r\nB1:%d\r\nB2:%d\r\nMB:%d\r\nMC:%d\r\nMD:%d\r\n", ac1, ac2, ac3, ac4, ac5, ac6, b1, b2, mb, mc, md);
    usart_puts(USART2, printfBuffer);
}

void BAROMETER_ClearBuffer()
{
    history_full = 0;
    history_current = history_buffer;
}

void BAROMETER_ReadToBuffer()
{
    uint32_t T, P;
    barometer_read(&T, &P);
    
    history_current->t = T;
    history_current->p = P;
    history_current->timestamp = FlightTime;
    
    history_current++;
    
    if (history_current == history_buffer + HISTORY_SIZE)
    {
        history_full = 1;
        history_current = history_buffer;
    }
}

void BAROMETER_BufferToFlash()
{
    int8_t i;
    if (history_full)
    {
        for(i = 0; i < HISTORY_SIZE; i++)
        {
            FLASH_PutPacket(BAROMETER_TEMPERATURE, history_buffer[i].timestamp, history_buffer[i].t);
            FLASH_PutPacket(BAROMETER_PRESSURE, history_buffer[i].timestamp, history_buffer[i].p);
        }   
    }
    else
    {
        for(i = 0; i < (history_current - history_buffer); i++)
        {
            FLASH_PutPacket(BAROMETER_TEMPERATURE, history_buffer[i].timestamp, history_buffer[i].t);
            FLASH_PutPacket(BAROMETER_PRESSURE, history_buffer[i].timestamp, history_buffer[i].p);
        }
    }
}

void BAROMETER_ReadToFlash()
{
    uint32_t T, P;
    barometer_read(&T, &P);
    FLASH_PutPacket(BAROMETER_TEMPERATURE, FlightTime, T);
    FLASH_PutPacket(BAROMETER_PRESSURE, FlightTime, P);
}

uint8_t BAROMETER_CheckLaunch()
{
    uint32_t baseAverageT = 0;
    uint32_t baseAverageP = 0;
    uint32_t currentAverageT = 0;
    uint32_t currentAverageP = 0;
    uint8_t i;
    history_t * cursor = history_current;
    
    if (history_full == 0)
        return 0;
   
    for (i = 0; i < SAMPLE_SIZE; i++)
    {
        currentAverageT += cursor->t;
        currentAverageP += cursor->p;
        
        cursor--;
        if (cursor <= history_buffer)
            cursor = &history_buffer[HISTORY_SIZE -1];
    }
    currentAverageT = currentAverageT / SAMPLE_SIZE;
    currentAverageP = currentAverageP / SAMPLE_SIZE;
    
     for (i = 0; i < SAMPLE_SIZE; i++)
    {
        baseAverageT += cursor->t;
        baseAverageP += cursor->p;
        
        cursor--;
        if (cursor <= history_buffer)
            cursor = &history_buffer[HISTORY_SIZE -1];
    }
    baseAverageT = baseAverageT / SAMPLE_SIZE;
    baseAverageP = baseAverageP / SAMPLE_SIZE;
        
    if ((currentAverageT > baseAverageT + TEMPERATURE_THRESHOLD)||( currentAverageT < baseAverageT - TEMPERATURE_THRESHOLD))
        return LAUNCH_SOURCE_TEMPERATURE;
    if ((currentAverageP > baseAverageP + PRESSURE_THRESHOLD)||( currentAverageP < baseAverageP - PRESSURE_THRESHOLD))
        return LAUNCH_SOURCE_PRESSURE;
    return 0;
}


static void barometer_read(uint32_t *t, uint32_t *p)
{
    uint8_t txBuffer[10];
    uint8_t rxBuffer[10];
    uint8_t sprintfBuffer[32];
    int32_t UT, UP, B3, B4, B5, B6, B7, X1, X2, X3, T, P, A;
    uint8_t oss = 0;

    /* Initiate temperature read */
    txBuffer[0] = 0x74;
    txBuffer[1] = 0x2E;
    ASSERT_CS();
    SPI1_Transfer(txBuffer, rxBuffer, 2);
    DEASSERT_CS();
    wait(4);
    UT = read16(0xF6);
        
    /* Initiate pressure read */    
    txBuffer[0] = 0x74;
    txBuffer[1] = 0x34;
    ASSERT_CS();
    SPI1_Transfer(txBuffer, rxBuffer, 2);
    DEASSERT_CS();
    wait(4);
    UP = read16(0xF6);

    /* Debug values from the datasheet
  UT = 27898;
  UP = 23843;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
  b1 = 6190;
  b2 = 4;
  ac3 = -14383;
  ac2 = -72;
  ac1 = 408;
  ac4 = 32741;*/
  

    
      // do temperature calculations
  X1=(UT-ac6)*(ac5)/(2<<14);
  X2=(mc*(2<<10))/(X1+md);
  B5=X1 + X2;
  T = (B5+8)/(2<<3);

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = (b2 * (B6 * (B6/(2<<11))))/(2<<10);
  X2 = (ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = (((ac1*4 + X3) << oss) + 2) / 4;

  X1 = (ac3 * B6)/(2<<12);
  X2 = (b1 * (B6 * B6/(2<<11)))/((int32_t)(2<<15));
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = (ac4 * (uint32_t)(X3 + 32768))/(2<<14);
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000 >> oss );
  
  P = (B7 / B4) * 2;
  
  X1 = (P/(2<<7)) * (P/(2<<7));
  X1 = (X1 * 3038)/(2<<15);
  X2 = (-7357 * P)/(2<<15);

  P = P + ((X1 + X2 + 3791)/(2<<3));
 /* A = 44330*(1-pow((P/1013.24f),(1/5.255f))); */ /* Floating point is HARD!*/
  
/*sprintf(sprintfBuffer, "UT:%d UP:%d T:%d P:%d\r\n", UT, UP, T, P);
usart_puts(USART1, sprintfBuffer);*/
    *p = P;
    *t = T;
}
