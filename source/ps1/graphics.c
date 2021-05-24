#include "ps1/graphics.h"
#include "shared/chip8_cpu.h"

#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdio.h>
#include <libapi.h>
#include <stdint.h>


#define ORDERING_TABLE_LENGTH (10)
#define PACKETMAX (300)

#define CHIP8_OUTPUT_SCALE 5 //5x = 320x160

GsOT gsot_header[2];
GsOT_TAG gsot_tag[2][1 << ORDERING_TABLE_LENGTH];

PACKET packet_area[2][PACKETMAX];

RECT c8_screen_rect;
GsIMAGE gs_c8_screen_image;
GsSPRITE gs_c8_screen_sprite;

volatile uint8_t vsync_counter;
uint8_t vsyncs_per_second;


volatile uint8_t frame_count;
volatile uint8_t frame_rate;

ushort ps1_screen_width;
ushort ps1_screen_height;

void _VSyncCallback()
{
	vsync_counter++;
	if (vsync_counter >= vsyncs_per_second)
	{
		frame_rate = frame_count;
		frame_count = 0;
		vsync_counter = 0;
	}
}

void DisplayAll()
{
    int active_buffer = GsGetActiveBuff();

	FntFlush(-1);
	GsSetWorkBase((PACKET *)packet_area[active_buffer]);
	GsClearOt(0, 0, &gsot_header[active_buffer]);
	GsSortSprite(&gs_c8_screen_sprite, &gsot_header[active_buffer], 0);
	DrawSync(0);

    frame_count++;
	VSync(0);
	GsSwapDispBuff();
	GsSortClear(0, 0, 75, &gsot_header[active_buffer]);

	GsDrawOt(&gsot_header[active_buffer]);
}

void InitGraphics()
{
    ps1_screen_width = 320;
    if (*(char *)0xbfc7ff52 == 'E') // SCEE
    {
        SetVideoMode(MODE_PAL);
        ps1_screen_height = 256;
        vsyncs_per_second = 50;
    }
    else
    {
        SetVideoMode(MODE_NTSC);
        ps1_screen_height = 240;
        vsyncs_per_second = 60;
    }

    GsInitGraph(ps1_screen_width, ps1_screen_height, GsNONINTER | GsOFSGPU, 1, 0);
    GsDefDispBuff(0, 0, 0, ps1_screen_height);

    gsot_header[0].length = ORDERING_TABLE_LENGTH;
    gsot_header[1].length = ORDERING_TABLE_LENGTH;
    gsot_header[0].org = gsot_tag[0];
    gsot_header[1].org = gsot_tag[1];
    GsClearOt(0, 0, &gsot_header[0]);
    GsClearOt(0, 0, &gsot_header[1]);

	FntLoad(960, 256);
	FntOpen(0, 8, 256, 200, 0, 512);

	InitScreenTexture(screen_tim);
	InitSprite(&gs_c8_screen_image, &gs_c8_screen_sprite);

	gs_c8_screen_sprite.x = (gs_c8_screen_sprite.w / 2) * CHIP8_OUTPUT_SCALE;
	gs_c8_screen_sprite.x += ((ps1_screen_width - (CHIP8_SCREEN_WIDTH * CHIP8_OUTPUT_SCALE)) / 2);
	gs_c8_screen_sprite.y = ((gs_c8_screen_sprite.h / 2) * CHIP8_OUTPUT_SCALE);
	gs_c8_screen_sprite.y += ((ps1_screen_height - (CHIP8_SCREEN_HEIGHT * CHIP8_OUTPUT_SCALE)) / 2);

	gs_c8_screen_sprite.scalex = 4096 * CHIP8_OUTPUT_SCALE;
	gs_c8_screen_sprite.scaley = 4096 * CHIP8_OUTPUT_SCALE;

	vsync_counter = 0;
	frame_count = 0;
	frame_rate = 0;
    VSyncCallback(_VSyncCallback);
}

void InitScreenTexture()
{
    GsGetTimInfo((unsigned long *)screen_tim + 1, &gs_c8_screen_image);
    if ((gs_c8_screen_image.pmode & (1 << 3)))
    {
        setRECT(&c8_screen_rect, gs_c8_screen_image.cx, gs_c8_screen_image.cy, gs_c8_screen_image.cw, gs_c8_screen_image.ch);
        LoadImage(&c8_screen_rect, gs_c8_screen_image.clut);
    }
    c8_screen_rect.x = gs_c8_screen_image.px;
    c8_screen_rect.y = gs_c8_screen_image.py;
    c8_screen_rect.w = gs_c8_screen_image.pw;
    c8_screen_rect.h = gs_c8_screen_image.ph;
    LoadImage(&c8_screen_rect, gs_c8_screen_image.pixel);
    DrawSync(0);
}

void InitSprite(GsIMAGE *im, GsSPRITE *sp)
{
    int bits;
    int widthCompression;
    RECT myRect;

    bits = im->pmode & 0x03;
    if (bits == 0)
        widthCompression = 4;
    else if (bits == 1)
        widthCompression = 2;
    else if (bits == 2)
        widthCompression = 1;
    else if (bits == 3)
    {
        //printf("\nunsupported file format (24bit tim)!\n");
    }

    myRect.x = im->px;
    myRect.y = im->py;
    myRect.w = im->pw;
    myRect.h = im->ph;
    LoadImage(&myRect, im->pixel); //loads image data to frame buffer

    //printf("\nimage bit type =%d\n", bits);

    sp->attribute = (bits << 24);
    sp->w = im->pw * widthCompression;
    sp->h = im->ph;
    sp->tpage = GetTPage(bits, 0, im->px, im->py);
    sp->u = 0;
    sp->v = 0;
    if (bits == 0 || bits == 1)
    {
        //checks if image is 4 or 8 bit
        myRect.x = im->cx;
        myRect.y = im->cy;
        myRect.w = im->cw;
        myRect.h = im->ch;
        LoadImage(&myRect, im->clut); //loads clut to frame buffer if needed
        sp->cx = im->cx;
        sp->cy = im->cy;
    }
    sp->r = 128;
    sp->g = 128;
    sp->b = 128;
    sp->mx = (im->pw * widthCompression) / 2;
    sp->my = im->ph / 2;
    sp->scalex = 4096;
    sp->scaley = 4096;
    sp->rotate = 0 * 4096;
}

void UpdateScreenTexture()
{
    for (int i = 0; i < (CHIP8_SCREEN_HEIGHT * CHIP8_SCREEN_WIDTH) / 2; i++)
    {
        ((uint8_t *)gs_c8_screen_image.pixel)[i] = ((c8_cpu.screen[(i * 2) + 1] << 4) | c8_cpu.screen[(i * 2)]);
    }
    LoadImage(&c8_screen_rect, gs_c8_screen_image.pixel);
}
