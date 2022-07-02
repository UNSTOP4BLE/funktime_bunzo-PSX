/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week1.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"

int windowanim;

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_screen, arc_screen_ptr[3];
	Gfx_Tex tex_back0; //Stage and back
	Gfx_Tex tex_back1; //Curtains

	//screen state
	Gfx_Tex tex_screen;
	u8 screen_frame, screen_tex_id;
	
	Animatable screen_animatable;
} Back_Week1;

//screen animation and rects
static const CharFrame screen_frame[] = {
	{0, {  3,   0, 242, 106}, { 0, 0}}, //green
	{0, {  3, 106, 242, 107}, { 0, 0}}, //red
	{1, {  3,   0, 241, 106}, { 2, 0}}, //cyan
	{1, {  3, 106, 242, 107}, { 0, 0}}, //pink
	{2, {  3,   0, 242, 106}, { 0, 0}}, //gl
};

static const Animation screen_anim[] = {
	{1, (const u8[]){4, ASCR_BACK, 0}},
	{1, (const u8[]){3, ASCR_BACK, 0}},
	{1, (const u8[]){2, ASCR_BACK, 0}},
	{1, (const u8[]){0, ASCR_BACK, 0}},
	{1, (const u8[]){1, ASCR_BACK, 0}},
};

//screen functions
void Week1_screen_SetFrame(void *user, u8 frame)
{
	Back_Week1 *this = (Back_Week1*)user;
	
	//Check if this is a new frame
	if (frame != this->screen_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &screen_frame[this->screen_frame = frame];
		if (cframe->tex != this->screen_tex_id)
			Gfx_LoadTex(&this->tex_screen, this->arc_screen_ptr[this->screen_tex_id = cframe->tex], 0);
	}
}

void Week1_screen_Draw(Back_Week1 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &screen_frame[this->screen_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, FIXED_DEC(869,1), FIXED_DEC(222,1)};
	Stage_DrawTex(&this->tex_screen, &src, &dst, stage.camera.bzoom);
}

void Back_Week1_DrawMD(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	fixed_t fx, fy;
	
	fx = stage.camera.x; 
	fy = stage.camera.y;

	RECT button_src = {115, 2, 125, 52};
	RECT_FIXED button_dst = {FIXED_DEC(-273,1) - fx, FIXED_DEC(72,1) - fy, FIXED_DEC(438,1), FIXED_DEC(112,1)};
	Stage_DrawTex(&this->tex_back1, &button_src, &button_dst, stage.camera.bzoom);
}

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

	switch ((stage.mode == StageMode_Swap) ? stage.player->animatable.anim : stage.opponent->animatable.anim)
	{
		case CharAnim_Left:
			windowanim = 1;
			break;
		case CharAnim_Down:
			windowanim = 2;
			break;
		case CharAnim_Up:
			windowanim = 3;
			break;
		case CharAnim_Right:
			windowanim = 4;
			break;
		default:
			break;
	}
	if (stage.song_step >= 95)
	{
		switch (windowanim)
		{
			case 1:
				Animatable_SetAnim(&this->screen_animatable, 1); 
				break;
			case 2:
				Animatable_SetAnim(&this->screen_animatable, 2); 
				break;
			case 3:
				Animatable_SetAnim(&this->screen_animatable, 3); 
				break;
			case 4:
				Animatable_SetAnim(&this->screen_animatable, 4); 
				break;
			default:
				Animatable_SetAnim(&this->screen_animatable, 0); 
				break;
		}
	}	
	if (stage.song_step < 0)
		windowanim = 0;

	Animatable_Animate(&this->screen_animatable, (void*)this, Week1_screen_SetFrame);
	Week1_screen_Draw(this, FIXED_DEC(-485,1) - fx, FIXED_DEC(-154,1) - fy);
	 
	RECT back_src = {0, 0, 256, 256};
	RECT_FIXED back_dst = {FIXED_DEC(-496,1) - fx, FIXED_DEC(-356,1) - fy, FIXED_DEC(920,1), FIXED_DEC(540,1)};
	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
}

void Back_Week1_Free(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	//Free screen archive
	Mem_Free(this->arc_screen);
	
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
	this->back.draw_md = Back_Week1_DrawMD;
	this->back.draw_bg = Back_Week1_DrawBG;
	this->back.free = Back_Week1_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK1\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);
	
	//Load screen textures
	this->arc_screen = IO_Read("\\WEEK1\\SCREEN.ARC;1");
	this->arc_screen_ptr[0] = Archive_Find(this->arc_screen, "win0.tim");
	this->arc_screen_ptr[1] = Archive_Find(this->arc_screen, "win1.tim");
	this->arc_screen_ptr[2] = Archive_Find(this->arc_screen, "win2.tim");
	

	//Initialize screen state
	Animatable_Init(&this->screen_animatable, screen_anim);
	Animatable_SetAnim(&this->screen_animatable, 0);
	this->screen_frame = this->screen_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
