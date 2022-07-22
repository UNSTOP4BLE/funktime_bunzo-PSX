/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_bf_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	BF_ArcMain_Idle,
	BF_ArcMain_Left0,
	BF_ArcMain_Left1,
	BF_ArcMain_Down0,
	BF_ArcMain_Down1,
	BF_ArcMain_Up0,
	BF_ArcMain_Up1,
	BF_ArcMain_Right0,
	BF_ArcMain_Right1,
	BF_ArcMain_Miss0,
	BF_ArcMain_Miss1,
	BF_ArcMain_Miss2,
	BF_ArcMain_Dead0, //BREAK
	BF_ArcMain_Dead1, //Mic Drop
	BF_ArcMain_Dead2, //Twitch
	BF_ArcMain_Retry, //Retry prompt
	
	BF_ArcMain_Max,
};

#define BF_Arc_Max BF_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BF_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bf_skull)];
	u8 skull_scale;
} Char_BF;

//Boyfriend player definitions
static const CharFrame char_bf_frame[] = {
	{BF_ArcMain_Idle, {  0,   0, 124,  94}, { 65,  92}}, //0 idle 1
	{BF_ArcMain_Idle, {124,   0, 124,  93}, { 65,  91}}, //1 idle 2
	{BF_ArcMain_Idle, {  0,  94, 123,  95}, { 64,  92}}, //2 idle 3
	{BF_ArcMain_Idle, {123,  93, 125,  96}, { 64,  94}}, //3 idle 4
	
	{BF_ArcMain_Left0, {0,    0,  247, 103}, {180, 101}}, //4 left 1
	{BF_ArcMain_Left0, {0,  103,  247, 106}, {191, 101}}, //5 left 2
	{BF_ArcMain_Left1, {0,    0,  234, 103}, {178, 101}}, //6 left 1
	{BF_ArcMain_Left1, {0,  103,  181, 103}, { 96, 102}}, //7 left 2
	
	{BF_ArcMain_Down0, {0,   0,  162, 126}, { 89, 99}}, //8 down 1
	{BF_ArcMain_Down0, {0, 126,  162, 126}, { 94, 97}}, //9 down 2
	{BF_ArcMain_Down1, {0,   0,  151, 122}, { 85, 99}}, //10 down 1
	{BF_ArcMain_Down1, {0, 122,  151, 102}, { 67, 100}}, //11 down 2
	
	{BF_ArcMain_Up0, {0,   0,  157, 119}, { 61, 103}}, //12 up 1
	{BF_ArcMain_Up0, {0, 119,  157, 132}, { 61, 103}}, //13 up 2
	{BF_ArcMain_Up1, {0,   0,  149, 116}, { 62, 102}}, //14 up 1
	{BF_ArcMain_Up1, {0, 116,  149, 104}, { 62, 102}}, //15 up 2
	
	{BF_ArcMain_Right0, {0,   0,  248, 104}, { 52, 101}}, //16 right 1
	{BF_ArcMain_Right0, {0, 104,  248, 108}, { 50, 101}}, //17 right 2
	{BF_ArcMain_Right1, {0,   0,  235, 103}, { 51, 101}}, //18 right 1
	{BF_ArcMain_Right1, {0, 103,  159, 103}, { 51, 101}}, //19 right 2

	{BF_ArcMain_Miss0, {  0,   0, 128, 103}, { 63, 101}}, //20 left miss 1
	{BF_ArcMain_Miss0, {128,   0, 128, 103}, { 63, 101}}, //21 left miss 1
	{BF_ArcMain_Miss0, {  0, 103, 128, 103}, { 64, 101}}, //22 left miss 1
	{BF_ArcMain_Miss0, {128, 103, 128, 103}, { 63, 101}}, //23 left miss 1

	{BF_ArcMain_Miss1, {  0,   0, 129, 103}, { 63, 101}}, //24 left miss 1
	//{BF_ArcMain_Miss1, {129,   0, 127, 103}, { 52, 101}}, //25 left miss 1
	{BF_ArcMain_Miss1, {  0, 103, 128, 103}, { 62, 101}}, //25 left miss 1
	{BF_ArcMain_Miss1, {128, 103, 128, 103}, { 63, 101}}, //26 left miss 1

	{BF_ArcMain_Miss2, {  0,   0, 129, 103}, { 63, 101}}, //27 left miss 1
	{BF_ArcMain_Miss2, {129,   0, 127, 103}, { 61, 101}}, //28 left miss 1
	{BF_ArcMain_Miss2, {  0, 103, 129, 103}, { 63, 101}}, //29 left miss 1

	{BF_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //30 dead0 0
	{BF_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //31 dead0 1
	{BF_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //32 dead0 2
	{BF_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //33 dead0 3

	{BF_ArcMain_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //34 dead1 0
	{BF_ArcMain_Dead1, {128,   0, 127, 128}, { 53,  98}}, //35 dead1 1
	{BF_ArcMain_Dead1, {  0, 128, 128, 127}, { 53,  98}}, //36 dead1 2
	{BF_ArcMain_Dead1, {128, 128, 127, 127}, { 53,  98}}, //37 dead1 3
	
	{BF_ArcMain_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //38 dead2 body twitch 0
	{BF_ArcMain_Dead2, {128,   0, 127, 128}, { 53,  98}}, //39 dead2 body twitch 1
	{BF_ArcMain_Dead2, {  0, 128, 128, 127}, { 53,  98}}, //40 dead2 balls twitch 0
	{BF_ArcMain_Dead2, {128, 128, 127, 127}, { 53,  98}}, //41 dead2 balls twitch 1
};

static const Animation char_bf_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 0}},     //CharAnim_Idle
	{2, (const u8[]){ 4,  5, 6, 7, ASCR_BACK, 0}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, 10, 11, ASCR_BACK, 0}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 0}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 0}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{2, (const u8[]){ 20, 21, 22, 23, 24, ASCR_BACK, 0}},     //PlayerAnim_LeftMiss
	{2, (const u8[]){ 25, 26, 27, 28, 29, ASCR_BACK, 0}},     //PlayerAnim_LeftMiss

/*
	{5, (const u8[]){30, 31, 32, 33, 33, 33, 33, 33, 33, 33, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){33, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){34, 35, 36, 37, 37, 37, 37, 37, 37, 37, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){37, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){38, 39, 37, 37, 37, 37, 37, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){40, 41, 37, 37, 37, 37, 37, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){37, 37, 37, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){40, 41, 37, ASCR_REPEAT}},  //PlayerAnim_Dead5 */
};

//Boyfriend player functions
void Char_BF_SetFrame(void *user, u8 frame)
{
	Char_BF *this = (Char_BF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BF_Tick(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_MissL &&
		     character->animatable.anim != PlayerAnim_MissR) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BF, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BF_SetFrame);
	Character_Draw(character, &this->tex, &char_bf_frame[this->frame]);
}

void Char_BF_SetAnim(Character *character, u8 anim)
{
	Char_BF *this = (Char_BF*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BF_ArcMain_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BF_Free(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BF_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BF *this = Mem_Alloc(sizeof(Char_BF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BF_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BF_Tick;
	this->character.set_anim = Char_BF_SetAnim;
	this->character.free = Char_BF_Free;
	
	Animatable_Init(&this->character.animatable, char_bf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.focus_x = FIXED_DEC(-53 - -55,1);
	this->character.focus_y = FIXED_DEC(25 - 120,1);
	this->character.focus_zoom = FIXED_DEC(8,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BF.ARC;1");

	const char **pathp = (const char *[]){
		"idle.tim",   //BF_ArcMain_Idle0
		"left0.tim",  //BF_ArcMain_Left
		"left1.tim",  //BF_ArcMain_Left
		"down0.tim",  //BF_ArcMain_Down
		"down1.tim",  //BF_ArcMain_Down
		"up0.tim",    //BF_ArcMain_Up
		"up1.tim",    //BF_ArcMain_Up
		"right0.tim", //BF_ArcMain_Right
		"right1.tim", //BF_ArcMain_Right
		"miss0.tim", //BF_ArcMain_Miss
		"miss1.tim", //BF_ArcMain_Miss
		"miss2.tim", //BF_ArcMain_Miss
		"dead0.tim", //BF_ArcMain_Dead0
		"dead1.tim", //BF_ArcMain_Dead1
		"dead2.tim", //BF_ArcMain_Dead2
		"retry.tim", //BF_ArcMain_Retry
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;

	return (Character*)this;
}
