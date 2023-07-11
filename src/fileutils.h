#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stdio.h>
#include <stdint.h>

uint8_t read_u8(FILE *fp);
uint16_t read_u16(FILE *fp);
uint32_t read_u32(FILE *fp);
uint64_t read_u64(FILE *fp);

int8_t read_i8(FILE *fp);
int16_t read_i16(FILE *fp);
int32_t read_i32(FILE *fp);
int64_t read_i64(FILE *fp);

#endif
