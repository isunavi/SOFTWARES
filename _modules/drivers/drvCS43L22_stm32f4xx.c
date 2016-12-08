#include "drvCS43L22.h"
#include "board.h"
#include "halI2C.h"

uint8_t I2C_buf[32];

#define VOLUME_CONVERT(x)    ((Volume > 100)? 100:((uint8_t)((Volume * 255) / 100)))
#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)
#define I2C_CS43L22_ADRESS      0x94 //(0x94 | ((0x0 << 1) & 0x0E))
#define I2C_CS43L22_INCR_YES    0x80
#define I2C_CS43L22_INCR_NO     0x00
#define I2C_CS43L22_REG_ID      0x01
#define I2C_CS43L22_REG_PwCtl1      0x02
/* Codec audio Standards */
#ifdef I2S_STANDARD_PHILLIPS
 #define  CODEC_STANDARD                0x04
 #define I2S_STANDARD                   I2S_Standard_Phillips         
#elif defined(I2S_STANDARD_MSB)
 #define  CODEC_STANDARD                0x00
 #define I2S_STANDARD                   I2S_Standard_MSB    
#elif defined(I2S_STANDARD_LSB)
 #define  CODEC_STANDARD                0x08
 #define I2S_STANDARD                   I2S_Standard_LSB    
#else 
 #error "Error: No audio communication standard selected !"
#endif /* I2S_STANDARD */

/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER         1
#define OUTPUT_DEVICE_HEADPHONE       2
#define OUTPUT_DEVICE_BOTH            3
#define OUTPUT_DEVICE_AUTO            4

msg_t Codec_WriteRegister (uint8_t adr, uint8_t dat)
{
    I2C_buf[0] = adr;
    I2C_buf[1] = dat;
    return halI2C_transmit ( I2C_CS43L22_ADRESS, &I2C_buf[0], 2, 0);
}


void DAC_Config (void)
{
    /*
    DAC_InitTypeDef  DAC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // DMA1 clock and GPIOA clock enable (to be used with DAC)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);

    // DAC Periph clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // DAC channel 1 & 2 (DAC_OUT1 = PA.4) configuration 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // DAC channel1 Configuration
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(AUDIO_DAC_CHANNEL, &DAC_InitStructure);

    // Enable DAC Channel1
    DAC_Cmd(AUDIO_DAC_CHANNEL, ENABLE);
    */
}


/**
  * @brief  Sets higher or lower the codec volume level.
  * @param  Volume: a byte value from 0 to 255 (refer to codec registers 
  *         description for more details).
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_VolumeCtrl (uint8_t Volume)
{
    uint32_t counter = 0;

    if (Volume > 0xE6)
    {
        /* Set the Master volume */
        counter += Codec_WriteRegister (0x20, Volume - 0xE7); 
        counter += Codec_WriteRegister (0x21, Volume - 0xE7);     
    }
    else
    {
        /* Set the Master volume */
        counter += Codec_WriteRegister (0x20, Volume + 0x19); 
        counter += Codec_WriteRegister (0x21, Volume + 0x19); 
    }

    return counter;  
}


/*======================== CS43L22 Audio Codec Control Functions ==============================*/
/**
  * @brief  Initializes the audio codec and all related interfaces (control 
  *         interface: I2C and audio interface: I2S)
  * @param  OutputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t drvCS43L22_init (uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
    uint32_t counter = 0; 
    uint8_t OutputDev;
    
    halI2C_init ();

    /* Reset the Codec Registers */
    //Codec_Reset();

    /* Initialize the Control interface of the Audio Codec */
    //Codec_CtrlInterface_Init();     

    /* Keep Codec powered OFF */
    counter += Codec_WriteRegister (0x02, 0x01);  
      
    counter += Codec_WriteRegister (0x04, 0xAF); /* SPK always OFF & HP always ON */
    OutputDev = 0xAF;

    /* Clock configuration: Auto detection */  
    counter += Codec_WriteRegister (0x05, 0x81);

    /* Set the Slave Mode and the audio Standard */  
    counter += Codec_WriteRegister (0x06, CODEC_STANDARD);
      
    /* Set the Master volume */
    Codec_VolumeCtrl (Volume);

    //if (CurrAudioInterface == AUDIO_INTERFACE_DAC)
    {
        /* Enable the PassThrough on AIN1A and AIN1B */
        counter += Codec_WriteRegister (0x08, 0x01);
        counter += Codec_WriteRegister (0x09, 0x01);

        /* Route the analog input to the HP line */
        counter += Codec_WriteRegister (0x0E, 0xC0);

        /* Set the Passthough volume */
        counter += Codec_WriteRegister (0x14, 0x00);
        counter += Codec_WriteRegister (0x15, 0x00);
    }

    /* Power on the Codec */
    counter += Codec_WriteRegister(0x02, 0x9E);  

    /* Additional configuration for the CODEC. These configurations are done to reduce
      the time needed for the Codec to power off. If these configurations are removed, 
      then a long delay should be added between powering off the Codec and switching 
      off the I2S peripheral MCLK clock (which is the operating clock for Codec).
      If this delay is not inserted, then the codec will not shut down properly and
      it results in high noise after shut down. */

    /* Disable the analog soft ramp */
    counter += Codec_WriteRegister (0x0A, 0x00);
    //if (CurrAudioInterface != AUDIO_INTERFACE_DAC)
    {  
        /* Disable the digital soft ramp */
        //counter += Codec_WriteRegister(0x0E, 0x04);
    }
    /* Disable the limiter attack level */
    counter += Codec_WriteRegister (0x27, 0x00);
    /* Adjust Bass and Treble levels */
    counter += Codec_WriteRegister (0x1F, 0x0F);
    /* Adjust PCM volume level */
    counter += Codec_WriteRegister (0x1A, 0x0A);
    counter += Codec_WriteRegister (0x1B, 0x0A);

    /* Configure the I2S peripheral */
    //Codec_AudioInterface_Init(AudioFreq);  

    /* Return communication control value */
    return counter;  
}

