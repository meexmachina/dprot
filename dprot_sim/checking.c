#include "checking.h"

/*************************************************************/
uint8_t crc8_table[256]; // 8-bit crc table

/*************************************************************/
void init_crc8()
{
	int i;
	int j;
	unsigned char crc;

	for (i=0; i<256; i++) {
		crc = i;
		for (j=0; j<8; j++) {
			crc = (crc << 1) ^ ((crc & 0x80) ? 0x07 : 0);
		}
		crc8_table[i] = crc & 0xFF;
	}
}

