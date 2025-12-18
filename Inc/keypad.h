
#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <stdint.h>

void keypad_init( void );

void keypad_task_1ms( void );

char keypad_getchar( void );

#endif /* KEYPAD_H_ */
