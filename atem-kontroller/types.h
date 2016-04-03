#pragma once

#include <cinttypes>

typedef union
{
	uint32_t u32;
	uint8_t u8[4];
} u32_u8;

typedef union
{
	uint16_t u16;
	uint8_t u8[2];
} u16_u8;
