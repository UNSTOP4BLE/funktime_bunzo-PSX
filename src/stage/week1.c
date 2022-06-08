/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week1.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"


#include "../pad.h"

int shit, ass;
//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Stage and back
	Gfx_Tex tex_back1; //Curtains
} Back_Week1;

//Week 1 background functions
void Back_Week1_DrawBG(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	fixed_t fx, fy;
	
	fixed_t beat_bop;
	if ((stage.song_step & 0x3) == 0)
		beat_bop = FIXED_UNIT - ((stage.note_scroll / 24) & FIXED_LAND);
	else
		beat_bop = 0;

	if (pad_state.held & PAD_UP)
		ass ++;
	if (pad_state.held & PAD_DOWN)
		ass --;
	if (pad_state.held & PAD_LEFT)
		shit --;
	if (pad_state.held & PAD_RIGHT)
		shit ++;

	//Draw boppers
	static const struct Back_Week1_LowerBop
	{
		RECT src;
		RECT_FIXED dst;
	} lbop_piece[] = {
		{{0, 78, 256, 154}, {FIXED_DEC(-422,1), FIXED_DEC(-276,1), FIXED_DEC(256 - 32,1), FIXED_DEC(154 - 32,1)}},
	};

	//Draw stage
	fx = stage.camera.x; 
	fy = stage.camera.y;
	
	FntPrint(" %d %d ", shit, ass);
	

	RECT window_src = {0, 0, 72, 78};
	RECT_FIXED window_dst = {FIXED_DEC(-447,1) - fx, FIXED_DEC(-301,1) - fy, FIXED_DEC(260,1), FIXED_DEC(163,1)};
	Stage_BlendTex(&this->tex_back1, &window_src, &window_dst, stage.camera.bzoom, 1);

	const struct Back_Week1_LowerBop *lbop_p = lbop_piece;
	for (size_t i = 0; i < COUNT_OF(lbop_piece); i++, lbop_p++)
	{
		RECT_FIXED lbop_dst = {
			lbop_p->dst.x - fx - (beat_bop << 1),
			lbop_p->dst.y - fy + (beat_bop << 3),
			lbop_p->dst.w + (beat_bop << 2),
			lbop_p->dst.h - (beat_bop << 3),
		};
		Stage_DrawTex(&this->tex_back1, &lbop_p->src, &lbop_dst, stage.camera.bzoom);
	}

	RECT back_src = {0, 0, 256, 256};
	RECT_FIXED back_dst = {FIXED_DEC(-496,1) - fx, FIXED_DEC(-356,1) - fy, FIXED_DEC(920,1), FIXED_DEC(540,1)};
	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
}

void Back_Week1_Free(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week1_New(void)
{
	//Allocate background structure
	Back_Week1 *this = (Back_Week1*)Mem_Alloc(sizeof(Back_Week1));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week1_DrawBG;
	this->back.free = Back_Week1_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK1\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
