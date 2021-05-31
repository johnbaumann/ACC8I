#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "shared/chip8_cpu.h"

void InitFilesystem(void);
void LoadFile(char *path, struct chip8_cpu *c8_cpu);

#endif // _FILESYSTEM_H_
