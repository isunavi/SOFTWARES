/*
    MMC/CD+ FAT16
*/

#include "modCD.h"
#include "modSysClock.h"

#include "defines.h"
#include "board.h"

#define SPI_PROG 1 // пока программный

#ifdef BOARD_STM32F4DISCOVERY
#define GPIO_CS                  GPIOA
#define GPIO_Pin_CS              GPIO_Pin_4
#define GPIO_SCK                 GPIOA
#define GPIO_Pin_SCK             GPIO_Pin_5
#define GPIO_MISO                GPIOA
#define GPIO_Pin_MISO            GPIO_Pin_6
#define GPIO_MOSI                GPIOA
#define GPIO_Pin_MOSI            GPIO_Pin_7
#define MMC_CS_L        { GPIO_CS->BSRRH = GPIO_Pin_CS; } /* MMC CS = 0 */
#define MMC_CS_H        { GPIO_CS->BSRRL = GPIO_Pin_CS; } /* MMC CS = 1 */
#define MMC_SCK_L       { GPIO_SCK->BSRRH = GPIO_Pin_SCK; }
#define MMC_SCL_H       { GPIO_SCK->BSRRL = GPIO_Pin_SCK; }
#define MMC_MISO        ( GPIO_MISO->IDR & GPIO_Pin_MISO )  // IN, master - MISO
#define MMC_MOSI_L      { GPIO_MOSI->BSRRH = GPIO_Pin_MOSI; } // OUT, master MOSI
#define MMC_MOSI_H      { GPIO_MOSI->BSRRL = GPIO_Pin_MOSI; }
#endif

#ifdef STM32F100
#define MMC_MISO        ( GPIO_MISO->IDR & GPIO_Pin_MISO ) 
#define MMC_MOSI_H 		{ GPIOB->BSRR = GPIO_BSRR_BS12; }
#define MMC_MOSI_L 		{ GPIOB->BSRR = GPIO_BSRR_BR12; } 
#define MMC_CS_H 		TRUE //GPIOB->BSRR = GPIO_BSRR_BS13
#define MMC_CS_L 		GPIOB->BSRR = GPIO_BSRR_BR13
#define MMC_SCK_L       { GPIO_SCK->BSRRH = GPIO_Pin_SCK; }
#define MMC_SCL_H       { GPIO_SCK->BSRRL = GPIO_Pin_SCK; }
#endif


void MMC_INIT_SYS(void)
{
#ifdef BOARD_STM32F4DISCOVERY
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_CS | GPIO_Pin_SCK | GPIO_Pin_MOSI;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_MISO;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
  
#ifdef STM32F103
    //------------ отключаем JTAG -------------
    // alternative func enable
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE; // отключаем JTAG
    AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP; // remap SPI1 to port B
    
	GPIOB->CRL |= (GPIO_CRL_MODE5 | GPIO_CRL_MODE3);
	GPIOB->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_CNF3);
	GPIOB->BSRR = (GPIO_BSRR_BR5 | GPIO_BSRR_BR3);

    GPIOA->CRH |= (GPIO_CRH_MODE15); // CS
	GPIOA->CRH &= ~(GPIO_CRH_CNF15);
	GPIOA->BSRR = (GPIO_BSRR_BR15);

	GPIOB->CRL &= ~GPIO_CRL_MODE4;
	GPIOB->CRL &= ~GPIO_CRL_CNF4_1;
	GPIOB->CRL |= GPIO_CRL_CNF4_0;
	GPIOB->ODR |= GPIO_ODR_ODR4;   //резистором к VDD
#endif
    
#ifdef STM32F100
	GPIOB->CRH |= (GPIO_CRH_MODE10 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13);
	GPIOB->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_CNF12 | GPIO_CRH_CNF13);
	GPIOB->BSRR = (GPIO_BSRR_BR10 | GPIO_BSRR_BR12 | GPIO_BSRR_BR13);

	GPIOB->CRH &= ~GPIO_CRH_MODE11;
	GPIOB->CRH &= ~GPIO_CRH_CNF11_1;
	GPIOB->CRH |= GPIO_CRH_CNF11_0;
	GPIOB->ODR |= GPIO_ODR_ODR11;   //резистором к VDD
#endif

#ifdef STM8L162
    GPIOE->DDR |=  (1<<3); //_bit.DDR3 = 1; //Настраиваем 3й пин порта E на выход
    GPIOE->CR1 |=  (1<<3);//_bit.C13 = 1; //Переключаем его в режим push-pull
    GPIOE->DDR |=  (1<<1);//_bit.DDR1 = 1; //Настраиваем 1й пин порта E на выход
    GPIOE->CR1 |=  (1<<1);//_bit.C11 = 1; //Переключаем его в режим push-pull
    GPIOE->DDR |=  (1<<0);//_bit.DDR0 = 1; //Настраиваем 0й пин порта E на выход
    GPIOE->CR1 |=  (1<<0);//_bit.C10 = 1; //Переключаем его в режим push-pull
	
    GPIOE->DDR &= ~(1<<2); //  вход
    GPIOE->CR1 |=  (1<<2);  // "подтяжка" 
    GPIOE->CR2 &= ~(1<<2); //  прерывания
#endif
}

// #ifdef STM8L162
// #include "../stm8l15x.h" 
// #include "../modUSART/modUSART.h" 

// /*
// #define MMC_SCL GPIO_CRL_MODE10
// #define MMC_DO  GPIO_CRL_MODE6 //IN master  MISO
// #define MMC_DI  3 //OUT master MOSI
// #define MMC_CS  2
// //#define PORT GPIOB
//  */
// //#define MMC_SCLr 		PE_ODR_bit.ODR3 = 0 //PORT->BSRR = GPIO_BSRR_BR10
// //#define MMC_SCLs 		PE_ODR_bit.ODR3 = 1 //PORT->BSRR = GPIO_BSRR_BS10
// #define MMC_SCLs  GPIOE->ODR |=  (1<<3); 
// #define MMC_SCLr  GPIOE->ODR &= ~(1<<3); 

// //#define MMC_DO 			(PE_IDR & (1<<2)) //PORT->IDR & GPIO_IDR_IDR11 //IN master  MISO
// #define MMC_DO 			(GPIOE->IDR & (1<<2)) //PORT->IDR & GPIO_IDR_IDR11 //IN master  MISO

// //#define MMC_DIr 		PE_ODR_bit.ODR1 = 0 //PORT->BSRR = GPIO_BSRR_BR12 //OUT master MOSI
// //#define MMC_DIs 		PE_ODR_bit.ODR1 = 1 //PORT->BSRR = GPIO_BSRR_BS12
// #define MMC_DIs  GPIOE->ODR |=  (1<<1); 
// #define MMC_DIr  GPIOE->ODR &= ~(1<<1); 

// //#define MMC_CSr 		PE_ODR_bit.ODR0 = 0 //PORT->BSRR = GPIO_BSRR_BR13
// //#define MMC_CSs 		PE_ODR_bit.ODR0 = 1 //PORT->BSRR = GPIO_BSRR_BS13
// #define MMC_CSs  GPIOE->ODR |=  (1<<0); 
// #define MMC_CSr  GPIOE->ODR &= ~(1<<0); 
// #endif

#if SPI_PROG
uint8_t modMMC_spi(uint8_t data) // Передать байт карте, программный спи
{ 
    uint8_t i, res = 0;
    for (i = 0; i < 8; i++)
    {
        if (data & 0x80)//Выставить бит данных
        {
            MMC_MOSI_H;
        } else {
            MMC_MOSI_L;
        }
        data = data << 1;
        MMC_SCL_H;//Импульс
	    res = res << 1;
        __NOP(); //
        //wait_ms(1);
	    if(0 != MMC_MISO) { res = res | 0x01; }  //Считать бит данных
        MMC_SCK_L;
	    __NOP(); //
        //wait_ms(1);
    }
	return res;
}
#else
uint8_t modMMC_spi(uint8_t data) // Передать байт карте, используем аппаратный спи
{ 
    uint8_t i, res = 0;

    return res;
}
#endif


//----------------------------- Команды MMC ----------------------------------------
#define MMC_GO_IDLE_STATE            (0+64)        ///< init SPI
#define MMC_SEND_OP_COND             (1+64)        ///< set card operational mode
#define MMC_SEND_CSD                 (9+64)        ///< get card's CSD
#define MMC_SEND_IF_COND             (8+64)
#define MMC_SEND_CID                 (10+64)        ///< get card's CID
#define MMC_SEND_STATUS              (13+64)
#define MMC_SET_BLOCKLEN             (16+64)        ///< Set number of bytes to transfer per block
#define MMC_READ_SINGLE_BLOCK        (17+64)        ///< read a block
#define MMC_WRITE_BLOCK              (24+64)        ///< write a block
#define MMC_PROGRAM_CSD              27+64
#define MMC_SET_WRITE_PROT           28+64
#define MMC_CLR_WRITE_PROT           29+64
#define MMC_SEND_WRITE_PROT          30+64
#define MMC_TAG_SECTOR_START         32+64
#define MMC_TAG_SECTOR_END           33+64
#define MMC_UNTAG_SECTOR             34+64
#define MMC_TAG_ERASE_GROUP_START    35+64        ///< Sets beginning of erase group (mass erase)
#define MMC_TAG_ERARE_GROUP_END      36+64        ///< Sets end of erase group (mass erase)
#define MMC_UNTAG_ERASE_GROUP        37+64        ///< Untag (unset) erase group (mass erase)
#define MMC_ERASE                    38+64        ///< Perform block/mass erase
#define MMC_CMD_SD_SEND_OP_COND      41+64
#define MMC_APP_CMD                  (55+64)
#define MMC_CRC_ON_OFF               (59+64)        ///< Turns CRC check on/off
//Ответы
#define R1 1
#define R2 2
#define R3 5
#define R7 5

#define MMC_BLOK_SIZE                512  //размер кластера на FAT16

uint8_t type_fname[][4] = { // список поддерживаемых расширений
    "RBF",
    "WAV",
    "BMP",
    "TXT",
};

static uint32_t modCD_timerA; // таймеры
void wait_ms(uint32_t time)
{
    modCD_timerA = modSysClock_getTime();
    while(1)
    {
        if (modSysClock_getPastTime(modCD_timerA, SYSCLOCK_GET_TIME_MKS_10) > time)
        {
            break;
        }
    }     
}

//uint32_t Temp;
uint32_t fat_base ; // Fat_base -Адрес начала FAT
uint32_t root_base ; // Адрес начала корневого каталога
uint32_t cluster_base ; // Cluster_base -(+)2 ластера и начинаются наши данные
uint32_t pbr_adr ; // Начальный адрес раздела 0 для PBR
uint16_t rezerv ; // Число секторов от PBR до FAT
uint16_t s_p_fat; //Число секторов отведенных FAT
uint8_t  s_p_cluster; //Число секторов в кластере 1-64
uint8_t   Resp[5]; // буфер-приемник (анализатор) ответов от карточки
//---------------------------------------------


// для FAT-16
uint32_t num, fatadr;

//Отправка команды карте SD|MMC
void modMMC_mmc_cmd(uint8_t com, uint32_t adr, uint8_t R)
{
    uint8_t res;
    uint16_t cnt;
    
    modMMC_spi(0xFF); // просто так, для очистки
    modMMC_spi(com); // код комманды
    //аргумент комманды
    modMMC_spi(adr >> 24);  // printf("%x,",b1);
    modMMC_spi(adr >> 16);  // printf("%x,",b2);
    modMMC_spi(adr >> 8);  // printf("%x,",b3);
    modMMC_spi(adr);  // printf("%x,",b4);
    // CRC
  	if (com == 64)
        com = 0x95; /* Valid CRC for CMD0(0) */
	if (com == (64 + 8))
        com = 0x87; /* Valid CRC for CMD8(0x1AA) */
    modMMC_spi(com);
    cnt = 0;
    do //Подождать R1 ответа
    {
        res = modMMC_spi(0xFF); cnt++;
    } while ((res & 0x80) && ( cnt < 0x0FFF));
    Resp[0] = res;
    switch(R)
    {
        case R2:
            Resp[1] = modMMC_spi(0xFF); //printf("{%x}",Resp[1]); break;
            break;
        //case R3: // @todo
        case R7: 
            Resp[4] = modMMC_spi(0xFF); //printf("{%x,",Resp[4]);
            Resp[3] = modMMC_spi(0xFF); //printf("%x,",Resp[3]);
            Resp[2] = modMMC_spi(0xFF); //printf("%x,",Resp[2]);
            Resp[1] = modMMC_spi(0xFF); //printf("%x}",Resp[1]);
            break;
        
        default: break;
	}
    
    //return; // TODO
}


msg_t modMMC_init(void)//Инициализация карты MMC|SD
{
    uint16_t ct;
    uint8_t dummy;

    MMC_INIT_SYS(); //Настройка портов и SPI
 
    MMC_CS_H;
    MMC_MOSI_H;
    MMC_SCK_L;
    wait_ms(100); // @todo!
    for (ct = 0;ct < (80); ct++) //74*8 импульсов ?
    {
        //spi(0xFF); // @todo!
	    MMC_SCL_H;//Фронт импульса
	    wait_ms(10);
	    MMC_SCK_L;//Спад испульса
	    wait_ms(10);
    }
    wait_ms(10); // @todo!
    
    MMC_CS_L; //CS опустить?
    modMMC_mmc_cmd(MMC_GO_IDLE_STATE, 0, R1);
    if (Resp[0] != 0x01) return FUNCTION_RETURN_ERROR;
    modMMC_mmc_cmd(MMC_SEND_IF_COND, 0x000001AA, R7); //CMD8    0x1AA
    if (Resp[0] & 0x04) //illegal сommand?
    {
        ct = 0;
        do
        {
            modMMC_mmc_cmd(MMC_APP_CMD, 0, R1); //CMD55
            modMMC_mmc_cmd(MMC_CMD_SD_SEND_OP_COND, 0, R3); //ACMD41
            ct = ct + 1;
        }
        while ((Resp[0] == 0x01)  && (ct < 0xFFF7)); //Ждем 0x01 ответа R1
        if(Resp[0] & 0x04)
        {
            ct = 0;
            do
            {
                modMMC_mmc_cmd(MMC_SEND_OP_COND, 0, R1); //CMD1
                modMMC_spi(0xFF);
                ct = ct + 1;
            }
            while ((Resp[0] == 0x01) && (ct < 0xFFF7)); //Ждем 0x01 ответа R1

            if (Resp[0] == 0)
            { //printf("MMC");
                return FUNCTION_RETURN_OK;
            }
        }
        if (Resp[0] == 0)
        { //printf("CD 1.x");
            return FUNCTION_RETURN_OK;
        }
    }
    if (Resp[0] == 1)
    {
        ct = 0;
        do
        {
            modMMC_mmc_cmd(MMC_APP_CMD, 0, R1); //CMD55
            modMMC_mmc_cmd(MMC_CMD_SD_SEND_OP_COND, 0, R3); //ACMD41
            ct = ct + 1;
        }
        while ((Resp[0] == 0x01)  && (ct < 0xFFF7)); //Ждем 0x01 ответа R1
        if (Resp[0] == 0)
        { //printf("CD 2.x");
            return FUNCTION_RETURN_OK;
        }
    }
    //printf("XZ, not respond!");
    return FUNCTION_RETURN_ERROR;
}


msg_t modMMC_read_block(uint32_t adr, uint8_t *buf)
{
    uint8_t emp;
    uint16_t c;
    
    modMMC_mmc_cmd(MMC_READ_SINGLE_BLOCK, adr << 8, R1);
    if (0 != Resp[0]) return FUNCTION_RETURN_ERROR; //21;//Выйти, если ответ не 0x00
    modMMC_spi (0xFF);
    c = 0;
    do
    { //Ждем начала пакета данных
        emp = modMMC_spi(0xFF); c++;
    } while ((emp != 0xFE) && (c < 0xFFF7));
    if (c >= 0xFFF7) return FUNCTION_RETURN_ERROR; //22;
    for (c = 0;c < MMC_BLOK_SIZE; c++) //cохраняем данные
    {
        buf[c] = modMMC_spi(0xFF); //tr(RAM[c]);//printf("%c",RAM[c]);
    }
    modMMC_spi(0xFF); //CRC
    modMMC_spi(0xFF);
    
    return FUNCTION_RETURN_OK;
}

msg_t  modMMC_write_block (uint32_t adr, uint8_t *buf)
{
    uint8_t emp;
    uint16_t c;
    
    modMMC_mmc_cmd(MMC_WRITE_BLOCK, adr << 8, R1); //CMD24
    if (Resp[0] != 0) return FUNCTION_RETURN_ERROR; //31;//Выйти, если ответ не 0x00
    modMMC_spi(0xFF);
    modMMC_spi(0xFE); //Начало пакета данных
    for (c = 0; c < MMC_BLOK_SIZE; c++) //Отослать данные
    {
        modMMC_spi(buf[c]);
    }
    modMMC_spi(0xFF); //CRC
    modMMC_spi(0xFF);
    emp = modMMC_spi(0xFF);
    if ((emp & 0x05) != 0x05) return FUNCTION_RETURN_ERROR; //32;//Выйти, если данные не приняты

    c = 0;
    do
    { //Ждем окончания busy состояния
        emp = modMMC_spi(0xFF); c++;
    } while ((emp != 0xFF) && (c < 0xFFF7));
    if (c >= 0xFFF7) 
    {
        return FUNCTION_RETURN_ERROR;
    } else {
        return FUNCTION_RETURN_OK;
    }
}


////////////////////////////FAT 16//////////////////////////////
msg_t  modMMC_fat16_init(uint8_t *buf) // Внимание! Слишком много магических чисел!
{
    msg_t respond = FUNCTION_RETURN_OK;
    uint8_t  smesh = 0;
    s_p_cluster = 0;
    rezerv = 0;
    s_p_fat = 0;
    fat_base = 0;
    root_base = 0;
    cluster_base = 0;
    uint32_t tmp32;

    // первое чтение
    if (FUNCTION_RETURN_ERROR == modMMC_read_block(0, buf)) //BOOT MBR
    {
        return FUNCTION_RETURN_ERROR;
    }
    pbr_adr = 0;
    //занесем значение первых 4х чеек во временный регистр и проверим
    tmp32 = (uint32_t)(buf[3] + buf[2] + buf[1] + buf[0]);
    if (tmp32 == 0) //если это нули то это MBR иначе это PBR
    {
        tmp32 = buf[448 + 9] << 8;
        tmp32 = (tmp32 + (buf[448 + 8] << 8));
        tmp32 = (tmp32 + (buf[448 + 7] << 8)) + buf[448 + 6];

        pbr_adr = (uint32_t)tmp32 * 512;
        modMMC_read_block( pbr_adr , buf); // @todo по идее, если мы смогли прочитать раньше, то все оки
    }
    s_p_cluster = buf[13]; // Общее число секторов в кластере 1-64
    //printf("s_p_cluster:%u\r\n",s_p_cluster); //1 2 8
    
    rezerv = ((uint32_t)buf[15] << 8) + buf[14];  // Число секторов от PBR до FAT // Число резервных секторов в резервной области раздела, начиная с первого сектора раздела
    //printf("rezerv:%u\r\n",rezerv);   //2 4
    
    smesh = (uint32_t)buf[12];  // Число описателей файлов    *256 + RAM[11]
    //printf("smesh:%u\r\n", smesh);   //512
    
    s_p_fat = ((uint32_t)buf[23] << 8) + buf[22];  //Число секторов, занимаемых одной копией FAT
    //printf("s_p_fat:%u\r\n",s_p_fat); //F3 EE
    
    // Fat_base - Адрес начала FAT
    fat_base = pbr_adr + (uint32_t)rezerv * 2;
    //printf("fat_base:%X\r\n",fat_base); //0x0400  0x0800
    
    // Root_base - Адрес начала корневого каталога  (Число секторов, занимаемых одной копией FAT*512*2(т.к. копии 2)
    root_base = fat_base + (uint32_t)2 * s_p_fat * 2;
    //printf("root_base:%X\r\n",root_base); //03D0  3c0
    
    // Cluster_base - (+)2 кластера и начинаются наши данные  512/16=32 секторов, в данном случае умножаем на 16, что не суть
    cluster_base = root_base + (uint32_t)smesh * 32;
    //printf("cluster_base:%X\r\n",cluster_base); //0410 400
    
    return FUNCTION_RETURN_OK;
}

//поиск файла
msg_t modMMC_scan_root_base(file_type type, uint32_t *fatadr, uint8_t *buf)
{
    uint8_t    n, m;
    uint32_t   i, j;
    
    *fatadr    = 0;
    for (j = 0; j < 10; j = i + 2)
    {
        modMMC_read_block(root_base, buf); // Читаем root+ set
        for (i = 0; i < 480; i = i + 32)
        {
            if ((buf[i] != 0xE5) && (buf[i] != 0x41)) // удаленный файл
            {
                //проверяем то ли расширение
                n = 3;
                for (m = 0; m < 3; m++)
                {
                    if (type_fname[type][m] == buf[i + m + 8])
                    {
                        n--;
                    }
                }
                /**
                    if ((type_fname[type][0] == buf[i + 8 + 0]) && // типа сравнение строк
                    (type_fname[type][1] == buf[i + 8 + 1]) &&
                    (type_fname[type][2] == buf[i + 8 + 2]))
                {
                */
                if (n == 0) // если ==0, то это наш файл, мы типа нашли его
                {
                    num = (uint32_t)buf[27 + i] * 256 + buf[26 + i]; //Вычисляем адрес файла
                    *fatadr = (uint32_t)(cluster_base + (uint32_t)(num - 2) * s_p_cluster * 2);
                    return FUNCTION_RETURN_OK;
                    //printf("Adress file:%x\r\n",fatadr);
                }
            }
        }
        root_base = root_base + 2;
    }
    
    return FUNCTION_RETURN_ERROR;
}
