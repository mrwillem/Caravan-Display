/**
  ******************************************************************************
  * @file    stm3210b_eval_lcd.c
  * @author  MCD Application Team
  * @version V4.6.1
  * @date    18-April-2011
  * @brief   This file includes the LCD driver for AM-240320LTNQW00H (LCD_HX8312),
  *          AM-240320L8TNQW00H (LCD_ILI9320), AM-240320LDTNQW00H (LCD_SPFD5408B)
  *          Liquid Crystal Display Module of STM3210B-EVAL board.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f103c8t6_lcd.h"
#include "../Common/fonts.c"
#include "spi.h"

/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @addtogroup STM3210B_EVAL
  * @{
  */
    
/** @defgroup STM3210B_EVAL_LCD 
  * @briefThis file includes the LCD driver for AM-240320LTNQW00H (LCD_HX8312),
  *          AM-240320L8TNQW00H (LCD_ILI9320), AM-240320LDTNQW00H (LCD_SPFD5408B)
  *          Liquid Crystal Display Module of STM3210B-EVAL board.
  * @{
  */ 

/** @defgroup STM3210B_EVAL_LCD_Private_Types
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Defines
  * @{
  */ 


#define START_BYTE         0x70
#define SET_INDEX          0x00
#define READ_STATUS        0x01
#define LCD_WRITE_REG      0x02
#define LCD_READ_REG       0x03
#define MAX_POLY_CORNERS   200
#define POLY_Y(Z)          ((int32_t)((Points + Z)->X))
#define POLY_X(Z)          ((int32_t)((Points + Z)->Y))
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Macros
  * @{
  */
#define ABS(X)  ((X) > 0 ? (X) : -(X))   
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Variables
  * @{
  */ 
static sFONT *LCD_Currentfonts;
/* Global variables to set the written text color */
static __IO uint16_t TextColor = 0x0000, BackColor = 0xFFFF;
static __IO uint32_t LCDType = LCD_ILI9320;
volatile uint8_t LCD_RX_Buffer[3];
volatile uint8_t LCD_TX_Buffer[3];
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Function_Prototypes
  * @{
  */ 
#ifndef USE_Delay
static void delay(vu32 nCount);
#endif /* USE_Delay*/


static void LCD_WriteRegILI9341(uint8_t LCD_Reg);
static void PutPixel(int16_t x, int16_t y);
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed);
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Functions
  * @{
  */ 

/**
  * @brief  DeInitializes the LCD.
  * @param  None
  * @retval None
  */
void LCD_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< LCD Display Off */
  LCD_DisplayOff();

  /*!< LCD_SPI disable */
  SPI_Cmd(LCD_SPI, DISABLE);
  
  /*!< LCD_SPI DeInit */
  SPI_I2S_DeInit(LCD_SPI);
   
  /*!< Disable SPI clock  */
  RCC_APB1PeriphClockCmd(LCD_SPI_CLK, DISABLE);
    
  /* Configure NCS in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_NCS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);
  
  /* Configure NWR(RNW), RS in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_RS_PIN;
  GPIO_Init(LCD_RS_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_NRST_PIN;
  GPIO_Init(LCD_NRST_GPIO_PORT, &GPIO_InitStructure);
  
  /* Configure SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN;
  GPIO_Init(LCD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MISO_PIN;
  GPIO_Init(LCD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MOSI_PIN;
  GPIO_Init(LCD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Setups the LCD.
  * @param  None
  * @retval None
  */
void LCD_Setup(void)
{
	/* The SPI Interface should be up already */
	  /* Configure the LCD Control pins --------------------------------------------*/
	  LCD_CtrlLinesConfig();

	  /* Set Display Reset */
	  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
	  LCD_CtrlLinesWrite(LCD_NRST_GPIO_PORT, LCD_NRST_PIN, Bit_SET);
	  _delay_(5);
	  LCD_CtrlLinesWrite(LCD_NRST_GPIO_PORT, LCD_NRST_PIN, Bit_RESET);

	  _delay_(5); /* Delay 50 ms */
	  _delay_(50); /* Delay another 50 ms */
	  LCD_CtrlLinesWrite(LCD_NRST_GPIO_PORT, LCD_NRST_PIN, Bit_SET);
	  _delay_(5); /* Delay 50 ms */
	  _delay_(50); /* Delay 50 ms */
	  /* Issue Device Specific command */
	  LCD_WriteRegILI9341(0xCB);
	  LCD_WriteRAM(0x39);
	  LCD_WriteRAM(0x2C);
	  LCD_WriteRAM(0x00);
	  LCD_WriteRAM(0x34);
	  LCD_WriteRAM(0x02);

	  /* Issue another device specific command */
	  LCD_WriteRegILI9341(0xCF);
      LCD_WriteRAM(0x00);
      LCD_WriteRAM(0XC1);
      LCD_WriteRAM(0X30);

      LCD_WriteRegILI9341(0xE8);
      LCD_WriteRAM(0x85);
      LCD_WriteRAM(0x00);
      LCD_WriteRAM(0x78);

      LCD_WriteRegILI9341(0xEA);
      LCD_WriteRAM(0x00);
      LCD_WriteRAM(0x00);

      LCD_WriteRegILI9341(0xED);
      LCD_WriteRAM(0x64);
      LCD_WriteRAM(0x03);
      LCD_WriteRAM(0X12);
      LCD_WriteRAM(0X81);

      LCD_WriteRegILI9341(0xF7);
      LCD_WriteRAM(0x20);

      LCD_WriteRegILI9341(0xC0);    //Power control
      LCD_WriteRAM(0x23);   //VRH[5:0]

      LCD_WriteRegILI9341(0xC1);    //Power control
      LCD_WriteRAM(0x10);   //SAP[2:0];BT[3:0]

      LCD_WriteRegILI9341(0xC5);    //VCM control
      LCD_WriteRAM(0x3e);   //Contrast
      LCD_WriteRAM(0x28);

      LCD_WriteRegILI9341(0xC7);    //VCM control2
      LCD_WriteRAM(0x86);   //--

      LCD_WriteRegILI9341(0x36);    // Memory Access Control
      LCD_WriteRAM(0x48);   //C8	   //48 68//28 E8

      LCD_WriteRegILI9341(0x3A);
      LCD_WriteRAM(0x55);

      LCD_WriteRegILI9341(0xB1);
      LCD_WriteRAM(0x00);
      LCD_WriteRAM(0x18);

      LCD_WriteRegILI9341(0xB6);    // Display Function Control
      LCD_WriteRAM(0x08);
      LCD_WriteRAM(0x82);
      LCD_WriteRAM(0x27);

      /*
              LCD_WriteRegILI9341(0xF2);    // 3Gamma Function Disable
              LCD_WriteRAM(0x00);

              LCD_WriteRegILI9341(0x26);    //Gamma curve selected
              LCD_WriteRAM(0x01);

              LCD_WriteRegILI9341(0xE0);    //Set Gamma
              LCD_WriteRAM(0x0F);
              LCD_WriteRAM(0x31);
              LCD_WriteRAM(0x2B);
              LCD_WriteRAM(0x0C);
              LCD_WriteRAM(0x0E);
              LCD_WriteRAM(0x08);
              LCD_WriteRAM(0x4E);
              LCD_WriteRAM(0xF1);
              LCD_WriteRAM(0x37);
              LCD_WriteRAM(0x07);
              LCD_WriteRAM(0x10);
              LCD_WriteRAM(0x03);
              LCD_WriteRAM(0x0E);
              LCD_WriteRAM(0x09);
              LCD_WriteRAM(0x00);

              LCD_WriteRegILI9341(0XE1);    //Set Gamma
              LCD_WriteRAM(0x00);
              LCD_WriteRAM(0x0E);
              LCD_WriteRAM(0x14);
              LCD_WriteRAM(0x03);
              LCD_WriteRAM(0x11);
              LCD_WriteRAM(0x07);
              LCD_WriteRAM(0x31);
              LCD_WriteRAM(0xC1);
              LCD_WriteRAM(0x48);
              LCD_WriteRAM(0x08);
              LCD_WriteRAM(0x0F);
              LCD_WriteRAM(0x0C);
              LCD_WriteRAM(0x31);
              LCD_WriteRAM(0x36);
              LCD_WriteRAM(0x0F);
      */
      LCD_WriteRegILI9341(0x11);    //Exit Sleep
      _delay_(12);

      LCD_WriteRegILI9341(0x29);    //Display on
      LCD_WriteRegILI9341(0x2c);
      _delay_(1);

}


/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval None
  */
void STM32F103C8T6_LCD_Init(void)
{
  /* Setups the LCD */
  LCDType = LCD_ILI9341;
  LCD_Setup();

  LCD_SetFont(&LCD_DEFAULT_FONT);
}

/**
  * @brief  Sets the LCD Text and Background colors.
  * @param  _TextColor: specifies the Text Color.
  * @param  _BackColor: specifies the Background Color.
  * @retval None
  */
void LCD_SetColors(__IO uint16_t _TextColor, __IO uint16_t _BackColor)
{
  TextColor = _TextColor; 
  BackColor = _BackColor;
}

/**
  * @brief  Gets the LCD Text and Background colors.
  * @param  _TextColor: pointer to the variable that will contain the Text 
            Color.
  * @param  _BackColor: pointer to the variable that will contain the Background 
            Color.
  * @retval None
  */
void LCD_GetColors(__IO uint16_t *_TextColor, __IO uint16_t *_BackColor)
{
  *_TextColor = TextColor; *_BackColor = BackColor;
}

/**
  * @brief  Sets the Text color.
  * @param  Color: specifies the Text color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetTextColor(__IO uint16_t Color)
{
  TextColor = Color;
}


/**
  * @brief  Sets the Background color.
  * @param  Color: specifies the Background color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetBackColor(__IO uint16_t Color)
{
  BackColor = Color;
}

/**
  * @brief  Sets the Text Font.
  * @param  fonts: specifies the font to be used.
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}


/**
  * @brief  Gets the Text Font.
  * @param  None.
  * @retval the used font.
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}

/**
  * @brief  Clears the selected line.
  * @param  Line: the Line to be cleared.
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..n
  * @retval None
  */
void LCD_ClearLine(uint8_t Line)
{
  uint16_t refcolumn = LCD_PIXEL_WIDTH - 1;
  /* Send the string character by character on lCD */
  while (((refcolumn + 1) & 0xFFFF) >= LCD_Currentfonts->Width)
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, ' ');
    /* Decrement the column position by 16 */
    refcolumn -= LCD_Currentfonts->Width;
  }
}

/**
  * @brief  Paints the whole screen.
  * @param  VL: Color to be painted.
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..n
  * @retval None
  */
/*void Pant(char VL)
{
  int i,j;
  Address_set(0,0,240,320);
  for(i=0;i<320;i++)
  {
    for (j=0;j<480;j++)
    {
      LCD_WriteRAM(VL);
    }
  }
}*/

/**
  * @brief  Sets x and y coordinates which to paint.
  * @param  x1,y1,x2,y2: coordinates to paint.
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..n
  * @retval None
  */
void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
   LCD_WriteRegILI9341(0x2a);
   LCD_WriteRAM(x1>>8);
   LCD_WriteRAM(x1);
   LCD_WriteRAM(x2>>8);
   LCD_WriteRAM(x2);

   LCD_WriteRegILI9341(0x2b);
   LCD_WriteRAM(y1>>8);
   LCD_WriteRAM(y1);
   LCD_WriteRAM(y2>>8);
   LCD_WriteRAM(y2);

   LCD_WriteRegILI9341(0x2C);
}

/**
  * @brief  Clears the hole LCD.
  * @param  Color: the color of the background.
  * @retval None
  */
void LCD_Clear(uint16_t Color)
{
  uint32_t index = 0;
  
  LCD_SetCursor(0x00, 0x00);

  for(index = 0; index < 76800; index++)
  {
    LCD_WriteRAM(Color>>8);
    LCD_WriteRAM(Color);
  }

}


/**
  * @brief  Sets the cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position. 
  * @retval None
  */
void LCD_SetCursor(uint8_t Xpos, uint16_t Ypos)
{
  Address_set(((uint16_t)Xpos),Ypos,240,320);
}


/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape.
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void LCD_DrawChar(uint8_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, i = 0;
  uint8_t Xaddress = 0;
   
  Xaddress = Xpos;
  
  LCD_SetCursor(Xaddress, Ypos);
  
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {

    for(i = 0; i < LCD_Currentfonts->Width; i++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> i)) == 0x00) &&(LCD_Currentfonts->Width <= 12))||
        (((c[index] & (0x1 << i)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))

      {
        LCD_WriteRAM(BackColor);
      }
      else
      {
        LCD_WriteRAM(TextColor);
      }
    }

    //Xaddress++;
    Ypos++;
    LCD_SetCursor(Xaddress, Ypos);
  }
}
void LCD_Draw8BitChar(uint8_t Xpos, uint16_t Ypos, const uint8_t *c)
{
  uint32_t index = 0, i = 0, row=0, temp=0;
  uint8_t Xaddress = 0;

  Xaddress = Xpos;

  LCD_SetCursor(Xaddress, Ypos);

  for(index = 0; index < (LCD_Currentfonts->RowBytes*LCD_Currentfonts->Height); temp++)
  {

    for(i = 0; i < LCD_Currentfonts->Width; temp++)
    {
    	for (row=0; row<8; row++)
    	{
    		if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> row)) == 0x00) &&(LCD_Currentfonts->Width <= 12))||
    		(((c[index] & (0x1 << row)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))

    		{
    			LCD_WriteRAM(BackColor);
    		}
    		else
    		{
    			LCD_WriteRAM(TextColor);
    		}
    		i++;
    	}
    	index++;
    }

    //Xaddress++;
    Ypos++;
    LCD_SetCursor(Xaddress, Ypos);
  }
}



/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @retval None
  */
void LCD_DisplayChar(uint8_t Line, uint16_t Column, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_DrawChar(Line, Column, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}
/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @retval None
  */

void LCD_Display8BitChar(uint8_t Line, uint16_t Column, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_Draw8BitChar(Line, Column, &LCD_Currentfonts->table[(uint16_t)(0.5*Ascii * LCD_Currentfonts->RowBytes * LCD_Currentfonts->Height)]);
}


/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  *ptr: pointer to string to display on LCD.
  * @retval None
  */
void LCD_DisplayStringLine(uint8_t Line, uint8_t *ptr)
{
  uint16_t refcolumn = LCD_PIXEL_WIDTH - 1;

  /* Send the string character by character on lCD */
  while ((*ptr != 0) & (((refcolumn + 1) & 0xFFFF) >= LCD_Currentfonts->Width))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, *ptr);
    /* Decrement the column position by 16 */
    refcolumn -= LCD_Currentfonts->Width;
    /* Point on the next character */
    ptr++;
  }
}


/**
  * @brief  Sets a display window
  * @param  Xpos: specifies the X buttom left position.
  * @param  Ypos: specifies the Y buttom left position.
  * @param  Height: display window height.
  * @param  Width: display window width.
  * @retval None
  */
void LCD_SetDisplayWindow(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Horizontal GRAM Start Address */
    if(Xpos >= Height)
    {
      LCD_WriteReg(LCD_REG_80, (Xpos - Height + 1));
    }
    else
    {
      LCD_WriteReg(LCD_REG_80, 0);
    }
    /* Horizontal GRAM End Address */
    LCD_WriteReg(LCD_REG_81, Xpos);
    /* Vertical GRAM Start Address */
    if(Ypos >= Width)
    {
      LCD_WriteReg(LCD_REG_82, (Ypos - Width + 1));
    }  
    else
    {
      LCD_WriteReg(LCD_REG_82, 0);
    }
    /* Vertical GRAM End Address */
    LCD_WriteReg(LCD_REG_83, Ypos);
  }
  else if(LCDType == LCD_HX8312)
  {  
    LCD_WriteReg(LCD_REG_1, 0xD0);
    LCD_WriteReg(LCD_REG_5, 0x14);
  
    LCD_WriteReg(LCD_REG_69, (Xpos - Height + 1));
    LCD_WriteReg(LCD_REG_70, Xpos);
 
    LCD_WriteReg(LCD_REG_71, (((Ypos - Width + 1) & 0x100)>> 8));
    LCD_WriteReg(LCD_REG_72, ((Ypos - Width + 1) & 0xFF));
    LCD_WriteReg(LCD_REG_73, ((Ypos & 0x100)>> 8));
    LCD_WriteReg(LCD_REG_74, (Ypos & 0xFF));
  }
  LCD_SetCursor(Xpos, Ypos);
}


/**
  * @brief  Disables LCD Window mode.
  * @param  None
  * @retval None
  */
void LCD_WindowModeDisable(void)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_SetDisplayWindow(239, 0x13F, 240, 320);
    LCD_WriteReg(LCD_REG_3, 0x1018);
  }
  else if(LCDType == LCD_HX8312)
  {
    LCD_WriteReg(LCD_REG_1, 0x50);
    LCD_WriteReg(LCD_REG_5, 0x04); 
  }
    
}


/**
  * @brief  Displays a line.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Length: line length.
  * @param  Direction: line direction.
  *   This parameter can be one of the following values: Vertical or Horizontal.
  * @retval None
  */
void LCD_DrawLine(uint8_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction)
{
	uint32_t i = 0;
	LCD_SetCursor(Xpos, Ypos);
	if(Direction == LCD_DIR_HORIZONTAL)
	{
		for(i = 0; i < Length; i++)
		{
			LCD_WriteRAM(TextColor);
		}
	}
	else
	{
		for(i = 0; i < Length; i++)
		{
			LCD_WriteRAM(TextColor);
			Xpos++;
			LCD_SetCursor(Xpos, Ypos);
		}
	}
}


/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: display rectangle height.
  * @param  Width: display rectangle width.
  * @retval None
  */
void LCD_DrawRect(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  LCD_DrawLine(Xpos, Ypos, Width, LCD_DIR_HORIZONTAL);
  LCD_DrawLine((Xpos + Height), Ypos, Width, LCD_DIR_HORIZONTAL);
  
  LCD_DrawLine(Xpos, Ypos, Height, LCD_DIR_VERTICAL);
  LCD_DrawLine(Xpos, (Ypos - Width + 1), Height, LCD_DIR_VERTICAL);
}


/**
  * @brief  Displays a circle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Radius
  * @retval None
  */
void LCD_DrawCircle(uint8_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;/* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    LCD_SetCursor(Xpos + CurX, Ypos + CurY);

    LCD_WriteRAM(TextColor);

    LCD_SetCursor(Xpos + CurX, Ypos - CurY);


    LCD_WriteRAM(TextColor);

    LCD_SetCursor(Xpos - CurX, Ypos + CurY);


    LCD_WriteRAM(TextColor);

    LCD_SetCursor(Xpos - CurX, Ypos - CurY);

      LCD_WriteRAM(TextColor);

    LCD_SetCursor(Xpos + CurY, Ypos + CurX);

      LCD_WriteRAM(TextColor);

    LCD_SetCursor(Xpos + CurY, Ypos - CurX);


      LCD_WriteRAM(TextColor);

    LCD_SetCursor(Xpos - CurY, Ypos + CurX);


      LCD_WriteRAM(TextColor);

    LCD_SetCursor(Xpos - CurY, Ypos - CurX);


      LCD_WriteRAM(TextColor);

    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
}


/**
  * @brief  Displays a monocolor picture.
  * @param  Pict: pointer to the picture array.
  * @retval None
  */
void LCD_DrawMonoPict(const uint32_t *Pict)
{
  uint32_t index = 0, i = 0;
  LCD_SetCursor(0, (LCD_PIXEL_WIDTH - 1)); 

  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  }

  for(index = 0; index < 2400; index++)
  {
    for(i = 0; i < 32; i++)
    {
      if((Pict[index] & (1 << i)) == 0x00)
      {
        LCD_WriteRAM(BackColor);
      }
      else
      {
        LCD_WriteRAM(TextColor);
      }
    }
  }

  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
  }
}

#ifdef USE_LCD_DrawBMP 
/**
  * @brief  Displays a bitmap picture loaded in the SPI Flash.
  * @param  BmpAddress: Bmp picture address in the SPI Flash.
  * @retval None
  */
void LCD_DrawBMP(uint32_t BmpAddress)
{
  uint32_t i = 0, size = 0;
  /* Read bitmap size */
  sFLASH_ReadBuffer((uint8_t*)&size, BmpAddress + 2, 4);
  /* get bitmap data address offset */
  sFLASH_ReadBuffer((uint8_t*)&i, BmpAddress + 10, 4);
  
  size = (size - i)/2;
  sFLASH_StartReadSequence(BmpAddress + i);
  /* Disable LCD_SPI  */
  SPI_Cmd(LCD_SPI, DISABLE);
  /* SPI in 16-bit mode */
  SPI_DataSizeConfig(LCD_SPI, SPI_DataSize_16b);
  /* Enable LCD_SPI  */
  SPI_Cmd(LCD_SPI, ENABLE);
  
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(LCD_REG_3, 0x1008);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  }
  
  /* Read bitmap data from SPI Flash and send them to LCD */
  for(i = 0; i < size; i++)
  {
    LCD_WriteRAM(__REV16(sFLASH_SendHalfWord(0xA5A5)));
  }
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
  }

  /* Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
  /* Disable LCD_SPI  */
  SPI_Cmd(LCD_SPI, DISABLE);
  /* SPI in 8-bit mode */
  SPI_DataSizeConfig(LCD_SPI, SPI_DataSize_8b);
  /* Enable LCD_SPI  */
  SPI_Cmd(LCD_SPI, ENABLE);

  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D = 01 (Horizontal : increment, Vertical : decrement) */
    /* AM = 1 (address is updated in vertical writing direction) */
    LCD_WriteReg(LCD_REG_3, 0x1018);
  }
}
#endif /* USE_LCD_DrawBMP */
 
/**
  * @brief  Displays a full rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: rectangle height.
  * @param  Width: rectangle width.
  * @retval None
  */
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  LCD_SetTextColor(TextColor);

  LCD_DrawLine(Xpos, Ypos, Width, LCD_DIR_HORIZONTAL);
  LCD_DrawLine((Xpos + Height), Ypos, Width, LCD_DIR_HORIZONTAL);
  
  LCD_DrawLine(Xpos, Ypos, Height, LCD_DIR_VERTICAL);
  LCD_DrawLine(Xpos, (Ypos - Width + 1), Height, LCD_DIR_VERTICAL);

  Width -= 2;
  Height--;
  Ypos--;

  LCD_SetTextColor(BackColor);

  while(Height--)
  {
    LCD_DrawLine(++Xpos, Ypos, Width, LCD_DIR_HORIZONTAL);    
  }

  LCD_SetTextColor(TextColor);
}

/**
  * @brief  Displays a full circle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Radius
  * @retval None
  */
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);

  CurX = 0;
  CurY = Radius;
  
  LCD_SetTextColor(BackColor);

  while (CurX <= CurY)
  {
    if(CurY > 0) 
    {
      LCD_DrawLine(Xpos - CurX, Ypos + CurY, 2*CurY, LCD_DIR_HORIZONTAL);
      LCD_DrawLine(Xpos + CurX, Ypos + CurY, 2*CurY, LCD_DIR_HORIZONTAL);
    }

    if(CurX > 0) 
    {
      LCD_DrawLine(Xpos - CurY, Ypos + CurX, 2*CurX, LCD_DIR_HORIZONTAL);
      LCD_DrawLine(Xpos + CurY, Ypos + CurX, 2*CurX, LCD_DIR_HORIZONTAL);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }

  LCD_SetTextColor(TextColor);
  LCD_DrawCircle(Xpos, Ypos, Radius);
}

/**
  * @brief  Displays an uni line (between two points).
  * @param  x1: specifies the point 1 x position.
  * @param  y1: specifies the point 1 y position.
  * @param  x2: specifies the point 2 x position.
  * @param  y2: specifies the point 2 y position.
  * @retval None
  */
void LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    PutPixel(x, y);             /* Draw the current pixel */
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }
}

/**
  * @brief  Displays an polyline (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLine(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0;

  if(PointCount < 2)
  {
    return;
  }

  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    LCD_DrawUniLine(X, Y, Points->X, Points->Y);
  }
}

/**
  * @brief  Displays an relative polyline (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @param  Closed: specifies if the draw is closed or not.
  *           1: closed, 0 : not closed.
  * @retval None
  */
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed)
{
  int16_t X = 0, Y = 0;
  pPoint First = Points;

  if(PointCount < 2)
  {
    return;
  }  
  X = Points->X;
  Y = Points->Y;
  while(--PointCount)
  {
    Points++;
    LCD_DrawUniLine(X, Y, X + Points->X, Y + Points->Y);
    X = X + Points->X;
    Y = Y + Points->Y;
  }
  if(Closed)
  {
    LCD_DrawUniLine(First->X, First->Y, X, Y);
  }  
}

/**
  * @brief  Displays a closed polyline (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLine(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLine(Points, PointCount);
  LCD_DrawUniLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
}

/**
  * @brief  Displays a relative polyline (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLineRelative(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 0);
}

/**
  * @brief  Displays a closed relative polyline (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLineRelative(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 1);
}


/**
  * @brief  Displays a  full polyline (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_FillPolyLine(pPoint Points, uint16_t PointCount)
{
  /*  public-domain code by Darel Rex Finley, 2007 */
  uint16_t  nodes = 0, nodeX[MAX_POLY_CORNERS], pixelX = 0, pixelY = 0, i = 0,
  j = 0, swap = 0;
  uint16_t  IMAGE_LEFT = 0, IMAGE_RIGHT = 0, IMAGE_TOP = 0, IMAGE_BOTTOM = 0;

  IMAGE_LEFT = IMAGE_RIGHT = Points->X;
  IMAGE_TOP= IMAGE_BOTTOM = Points->Y;

  for(i = 1; i < PointCount; i++)
  {
    pixelX = POLY_X(i);
    if(pixelX < IMAGE_LEFT)
    {
      IMAGE_LEFT = pixelX;
    }
    if(pixelX > IMAGE_RIGHT)
    {
      IMAGE_RIGHT = pixelX;
    }
    
    pixelY = POLY_Y(i);
    if(pixelY < IMAGE_TOP)
    { 
      IMAGE_TOP = pixelY;
    }
    if(pixelY > IMAGE_BOTTOM)
    {
      IMAGE_BOTTOM = pixelY;
    }
  }
  
  LCD_SetTextColor(BackColor);  

  /*  Loop through the rows of the image. */
  for (pixelY = IMAGE_TOP; pixelY < IMAGE_BOTTOM; pixelY++) 
  {  
    /* Build a list of nodes. */
    nodes = 0; j = PointCount-1;

    for (i = 0; i < PointCount; i++) 
    {
      if (POLY_Y(i)<(double) pixelY && POLY_Y(j)>=(double) pixelY || POLY_Y(j)<(double) pixelY && POLY_Y(i)>=(double) pixelY) 
      {
        nodeX[nodes++]=(int) (POLY_X(i)+((pixelY-POLY_Y(i))*(POLY_X(j)-POLY_X(i)))/(POLY_Y(j)-POLY_Y(i))); 
      }
      j = i; 
    }
  
    /* Sort the nodes, via a simple "Bubble" sort. */
    i = 0;
    while (i < nodes-1) 
    {
      if (nodeX[i]>nodeX[i+1]) 
      {
        swap = nodeX[i]; 
        nodeX[i] = nodeX[i+1]; 
        nodeX[i+1] = swap; 
        if(i)
        {
          i--; 
        }
      }
      else 
      {
        i++;
      }
    }
  
    /*  Fill the pixels between node pairs. */
    for (i = 0; i < nodes; i+=2) 
    {
      if(nodeX[i] >= IMAGE_RIGHT) 
      {
        break;
      }
      if(nodeX[i+1] > IMAGE_LEFT) 
      {
        if (nodeX[i] < IMAGE_LEFT)
        {
          nodeX[i]=IMAGE_LEFT;
        }
        if(nodeX[i+1] > IMAGE_RIGHT)
        {
          nodeX[i+1] = IMAGE_RIGHT;
        }
        LCD_SetTextColor(BackColor);
        LCD_DrawLine(pixelY, nodeX[i+1], nodeX[i+1] - nodeX[i], LCD_DIR_HORIZONTAL);
        LCD_SetTextColor(TextColor);
        PutPixel(pixelY, nodeX[i+1]);
        PutPixel(pixelY, nodeX[i]);
        /* for (j=nodeX[i]; j<nodeX[i+1]; j++) PutPixel(j,pixelY); */
      }
    }
  } 

  /* draw the edges */
  LCD_SetTextColor(TextColor);
}

/**
  * @brief  Reset LCD control line(/CS) and Send Start-Byte
  * @param  Start_Byte: the Start-Byte to be sent
  * @retval None
  */
void LCD_nCS_StartByte(uint8_t Start_Byte)
{
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);

  SPI_I2S_SendData(LCD_SPI, Start_Byte);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }
}


/**
  * @brief  Writes index to select the LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void LCD_WriteRegIndex(uint8_t LCD_Reg)
{
  /* Reset LCD control line(/CS) and Send Start-Byte */
  LCD_nCS_StartByte(START_BYTE | SET_INDEX);

  /* Write 16-bit Reg Index (High Byte is 0) */
  SPI_I2S_SendData(LCD_SPI, 0x00);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  SPI_I2S_SendData(LCD_SPI, LCD_Reg);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
}




/**
  * @brief  Reads the selected LCD Register.
  * @param  LCD_Reg: address of the selected register.
  * @retval LCD Register Value.
  */
uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
  uint16_t tmp = 0;
  uint8_t i = 0;
  
  /* LCD_SPI prescaler: 4 */
  LCD_SPI->CR1 &= 0xFFC7;
  LCD_SPI->CR1 |= 0x0008;
  /* Write 16-bit Index (then Read Reg) */
  LCD_WriteRegIndex(LCD_Reg);
  /* Read 16-bit Reg */
  /* Reset LCD control line(/CS) and Send Start-Byte */
  LCD_nCS_StartByte(START_BYTE | LCD_READ_REG);
  
  for(i = 0; i < 5; i++)
  {
    SPI_I2S_SendData(LCD_SPI, 0xFF);
    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
    {
    }
    /* One byte of invalid dummy data read after the start byte */
    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
    {    
    }
    SPI_I2S_ReceiveData(LCD_SPI); 
  }

  SPI_I2S_SendData(LCD_SPI, 0xFF);

  /* Read upper byte */
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  /* Read lower byte */
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }
  tmp = SPI_I2S_ReceiveData(LCD_SPI);
  
  
  SPI_I2S_SendData(LCD_SPI, 0xFF);
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  /* Read lower byte */
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }

  tmp = ((tmp & 0xFF) << 8) | SPI_I2S_ReceiveData(LCD_SPI);
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);

  /* LCD_SPI prescaler: 2 */
  LCD_SPI->CR1 &= 0xFFC7;

  return tmp;
}


/**
  * @brief  Prepare to write to the LCD RAM.
  * @param  None
  * @retval None
  */
void LCD_WriteRAM_Prepare(void)
{
  LCD_WriteRegIndex(LCD_REG_34); /* Select GRAM Reg */

  /* Reset LCD control line(/CS) and Send Start-Byte */
  LCD_nCS_StartByte(START_BYTE | LCD_WRITE_REG);
}


/**
  * @brief  Writes 1 word to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
void LCD_WriteRAMWord(uint16_t RGB_Code)
{


  LCD_WriteRAM(RGB_Code);

}


static void PutPixel(int16_t x, int16_t y)
{
  if(x < 0 || x > 239 || y < 0 || y > 319)
  {
    return;
  }
  LCD_DrawLine(x, y, 1, LCD_DIR_HORIZONTAL);
}

/**
  * @brief  Writes Command on selected ILI9341 register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
static void LCD_WriteRegILI9341(uint8_t LCD_Reg)
{

 	while(SPI1_BLOCK != SPI_BLOCK_FREE)
	{
	}
	SPI1_BLOCK = SPI_BLOCK_LCD_WRITE_REG;
	LCD_TX_Buffer[0]=(uint8_t)LCD_Reg;
	SPI1_send(1, SPI_BLOCK_LCD_WRITE_REG, (uint32_t)LCD_TX_Buffer, (uint32_t)LCD_RX_Buffer);
	//while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) == SET);
	//GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);
/*
	  LCD_CtrlLinesWrite(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_RESET);

	  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);



	  SPI_I2S_SendData(LCD_SPI, LCD_Reg);

	  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
	  {
	  }
	  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
*/

}




/**
  * @brief  Writes to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @param  LCD_RegValue: value to write to the selected register.
  * @retval None
  */
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD_WriteRegILI9341(LCD_Reg);
	LCD_WriteRAM((uint8_t)LCD_RegValue);
}


/**
  * @brief  Writes to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
void LCD_WriteRAM(uint16_t RGB_Code)
{

	while(SPI1_BLOCK != SPI_BLOCK_FREE)
	{
	}
	SPI1_BLOCK = SPI_BLOCK_LCD_WRITE_RAM;
	LCD_TX_Buffer[0]=(uint8_t)RGB_Code;
	SPI1_send(1, SPI_BLOCK_LCD_WRITE_RAM, (uint32_t)LCD_TX_Buffer, (uint32_t)LCD_RX_Buffer);
	//while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) == SET);
	//GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);
	/*
	  LCD_CtrlLinesWrite(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
	  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);

	  SPI_I2S_SendData(LCD_SPI, ((uint8_t)RGB_Code));
	  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
	  {
	  }
	  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
	  */
}


/**
  * @brief  Power on the LCD.
  * @param  None
  * @retval None
  */
void LCD_PowerOn(void)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Power On sequence ---------------------------------------------------------*/
    LCD_WriteReg(LCD_REG_16, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(LCD_REG_17, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_WriteReg(LCD_REG_18, 0x0000); /* VREG1OUT voltage */
    LCD_WriteReg(LCD_REG_19, 0x0000); /* VDV[4:0] for VCOM amplitude */
    _delay_(20);               /* Dis-charge capacitor power voltage (200ms) */
    LCD_WriteReg(LCD_REG_16, 0x17B0); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(LCD_REG_17, 0x0137); /* DC1[2:0], DC0[2:0], VC[2:0] */
    _delay_(5);                /* Delay 50 ms */
    LCD_WriteReg(LCD_REG_18, 0x0139); /* VREG1OUT voltage */
    _delay_(5);                /* delay 50 ms */
    LCD_WriteReg(LCD_REG_19, 0x1d00); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(LCD_REG_41, 0x0013); /* VCM[4:0] for VCOMH */
    _delay_(5);                /* delay 50 ms */
    LCD_WriteReg(LCD_REG_7, 0x0173);  /* 262K color and display ON */
  }
  else if(LCDType == LCD_HX8312)
  {  
    /* Power On Set */
    LCD_WriteReg(LCD_REG_28, 0x73);
    LCD_WriteReg(LCD_REG_36, 0x74);
    LCD_WriteReg(LCD_REG_30, 0x01);
    LCD_WriteReg(LCD_REG_24, 0xC1);
    _delay_(1); /* Delay 10 ms */
    LCD_WriteReg(LCD_REG_24, 0xE1);
    LCD_WriteReg(LCD_REG_24, 0xF1);
    _delay_(6); /* Delay 60 ms */
    LCD_WriteReg(LCD_REG_24, 0xF5);
    _delay_(6); /* Delay 60 ms */
    LCD_WriteReg(LCD_REG_27, 0x09);
    _delay_(1); /* Delay 10 ms */
    LCD_WriteReg(LCD_REG_31, 0x11);
    LCD_WriteReg(LCD_REG_32, 0x0E);
    LCD_WriteReg(LCD_REG_30, 0x81);
    _delay_(1); /* Delay 10 ms */
  }
}


/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOn(void)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Display On */
    LCD_WriteReg(LCD_REG_7, 0x0173); /* 262K color and display ON */
  }
  else if(LCDType == LCD_HX8312)
  {  
    LCD_WriteReg(LCD_REG_1, 0x50);
    LCD_WriteReg(LCD_REG_5, 0x04);
    /* Display On */
    LCD_WriteReg(LCD_REG_0, 0x80);
    LCD_WriteReg(LCD_REG_59, 0x01);
    _delay_(4); /* Delay 40 ms */
    LCD_WriteReg(LCD_REG_0, 0x20);
  }  
}


/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOff(void)
{
	  /* Set Display Off */
	  LCD_WriteRegILI9341(0x28);
	  _delay_(4); /* Delay 40 ms */
	  /* Set Sleep Mode */
	  LCD_WriteRegILI9341(0x10);
}


/**
  * @brief  Configures LCD control lines in Output Push-Pull mode.
  * @param  None
  * @retval None
  */
void LCD_CtrlLinesConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(LCD_NCS_GPIO_CLK | LCD_NRST_GPIO_CLK | LCD_RS_GPIO_CLK, ENABLE);

  /* Configure Chip Select (NCS) in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_NCS_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);
  

  /* Configure RESET(NRST), Register Select (RS) in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_RS_PIN;
  GPIO_Init(LCD_RS_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_NRST_PIN;
  GPIO_Init(LCD_NRST_GPIO_PORT, &GPIO_InitStructure);
  
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
  LCD_CtrlLinesWrite(LCD_NRST_GPIO_PORT, LCD_NRST_PIN, Bit_SET);
  LCD_CtrlLinesWrite(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
}


/**
  * @brief  Sets or reset LCD control lines.
  * @param  GPIOx: where x can be B or D to select the GPIO peripheral.
  * @param  CtrlPins: the Control line.
  *   This parameter can be:
  *     @arg LCD_NCS_PIN: Chip Select pin
  *     @arg LCD_NWR_PIN: Read/Write Selection pin
  *     @arg LCD_RS_PIN: Register/RAM Selection pin
  * @param  BitVal: specifies the value to be written to the selected bit.
  *   This parameter can be:
  *     @arg Bit_RESET: to clear the port pin
  *     @arg Bit_SET: to set the port pin
  * @retval None
  */

void LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal)
{
  /* Set or Reset the control line */
  GPIO_WriteBit(GPIOx, CtrlPins, BitVal);
}

/**
  * @brief  Configures the LCD_SPI interface.
  * @param  None
  * @retval None
  */

void LCD_SPIConfig(void)
{
	/* Only the Pins for ChipSelect (CS), RegisterSelect (RS) and Reset (RST)
	 * are configured, the rest is done in the general SPI configuration.
	 */
	SPI_InitTypeDef		SPI_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	/* Enable GPIO clock */
	//RCC_APB2PeriphClockCmd(LCD_SPI_SCK_GPIO_CLK | LCD_SPI_MISO_GPIO_CLK | LCD_SPI_MOSI_GPIO_CLK, ENABLE);

  /* Enable SPI clock  */


  RCC_APB2PeriphClockCmd(LCD_SPI_DATA_PERIPH | LCD_SPI_AFIO_PERIPH | LCD_SPI_CLK, ENABLE);

  /* Configure SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(LCD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MOSI_PIN;
  GPIO_Init(LCD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
  
  /* Configure MISO IN Floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MISO_PIN;
  GPIO_Init(LCD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);	
  
  SPI_I2S_DeInit(LCD_SPI);
  
  /* SPI Bus Config */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
 

  SPI_Init(LCD_SPI, &SPI_InitStructure);

  /* SPI enable */
  SPI_Cmd(LCD_SPI, ENABLE);
}


/**
  * @brief  Displays a pixel.
  * @param  x: pixel x.
  * @param  y: pixel y.  
  * @retval None
  */
#ifndef USE_Delay
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void delay(vu32 nCount)
{
  vu32 index = 0; 
  for(index = (100000 * nCount); index != 0; index--)
  {
  }
}
#endif /* USE_Delay*/
/**
  * @}
  */ 



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
