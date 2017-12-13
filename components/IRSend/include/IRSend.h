#ifndef IRSend_H_
#define IRSend_H_

#include <stdint.h>

void IRSendInit(uint8_t pin, uint8_t port);

void IRSendIR(uint8_t data);

#endif
