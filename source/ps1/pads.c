#include "ps1/pads.h"
#include "shared/chip8_cpu.h"

#include <sys/types.h>
#include <libetc.h>
#include <stdint.h>

u_long pad_mask;
u_long old_pad_mask;

extern uint8_t cpu_rate;
extern bool program_paused;
extern void LoadROM();

void HandleSystemPadEvents()
{
    // Restart
    if (pad_mask & Pad1Select && !(old_pad_mask & Pad1Select))
    {
        Chip8_Initialize(&c8_cpu);
        LoadROM();
    }
    // Restart

    // Pause
    if (pad_mask & Pad1Start && !(old_pad_mask & Pad1Start))
    {
        program_paused = !program_paused;
    }
    // Pause

    // Decrese CPU Speed
    if (pad_mask & Pad1L1)
    {
        if(cpu_rate > 1U)
        {
            cpu_rate--;
        }
    }
    // Decrese CPU Speed

    // Increase CPU Speed
    if (pad_mask & Pad1R1)
    {
        if(cpu_rate < 100U)
        {
            cpu_rate++;
        }
    }
    // Increase CPU Speed
}

void HandleCHIP8KeyboardEvents()
{
    // Left
    if (pad_mask & Pad1Left)
    {
        Chip8_KeyPressed(&c8_cpu, 0x05);
    }
    else if (old_pad_mask & Pad1Left)
    {
        Chip8_KeyReleased(&c8_cpu, 0x05);
    }
    // Left

    // Down
    if (pad_mask & Pad1Down)
    {
        Chip8_KeyPressed(&c8_cpu, 0x07);
    }
    else if (old_pad_mask & Pad1Down)
    {
        Chip8_KeyReleased(&c8_cpu, 0x07);
    }
    // Down

    // Right
    if (pad_mask & Pad1Right)
    {
        Chip8_KeyPressed(&c8_cpu, 0x06);
    }
    else if (old_pad_mask & Pad1Right)
    {
        Chip8_KeyReleased(&c8_cpu, 0x06);
    }
    // Right

    // Rotate
    if (pad_mask & Pad1x)
    {
        Chip8_KeyPressed(&c8_cpu, 0x04);
    }
    else if (old_pad_mask & Pad1x)
    {
        Chip8_KeyReleased(&c8_cpu, 0x04);
    }
    // Rotate
}

void InitPads()
{
    PadInit(0);
    pad_mask = old_pad_mask = PadRead(0);
}

void UpdatePads()
{
    old_pad_mask = pad_mask;
    pad_mask = PadRead(0);
}