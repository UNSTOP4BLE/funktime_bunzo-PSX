/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "deadsc.h"
#include "gfx.h"

int anim;
int animspeed;

DeadSc_Tex deadsc_tex;

typedef struct 
{
	int x;
	int y; 

} DedFrame;

const DedFrame frame0 = {  0,   0};
const DedFrame frame1 = {128,   0};
const DedFrame frame2 = {  0, 128};
const DedFrame frame3 = {128, 128};

DedFrame realframe;

void DeadSc_Init()
{
	anim = 0;
	animspeed = 0;
	realframe = frame0;
}

void DeadSc_Play(int animcooldown)
{
	animspeed ++;
	if (animspeed >= animcooldown)
	{
		anim ++;
		animspeed = 0;
	}

	switch (anim)
	{
		case 0:
			realframe = frame0;
			break;
		case 1:
			realframe = frame1;
			break;
		case 2:
			realframe = frame2;
			break;
		case 3:
			realframe = frame3;
			break;
		case 4:
			realframe = frame0;
			break;
		case 5:
			realframe = frame1;
			break;
		case 6:
			realframe = frame2;
			break;
		default: 
			realframe = frame3;
			break;
	}
	RECT src = {realframe.x, realframe.y, 127, 127};
	RECT dst = {(SCREEN_WIDTH - 320) >> 1, (SCREEN_HEIGHT - 240) >> 1, 320, 240};
	if (anim > 3)
		Gfx_DrawTex(&deadsc_tex.sc1, &src, &dst);
	else 
		Gfx_DrawTex(&deadsc_tex.sc0, &src, &dst);
}