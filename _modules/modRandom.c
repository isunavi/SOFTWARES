#include "modRandom.h"
#include "board.h"


// максимальное число попыток инициализации
#define RAND_MAX_ATTEMPTS 5

#define POLINOM_32 0xE0000200

//#define SORS_RAND(n)   (*((volatile uint32_t *)(0x20000000 + 4 * n)))

//
uint32_t modRnd_rand32;

/**
int _rand(void) // RAND_MAX assumed to be 32767.
{
  static unsigned long next = 1;
  next = next * 1103515245 + 12345;
  return next >> 16;
}
*/

/**
 * Функция-рандомайзер 32 бит
 * @return рандомное число в диапазоне от 0 to 429467296-1
 */
uint32_t _rand32 (void)
{
    uint32_t i;
    
    for (i = 0; i < 32; i++)
    {
        if (modRnd_rand32 & 0x00000001)
        {
            modRnd_rand32 = (modRnd_rand32 >> 1) ^ POLINOM_32;
        } else {
            modRnd_rand32 = (modRnd_rand32 >> 1);
        }
    }
    
    return modRnd_rand32;
}


uint16_t _rand16 (void)
{
    uint32_t i;
    
    for (i = 0; i < 16; i++)
    {
        if (modRnd_rand32 & 0x00000001)
        {
            modRnd_rand32 = (modRnd_rand32 >> 1) ^ POLINOM_32;
        } else {
            modRnd_rand32 = (modRnd_rand32 >> 1);
        }
    }
    
    return modRnd_rand32;
}


uint8_t  _rand8 (void)
{
    uint32_t i;
    
    for (i = 0; i < 8; i++)
    {
        if (modRnd_rand32 & 0x00000001)
        {
            modRnd_rand32 = (modRnd_rand32 >> 1) ^ POLINOM_32;
        } else {
            modRnd_rand32 = (modRnd_rand32 >> 1);
        }
    }
    
    return modRnd_rand32;
}


uint8_t  _rand1 (void)
{
    if (modRnd_rand32 & 0x00000001)
    {
        modRnd_rand32 = (modRnd_rand32 >> 1) ^ POLINOM_32;
    } else {
        modRnd_rand32 = (modRnd_rand32 >> 1);
    }

    return modRnd_rand32 & 0x01;
}


void  _rand (uint8_t *buf, uint32_t num)
{
    uint32_t i, j;
    
    for (i = 0; i < num; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (modRnd_rand32 & 0x00000001)
            {
                modRnd_rand32 = (modRnd_rand32 >> 1) ^ POLINOM_32;
            } else {
                modRnd_rand32 = (modRnd_rand32 >> 1);
            }
        }
        buf [i] = modRnd_rand32;
    }
}


void  _rand_real (uint8_t *buf, uint32_t num) //ADC conf. & work DESTROED!!!
{
#if (BOARD_STM32F4DISCOVERY || BOARD_STM32F411_REBORN)
    uint32_t i, j, tmp8;
    
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;     // подаем такты на АЦП
	ADC1->CR2 |= ADC_CR2_ADON;              // подаем питание на ФЦП
	//ADC1->CR2 |= ADC_CR2_TSVREFE;   // подаем питание на темп. сенсор и датчик напр.
	ADC1->CR2 |= ADC_CR2_EXTSEL;    // запуск преобразования по установки бита swstart
	//ADC1->CR2 |= ADC_CR2_EXTTRIG;   // включаем запуск от внешнего события (у нас это свтарт)
	ADC1->SMPR1 |= ADC_SMPR1_SMP16; // ставлю макс. кол-во цыклов (239.5) на преобразование для 16 канала где теп. сенсор
	ADC1->SQR3 &= ~ADC_SQR3_SQ1;
	ADC1->SQR3 |= 6; //ADC_SQR3_SQ1_4;    // выбираем 16 (0b10000) канал для 1 преобразования     (кол-во преобразования по-умолчанию 1)
    
    for (j = 0; j < num; j++)
    {
        buf [j] = 0;
        for (i = 0; i < 8; i++)
        {
            ADC1->CR2 |= ADC_CR2_SWSTART; // запуск преобразования
            while (!(ADC1->SR & ADC_SR_EOC)) {}; // ждем конца преобразования
            //buf [j] ^tmp8 = (uint8_t)((ADC1->DR << i) & (1 << i));
            tmp8 = (uint16_t)(ADC1->DR & 0x0001);
            buf [j] |= tmp8;
            buf [j] = buf [j] << 1;
        }
    }

#endif
}


#if MODRANDOM_4096BIT_EN
// #define SIZE_  (64/32)
// const unsigned long c_rand[SIZE_]={
//     0,
//     0xB0000000
// };
//  // ! обязательно посл. 0!
// unsigned long _rand[(SIZE_+1)] = {676667,15655567 ,0};


//@todo repair!
#define MODRAND_4096BIT_SIZE  (4096UL / 8 / 4) //число слов

static const uint32_t c_rand[MODRAND_4096BIT_SIZE] = { // 1024; 1015; 1002; 1001
    0, 0, 0, 0, 
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 
    0x00800600
};


 //обязательно последнее слово 0x00000000 !!!
uint32_t _rand[(MODRAND_4096BIT_SIZE +1)]; // = {676667, 15655567 ,0};

uint32_t _rand4096_32 (void)
{
    uint8_t i, j, m;
    uint32_t n;
	  
    for (i = 0; i < 32; i++)
    {
        m = _rand[0] & 0x00000001;
        for (j = 0; j < MODRAND_4096BIT_SIZE; j++)
        {
            _rand[j] = _rand[j] >> 1;
            n = (_rand[j +1] & 0x00000001);
            _rand[j] |= (n << 31);
        } 
        if (0 != m)
        { 
            for (j = 0; j < (MODRAND_4096BIT_SIZE); j++)
            {
                _rand[j] = _rand[j] ^ c_rand[j];
            }
        }
    }
    
    return _rand[0];
}


uint8_t _rand4096_8 (void)
{
    uint8_t i, j, m;
    uint32_t n;
	  
    for (i = 0; i < 8; i++)
    {
        m = _rand[0] & 0x01;
        for (j = 0; j < (MODRAND_4096BIT_SIZE * 4); j++)
        {
            _rand[j] = _rand[j] >> 1;
            n = (_rand[j +1] & 0x00000001);
            _rand[j] |= (n << 31);
        } 
        if (0 != m)
        { 
            for (j = 0; j < (MODRAND_4096BIT_SIZE * 4); j++)
            {
                _rand[j] = _rand[j] ^ c_rand[j];
            }
        }
    }
    
    return _rand[0];
}


#endif



uint32_t _srand (void)
{
#if BOARD_TEST
    static const uint8_t TIME[10] = __TIME__;
    uint8_t *pST32 = (uint8_t *) &modRnd_rand32;
    //for (i = 0; i < sizeof(uint32_t); i++)
    {
        //modRnd_rand32 += TIME[i]; //@todo Добавить проверку на 0!
        pST32[3] = TIME[3] - 0x30;
        pST32[2] = TIME[4] - 0x30;
        pST32[1] = TIME[6] - 0x30;
        pST32[0] = TIME[7] - 0x30;
    }  
#endif
#if BOARD_STM32F4DISCOVERY_
    //PA0 – вход АЦП
    GPIO_InitTypeDef GPIO_InitStructure; 
    ADC_InitTypeDef  ADC_InitStructure;   
    ADC_CommonInitTypeDef  ADC_CommonInitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* PCLK2 is the APB2 clock */
    /* ADCCLK = PCLK2/6 = 72/6 = 12MHz*/
    ///RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* Enable ADC1 clock so that we can talk to it */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    /* Put everything back to power-on defaults */
    ADC_DeInit();

//     /* ADC1 Configuration ------------------------------------------------------*/
//     /* ADC1 and ADC2 operate independently */
//     // ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
//     /* Disable the scan conversion so we do one at a time */
//     ADC_InitStructure.ADC_ScanConvMode = DISABLE;
//     /* Don't do contimuous conversions - do them on demand */
//     ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
//     /* Start conversin by software, not an external trigger */
//     // ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//     /* Conversions are 12 bit - put them in the lower 12 bits of the result */
//     ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//     /* Say how many channels would be used by the sequencer */
//     ADC_InitStructure.ADC_NbrOfConversion = 1;


    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge =  ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv =      ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_9Cycles;
    
    ADC_CommonInit(&ADC_CommonInitStructure);
    /* Now do the setup */
    ADC_Init(ADC1, &ADC_InitStructure);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);

//         /* Enable ADC1 reset calibaration register */
//         ADC_ResetCalibration(ADC1);
//         /* Check the end of ADC1 reset calibration register */
//         while(ADC_GetResetCalibrationStatus(ADC1));
//         /* Start ADC1 calibaration */
//         ADC_StartCalibration(ADC1);
//         /* Check the end of ADC1 calibration */
//         while(ADC_GetCalibrationStatus(ADC1));


    while(1)//attempts++ < MAX_ATTEMPTS) 
    {
        uint32_t i;
        // читаем ацп, самый младший бит самый мусорный, им и инициализируем
        for (i = 0; i < 32; i++)
        {
            // Читаем нулевой канал
            ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);
            // Start the conversion
            ADC_SoftwareStartConv(ADC1);
            // Wait until conversion completion
            while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
            // Get the conversion value
            modRnd_rand32 ^= (uint32_t)((ADC_GetConversionValue(ADC1) << i) & (1 << i));
        }
        if (modRnd_rand32 != 0) // проверка на 0
        {
#if MODRANDOM_TRUE_4096
            uint32_t j = 0;
            _rand[j] = modRnd_rand32;
            j++;
            if (j>= SIZE_)
                break;
#endif
            break;
        }
    }
#endif
		
#if BOARD_STM32EV103_V2
    extern ADC_HandleTypeDef hadc1;
    
    HAL_ADC_Start (&hadc1);
    while(1) 
    {
        uint32_t i;
        // читаем ацп, самый младший бит самый мусорный, им и инициализируем
        for (i = 0; i < 32; i++)
        {
            
            HAL_ADC_PollForConversion (&hadc1, 10);
            modRnd_rand32 ^= (uint32_t)((HAL_ADC_GetValue (&hadc1) << i) & (1<<i)); //@todo!!!
        }
        if (modRnd_rand32 != 0) // проверка на 0
        {
#if MODRANDOM_TRUE_4096
            uint32_t j = 0;
            _rand[j] = modRnd_rand32;
            j++;
            if (j>= SIZE_)
                break;
#endif
            break;
        }
         
    }
#endif
#if (BOARD_OSCILLOSCOPE_MKII || BOARD_EVAL_V1 || BOARD_STM32EV103 || BOARD_STM32F100_REBORN)
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;     // подаем такты на АЦП
	ADC1->CR2 |= ADC_CR2_ADON;              // подаем питание на ФЦП
	ADC1->CR2 |= ADC_CR2_TSVREFE;   // подаем питание на темп. сенсор и датчик напр.
	ADC1->CR2 |= ADC_CR2_EXTSEL;    // запуск преобразования по установки бита swstart
	ADC1->CR2 |= ADC_CR2_EXTTRIG;   // включаем запуск от внешнего события (у нас это свтарт)
	ADC1->SMPR1 |= ADC_SMPR1_SMP16; // ставлю макс. кол-во цыклов (239.5) на преобразование для 16 канала где теп. сенсор
	ADC1->SQR3 &= ~ADC_SQR3_SQ1;
	ADC1->SQR3 |= 6; //ADC_SQR3_SQ1_4;    // выбираем 16 (0b10000) канал для 1 преобразования     (кол-во преобразования по-умолчанию 1)

    while(1)
    {
        uint32_t i;
        //
        for (i = 0; i < 32; i++)
        {
        	ADC1->CR2 |= ADC_CR2_SWSTART; // запуск преобразования
			// ждем конца преобразования
			while (!(ADC1->SR & ADC_SR_EOC)) {};
            modRnd_rand32 ^= (uint32_t)((ADC1->DR << i) & (1 << i)); //TODO!!!
        }
        if (modRnd_rand32 != 0)
        {
            break;
        }

    }
#endif
    
#if (BOARD_STM32F4DISCOVERY || BOARD_STM32F411_REBORN)
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;     // подаем такты на АЦП
	ADC1->CR2 |= ADC_CR2_ADON;              // подаем питание на ФЦП
	//ADC1->CR2 |= ADC_CR2_TSVREFE;   // подаем питание на темп. сенсор и датчик напр.
	ADC1->CR2 |= ADC_CR2_EXTSEL;    // запуск преобразования по установки бита swstart
	//ADC1->CR2 |= ADC_CR2_EXTTRIG;   // включаем запуск от внешнего события (у нас это свтарт)
	ADC1->SMPR1 |= ADC_SMPR1_SMP16; // ставлю макс. кол-во цыклов (239.5) на преобразование для 16 канала где теп. сенсор
	ADC1->SQR3 &= ~ADC_SQR3_SQ1;
	ADC1->SQR3 |= 6; //ADC_SQR3_SQ1_4;    // выбираем 16 (0b10000) канал для 1 преобразования     (кол-во преобразования по-умолчанию 1)

    while(1)
    {
        uint32_t i;
        //
        for (i = 0; i < 32; i++)
        {
        	ADC1->CR2 |= ADC_CR2_SWSTART; // запуск преобразования
			// ждем конца преобразования
			while (!(ADC1->SR & ADC_SR_EOC)) {};
            modRnd_rand32 ^= (uint32_t)((ADC1->DR << i) & (1 << i)); //TODO!!!
        }
        if (modRnd_rand32 != 0)
        {
            break;
        }

    }
#endif
    
#if (BOARD_STM8MICRO_V1)
    //modRnd_rand32 = 0x5648A654; return;
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
        ADC1_CHANNEL_12,
        ADC1_PRESSEL_FCPU_D2,
        ADC1_EXTTRIG_TIM, DISABLE,
        ADC1_ALIGN_RIGHT,
        ADC1_SCHMITTTRIG_CHANNEL12, DISABLE); // ENABLE
    ADC1_Cmd(ENABLE);
    while (1) 
    {
        uint32_t tmp32;
        // читаем ацп, самый младший бит самый мусорный, им и инициализируем
        for (uint32_t i = 0; i < 32; i++)
        {
            ADC1_StartConversion(); //Для запуска АЦП
            while (ADC1_GetFlagStatus(ADC1_FLAG_EOC) == RESET); //ждем
            tmp32 = (ADC1_GetConversionValue() & 0x00000001);
            modRnd_rand32 ^= tmp32;
            modRnd_rand32 = modRnd_rand32 << 1;
        }
        if (modRnd_rand32 != 0) // проверка на 0
        {
#if MODRANDOM_TRUE_4096
            uint32_t j = 0;
            _rand[j] = modRnd_rand32;
            j++;
            if (j>= SIZE_)
                break;
#endif
            break;
        }
    }
#endif
    return modRnd_rand32;
}


void _set_seed (uint32_t val)
{
     if (0 != val)
         modRnd_rand32 = val;
}
