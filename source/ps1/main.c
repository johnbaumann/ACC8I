#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdio.h>
#include <libapi.h>
#include <stdint.h>

#include "pad.h"
#include "shared/chip8_cpu.h"

#define OT_LENGTH (10)


#define PACKETMAX (2048)
#define PACKETMAX2 (PACKETMAX*24)


#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	240

GsOT myOT[2];
GsOT_TAG myOT_TAG[2][1<<OT_LENGTH];
GsIMAGE 	tim;
PACKET GPUPacketArea[2][PACKETMAX2];

u_long pad;

extern uint8_t chip8_rom[];


void HandlePad(void);
void InitGraphics(void);
void DisplayAll(int);
int main(void);


int main(void) {
	int activeBuffer=0;

	InitGraphics();
	PadInit(0);

	FntLoad(960, 256);
	FntOpen(32, 32, 256, 200, 0, 512);

	while (1) {

		activeBuffer = GsGetActiveBuff();
		GsSetWorkBase((PACKET*)GPUPacketArea[activeBuffer]);
		GsClearOt(0, 0, &myOT[activeBuffer]);

		FntPrint("Hello world\n");
		FntPrint("Chip8-ROM: %x", chip8_rom);

		DisplayAll(activeBuffer);
	}
	return 0;
}

void InitGraphics(void) {

	if (*(char *)0xbfc7ff52=='E')
		SetVideoMode(1);		//PAL MODE
	else
		SetVideoMode(0);		// NTSC MODE

if (SCREEN_HEIGHT<=256)	{ GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsNONINTER|GsOFSGPU, 1, 0); } else { GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, 2, 0, 0); };


if (SCREEN_HEIGHT<=256)	{ GsDefDispBuff(0, 0 , 0, SCREEN_HEIGHT); };
if (SCREEN_HEIGHT>=480)	{ GsDefDispBuff(0, 0, 0, 0); };


	myOT[0].length = OT_LENGTH;
	myOT[1].length = OT_LENGTH;
	myOT[0].org = myOT_TAG[0];
	myOT[1].org = myOT_TAG[1];
	GsClearOt(0,0,&myOT[0]);
	GsClearOt(0,0,&myOT[1]);
}


void DisplayAll(int activeBuffer) {
	FntFlush(-1);
	DrawSync(0);

	VSync(0);
	GsSwapDispBuff();
	GsSortClear(0,0,0,&myOT[activeBuffer]);
	GsDrawOt(&myOT[activeBuffer]);
}

void HandlePad(void) 
{

       
        pad=PadRead(0);


//        if (pad&Pad1L1)         FntPrint("L1\n");
//        if (pad&Pad1L2)         FntPrint("L2\n");
//        if (pad&Pad1R1)         FntPrint("R1\n");
//        if (pad&Pad1R2)         FntPrint("R2\n");


}

void initTexture(u_long *addr)
{
   RECT rect;
   addr++;
   GsGetTimInfo(addr, &tim);
   rect.x=tim.px;
   rect.y=tim.py;
   rect.w=tim.pw;
   rect.h=tim.ph;
   LoadImage(&rect,tim.pixel);  
   if(!(tim.pmode & (1<<3))) return;
   setRECT(&rect, tim.cx, tim.cy, tim.cw, tim.ch);
   LoadImage(&rect, tim.clut);
}