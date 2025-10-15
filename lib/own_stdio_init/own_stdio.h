#ifndef OWN_STDIO_INIT_OWN_STDIO_H
#define OWN_STDIO_INIT_OWN_STDIO_H

#include <stdint.h>
#include <stddef.h>

void own_stdio_init(uint32_t baud_rate);
void own_stdio_reset_display();
void own_stdio_set_input_limit(size_t limit);

#endif
