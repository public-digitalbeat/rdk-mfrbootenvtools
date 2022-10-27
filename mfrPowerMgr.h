#ifndef __MFRPOWERMGR_H__
#define __MFRPOWERMGR_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

bool pmInit(void);
bool pmReadPowerState(uint32_t *pmState);
bool pmSavePowerState(uint32_t pmState);

#endif /* __MFRPOWERMGR_H__ */
