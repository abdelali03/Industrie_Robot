//*******************************************************************
/*!
\file   config.h
\author Thomas Breuer
\date   23.03.2023
\brief  Board specific configuration
*/

//*******************************************************************
/*
Board:    STM32L100-Discovery
*/

//*******************************************************************
#include "Hardware/Peripheral/Display/DisplayChar_DIP204spi.cpp"

//*******************************************************************
using namespace EmbSysLib::Hw;
using namespace EmbSysLib::Dev;
using namespace EmbSysLib::Ctrl;
using namespace EmbSysLib::Mod;

//*******************************************************************
PinConfig::MAP PinConfig::table[] =
{
  // SPI
  SPI2_MOSI_PB15,
  SPI2_MISO_PB14,
  SPI2_SCK_PB13,
  SPI2_NSS_PB12,

  // UART
  USART1_TX_PA9,
  USART1_RX_PA10,

  END_OF_TABLE
};

//-------------------------------------------------------------------
// Timer
//-------------------------------------------------------------------
Timer_Mcu   timer( Timer_Mcu::TIM_10, 10000L/*us*/ );
Timer_Mcu   timermotor( Timer_Mcu::TIM_9, 1000L/*us*/ );
TaskManager taskManager( timer );
TaskManager taskManagermotor( timermotor );

//-------------------------------------------------------------------
// Port + Digital
//-------------------------------------------------------------------
Port_Mcu   portA( Port_Mcu::PA );
Port_Mcu   portB( Port_Mcu::PB );
Port_Mcu   portC( Port_Mcu::PC );
Port_Mcu   portD( Port_Mcu::PD );

Digital    led0    ( portC, 8, Digital::Out,  0 ); // LD4 (blue)
Digital    btnA    ( portA, 0, Digital::In,   0 ); // B1 (user button)
Digital    btnB    (portC, 1, Digital::InPU, 0);
Digital    btnC    (portC, 6, Digital::InPU, 0);
Digital    btnD    (portC, 7, Digital::InPU, 0);
Digital    rotA    ( portA, 8, Digital::InPU, 1 );
Digital    rotB    ( portA, 1, Digital::InPU, 1 );
Digital    rotCtrl ( portA,15, Digital::InPU, 1 );

DigitalEncoderRotaryknob  enc( &rotA, &rotB, &rotCtrl, taskManager );

//-------------------------------------------------------------------
// UART
//-------------------------------------------------------------------
Uart_Mcu  uart    ( Uart_Mcu::USART_1, 9600, 100, 100 );
Terminal  terminal( uart, 255,255,"#", "!\r\n" );

//-------------------------------------------------------------------
// Display
//-------------------------------------------------------------------
SPImaster_Mcu         spi          ( SPImaster_Mcu::SPI_2, SPImaster_Mcu::CR_1000kHz, SPImaster_Mcu::CPOL_H_CPHA_H );
SPImaster::Device     spiDevDisplay( spi, portB, 12 );
DisplayChar_DIP204spi dispHw       ( spiDevDisplay );
ScreenChar            disp         ( dispHw );

//-------------------------------------------------------------------
// InduRobot
//-------------------------------------------------------------------
Digital  enable( portD, 2, Digital::Out, 1 );
//winkel
Digital  winkel_minus( portB, 4, Digital::Out,  0 );
Digital  winkel_plus ( portB, 5, Digital::Out,  0 );
Digital  endschalter_winkel( portA, 2, Digital::InPU, 0 );
Digital  increment_winkel  ( portA, 3, Digital::InPU, 0 );
//radius
Digital  radius_minus( portB, 0, Digital::Out,  0 );
Digital  radius_plus( portB, 1, Digital::Out,  0 );
Digital  endschalter_radius( portA, 6, Digital::InPU, 0 );
Digital  increment_radius  ( portA, 7, Digital::InPU, 0 );
//hoehe
Digital  hoehe_minus( portB, 6, Digital::Out,  0 );
Digital  hoehe_plus( portB, 7, Digital::Out,  0 );
Digital  endschalter_hoehe( portC, 2, Digital::InPU, 0 );
Digital  increment_hoehe( portC, 3, Digital::InPU, 0 );
//greifer
Digital  greifer_minus( portB, 8, Digital::Out,  0 );
Digital  greifer_plus( portB, 9, Digital::Out,  0 );
Digital  endschalter_greifer( portC, 4, Digital::InPU, 0 );
Digital  increment_greifer( portC, 5, Digital::InPU, 0 );
