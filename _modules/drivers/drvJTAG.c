#include "drvJTAG.h"
#include "defines.h"
#include "board.h"

#include "debug.h" //delay


uint8_t TAPstate;

//-----------------------------------------------

void drvJTAG_init (void)
{
//     RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |
//                     RCC_APB2ENR_IOPAEN |
//                     RCC_APB2ENR_IOPBEN;

//     GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);  
//     GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // Requires AFIO clock enabled, Map JTAG Off pins PB3,4,5, SWD still viable on PA13,14
//     //AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
//     //AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG) | AFIO_MAPR_SWJ_CFG_DISABLE;
//     
//      // ����������� MODE � CNF
//     GPIOB->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_MODE3 |
//         GPIO_CRL_CNF4 | GPIO_CRL_CNF3);
//     GPIOB->CRL |=  (GPIO_CRL_MODE4 | GPIO_CRL_MODE3); // ����������� ����� 50MHz
//     
//     GPIOB->CRH &= ~(GPIO_CRH_MODE15 |
//         GPIO_CRH_CNF15);
//     GPIOB->CRH |=  (GPIO_CRH_MODE15);
//     
//     GPIO_InitTypeDef GPIO_InitStructure;
//     GPIO_StructInit(&GPIO_InitStructure);
//     GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
// 	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //GPIO_Mode_IN_FLOATING;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
//     

    JTAG_TCK_L;
    JTAG_TMS_H;
    JTAG_TDI_L;
    //GPIO initing - user problem
    TAPstate = JTAG_STATE_RESET;
}


void drvJTAG_deinit (void)
{
    JTAG_TCK_L;
    JTAG_TMS_H;
    JTAG_TDI_H;
}


void drvJTAG_tck(void)
{
   JTAG_DELAY;
   JTAG_TCK_H;
   JTAG_DELAY;
   JTAG_TCK_L;
   JTAG_DELAY;
}


void drvJTAG_res(void)
{
    JTAG_TMS_H;
    drvJTAG_tck ();
    drvJTAG_tck ();
    drvJTAG_tck ();
    drvJTAG_tck ();
    drvJTAG_tck ();
    TAPstate = JTAG_STATE_RESET;
}


void drvJTAG_setState (uint8_t state)
{
//     while (TAPstate != state)
//     {
// //         if (next[TAPstate]) {
// //             
// //         } else {
// //             
// //         }
//     }
    JTAG_TMS_L;
    drvJTAG_tck ();
    TAPstate = JTAG_STATE_IDLE;
}


void drvJTAG_exitIDLE (void) // ����������!
{
    if ((JTAG_STATE_DR_EXIT1 == TAPstate) || (JTAG_STATE_IR_EXIT1 == TAPstate))
    {
        JTAG_TMS_H;
        drvJTAG_tck (); //9->16 or 
    }
    JTAG_TMS_L;
    drvJTAG_tck ();
    TAPstate = JTAG_STATE_IDLE;
}


void drvJTAG_loadComm (uint16_t com)
{
    uint16_t i;
    
    drvJTAG_exitIDLE ();
    if (JTAG_STATE_IDLE == TAPstate) // SIR  select inst. reg
    {
        JTAG_TMS_H; //IDLE
        drvJTAG_tck (); // 3
        drvJTAG_tck (); // 10
        JTAG_TMS_L;
        drvJTAG_tck (); // 11
        drvJTAG_tck (); // 12
        TAPstate = JTAG_STATE_IR_SHIFT;
        for (i = 0; i < 10; i++)
        {
            if ((com & (uint16_t)(1 << (i))) != 0)
            { 
                JTAG_TDI_H;
            }
            else
            { 
                JTAG_TDI_L; 
            }

            if (9 == i)
            {
                JTAG_TMS_H;
                TAPstate = JTAG_STATE_IR_EXIT1;
            }
            drvJTAG_tck ();
        }
        drvJTAG_exitIDLE();
    }
}


/** ����� ������� ����� JTAG 
 * ���� esc == 1, �� ��� ������� ����, ��� ������ ���������
 */
uint16_t drvJTAG_xputData (uint16_t data, uint8_t esc)
{
    uint16_t i, tmp;
    
    if (JTAG_STATE_IDLE == TAPstate)
    {
        JTAG_TMS_H;
        drvJTAG_tck (); // 3
        JTAG_TMS_L;
        drvJTAG_tck ();
        drvJTAG_tck (); // 4, 5
        TAPstate = JTAG_STATE_DR_SHIFT;
    }
    tmp = 0;
    for (i = 0; i < 16; i++)
    {
        if (0 != (data & (1 << (i))))
        { 
            JTAG_TDI_H;
        }
        else
        { 
            JTAG_TDI_L; 
        }
        if (0 != JTAG_TDO_IN) // reading data
        {
            tmp |= (1 << (i));
        }
        if (((16 - 1) == i) && esc)
        {
            JTAG_TMS_H;
            drvJTAG_tck ();
            TAPstate = JTAG_STATE_DR_EXIT1;
            drvJTAG_exitIDLE();
            return tmp;
        }
        drvJTAG_tck ();
    }
    
    return tmp;
}


void drvJTAG_loadAddress (uint16_t adr)
{
    uint16_t i, tmp;
    
    drvJTAG_loadComm (CJTAG_ISC_ADDRESS);
    if (JTAG_STATE_IDLE == TAPstate)
    {
        JTAG_TMS_H;
        drvJTAG_tck (); // 3
        JTAG_TMS_L;
        drvJTAG_tck ();
        drvJTAG_tck (); // 4, 5
    }
    TAPstate = JTAG_STATE_DR_SHIFT;
    tmp = adr;
    for (i = 0; i < 13; i++)
    {
        if ((tmp & (1 << (i))) != 0) //
        { 
            JTAG_TDI_H;
        }
        else
        { 
            JTAG_TDI_L;
        }
        if (12 == i)
        {
            JTAG_TMS_H;
            TAPstate = JTAG_STATE_DR_EXIT1;
        }
        drvJTAG_tck ();
    }
    drvJTAG_exitIDLE ();
}




// _res();
// _exit_IDLE();
// //_load_C(IDCODE);//006
// //for(m=0;m<2;m++)
// {
// //on=_load_D(0xff,0);
// //putchar(on);//printf("%X%X",on>>4,on & 0x0f);
// };
// //on=_load_D(0xff,1);
// //putchar(on);//printf("%X%X",on>>4,on & 0x0f);

// //printf("%X%X",str[n]>>4,str[n] & 0x0f);printf("\r\n");
// //printf("CHECKING SILICON ID :");
// _load_C(ISC_ENABLE); //2CC
// _load_ADR(0x0089); //���������� �����
// _load_C(ISC_READ); //205
// for(m=0;m<5;m++)//������ 80 ���
// {
// on=_load_D(0xff,0);
// putchar(on);//printf("%X%X",on>>4,on & 0x0f);
// on=_load_D(0xff,1);
// putchar(on);//printf("%X%X",on>>4,on & 0x0f);
// };

// putchar('}');
// _load_C(ISC_DISABLE);//201
// _res();//for EXIT
// PORTC.LED_A=0;
// } break;
// case 'r':{
// PORTC.LED_A=1;
// _res();
// _exit_IDLE();
// _load_C(ISC_ENABLE); //2CC
// _load_ADR(0x0000);
// _load_C(ISC_READ);
// cm=0;
// putchar('{');//printf("READ CHIP\r\n");
// while(cm<3328)//������
// {
// on=_load_D(0xff,0);
// putchar(on);//printf("%X%X",on>>4,on & 0x0f);
// on=_load_D(0xff,1);
// putchar(on);//printf("%X%X",on>>4,on & 0x0f);
// cm++;
// delay_ms(2);//� �� ���� �� ������ �������
// };
// _load_ADR(0x0001);
// _load_C(ISC_READ);
// cm=0;
// putchar(':');
// while(cm<512)//������  -512
// {
// on=_load_D(0xff,0);
// putchar(on);//printf("%X%X",on>>4,on & 0x0f);
// on=_load_D(0xff,1);
// putchar(on);//printf("%X%X",on>>4,on & 0x0f);
// cm++;
// delay_ms(2);
// };

// _load_C(ISC_DISABLE);//201
// _res();
// putchar('}');
// PORTC.LED_A=0;
// } break;
// case 'e':{
// PORTC.LED_A=1;
// putchar('{');//printf("Erasing.");
// _res();
// _exit_IDLE();
// _load_C(ISC_ENABLE);
// _load_ADR(0x0011);//�������� ���� ������(���� �����)
// _load_C(ISC_ERASE);
// delay_ms(500);//
// //putchar('.');
// _load_ADR(0x0001);//500 ms for each sector of the UFM block
// _load_C(ISC_ERASE);
// delay_ms(500);
// //putchar('.');
// _load_ADR(0x0000);//500 ms for the CFM block and
// _load_C(ISC_ERASE);
// delay_ms(500);
// //putchar('.');
// _load_C(ISC_DISABLE);//201
// _res();//for EXIT
// putchar('}');//printf("OK\r\n");
// PORTC.LED_A=0;
// } break;

// case '?':{putchar('!');} break;
// //====================================
// case 'w':{
// PORTC.LED_A=1;
// _res();
// _exit_IDLE();
// _load_C(ISC_ENABLE); //2CC
// _load_ADR(0x0000);
// _load_C(ISC_PROGRAM);
// putchar('{');

// cm=0;
// while(cm<3328)
// {
// a=getchar();
// _load_D(a,0);
// a=getchar();
// _load_D(a,1);
// cm++;
// _r_tck(1000);
// };

// putchar(':');
// _load_ADR(0x0001);
// _load_C(ISC_PROGRAM);
// cm=0;
// while(cm<512)
// {
// a=getchar();
// _load_D(a,0);
// a=getchar();
// _load_D(a,1);
// cm++;
// _r_tck(1000);
// };
// putchar(':');
// //=========================
// _load_ADR(0x0000);
// _load_C(ISC_PROGRAM);
// cm=0;
// while(cm<4)
// {
// a=getchar();
// _load_D(a,0);
// a=getchar();
// _load_D(a,1);
// cm++;
// _r_tck(1000);
// };


// _load_C(ISC_DISABLE);//201
// _res();//for EXIT
// putchar('}');
// PORTC.LED_A=0;
// } break;
// default:putchar('#');
// }

