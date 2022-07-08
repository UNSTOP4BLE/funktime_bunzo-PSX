/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bunzom.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//bunzom character structure
enum
{
	bunzom_ArcMain_Idle,
	
	bunzom_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[bunzom_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_bunzom;

//bunzom character definitions
static const CharFrame char_bunzom_frame[] = {
	{bunzom_ArcMain_Idle, {  0,   0,  81, 64}, { 1, 0}}, 
	{bunzom_ArcMain_Idle, { 81,   0,  80, 64}, { 0, 0}},
	{bunzom_ArcMain_Idle, {161,   0,  80, 64}, { 0, 0}},
};

static const Animation char_bunzom_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//bunzom character functions
void Char_bunzom_SetFrame(void *user, u8 frame)
{
	Char_bunzom *this = (Char_bunzom*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bunzom_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_bunzom_Tick(Character *character)
{
	Char_bunzom *this = (Char_bunzom*)character;

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);

	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_bunzom_SetFrame);
	Character_Draw(character, &this->tex, &char_bunzom_frame[this->frame]);
}

void Char_bunzom_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_bunzom_Free(Character *character)
{
	Char_bunzom *this = (Char_bunzom*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_bunzom_New(fixed_t x, fixed_t y)
{
	//Allocate bunzom object
	Char_bunzom *this = Mem_Alloc(sizeof(Char_bunzom));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_bunzom_New] Failed to allocate bunzom object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_bunzom_Tick;
	this->character.set_anim = Char_bunzom_SetAnim;
	this->character.free = Char_bunzom_Free;
	
	Animatable_Init(&this->character.animatable, char_bunzom_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(-64,1);
	this->character.focus_y = FIXED_DEC(-93,1);
	this->character.focus_zoom = FIXED_DEC(4,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BUNZOM.ARC;1");
	
	const char **pathp = (const char *[]){
		"bunzom.tim", //bunzom_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);

	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
