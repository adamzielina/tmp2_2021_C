#ifndef KLAW_H
#define KLAW_H

#include "frdm_bsp.h"

#define PTA5_8_MASK	0x000001E0
#define PTA9_12_MASK	0x00001E00
#define KROK1	0xFFFFFFDF
#define KROK2	0xFFFFFFBF
#define KROK3	0xFFFFFF7F
#define KROK4	0xFFFFFEFF


void Klaw_Init(void);
void Klaw_S2_Int(void);

#endif  /* KLAW_H */
