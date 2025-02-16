#include "main.h"
#include "Wrappers_Spi.h"
#include "Wrappers_Gpio.h"
#include "Libs_ILI9341_Driver.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
volatile uint16_t l_ScreenHeight_u16 = ILI9341_SCREEN_HEIGHT;
volatile uint16_t l_ScreenWidth_u16	 = ILI9341_SCREEN_WIDTH;

// /************************************************************************************************************
//  * GLOBAL VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
inline static void Libs_ILI9341_Enable(void);
inline static void Libs_ILI9341_SPI_Init(void);
inline static void Libs_ILI9341_WriteCommand(uint8_t Command);
inline static void Libs_ILI9341_WriteData(uint8_t Data);
static void Libs_ILI9341_DrawColourBurst(uint16_t Colour, uint32_t Size);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/*Enable LCD display*/
inline static void Libs_ILI9341_Enable(void)
{
	Wrappers_Gpio_Write(LogicalChannel_2, 1);
}

/* Select slave ILI9341 */
inline static void Libs_ILI9341_SPI_Init(void)
{
	Wrappers_Gpio_Write(LogicalChannel_1, 0);	//CS OFF
}

/* Send command */
inline static void Libs_ILI9341_WriteCommand(uint8_t Command)
{
	Wrappers_Gpio_Write(LogicalChannel_1, 0);
	Wrappers_Gpio_Write(LogicalChannel_3, 0);
	Wrappers_Spi_Transmit(SPI_Screen, &Command, 1);
	Wrappers_Gpio_Write(LogicalChannel_1, 1);
}

/* Send Data */
inline static void Libs_ILI9341_WriteData(uint8_t Data)
{
	Wrappers_Gpio_Write(LogicalChannel_3, 1);	
	Wrappers_Gpio_Write(LogicalChannel_1, 0);
	Wrappers_Spi_Transmit(SPI_Screen, &Data, 1);	
	Wrappers_Gpio_Write(LogicalChannel_1, 1);
}

/*Sends block colour information to LCD*/
static void Libs_ILI9341_DrawColourBurst(uint16_t Colour, uint32_t Size)
{
	//SENDS COLOUR
	uint32_t Buffer_Size = 0;
	if((Size*2) < ILI9341_BURST_MAX_SIZE)
	{
		Buffer_Size = Size;
	}
	else
	{
		Buffer_Size = ILI9341_BURST_MAX_SIZE;
	}
		
	Wrappers_Gpio_Write(LogicalChannel_3, 1);	
	Wrappers_Gpio_Write(LogicalChannel_1, 0);

	unsigned char chifted = 	Colour>>8;;
	unsigned char burst_buffer[Buffer_Size];
	for(uint32_t j = 0; j < Buffer_Size; j+=2)
	{
		burst_buffer[j] = 	chifted;
		burst_buffer[j+1] = Colour;
	}

	uint32_t Sending_Size = Size*2;
	uint32_t Sending_in_Block = Sending_Size/Buffer_Size;
	uint32_t Remainder_from_block = Sending_Size%Buffer_Size;

	if(Sending_in_Block != 0)
	{
		for(uint32_t j = 0; j < (Sending_in_Block); j++)
		{
			Wrappers_Spi_Transmit(SPI_Screen, (unsigned char *)burst_buffer, Buffer_Size);	
		}
	}

	//REMAINDER!
	Wrappers_Spi_Transmit(SPI_Screen, (unsigned char *)burst_buffer, Remainder_from_block);	
		
	Wrappers_Gpio_Write(LogicalChannel_1, 1);
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
/*HARDWARE RESET*/
void Libs_ILI9341_Reset(void)
{
	Wrappers_Gpio_Write(LogicalChannel_2, 0);
	HAL_Delay(200);
	Wrappers_Gpio_Write(LogicalChannel_1, 0);
	HAL_Delay(200);
	Wrappers_Gpio_Write(LogicalChannel_2, 1);	
}

/* Set Address - Location block - to draw into */
void Libs_ILI9341_SetAddress(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
	Libs_ILI9341_WriteCommand(0x2A);
	Libs_ILI9341_WriteData(X1>>8);
	Libs_ILI9341_WriteData(X1);
	Libs_ILI9341_WriteData(X2>>8);
	Libs_ILI9341_WriteData(X2);

	Libs_ILI9341_WriteCommand(0x2B);
	Libs_ILI9341_WriteData(Y1>>8);
	Libs_ILI9341_WriteData(Y1);
	Libs_ILI9341_WriteData(Y2>>8);
	Libs_ILI9341_WriteData(Y2);

	Libs_ILI9341_WriteCommand(0x2C);
}

/*Ser rotation of the screen - changes x0 and y0*/
void Libs_ILI9341_SetRotation(uint8_t p_Rotation_u8) 
{
	Libs_ILI9341_WriteCommand(0x36);
	HAL_Delay(1);

	switch(p_Rotation_u8) 
	{
		case ILI9341_SCREEN_VERTICAL_1:
			Libs_ILI9341_WriteData(0x40|0x08);
			l_ScreenWidth_u16 = 240;
			l_ScreenHeight_u16 = 320;
			break;
		case ILI9341_SCREEN_HORIZONTAL_1:
			Libs_ILI9341_WriteData(0x20|0x08);
			l_ScreenWidth_u16  = 320;
			l_ScreenHeight_u16 = 240;
			break;
		case ILI9341_SCREEN_VERTICAL_2:
			Libs_ILI9341_WriteData(0x80|0x08);
			l_ScreenWidth_u16  = 240;
			l_ScreenHeight_u16 = 320;
			break;
		case ILI9341_SCREEN_HORIZONTAL_2:
			Libs_ILI9341_WriteData(0x40|0x80|0x20|0x08);
			l_ScreenWidth_u16  = 320;
			l_ScreenHeight_u16 = 240;
			break;
		default:
			//EXIT IF SCREEN ROTATION NOT VALID!
			break;
	}
}

/*Initialize LCD display*/
void Libs_ILI9341_Init(void)
{
	Libs_ILI9341_Enable();
	Libs_ILI9341_SPI_Init();
	Libs_ILI9341_Reset();

	//SOFTWARE RESET
	Libs_ILI9341_WriteCommand(0x01);
	HAL_Delay(1000);
		
	//POWER CONTROL A
	Libs_ILI9341_WriteCommand(0xCB);
	Libs_ILI9341_WriteData(0x39);
	Libs_ILI9341_WriteData(0x2C);
	Libs_ILI9341_WriteData(0x00);
	Libs_ILI9341_WriteData(0x34);
	Libs_ILI9341_WriteData(0x02);

	//POWER CONTROL B
	Libs_ILI9341_WriteCommand(0xCF);
	Libs_ILI9341_WriteData(0x00);
	Libs_ILI9341_WriteData(0xC1);
	Libs_ILI9341_WriteData(0x30);

	//DRIVER TIMING CONTROL A
	Libs_ILI9341_WriteCommand(0xE8);
	Libs_ILI9341_WriteData(0x85);
	Libs_ILI9341_WriteData(0x00);
	Libs_ILI9341_WriteData(0x78);

	//DRIVER TIMING CONTROL B
	Libs_ILI9341_WriteCommand(0xEA);
	Libs_ILI9341_WriteData(0x00);
	Libs_ILI9341_WriteData(0x00);

	//POWER ON SEQUENCE CONTROL
	Libs_ILI9341_WriteCommand(0xED);
	Libs_ILI9341_WriteData(0x64);
	Libs_ILI9341_WriteData(0x03);
	Libs_ILI9341_WriteData(0x12);
	Libs_ILI9341_WriteData(0x81);

	//PUMP RATIO CONTROL
	Libs_ILI9341_WriteCommand(0xF7);
	Libs_ILI9341_WriteData(0x20);

	//POWER CONTROL,VRH[5:0]
	Libs_ILI9341_WriteCommand(0xC0);
	Libs_ILI9341_WriteData(0x23);

	//POWER CONTROL,SAP[2:0];BT[3:0]
	Libs_ILI9341_WriteCommand(0xC1);
	Libs_ILI9341_WriteData(0x10);

	//VCM CONTROL
	Libs_ILI9341_WriteCommand(0xC5);
	Libs_ILI9341_WriteData(0x3E);
	Libs_ILI9341_WriteData(0x28);

	//VCM CONTROL 2
	Libs_ILI9341_WriteCommand(0xC7);
	Libs_ILI9341_WriteData(0x86);

	//MEMORY ACCESS CONTROL
	Libs_ILI9341_WriteCommand(0x36);
	Libs_ILI9341_WriteData(0x48);

	//PIXEL FORMAT
	Libs_ILI9341_WriteCommand(0x3A);
	Libs_ILI9341_WriteData(0x55);

	//FRAME RATIO CONTROL, STANDARD RGB COLOR
	Libs_ILI9341_WriteCommand(0xB1);
	Libs_ILI9341_WriteData(0x00);
	Libs_ILI9341_WriteData(0x18);

	//DISPLAY FUNCTION CONTROL
	Libs_ILI9341_WriteCommand(0xB6);
	Libs_ILI9341_WriteData(0x08);
	Libs_ILI9341_WriteData(0x82);
	Libs_ILI9341_WriteData(0x27);

	//3GAMMA FUNCTION DISABLE
	Libs_ILI9341_WriteCommand(0xF2);
	Libs_ILI9341_WriteData(0x00);

	//GAMMA CURVE SELECTED
	Libs_ILI9341_WriteCommand(0x26);
	Libs_ILI9341_WriteData(0x01);

	//POSITIVE GAMMA CORRECTION
	Libs_ILI9341_WriteCommand(0xE0);
	Libs_ILI9341_WriteData(0x0F);
	Libs_ILI9341_WriteData(0x31);
	Libs_ILI9341_WriteData(0x2B);
	Libs_ILI9341_WriteData(0x0C);
	Libs_ILI9341_WriteData(0x0E);
	Libs_ILI9341_WriteData(0x08);
	Libs_ILI9341_WriteData(0x4E);
	Libs_ILI9341_WriteData(0xF1);
	Libs_ILI9341_WriteData(0x37);
	Libs_ILI9341_WriteData(0x07);
	Libs_ILI9341_WriteData(0x10);
	Libs_ILI9341_WriteData(0x03);
	Libs_ILI9341_WriteData(0x0E);
	Libs_ILI9341_WriteData(0x09);
	Libs_ILI9341_WriteData(0x00);

	//NEGATIVE GAMMA CORRECTION
	Libs_ILI9341_WriteCommand(0xE1);
	Libs_ILI9341_WriteData(0x00);
	Libs_ILI9341_WriteData(0x0E);
	Libs_ILI9341_WriteData(0x14);
	Libs_ILI9341_WriteData(0x03);
	Libs_ILI9341_WriteData(0x11);
	Libs_ILI9341_WriteData(0x07);
	Libs_ILI9341_WriteData(0x31);
	Libs_ILI9341_WriteData(0xC1);
	Libs_ILI9341_WriteData(0x48);
	Libs_ILI9341_WriteData(0x08);
	Libs_ILI9341_WriteData(0x0F);
	Libs_ILI9341_WriteData(0x0C);
	Libs_ILI9341_WriteData(0x31);
	Libs_ILI9341_WriteData(0x36);
	Libs_ILI9341_WriteData(0x0F);

	//EXIT SLEEP
	Libs_ILI9341_WriteCommand(0x11);
	HAL_Delay(120);

	//TURN ON DISPLAY
	Libs_ILI9341_WriteCommand(0x29);

	//STARTING ROTATION
	Libs_ILI9341_SetRotation(ILI9341_SCREEN_VERTICAL_1);
}

//FILL THE ENTIRE SCREEN WITH SELECTED COLOUR (either #define-d ones or custom 16bit)
/*Sets address (entire screen) and Sends Height*Width ammount of colour information to LCD*/
void Libs_ILI9341_FillScreen(uint16_t Colour)
{
	Libs_ILI9341_SetAddress(0,0,l_ScreenWidth_u16,l_ScreenHeight_u16);	
	Libs_ILI9341_DrawColourBurst(Colour, l_ScreenWidth_u16*l_ScreenHeight_u16);	
}

//DRAW PIXEL AT XY POSITION WITH SELECTED COLOUR
//
//Location is dependant on screen orientation. x0 and y0 locations change with orientations.
//Using pixels to draw big simple structures is not recommended as it is really slow
//Try using either rectangles or lines if possible
//
void Libs_ILI9341_DrawPixel(uint16_t X, uint16_t Y, uint16_t Colour)
{
    if ((X >= l_ScreenWidth_u16) || (Y >= l_ScreenHeight_u16)) return; // OUT OF BOUNDS!

    uint8_t Temp_Buffer[4];

    // Set column address (X)
    Libs_ILI9341_WriteCommand(0x2A);
    Temp_Buffer[0] = X >> 8;
    Temp_Buffer[1] = X;
    Temp_Buffer[2] = (X + 1) >> 8;
    Temp_Buffer[3] = (X + 1);
    Wrappers_Spi_Transmit(SPI_Screen, Temp_Buffer, 4);

    // Set row address (Y)
    Libs_ILI9341_WriteCommand(0x2B);
    Temp_Buffer[0] = Y >> 8;
    Temp_Buffer[1] = Y;
    Temp_Buffer[2] = (Y + 1) >> 8;
    Temp_Buffer[3] = (Y + 1);
    Wrappers_Spi_Transmit(SPI_Screen, Temp_Buffer, 4);

    // Write pixel color
    Libs_ILI9341_WriteCommand(0x2C);
    Temp_Buffer[0] = Colour >> 8;
    Temp_Buffer[1] = Colour;
    Wrappers_Spi_Transmit(SPI_Screen, Temp_Buffer, 2);
}


//DRAW RECTANGLE OF SET SIZE AND HEIGTH AT X and Y POSITION WITH CUSTOM COLOUR
//
//Rectangle is hollow. X and Y positions mark the upper left corner of rectangle
//As with all other draw calls x0 and y0 locations dependant on screen orientation
//

void Libs_ILI9341_DrawRectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour)
{
	if((X >=l_ScreenWidth_u16) || (Y >=l_ScreenHeight_u16)) return;
	if((X+Width-1)>=l_ScreenWidth_u16)
		{
			Width=l_ScreenWidth_u16-X;
		}
	if((Y+Height-1)>=l_ScreenHeight_u16)
		{
			Height=l_ScreenHeight_u16-Y;
		}
	Libs_ILI9341_SetAddress(X, Y, X+Width-1, Y+Height-1);
	Libs_ILI9341_DrawColourBurst(Colour, Height*Width);
}

//DRAW LINE FROM X,Y LOCATION to X+Width,Y LOCATION
void Libs_ILI9341_DrawHorizontalLine(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Colour)
{
	if((X >=l_ScreenWidth_u16) || (Y >=l_ScreenHeight_u16)) return;
	if( (X+Width-1) >= l_ScreenWidth_u16)
	{
		Width=l_ScreenWidth_u16-X;
	}
	Libs_ILI9341_SetAddress(X, Y, X+Width-1, Y);
	Libs_ILI9341_DrawColourBurst(Colour, Width);
}

//DRAW LINE FROM X,Y LOCATION to X,Y+Height LOCATION
void Libs_ILI9341_DrawVerticalLine(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Colour)
{
	if((X >=l_ScreenWidth_u16) || (Y >=l_ScreenHeight_u16)) return;
	if((Y+Height-1)>=l_ScreenHeight_u16)
		{
			Height=l_ScreenHeight_u16-Y;
		}
	Libs_ILI9341_SetAddress(X, Y, X, Y+Height-1);
	Libs_ILI9341_DrawColourBurst(Colour, Height);
}

