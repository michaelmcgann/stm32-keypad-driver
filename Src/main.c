
#include "stm32f407xx.h"
#include "keypad.h"
#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

// Column pins: PE8 - PE11 (input) (pull up)
// Row pins: PE12 - PE15 (output)

//#define REG32(addr)    ( *( volatile uint32_t *) ( addr ) )
//
//// Base addresses
//#define PERIPH_BASE    ( 0x40000000UL )
//#define AHB1_BASE      ( PERIPH_BASE + 0x00020000UL )
//#define RCC_BASE       ( PERIPH_BASE + 0x00023800UL )
//
//#define GPIOE_BASE     ( AHB1_BASE + 0x1000UL )
//
//// RCC Registers
//#define RCC_AHB1ENR    REG32( RCC_BASE + 0x30UL )
//
//// GPIOE Registers
//#define GPIOE_MODER    REG32( GPIOE_BASE + 0x00UL )
//#define GPIOE_OTYPER   REG32( GPIOE_BASE + 0x04UL )
//#define GPIOE_OSPEEDR  REG32( GPIOE_BASE + 0x08UL )
//#define GPIOE_PUPDR    REG32( GPIOE_BASE + 0x0CUL )
//#define GPIOE_IDR      REG32( GPIOE_BASE + 0x10UL )
//#define GPIOE_ODR      REG32( GPIOE_BASE + 0x14UL )
//#define GPIOE_BSRR     REG32( GPIOE_BASE + 0x18UL )

#define CPU_HZ 16000000u

//// Bit helpers
//#define BIT(n)         ( 1UL << (n) )

//// Keypad mapping
//#define COL0 8
//#define COL1 9
//#define COL2 10
//#define COL3 11
//
//#define ROW0 12
//#define ROW1 13
//#define ROW2 14
//#define ROW3 15

//#define COL_MASK     ( BIT(COL0) | BIT(COL1) | BIT(COL2) | BIT(COL3) )  // 0000111100000000
//#define ROW_MASK     ( BIT(ROW0) | BIT(ROW1) | BIT(ROW2) | BIT(ROW3) )  // 1111000000000000

// GPIO speed values per pin (2 bits each): 00=low, 01=medium, 10=fast, 11=high
//#define ROW_SPEED    ( 0x2UL )
//
//#define DEBOUNCE_BITS  5u
//#define DEBOUNCE_MASK  ( ( 1u << DEBOUNCE_BITS ) - 1u )

//void keypad_init            ( void );
//static void q_push          ( char k );
//char keypad_getchar         ( void );
//void keypad_task_1ms        ( void );
static void systick_init_1ms( uint32_t cpuhz );
void SysTick_Handler( void );

//static uint8_t  s_db[4]      = {0};
//static uint8_t  s_hist[4][4] = {{0}};
//static uint32_t s_active_row = 0;

//#define QSIZE 8u
//static char s_q[QSIZE];
//static uint8_t s_q_w = 0, s_q_r = 0;

extern uint32_t SystemCoreClock;

//static const char keymap[4][4] = {
//		{'1','2','3','A'},
//		{'4','5','6','B'},
//		{'7','8','9','C'},
//		{'*','0','#','D'}
//};

int main(void) {

	keypad_init();
	systick_init_1ms( CPU_HZ );

	while ( 1 ) {

		char k = keypad_getchar();

		if ( k ) {
			// Do something with k here..
			__NOP();
		}
	}
} // END MAIN

void systick_init_1ms( uint32_t cpu_hz ) {
	SysTick->LOAD = ( cpu_hz / 1000u ) - 1u;
	SysTick->VAL = 0u;
	SysTick-> CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler( void ) {
	keypad_task_1ms();
}
