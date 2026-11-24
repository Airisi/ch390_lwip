#ifndef F072_NETWORK_APP_H
#define F072_NETWORK_APP_H
#include <stdint.h>
extern volatile uint8_t network_hw_ready, network_last_error;
extern volatile uint32_t network_hw_failures;
void network_app_init(void);
void network_app_poll(void);
#endif
