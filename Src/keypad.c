
#include "stm32f407xx.h"
#include "keypad.h"

#define REG32(addr)    ( *( volatile uint32_t *) ( addr ) )

// Base addresses
#define PERIPH_BASE    ( 0x40000000UL )
#define AHB1_BASE      ( PERIPH_BASE + 0x00020000UL )
#define RCC_BASE       ( PERIPH_BASE + 0x00023800UL )

#define GPIOE_BASE     ( AHB1_BASE + 0x1000UL )

// RCC Registers
#define RCC_AHB1ENR    REG32( RCC_BASE + 0x30UL )

// GPIOE Registers
#define GPIOE_MODER    REG32( GPIOE_BASE + 0x00UL )
#define GPIOE_OTYPER   REG32( GPIOE_BASE + 0x04UL )
#define GPIOE_OSPEEDR  REG32( GPIOE_BASE + 0x08UL )
#define GPIOE_PUPDR    REG32( GPIOE_BASE + 0x0CUL )
#define GPIOE_IDR      REG32( GPIOE_BASE + 0x10UL )
#define GPIOE_ODR      REG32( GPIOE_BASE + 0x14UL )
#define GPIOE_BSRR     REG32( GPIOE_BASE + 0x18UL )

// Bit helpers
#define BIT(n)         ( 1UL << (n) )

// Keypad mapping
#define COL0 8
#define ROW0 12

#define COL_MASK     ( BIT(8) | BIT(9) | BIT(10) | BIT(11) )  // 0000111100000000
#define ROW_MASK     ( BIT(12) | BIT(13) | BIT(14) | BIT(15) )  // 1111000000000000

// GPIO speed values per pin (2 bits each): 00=low, 01=medium, 10=fast, 11=high
#define ROW_SPEED    ( 0x2UL )

#define DEBOUNCE_BITS  5u
#define DEBOUNCE_MASK  ( ( 1u << DEBOUNCE_BITS ) - 1u )

static const char keymap[4][4] = {
		{'1','2','3','A'},
		{'4','5','6','B'},
		{'7','8','9','C'},
		{'*','0','#','D'}
};

static uint8_t  s_db[4]      = {0};
static uint8_t  s_hist[4][4] = {{0}};
static uint32_t s_active_row = 0;

#define QSIZE 8u
static char s_q[QSIZE];
static uint8_t s_q_w = 0, s_q_r = 0;

static void q_push( char k ) {

	uint8_t next = ( uint8_t )(( s_q_w + 1u ) % QSIZE );
	if ( next != s_q_r ) {
		s_q[s_q_w] = k;
		s_q_w = next;
	}
}

char keypad_getchar( void ) {

	if ( s_q_r == s_q_w ) return 0;
	char k = s_q[s_q_r];
	s_q_r = ( uint8_t )( ( s_q_r + 1u ) % QSIZE );
	return k;
}

void keypad_init( void ) {

	// Enable clock for GPIOE
	RCC_AHB1ENR |= BIT(4);
	(void)RCC_AHB1ENR;

	// Clear 2 bit MODE bits for pins 8-15 (this also sets 8-11 as input)
	uint32_t moder = GPIOE_MODER;
	for ( uint32_t p = 8; p <= 15; p++ ) {
		moder &= ~( 3UL << ( p * 2 ) );
	}

	// Set pins 12-15 to output
	for ( uint32_t p = 12; p <= 15; p++ ) {
		moder |= ( 1UL << ( p * 2 ) );
	}
	GPIOE_MODER = moder;

    //    PE8-PE11 pull-up (01)
    //    PE12-PE15 no pull (00)
	uint32_t pupdr = GPIOE_PUPDR;
	for( uint32_t p = 8; p <= 11; p++ ) {
		pupdr &= ~( 3UL << ( p * 2 ) );
		pupdr |= ( 1UL << ( p * 2 ) );
	}

	for( uint32_t p = 12; p <= 15; p++ ) {
		pupdr &= ~( 3UL << ( p * 2 ) );
	}
	GPIOE_PUPDR = pupdr;

	// Output type: push-pull (0) for PE12-PE15
	GPIOE_OTYPER &= ~( ROW_MASK );

	// Output speed for PE12-PE15
	uint32_t ospeedr = GPIOE_OSPEEDR;
	for( uint32_t p = 12; p <= 15; p++ ) {
		ospeedr &= ~( 3UL << ( p * 2 ) );
		ospeedr |= ( ROW_SPEED << ( p * 2 ) );
	}
	GPIOE_OSPEEDR = ospeedr;

	// set rows to high default state
	GPIOE_BSRR = ROW_MASK;

}

void keypad_task_1ms( void ) {

	// Drive all rows HIGH
	GPIOE_BSRR = ROW_MASK;

	// Drive one row low
	uint32_t row_pin = ROW0 + s_active_row;  // 12 - 15
	GPIOE_BSRR = BIT( row_pin + 16 );

	// Read columns, 1 = no press, 0 = pressed, then invert
	uint32_t cols = ( GPIOE_IDR & COL_MASK ) >> COL0;
	uint8_t pressed = ( uint8_t ) ( ~cols ) & 0x0Fu;

	// Debounce each key in row with shift-register history
	for ( uint32_t c = 0; c < 4; c++ ) {
		uint8_t bit = ( pressed >> c ) & 1u;

		uint8_t history = s_hist[s_active_row][c];
		history = ( uint8_t ) ( ( ( history << 1 ) | bit ) & DEBOUNCE_MASK );
		s_hist[s_active_row][c] = history;

		// check if stable pressed
		if ( history == DEBOUNCE_MASK ) {
			if ( ( s_db[s_active_row] & ( 1u << c ) ) == 0 ) {
				s_db[s_active_row] |= ( 1u << c );
				q_push( keymap[s_active_row][c] );
			} // END IF NEW PRESS
		} // END IF DEBOUNCE THRESHOLD REACHED

		else if ( history == 0u) {
			s_db[s_active_row] &= ( uint8_t ) ~( 1u << c );
		}

	} // END FOR LOOP OVER COLUMNS

	s_active_row = ( s_active_row + 1u ) & 3u;

}
