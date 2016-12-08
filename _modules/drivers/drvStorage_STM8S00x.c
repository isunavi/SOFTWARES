#include "halEEPROM.h"
#include "board.h"

/**
    https://github.com/yuxiao/stm8s_lib/blob/master/eeprom.c


 */

struct SavedDomain_t{
    uint8_t hh;
    uint8_t hh1;
    uint8_t hh2;
    uint8_t hh3;
    uint8_t hh4;
    uint8_t hh5;
};

//const SavedDomain_t* SavedDomain = (SavedDomain_t *)(HAL_FLASH_BASE + 1024 * 4);

// #define SavedDomain (*(SavedDomain_t*)(FLASH_BASE+1024*14));
//const SavedDomain_t* SavedDomain = (SavedDomain_t*)(FLASH_BASE+1024*14);



// //������� ��������� true ����� ����� ������� ��� ������ ������.
// uint8_t halFLASH_ready(void)
// {
//     return !(FLASH->SR & FLASH_SR_BSY);
// }



void halFLASH_eraseAllPages(void)
{
    /*
    FLASH->CR |= FLASH_CR_MER; //������������� ��� �������� ���� �������
    FLASH->CR |= FLASH_CR_STRT; //������ ��������
//     while(!halFLASH_ready()){}; // �������� ����������.. ���� ��� ��� �������� �� � ���� �����...

//     FLASH->CR &= FLASH_CR_MER;
    */
}


void halFLASH_erasePage(uint32_t address)
{
    /*
    while (FLASH->SR & FLASH_SR_BSY){};
    if (FLASH->SR & FLASH_SR_EOP)
    {
        FLASH->SR = FLASH_SR_EOP;
    }
    
    FLASH->CR |= FLASH_CR_PER; //������������� ��� �������� ����� ��������
    FLASH->AR = address; // ������ � �����
    FLASH->CR |= FLASH_CR_STRT; // ��������� �������� 
    while (!(FLASH->SR & FLASH_SR_EOP)){}; //���� ���� �������� ��������. 
    FLASH->SR = FLASH_SR_EOP;
    FLASH->CR &= ~FLASH_CR_PER; //���������� ��� �������
    */
}





void halFLASH_flashWrite (uint32_t address, uint8_t *data, uint32_t cnt)
{
/*    uint32_t i;

    while (FLASH->SR & FLASH_SR_BSY){}; //������� ���������� ����� � ������
    if (FLASH->SR & FLASH_SR_EOP)
    {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PG; //��������� ���������������� �����

    for (i = 0; i < cnt; i += 2)
    {   //����� ������� 2 �����
        *(volatile uint16_t *)(address + i) = (((uint16_t)data[i + 1]) << 8) + data[i];
        while (!(FLASH->SR & FLASH_SR_EOP)){};
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR &= ~(FLASH_CR_PG); //��������� ���������������� �����
    */
}


uint16_t halFLASH_read (uint32_t address)
{
    return (*(volatile uint16_t *) address);
}


void halFLASH_flashRead(uint32_t address, uint8_t *data, uint32_t cnt)
{
    uint32_t i;

    for (i = 0; i < cnt; i += 2)
    {   //����� ������� 2 �����
        data[i]     = halFLASH_read(address + i) & 0x00FF;
        data[i + 1] = halFLASH_read(address + i) >> 8;
    }
}





msg_t      halEEPROM_write (uint16_t adress, uint8_t *data, uint16_t cnt) {
    msg_t resp = FUNCTION_RETURN_OK;
    uint8_t tmp8, _tmp8;
    
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    /* Define FLASH programming time */
    FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
    while (cnt--) {
        tmp8 = FLASH_ReadByte(FLASH_DATA_START_PHYSICAL_ADDRESS + adress);
        _tmp8 = *data;
        if (tmp8 != _tmp8) { // protect EEPROM for often writing
            FLASH_EraseByte (FLASH_DATA_START_PHYSICAL_ADDRESS + adress);
            FLASH_ProgramByte (FLASH_DATA_START_PHYSICAL_ADDRESS + adress, _tmp8);
            tmp8 = FLASH_ReadByte (FLASH_DATA_START_PHYSICAL_ADDRESS + adress);
            if (tmp8 != _tmp8) { // ���� ��������� ������, ������
                resp = FUNCTION_RETURN_ERROR;
                break;
            }
        }
        data++;
        adress++;
    }
    FLASH_Lock(FLASH_MEMTYPE_DATA);
    return resp;
}


msg_t      halEEPROM_read (uint16_t adress, uint8_t *data, uint16_t cnt) {
    msg_t resp = FUNCTION_RETURN_OK;
    
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    while (cnt--) {
        *data =  FLASH_ReadByte(FLASH_DATA_START_PHYSICAL_ADDRESS + adress);
        data++;
        adress++;
    }
    FLASH_Lock(FLASH_MEMTYPE_DATA);
    
    return resp;
}