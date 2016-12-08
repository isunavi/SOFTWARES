
#define SSD1306_64_32                           1

#define SCREEN_W	                64
#define SCREEN_H                    32
#define LCD_ORIENTATION_GLOBAL      LCD_ORIENTATION_NORMAL
#define LCD_BUF_SIZE                SCREEN_W

// colors
#define COLOR_BLACK                 0x00
#define COLOR_WHITE                 0xFF
    
#define SSD1306_SETCONTRAST                     0x81
#define SSD1306_DISPLAYALLON_RESUME             0xA4
#define SSD1306_DISPLAYALLON                    0xA5
#define SSD1306_NORMALDISPLAY                   0xA6
#define SSD1306_INVERTDISPLAY                   0xA7
#define SSD1306_DISPLAYOFF                      0xAE
#define SSD1306_DISPLAYON                       0xAF
#define SSD1306_SETDISPLAYOFFSET                0xD3
#define SSD1306_SETCOMPINS                      0xDA
#define SSD1306_SETVCOMDETECT                   0xDB
#define SSD1306_SETDISPLAYCLOCKDIV              0xD5
#define SSD1306_SETPRECHARGE                    0xD9
#define SSD1306_SETMULTIPLEX                    0xA8
#define SSD1306_SETLOWCOLUMN                    0x00
#define SSD1306_SETHIGHCOLUMN                   0x10
#define SSD1306_SETSTARTLINE                    0x40
#define SSD1306_MEMORYMODE                      0x20
#define SSD1306_COLUMNADDR                      0x21
#define SSD1306_PAGEADDR                        0x22
#define SSD1306_COMSCANINC                      0xC0
#define SSD1306_COMSCANDEC                      0xC8
#define SSD1306_SEGREMAP                        0xA0
#define SSD1306_CHARGEPUMP                      0x8D
#define SSD1306_EXTERNALVCC                     0x1
#define SSD1306_SWITCHCAPVCC                    0x2
// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL                 0x2F
#define SSD1306_DEACTIVATE_SCROLL               0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA        0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL         0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL          0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A



void halLCD_send_command (uint8_t c) {
    uint8_t respond;
    uint8_t buf[2] = {0x00, c};// Co = 0, D/C = 0
    respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 2, 1000);
}

void halLCD_init (uint8_t mode) { //uint8_t vccstate, uint8_t i2caddr, bool_t reset)

    uint8_t vccstate = SSD1306_SWITCHCAPVCC;
 
    HAL_GPIO_WritePin (GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_Delay (1);
    HAL_GPIO_WritePin (GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_Delay (2);
    
    // Init sequence
    halLCD_send_command (SSD1306_DISPLAYOFF);                    // 0xAE
    
    halLCD_send_command (0x00); 
    halLCD_send_command (0x12); 
    halLCD_send_command (0x00); 
    halLCD_send_command (0xB0); 
    

    halLCD_send_command (SSD1306_SETCONTRAST);                  // 0x81
#if SSD1306_64_32
    if (vccstate == SSD1306_EXTERNALVCC)
    { halLCD_send_command (0x10); }
    else
    { halLCD_send_command (0x1F); } //0x4F
#endif
    
    halLCD_send_command (0xA1); 

    halLCD_send_command (SSD1306_NORMALDISPLAY);                // 0xA6

    halLCD_send_command (SSD1306_SETMULTIPLEX);                 // 0xA8
    halLCD_send_command (SCREEN_H - 1);

    halLCD_send_command (SSD1306_COMSCANDEC);                   //0xC8
    
    halLCD_send_command (SSD1306_SETDISPLAYOFFSET);             // 0xD3
    halLCD_send_command (0x0);                                  // no offset
    
    halLCD_send_command (SSD1306_SETDISPLAYCLOCKDIV);           // 0xD5
    halLCD_send_command (0x80);                                 // the suggested ratio 0x80

    
    halLCD_send_command (SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC)
    { halLCD_send_command (0x22); }
    else
    { halLCD_send_command (0xF1); }


    halLCD_send_command (SSD1306_SETCOMPINS);                   // 0xDA
#if SSD1306_64_32
    halLCD_send_command (0x12); //0x2
#endif
#if SSD1306_128_32
    halLCD_send_command (0x2); //0x2
#endif
    
    halLCD_send_command (SSD1306_SETVCOMDETECT);                // 0xDB
    halLCD_send_command (0x40);
    
    
    halLCD_send_command (SSD1306_CHARGEPUMP);                   // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC)
    { halLCD_send_command (0x10); }
    else
    { halLCD_send_command (0x14); } //0x14

    halLCD_send_command (SSD1306_DISPLAYON); //--turn on oled panel
    
    HAL_Delay (100);
}


// Set page address 0~15
void Set_Page_Address (uint8_t add) {	
    add += SCREEN_H;
    add = 0xB0 | add;
    uint8_t respond;
    uint8_t buf[2] = {0x80, add};
    respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 2, 1000);
}


void Set_Column_Address (uint8_t add) {	
    add += SCREEN_H;
    uint8_t respond;
    uint8_t buf[4] = {0x80, (0x10 | (add >> 4)), 0x80, (0x0F & add)};
    respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 4, 1000);
}


void halLCD_update (void) {
    uint8_t respond;
    uint8_t buf[1 + SCREEN_W];
    for (uint8_t i = 0; i < (SCREEN_H / 8); i++) {
        Set_Page_Address(i);
        Set_Column_Address(0x00);
        
        buf[0] = 0x40;
        for (uint8_t j = 0; j < SCREEN_W; j++) { //16
            buf[1 + j] = screen_buf[i][j];
        }
        respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, (1 + SCREEN_W), 1000);

    }
}


unsigned char  pic[]= {
/*--64x32  --*/
0x0F,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x08,0xFF,0x08,0xF8,0x00,0xFE,0x92,
0x92,0x92,0xFE,0x00,0x00,0x00,0x00,0xFF,0x11,0x11,0x11,0x11,0x11,0x11,0xFF,0x00,
0x00,0x02,0x22,0x32,0xAA,0x26,0xFB,0x22,0x62,0xA2,0x22,0x02,0x00,0x04,0x04,0x04,
0x84,0x7C,0x15,0x16,0x14,0xF4,0x06,0x04,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x0F,
0x00,0x00,0x00,0xF0,0xF0,0x90,0x90,0x90,0x00,0x86,0x41,0x40,0xC7,0x84,0x05,0x84,
0xC4,0x44,0x45,0xC6,0x80,0x40,0xE0,0xF7,0x42,0x02,0x02,0x02,0x02,0xF2,0xF7,0x10,
0x10,0xF2,0xE2,0x01,0xD0,0xD4,0x07,0x80,0xC0,0x40,0x41,0xC2,0x80,0x04,0xD4,0xD2,
0x01,0xC0,0xC4,0x44,0x44,0xC3,0x80,0x00,0x80,0xC0,0x40,0x40,0xC0,0x80,0x00,0x00,
0xC0,0xC0,0xC0,0xCF,0xCF,0xC8,0xC8,0xC8,0xC0,0xC6,0xCF,0xC9,0xCF,0xCF,0xC0,0xC5,
0xCD,0xCB,0xCB,0xCE,0xC6,0xC0,0xC7,0xCF,0xC8,0xC0,0xC0,0xC0,0xC0,0xCF,0xCF,0xC1,
0xC3,0xC7,0xCC,0xC8,0xCF,0xCF,0xC0,0xC5,0xCD,0xCB,0xCB,0xCE,0xC6,0xC0,0xCF,0xCF,
0xC0,0xCF,0xCF,0xC0,0xC0,0xCF,0xCF,0xC0,0xD7,0xEF,0xE8,0xE8,0xFF,0xDF,0xC0,0xC0,
0xF0,0x80,0x80,0x00,0xFE,0x92,0x92,0x92,0x00,0xFE,0x12,0x32,0x6C,0x80,0x00,0x20,
0x20,0x00,0x00,0x7C,0x82,0x82,0x7C,0x00,0xFE,0x80,0x80,0x80,0x00,0xFE,0x92,0x92,
0x92,0x00,0xFE,0x82,0x82,0x7C,0x00,0x7C,0x82,0x82,0x7C,0x00,0x60,0x58,0xFE,0x40,
0x00,0x9C,0x92,0xD2,0x7C,0x00,0x10,0x10,0x00,0x82,0xFE,0x80,0x00,0x80,0x80,0xF0,
};

void Display_Picture (unsigned char pic[])
{
    unsigned char i,j,num=0;
    uint8_t respond;
    uint8_t buf[1 + SCREEN_W];
    
	for(i=0;i<4;i++) {
        Set_Page_Address(i);
        Set_Column_Address(0x00);
        buf[0] = 0x40;
        for (uint8_t j = 0; j < SCREEN_W; j++) { //16
            buf[1 + j] = pic[i*SCREEN_W+j];
        }
        respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, (1 + SCREEN_W), 1000);
	}
    return;
}
