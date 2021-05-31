#include "ps1/filesystem.h"

//#include <libsn.h>
#include "kernel/pcdrv.h" // nugget ftw!

#include <stdint.h>
#include <stdio.h>


#include "shared/chip8_cpu.h"

#define USEPCDRV 1

bool pcdrv_active;

static int InitCDROM();
static int InitPCDRV();
static void LoadFileCDROM(char *path, struct chip8_cpu *cpu);
static void LoadFilePCDRV(char *path, struct chip8_cpu *cpu);

void InitFilesystem(void)
{
    int pcinit_result;
    if (USEPCDRV)
    {
        pcinit_result = InitPCDRV();
        pcinit_result = 0; // Workaround bad return value for now
        if(pcinit_result < 0)
        {
            printf("Failed to load PCDRV, falling back to CDROM. pcinit_result = %i\n", pcinit_result);
            pcdrv_active = false;
            InitCDROM();
        }
        else
        {
            printf("PCDRV initialized\n");
            pcdrv_active = true;
        }

    }
    else
    {
        InitCDROM();
    }
}

static int InitCDROM()
{

}

static int InitPCDRV()
{
    return PCinit();
}

void LoadFile(char *path, struct chip8_cpu *c8_cpu)
{
    if (pcdrv_active)
    {
        LoadFilePCDRV(path, c8_cpu);
    }
    else
    {
        LoadFileCDROM(path, c8_cpu);
    }
}

static void LoadFileCDROM(char *path, struct chip8_cpu *c8_cpu)
{

}

static void LoadFilePCDRV(char *path, struct chip8_cpu *c8_cpu)
{
    int file_handle;
    int file_length;
    char file_contents[0xDFF];

	printf("Load_ROM_PCDRV entry point\n");
	if (InitPCDRV() == 0)
	{
		printf("Finished PCinit(), proceed with loading file\n");
		file_handle = PCopen(path, 0, 0);

		if (file_handle < 0)
		{
			printf("Failed to open file\n");
		}
		else
		{
			printf("Handle = %i\n", file_handle);
			file_length = PClseek(file_handle, 0, 2);
            printf("file_length = %i\n", file_length);
			PClseek(file_handle, 0, 0);
			if (file_length > 0xDFF) // 0xFFF - 0x200 = C8 Program Space
			{
				printf("Input file too large\n");
			}
			else if (file_length > 0)
			{
                if(PCread(file_handle, file_contents, file_length) == file_length)
                {
                    // Succesful Read
                    // Copy file contents to cpu memory
                    for (int i = 0; i < file_length; i++)
                    {
                        c8_cpu->memory[0x200 + i] = file_contents[i];
                    }
                }
                else
                {
                    printf("Failure, file length mismatch on read.\n");
                }
			}
			else
			{
				printf("Failed to retreive contents of file(file_length = 0)\n");
			}
			PCclose(file_handle);
		}
	}
}