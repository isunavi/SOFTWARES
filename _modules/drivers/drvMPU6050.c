#include "drvMPU6050.h"
#include "halI2C.h"
#include "board.h"
#include "debug.h"

// https://github.com/dapetcu21/pmpoi/blob/master/lib/mpu6050.c


#if I2C_MPU6050

#define MPU6050_RA_XG_OFFS_TC       0x00 //[7] PWR_MODE, [6:1] XG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_YG_OFFS_TC       0x01 //[7] PWR_MODE, [6:1] YG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_ZG_OFFS_TC       0x02 //[7] PWR_MODE, [6:1] ZG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_X_FINE_GAIN      0x03 //[7:0] X_FINE_GAIN
#define MPU6050_RA_Y_FINE_GAIN      0x04 //[7:0] Y_FINE_GAIN
#define MPU6050_RA_Z_FINE_GAIN      0x05 //[7:0] Z_FINE_GAIN
#define MPU6050_RA_XA_OFFS_H        0x06 //[15:0] XA_OFFS
#define MPU6050_RA_XA_OFFS_L_TC     0x07
#define MPU6050_RA_YA_OFFS_H        0x08 //[15:0] YA_OFFS
#define MPU6050_RA_YA_OFFS_L_TC     0x09
#define MPU6050_RA_ZA_OFFS_H        0x0A //[15:0] ZA_OFFS
#define MPU6050_RA_ZA_OFFS_L_TC     0x0B
#define MPU6050_RA_XG_OFFS_USRH     0x13 //[15:0] XG_OFFS_USR
#define MPU6050_RA_XG_OFFS_USRL     0x14
#define MPU6050_RA_YG_OFFS_USRH     0x15 //[15:0] YG_OFFS_USR
#define MPU6050_RA_YG_OFFS_USRL     0x16
#define MPU6050_RA_ZG_OFFS_USRH     0x17 //[15:0] ZG_OFFS_USR
#define MPU6050_RA_ZG_OFFS_USRL     0x18
#define MPU6050_RA_SMPLRT_DIV       0x19
#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_GYRO_CONFIG      0x1B
#define MPU6050_RA_ACCEL_CONFIG     0x1C
#define MPU6050_RA_FF_THR           0x1D
#define MPU6050_RA_FF_DUR           0x1E
#define MPU6050_RA_MOT_THR          0x1F
#define MPU6050_RA_MOT_DUR          0x20
#define MPU6050_RA_ZRMOT_THR        0x21
#define MPU6050_RA_ZRMOT_DUR        0x22
#define MPU6050_RA_FIFO_EN          0x23
#define MPU6050_RA_I2C_MST_CTRL     0x24
#define MPU6050_RA_I2C_SLV0_ADDR    0x25
#define MPU6050_RA_I2C_SLV0_REG     0x26
#define MPU6050_RA_I2C_SLV0_CTRL    0x27
#define MPU6050_RA_I2C_SLV1_ADDR    0x28
#define MPU6050_RA_I2C_SLV1_REG     0x29
#define MPU6050_RA_I2C_SLV1_CTRL    0x2A
#define MPU6050_RA_I2C_SLV2_ADDR    0x2B
#define MPU6050_RA_I2C_SLV2_REG     0x2C
#define MPU6050_RA_I2C_SLV2_CTRL    0x2D
#define MPU6050_RA_I2C_SLV3_ADDR    0x2E
#define MPU6050_RA_I2C_SLV3_REG     0x2F
#define MPU6050_RA_I2C_SLV3_CTRL    0x30
#define MPU6050_RA_I2C_SLV4_ADDR    0x31
#define MPU6050_RA_I2C_SLV4_REG     0x32
#define MPU6050_RA_I2C_SLV4_DO      0x33
#define MPU6050_RA_I2C_SLV4_CTRL    0x34
#define MPU6050_RA_I2C_SLV4_DI      0x35
#define MPU6050_RA_I2C_MST_STATUS   0x36
#define MPU6050_RA_INT_PIN_CFG      0x37
#define MPU6050_RA_INT_ENABLE       0x38
#define MPU6050_RA_DMP_INT_STATUS   0x39
#define MPU6050_RA_INT_STATUS       0x3A
#define MPU6050_RA_ACCEL_XOUT_H     0x3B
#define MPU6050_RA_ACCEL_XOUT_L     0x3C
#define MPU6050_RA_ACCEL_YOUT_H     0x3D
#define MPU6050_RA_ACCEL_YOUT_L     0x3E
#define MPU6050_RA_ACCEL_ZOUT_H     0x3F
#define MPU6050_RA_ACCEL_ZOUT_L     0x40
#define MPU6050_RA_TEMP_OUT_H       0x41
#define MPU6050_RA_TEMP_OUT_L       0x42
#define MPU6050_RA_GYRO_XOUT_H      0x43
#define MPU6050_RA_GYRO_XOUT_L      0x44
#define MPU6050_RA_GYRO_YOUT_H      0x45
#define MPU6050_RA_GYRO_YOUT_L      0x46
#define MPU6050_RA_GYRO_ZOUT_H      0x47
#define MPU6050_RA_GYRO_ZOUT_L      0x48
#define MPU6050_RA_EXT_SENS_DATA_00 0x49
#define MPU6050_RA_EXT_SENS_DATA_01 0x4A
#define MPU6050_RA_EXT_SENS_DATA_02 0x4B
#define MPU6050_RA_EXT_SENS_DATA_03 0x4C
#define MPU6050_RA_EXT_SENS_DATA_04 0x4D
#define MPU6050_RA_EXT_SENS_DATA_05 0x4E
#define MPU6050_RA_EXT_SENS_DATA_06 0x4F
#define MPU6050_RA_EXT_SENS_DATA_07 0x50
#define MPU6050_RA_EXT_SENS_DATA_08 0x51
#define MPU6050_RA_EXT_SENS_DATA_09 0x52
#define MPU6050_RA_EXT_SENS_DATA_10 0x53
#define MPU6050_RA_EXT_SENS_DATA_11 0x54
#define MPU6050_RA_EXT_SENS_DATA_12 0x55
#define MPU6050_RA_EXT_SENS_DATA_13 0x56
#define MPU6050_RA_EXT_SENS_DATA_14 0x57
#define MPU6050_RA_EXT_SENS_DATA_15 0x58
#define MPU6050_RA_EXT_SENS_DATA_16 0x59
#define MPU6050_RA_EXT_SENS_DATA_17 0x5A
#define MPU6050_RA_EXT_SENS_DATA_18 0x5B
#define MPU6050_RA_EXT_SENS_DATA_19 0x5C
#define MPU6050_RA_EXT_SENS_DATA_20 0x5D
#define MPU6050_RA_EXT_SENS_DATA_21 0x5E
#define MPU6050_RA_EXT_SENS_DATA_22 0x5F
#define MPU6050_RA_EXT_SENS_DATA_23 0x60
#define MPU6050_RA_MOT_DETECT_STATUS    0x61
#define MPU6050_RA_I2C_SLV0_DO      0x63
#define MPU6050_RA_I2C_SLV1_DO      0x64
#define MPU6050_RA_I2C_SLV2_DO      0x65
#define MPU6050_RA_I2C_SLV3_DO      0x66
#define MPU6050_RA_I2C_MST_DELAY_CTRL   0x67
#define MPU6050_RA_SIGNAL_PATH_RESET    0x68
#define MPU6050_RA_MOT_DETECT_CTRL      0x69
#define MPU6050_RA_USER_CTRL        0x6A
#define MPU6050_RA_PWR_MGMT_1       0x6B
#define MPU6050_RA_PWR_MGMT_2       0x6C
#define MPU6050_RA_BANK_SEL         0x6D
#define MPU6050_RA_MEM_START_ADDR   0x6E
#define MPU6050_RA_MEM_R_W          0x6F
#define MPU6050_RA_DMP_CFG_1        0x70
#define MPU6050_RA_DMP_CFG_2        0x71
#define MPU6050_RA_FIFO_COUNTH      0x72
#define MPU6050_RA_FIFO_COUNTL      0x73
#define MPU6050_RA_FIFO_R_W         0x74
#define MPU6050_RA_WHO_AM_I         0x75

#define MPU6050_TC_PWR_MODE_BIT     7
#define MPU6050_TC_OFFSET_BIT       6
#define MPU6050_TC_OFFSET_LENGTH    6
#define MPU6050_TC_OTP_BNK_VLD_BIT  0

#define MPU6050_VDDIO_LEVEL_VLOGIC  0
#define MPU6050_VDDIO_LEVEL_VDD     1

#define MPU6050_CFG_EXT_SYNC_SET_BIT    5
#define MPU6050_CFG_EXT_SYNC_SET_LENGTH 3
#define MPU6050_CFG_DLPF_CFG_BIT    2
#define MPU6050_CFG_DLPF_CFG_LENGTH 3

#define MPU6050_EXT_SYNC_DISABLED       0x0
#define MPU6050_EXT_SYNC_TEMP_OUT_L     0x1
#define MPU6050_EXT_SYNC_GYRO_XOUT_L    0x2
#define MPU6050_EXT_SYNC_GYRO_YOUT_L    0x3
#define MPU6050_EXT_SYNC_GYRO_ZOUT_L    0x4
#define MPU6050_EXT_SYNC_ACCEL_XOUT_L   0x5
#define MPU6050_EXT_SYNC_ACCEL_YOUT_L   0x6
#define MPU6050_EXT_SYNC_ACCEL_ZOUT_L   0x7

#define MPU6050_DLPF_BW_256         0x00
#define MPU6050_DLPF_BW_188         0x01
#define MPU6050_DLPF_BW_98          0x02
#define MPU6050_DLPF_BW_42          0x03
#define MPU6050_DLPF_BW_20          0x04
#define MPU6050_DLPF_BW_10          0x05
#define MPU6050_DLPF_BW_5           0x06

#define MPU6050_GCONFIG_FS_SEL_BIT      4
#define MPU6050_GCONFIG_FS_SEL_LENGTH   2

#define MPU6050_GYRO_FS_250         0x00
#define MPU6050_GYRO_FS_500         0x01
#define MPU6050_GYRO_FS_1000        0x02
#define MPU6050_GYRO_FS_2000        0x03

#define MPU6050_ACONFIG_XA_ST_BIT           7
#define MPU6050_ACONFIG_YA_ST_BIT           6
#define MPU6050_ACONFIG_ZA_ST_BIT           5
#define MPU6050_ACONFIG_AFS_SEL_BIT         4
#define MPU6050_ACONFIG_AFS_SEL_LENGTH      2
#define MPU6050_ACONFIG_ACCEL_HPF_BIT       2
#define MPU6050_ACONFIG_ACCEL_HPF_LENGTH    3

#define MPU6050_ACCEL_FS_2          0x00
#define MPU6050_ACCEL_FS_4          0x01
#define MPU6050_ACCEL_FS_8          0x02
#define MPU6050_ACCEL_FS_16         0x03

#define MPU6050_DHPF_RESET          0x00
#define MPU6050_DHPF_5              0x01
#define MPU6050_DHPF_2P5            0x02
#define MPU6050_DHPF_1P25           0x03
#define MPU6050_DHPF_0P63           0x04
#define MPU6050_DHPF_HOLD           0x07

#define MPU6050_TEMP_FIFO_EN_BIT    7
#define MPU6050_XG_FIFO_EN_BIT      6
#define MPU6050_YG_FIFO_EN_BIT      5
#define MPU6050_ZG_FIFO_EN_BIT      4
#define MPU6050_ACCEL_FIFO_EN_BIT   3
#define MPU6050_SLV2_FIFO_EN_BIT    2
#define MPU6050_SLV1_FIFO_EN_BIT    1
#define MPU6050_SLV0_FIFO_EN_BIT    0

#define MPU6050_MULT_MST_EN_BIT     7
#define MPU6050_WAIT_FOR_ES_BIT     6
#define MPU6050_SLV_3_FIFO_EN_BIT   5
#define MPU6050_I2C_MST_P_NSR_BIT   4
#define MPU6050_I2C_MST_CLK_BIT     3
#define MPU6050_I2C_MST_CLK_LENGTH  4

#define MPU6050_CLOCK_DIV_348       0x0
#define MPU6050_CLOCK_DIV_333       0x1
#define MPU6050_CLOCK_DIV_320       0x2
#define MPU6050_CLOCK_DIV_308       0x3
#define MPU6050_CLOCK_DIV_296       0x4
#define MPU6050_CLOCK_DIV_286       0x5
#define MPU6050_CLOCK_DIV_276       0x6
#define MPU6050_CLOCK_DIV_267       0x7
#define MPU6050_CLOCK_DIV_258       0x8
#define MPU6050_CLOCK_DIV_500       0x9
#define MPU6050_CLOCK_DIV_471       0xA
#define MPU6050_CLOCK_DIV_444       0xB
#define MPU6050_CLOCK_DIV_421       0xC
#define MPU6050_CLOCK_DIV_400       0xD
#define MPU6050_CLOCK_DIV_381       0xE
#define MPU6050_CLOCK_DIV_364       0xF

#define MPU6050_I2C_SLV_RW_BIT      7
#define MPU6050_I2C_SLV_ADDR_BIT    6
#define MPU6050_I2C_SLV_ADDR_LENGTH 7
#define MPU6050_I2C_SLV_EN_BIT      7
#define MPU6050_I2C_SLV_BYTE_SW_BIT 6
#define MPU6050_I2C_SLV_REG_DIS_BIT 5
#define MPU6050_I2C_SLV_GRP_BIT     4
#define MPU6050_I2C_SLV_LEN_BIT     3
#define MPU6050_I2C_SLV_LEN_LENGTH  4

#define MPU6050_I2C_SLV4_RW_BIT         7
#define MPU6050_I2C_SLV4_ADDR_BIT       6
#define MPU6050_I2C_SLV4_ADDR_LENGTH    7
#define MPU6050_I2C_SLV4_EN_BIT         7
#define MPU6050_I2C_SLV4_INT_EN_BIT     6
#define MPU6050_I2C_SLV4_REG_DIS_BIT    5
#define MPU6050_I2C_SLV4_MST_DLY_BIT    4
#define MPU6050_I2C_SLV4_MST_DLY_LENGTH 5

#define MPU6050_MST_PASS_THROUGH_BIT    7
#define MPU6050_MST_I2C_SLV4_DONE_BIT   6
#define MPU6050_MST_I2C_LOST_ARB_BIT    5
#define MPU6050_MST_I2C_SLV4_NACK_BIT   4
#define MPU6050_MST_I2C_SLV3_NACK_BIT   3
#define MPU6050_MST_I2C_SLV2_NACK_BIT   2
#define MPU6050_MST_I2C_SLV1_NACK_BIT   1
#define MPU6050_MST_I2C_SLV0_NACK_BIT   0

#define MPU6050_INTCFG_INT_LEVEL_BIT        7
#define MPU6050_INTCFG_INT_OPEN_BIT         6
#define MPU6050_INTCFG_LATCH_INT_EN_BIT     5
#define MPU6050_INTCFG_INT_RD_CLEAR_BIT     4
#define MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT  3
#define MPU6050_INTCFG_FSYNC_INT_EN_BIT     2
#define MPU6050_INTCFG_I2C_BYPASS_EN_BIT    1
#define MPU6050_INTCFG_CLKOUT_EN_BIT        0

#define MPU6050_INTMODE_ACTIVEHIGH  0x00
#define MPU6050_INTMODE_ACTIVELOW   0x01

#define MPU6050_INTDRV_PUSHPULL     0x00
#define MPU6050_INTDRV_OPENDRAIN    0x01

#define MPU6050_INTLATCH_50USPULSE  0x00
#define MPU6050_INTLATCH_WAITCLEAR  0x01

#define MPU6050_INTCLEAR_STATUSREAD 0x00
#define MPU6050_INTCLEAR_ANYREAD    0x01

#define MPU6050_INTERRUPT_FF_BIT            7
#define MPU6050_INTERRUPT_MOT_BIT           6
#define MPU6050_INTERRUPT_ZMOT_BIT          5
#define MPU6050_INTERRUPT_FIFO_OFLOW_BIT    4
#define MPU6050_INTERRUPT_I2C_MST_INT_BIT   3
#define MPU6050_INTERRUPT_PLL_RDY_INT_BIT   2
#define MPU6050_INTERRUPT_DMP_INT_BIT       1
#define MPU6050_INTERRUPT_DATA_RDY_BIT      0

// TODO: figure out what these actually do
// UMPL source code is not very obivous
#define MPU6050_DMPINT_5_BIT            5
#define MPU6050_DMPINT_4_BIT            4
#define MPU6050_DMPINT_3_BIT            3
#define MPU6050_DMPINT_2_BIT            2
#define MPU6050_DMPINT_1_BIT            1
#define MPU6050_DMPINT_0_BIT            0

#define MPU6050_MOTION_MOT_XNEG_BIT     7
#define MPU6050_MOTION_MOT_XPOS_BIT     6
#define MPU6050_MOTION_MOT_YNEG_BIT     5
#define MPU6050_MOTION_MOT_YPOS_BIT     4
#define MPU6050_MOTION_MOT_ZNEG_BIT     3
#define MPU6050_MOTION_MOT_ZPOS_BIT     2
#define MPU6050_MOTION_MOT_ZRMOT_BIT    0

#define MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT   7
#define MPU6050_DELAYCTRL_I2C_SLV4_DLY_EN_BIT   4
#define MPU6050_DELAYCTRL_I2C_SLV3_DLY_EN_BIT   3
#define MPU6050_DELAYCTRL_I2C_SLV2_DLY_EN_BIT   2
#define MPU6050_DELAYCTRL_I2C_SLV1_DLY_EN_BIT   1
#define MPU6050_DELAYCTRL_I2C_SLV0_DLY_EN_BIT   0

#define MPU6050_PATHRESET_GYRO_RESET_BIT    2
#define MPU6050_PATHRESET_ACCEL_RESET_BIT   1
#define MPU6050_PATHRESET_TEMP_RESET_BIT    0

#define MPU6050_DETECT_ACCEL_ON_DELAY_BIT       5
#define MPU6050_DETECT_ACCEL_ON_DELAY_LENGTH    2
#define MPU6050_DETECT_FF_COUNT_BIT             3
#define MPU6050_DETECT_FF_COUNT_LENGTH          2
#define MPU6050_DETECT_MOT_COUNT_BIT            1
#define MPU6050_DETECT_MOT_COUNT_LENGTH         2

#define MPU6050_DETECT_DECREMENT_RESET  0x0
#define MPU6050_DETECT_DECREMENT_1      0x1
#define MPU6050_DETECT_DECREMENT_2      0x2
#define MPU6050_DETECT_DECREMENT_4      0x3

#define MPU6050_USERCTRL_DMP_EN_BIT             7
#define MPU6050_USERCTRL_FIFO_EN_BIT            6
#define MPU6050_USERCTRL_I2C_MST_EN_BIT         5
#define MPU6050_USERCTRL_I2C_IF_DIS_BIT         4
#define MPU6050_USERCTRL_DMP_RESET_BIT          3
#define MPU6050_USERCTRL_FIFO_RESET_BIT         2
#define MPU6050_USERCTRL_I2C_MST_RESET_BIT      1
#define MPU6050_USERCTRL_SIG_COND_RESET_BIT     0

#define MPU6050_PWR1_DEVICE_RESET_BIT   7
#define MPU6050_PWR1_SLEEP_BIT          6
#define MPU6050_PWR1_CYCLE_BIT          5
#define MPU6050_PWR1_TEMP_DIS_BIT       3
#define MPU6050_PWR1_CLKSEL_BIT         2
#define MPU6050_PWR1_CLKSEL_LENGTH      3

#define MPU6050_CLOCK_INTERNAL          0x00
#define MPU6050_CLOCK_PLL_XGYRO         0x01
#define MPU6050_CLOCK_PLL_YGYRO         0x02
#define MPU6050_CLOCK_PLL_ZGYRO         0x03
#define MPU6050_CLOCK_PLL_EXT32K        0x04
#define MPU6050_CLOCK_PLL_EXT19M        0x05
#define MPU6050_CLOCK_KEEP_RESET        0x07

#define MPU6050_PWR2_LP_WAKE_CTRL_BIT       7
#define MPU6050_PWR2_LP_WAKE_CTRL_LENGTH    2
#define MPU6050_PWR2_STBY_XA_BIT            5
#define MPU6050_PWR2_STBY_YA_BIT            4
#define MPU6050_PWR2_STBY_ZA_BIT            3
#define MPU6050_PWR2_STBY_XG_BIT            2
#define MPU6050_PWR2_STBY_YG_BIT            1
#define MPU6050_PWR2_STBY_ZG_BIT            0

#define MPU6050_WAKE_FREQ_1P25      0x0
#define MPU6050_WAKE_FREQ_2P5       0x1
#define MPU6050_WAKE_FREQ_5         0x2
#define MPU6050_WAKE_FREQ_10        0x3

#define MPU6050_BANKSEL_PRFTCH_EN_BIT       6
#define MPU6050_BANKSEL_CFG_USER_BANK_BIT   5
#define MPU6050_BANKSEL_MEM_SEL_BIT         4
#define MPU6050_BANKSEL_MEM_SEL_LENGTH      5

#define MPU6050_WHO_AM_I_BIT        6
#define MPU6050_WHO_AM_I_LENGTH     6

#define MPU6050_DMP_MEMORY_BANKS        8
#define MPU6050_DMP_MEMORY_BANK_SIZE    256
#define MPU6050_DMP_MEMORY_CHUNK_SIZE   16


void __delay_ms (volatile uint32_t delay)
{
    delay = delay * 1000;
    while (--delay > 0) {};
}


#if MPU6050_GETATTITUDE == 1 || MPU6050_GETATTITUDE == 2
#include <math.h>  //include libm
#endif

//path to i2c fleury lib
//#include MPU6050_I2CFLEURYPATH

volatile uint8_t drvMPU5060_buffer[14];

/*
 * read bytes from chip register
 */
int8_t drvMPU6050_readBytes (uint8_t regAddr, uint8_t length, uint8_t *data)
{
	int8_t count = 0;
    uint8_t I2C_buf[2];
    
	if(length > 0)
    {
        I2C_buf [0] = regAddr;
        if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_MPU6050_ADDRESS, &I2C_buf[0], 1, 100))
        {
            if (FUNCTION_RETURN_OK == halI2C_receive (I2C_MPU6050_ADDRESS, &data[0], length, 100))
            {
                count = length;
            }
        }
	}
	return count;
}

/*
 * read 1 byte from chip register
 */
int8_t drvMPU6050_readByte(uint8_t regAddr, uint8_t *data) {
    return drvMPU6050_readBytes (regAddr, 1, data);
}


/*
 * write bytes to chip register
 */
msg_t drvMPU6050_writeBytes (uint8_t regAddr, uint8_t length, uint8_t *data)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t i = 0;
    uint8_t I2C_buf[32];
    
	if(length > 0)
    {
		//write data
        I2C_buf [0] = regAddr;
        for (i = 0; i < length; i++)
        {
            I2C_buf [i +1] = data[i];
        }
        if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_MPU6050_ADDRESS, &I2C_buf[0], length + 1, 100))
        {
            respond = FUNCTION_RETURN_OK;
        }
	}
    return respond;
}


/*
 * write 1 byte to chip register
 */
void drvMPU6050_writeByte (uint8_t regAddr, uint8_t data)
{
    //return 
        drvMPU6050_writeBytes (regAddr, 1, &data);
}


/*
 * read bits from chip register
 */
int8_t drvMPU6050_readBits (uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    int8_t count = 0;
    if (length > 0) {
		uint8_t b;
		if ((count = drvMPU6050_readByte (regAddr, &b)) != 0) {
			uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
			b &= mask;
			b >>= (bitStart - length + 1);
			*data = b;
		}
    }
    return count;
}

/*
 * read 1 bit from chip register
 */
int8_t drvMPU6050_readBit (uint8_t regAddr, uint8_t bitNum, uint8_t *data)
{
    uint8_t b;
    uint8_t count = drvMPU6050_readByte (regAddr, &b);
    *data = b & (1 << bitNum);
    return count;
}

/*
 * write bit/bits to chip register
 */
void drvMPU6050_writeBits (uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) 
{
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
	if (length > 0) {
		uint8_t b = 0;
		if (drvMPU6050_readByte (regAddr, &b) != 0)  //get current data
        {
			uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
			data <<= (bitStart - length + 1); // shift data into correct position
			data &= mask; // zero all non-important bits in data
			b &= ~(mask); // zero all important bits in existing byte
			b |= data; // combine data with existing byte
			drvMPU6050_writeByte(regAddr, b);
		}
	}
}


/*
 * write one bit to chip register
 */
void drvMPU6050_writeBit (uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    drvMPU6050_readByte(regAddr, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    drvMPU6050_writeByte(regAddr, b);
}


#if MPU6050_GETATTITUDE == 2
/*
 * write word/words to chip register
 */
void drvMPU6050_writeWords (uint8_t regAddr, uint8_t length, uint16_t* data)
{
	if (length > 0)
    {
		uint8_t i = 0;
		//write data
		i2c_start(MPU6050_ADDR | I2C_WRITE);
		i2c_write(regAddr); //reg
		for (i = 0; i < length * 2; i++) {
			i2c_write((uint8_t)(data[i++] >> 8)); // send MSB
			i2c_write((uint8_t)data[i]);          // send LSB
		}
		i2c_stop();
	}
}


/*
 * set a chip memory bank
 */
void drvMPU6050_setMemoryBank (uint8_t bank, uint8_t prefetchEnabled, uint8_t userBank)
{
    bank &= 0x1F;
    if (userBank) bank |= 0x20;
    if (prefetchEnabled) bank |= 0x40;
    drvMPU6050_writeByte(MPU6050_RA_BANK_SEL, bank);
}


/*
 * set memory start address
 */
void drvMPU6050_setMemoryStartAddress(uint8_t address)
{
	drvMPU6050_writeByte(MPU6050_RA_MEM_START_ADDR, address);
}


/*
 * read a memory block
 */
void drvMPU6050_readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address)
{
	drvMPU6050_setMemoryBank(bank, 0, 0);
	drvMPU6050_setMemoryStartAddress(address);
    uint8_t chunkSize;
    for (uint16_t i = 0; i < dataSize;)
    {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;

        // read the chunk of data as specified
        drvMPU6050_readBytes(MPU6050_RA_MEM_R_W, chunkSize, data + i);

        // increase byte index by [chunkSize]
        i += chunkSize;

        // uint8_t automatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize)
        {
            if (address == 0) bank++;
            drvMPU6050_setMemoryBank(bank, 0, 0);
            drvMPU6050_setMemoryStartAddress(address);
        }
    }
}

/*
 * write a memory block
 */
uint8_t drvMPU6050_writeMemoryBlock (const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, uint8_t verify, uint8_t useProgMem)
{
	drvMPU6050_setMemoryBank(bank, 0, 0);
	drvMPU6050_setMemoryStartAddress(address);
    uint8_t chunkSize;
    uint8_t *verifyBuffer = 0;
    uint8_t *progBuffer = 0;
    uint16_t i;
    uint8_t j;
    if (verify) verifyBuffer = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
    if (useProgMem) progBuffer = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
    for (i = 0; i < dataSize;)
    {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;

        if (useProgMem)
        {
            // write the chunk of data as specified
            for (j = 0; j < chunkSize; j++) progBuffer[j] = pgm_read_byte(data + i + j);
        }
        else
        {
            // write the chunk of data as specified
            progBuffer = (uint8_t *)data + i;
        }

        drvMPU6050_writeBytes(MPU6050_RA_MEM_R_W, chunkSize, progBuffer);

        // verify data if needed
        if (verify && verifyBuffer)
        {
        	drvMPU6050_setMemoryBank(bank, 0, 0);
            drvMPU6050_setMemoryStartAddress(address);
            drvMPU6050_readBytes(MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer);
            if (memcmp(progBuffer, verifyBuffer, chunkSize) != 0)
            {
                free(verifyBuffer);
                if (useProgMem) free(progBuffer);
                return 0; // uh oh.
            }
        }

        // increase byte index by [chunkSize]
        i += chunkSize;

        // uint8_t automatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize)
        {
            if (address == 0) bank++;
            drvMPU6050_setMemoryBank(bank, 0, 0);
            drvMPU6050_setMemoryStartAddress(address);
        }
    }
    if (verify) free(verifyBuffer);
    if (useProgMem) free(progBuffer);
    return 1;
}


/*
 * write a dmp configuration set
 */
uint8_t drvMPU6050_writeDMPConfigurationSet (const uint8_t *data, uint16_t dataSize, uint8_t useProgMem) {
    uint8_t *progBuffer = 0;
    uint8_t success, special;
    uint16_t i, j;
    if (useProgMem)
    {
        progBuffer = (uint8_t *)malloc(8); // assume 8-byte blocks, realloc later if necessary
    }

    // config set data is a long string of blocks with the following structure:
    // [bank] [offset] [length] [byte[0], byte[1], ..., byte[length]]
    uint8_t bank, offset, length;
    for (i = 0; i < dataSize;) {
        if (useProgMem) {
            bank = pgm_read_byte(data + i++);
            offset = pgm_read_byte(data + i++);
            length = pgm_read_byte(data + i++);
        } else {
            bank = data[i++];
            offset = data[i++];
            length = data[i++];
        }

        // write data or perform special action
        if (length > 0) {
            // regular block of data to write
            if (useProgMem) {
                if (sizeof(progBuffer) < length) progBuffer = (uint8_t *)realloc(progBuffer, length);
                for (j = 0; j < length; j++) progBuffer[j] = pgm_read_byte(data + i + j);
            } else {
                progBuffer = (uint8_t *)data + i;
            }
            success = drvMPU6050_writeMemoryBlock(progBuffer, length, bank, offset, 1, 0);
            i += length;
        } else {
            // special instruction
            // NOTE: this kind of behavior (what and when to do certain things)
            // is totally undocumented. This code is in here based on observed
            // behavior only, and exactly why (or even whether) it has to be here
            // is anybody's guess for now.
            if (useProgMem) {
                special = pgm_read_byte(data + i++);
            } else {
                special = data[i++];
            }
            if (special == 0x01) {
                // enable DMP-related interrupts

            	//drvMPU6050_writeBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_ZMOT_BIT, 1); //setIntZeroMotionEnabled
            	//drvMPU6050_writeBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FIFO_OFLOW_BIT, 1); //setIntFIFOBufferOverflowEnabled
            	//drvMPU6050_writeBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DMP_INT_BIT, 1); //setIntDMPEnabled
            	drvMPU6050_writeByte(MPU6050_RA_INT_ENABLE, 0x32);  // single operation

                success = 1;
            } else {
                // unknown special command
                success = 0;
            }
        }

        if (!success) {
            if (useProgMem) free(progBuffer);
            return 0; // uh oh
        }
    }
    if (useProgMem) free(progBuffer);
    return 1;
}

/*
 * get the fifo count
 */
uint16_t drvMPU6050_getFIFOCount(void) {
	drvMPU6050_readBytes(MPU6050_RA_FIFO_COUNTH, 2, (uint8_t *)buffer);
    return (((uint16_t)buffer[0]) << 8) | buffer[1];
}

/*
 * read fifo bytes
 */
void drvMPU6050_getFIFOBytes(uint8_t *data, uint8_t length) {
	drvMPU6050_readBytes(MPU6050_RA_FIFO_R_W, length, data);
}

/*
 * get the interrupt status
 */
uint8_t drvMPU6050_getIntStatus(void) {
	drvMPU6050_readByte(MPU6050_RA_INT_STATUS, (uint8_t *)buffer);
    return buffer[0];
}

/*
 * reset fifo
 */
void drvMPU6050_resetFIFO(void) {
	drvMPU6050_writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, 1);
}

/*
 * get gyro offset X
 */
int8_t drvMPU6050_getXGyroOffset(void) {
	drvMPU6050_readBits(MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, (uint8_t *)buffer);
    return buffer[0];
}

/*
 * set gyro offset X
 */
void drvMPU6050_setXGyroOffset(int8_t offset) {
	drvMPU6050_writeBits(MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

/*
 * get gyro offset Y
 */
int8_t drvMPU6050_getYGyroOffset(void) {
	drvMPU6050_readBits(MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, (uint8_t *)buffer);
    return buffer[0];
}

/*
 * set gyro offset Y
 */
void drvMPU6050_setYGyroOffset(int8_t offset) {
	drvMPU6050_writeBits(MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

/*
 * get gyro offset Z
 */
int8_t drvMPU6050_getZGyroOffset(void) {
	drvMPU6050_readBits(MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, (uint8_t *)buffer);
    return buffer[0];
}

/*
 * set gyro offset Z
 */
void drvMPU6050_setZGyroOffset(int8_t offset) {
	drvMPU6050_writeBits(MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}
#endif

/*
 * set sleep disabled
 */
void drvMPU6050_setSleepDisabled() {
	drvMPU6050_writeBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, 0);
}

/*
 * set sleep enabled
 */
void drvMPU6050_setSleepEnabled(void) {
	drvMPU6050_writeBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, 1);
}


/*
 * test connectino to chip
 */
uint8_t drvMPU6050_testConnection (void)
{
	drvMPU6050_readBits (MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, (uint8_t *)drvMPU5060_buffer);
	if (drvMPU5060_buffer[0] == 0x34)
		return TRUE;
	else
		return FALSE;
}

/*
 * initialize the accel and gyro
 */
void drvMPU6050_init(void) {
	//allow drvMPU6050 chip clocks to start up
	__delay_ms(100);

	//set sleep disabled
	drvMPU6050_setSleepDisabled();
	//wake up delay needed sleep disabled
	__delay_ms(10);

	//set clock source
	//  it is highly recommended that the device be configured to use one of the gyroscopes (or an external clock source)
	//  as the clock reference for improved stability
	drvMPU6050_writeBits(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, MPU6050_CLOCK_PLL_XGYRO);
	//set DLPF bandwidth to 42Hz
	drvMPU6050_writeBits(MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, MPU6050_DLPF_BW_42);
    //set sampe rate
	drvMPU6050_writeByte(MPU6050_RA_SMPLRT_DIV, 4); //1khz / (1 + 4) = 200Hz
	//set gyro range
	drvMPU6050_writeBits(MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, MPU6050_GYRO_FS);
	//set accel range
	drvMPU6050_writeBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS);

	#if MPU6050_GETATTITUDE == 1
	#error "Do not enable timer 0 it is in use elsewhere!"
	//MPU6050_TIMER0INIT
	#endif
}

//can not accept many request if we alreay have getattitude requests
/*
 * get raw data
 */
void drvMPU6050_getRawData(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
	drvMPU6050_readBytes (MPU6050_RA_ACCEL_XOUT_H, 14, (uint8_t *)drvMPU5060_buffer);

    *ax = (((int16_t)drvMPU5060_buffer[0]) << 8) | drvMPU5060_buffer[1];
    *ay = (((int16_t)drvMPU5060_buffer[2]) << 8) | drvMPU5060_buffer[3];
    *az = (((int16_t)drvMPU5060_buffer[4]) << 8) | drvMPU5060_buffer[5];
    *gx = (((int16_t)drvMPU5060_buffer[8]) << 8) | drvMPU5060_buffer[9];
    *gy = (((int16_t)drvMPU5060_buffer[10]) << 8) | drvMPU5060_buffer[11];
    *gz = (((int16_t)drvMPU5060_buffer[12]) << 8) | drvMPU5060_buffer[13];
}

/*
 * get raw data converted to g and deg/sec values
 */
void drvMPU6050_getConvData(double* axg, double* ayg, double* azg, double* gxds, double* gyds, double* gzds) {
	int16_t ax = 0;
	int16_t ay = 0;
	int16_t az = 0;
	int16_t gx = 0;
	int16_t gy = 0;
	int16_t gz = 0;
	drvMPU6050_getRawData (&ax, &ay, &az, &gx, &gy, &gz);

	#if MPU6050_CALIBRATEDACCGYRO == 1
    *axg = (double)(ax-MPU6050_AXOFFSET)/MPU6050_AXGAIN;
    *ayg = (double)(ay-MPU6050_AYOFFSET)/MPU6050_AYGAIN;
    *azg = (double)(az-MPU6050_AZOFFSET)/MPU6050_AZGAIN;
    *gxds = (double)(gx-MPU6050_GXOFFSET)/MPU6050_GXGAIN;
	*gyds = (double)(gy-MPU6050_GYOFFSET)/MPU6050_GYGAIN;
	*gzds = (double)(gz-MPU6050_GZOFFSET)/MPU6050_GZGAIN;
	#else
    *axg = (double)(ax)/MPU6050_AGAIN;
    *ayg = (double)(ay)/MPU6050_AGAIN;
    *azg = (double)(az)/MPU6050_AGAIN;
    *gxds = (double)(gx)/MPU6050_GGAIN;
	*gyds = (double)(gy)/MPU6050_GGAIN;
	*gzds = (double)(gz)/MPU6050_GGAIN;
	#endif
}

#if MPU6050_GETATTITUDE == 1

volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
volatile float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;
/*
 * Mahony update function (for 6DOF)
 */
void drvMPU6050_mahonyUpdate(float gx, float gy, float gz, float ax, float ay, float az) {
	float norm;
	float halfvx, halfvy, halfvz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		norm = sqrt(ax * ax + ay * ay + az * az);
		ax /= norm;
		ay /= norm;
		az /= norm;

		// Estimated direction of gravity and vector perpendicular to magnetic flux
		halfvx = q1 * q3 - q0 * q2;
		halfvy = q0 * q1 + q2 * q3;
		halfvz = q0 * q0 - 0.5f + q3 * q3;

		// Error is sum of cross product between estimated and measured direction of gravity
		halfex = (ay * halfvz - az * halfvy);
		halfey = (az * halfvx - ax * halfvz);
		halfez = (ax * halfvy - ay * halfvx);

		// Compute and apply integral feedback if enabled
		if(drvMPU6050_mahonytwoKiDef > 0.0f) {
			integralFBx += drvMPU6050_mahonytwoKiDef * halfex * (1.0f / drvMPU6050_mahonysampleFreq);	// integral error scaled by Ki
			integralFBy += drvMPU6050_mahonytwoKiDef * halfey * (1.0f / drvMPU6050_mahonysampleFreq);
			integralFBz += drvMPU6050_mahonytwoKiDef * halfez * (1.0f / drvMPU6050_mahonysampleFreq);
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		} else {
			integralFBx = 0.0f;	// prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += drvMPU6050_mahonytwoKpDef * halfex;
		gy += drvMPU6050_mahonytwoKpDef * halfey;
		gz += drvMPU6050_mahonytwoKpDef * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / drvMPU6050_mahonysampleFreq));		// pre-multiply common factors
	gy *= (0.5f * (1.0f / drvMPU6050_mahonysampleFreq));
	gz *= (0.5f * (1.0f / drvMPU6050_mahonysampleFreq));
	qa = q0;
	qb = q1;
	qc = q2;
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx);

	// Normalise quaternion
	norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 /= norm;
	q1 /= norm;
	q2 /= norm;
	q3 /= norm;
}

/*
 * update quaternion
 */
void drvMPU6050_updateQuaternion(void) {
	int16_t ax = 0;
	int16_t ay = 0;
	int16_t az = 0;
	int16_t gx = 0;
	int16_t gy = 0;
	int16_t gz = 0;
	double axg = 0;
	double ayg = 0;
	double azg = 0;
	double gxrs = 0;
	double gyrs = 0;
	double gzrs = 0;

	//get raw data
	while(1) {
		drvMPU6050_readBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_DATA_RDY_BIT, (uint8_t *)buffer);
		if(buffer[0])
			break;
		_delay_us(10);
	}

	drvMPU6050_readBytes(MPU6050_RA_ACCEL_XOUT_H, 14, (uint8_t *)buffer);
    ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    az = (((int16_t)buffer[4]) << 8) | buffer[5];
    gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    gz = (((int16_t)buffer[12]) << 8) | buffer[13];

	#if MPU6050_CALIBRATEDACCGYRO == 1
	axg = (double)(ax-MPU6050_AXOFFSET)/MPU6050_AXGAIN;
	ayg = (double)(ay-MPU6050_AYOFFSET)/MPU6050_AYGAIN;
	azg = (double)(az-MPU6050_AZOFFSET)/MPU6050_AZGAIN;
	gxrs = (double)(gx-MPU6050_GXOFFSET)/MPU6050_GXGAIN*0.01745329; //degree to radians
	gyrs = (double)(gy-MPU6050_GYOFFSET)/MPU6050_GYGAIN*0.01745329; //degree to radians
	gzrs = (double)(gz-MPU6050_GZOFFSET)/MPU6050_GZGAIN*0.01745329; //degree to radians
	#else
	axg = (double)(ax)/MPU6050_AGAIN;
	ayg = (double)(ay)/MPU6050_AGAIN;
	azg = (double)(az)/MPU6050_AGAIN;
	gxrs = (double)(gx)/MPU6050_GGAIN*0.01745329; //degree to radians
	gyrs = (double)(gy)/MPU6050_GGAIN*0.01745329; //degree to radians
	gzrs = (double)(gz)/MPU6050_GGAIN*0.01745329; //degree to radians
	#endif

    //compute data
    drvMPU6050_mahonyUpdate(gxrs, gyrs, gzrs, axg, ayg, azg);
}

/*
 * update timer for attitude
 */
ISR(TIMER0_OVF_vect) {
	drvMPU6050_updateQuaternion();
}

/*
 * get quaternion
 */
void drvMPU6050_getQuaternion(double *qw, double *qx, double *qy, double *qz) {
	*qw = q0;
	*qx = q1;
	*qy = q2;
	*qz = q3;
}

/*
 * get euler angles
 * aerospace sequence, to obtain sensor attitude:
 * 1. rotate around sensor Z plane by yaw
 * 2. rotate around sensor Y plane by pitch
 * 3. rotate around sensor X plane by roll
 */
void drvMPU6050_getRollPitchYaw(double *roll, double *pitch, double *yaw) {
	*yaw = atan2(2*q1*q2 - 2*q0*q3, 2*q0*q0 + 2*q1*q1 - 1);
	*pitch = -asin(2*q1*q3 + 2*q0*q2);
	*roll = atan2(2*q2*q3 - 2*q0*q1, 2*q0*q0 + 2*q3*q3 - 1);
}

#endif

#endif
