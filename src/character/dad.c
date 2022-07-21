/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "dad.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../audio.h"

static u32 Sounds[1];
static int soundcooldown;

//Dad character structure
enum
{
	Dad_ArcMain_Idle0,
	Dad_ArcMain_Idle1,
	Dad_ArcMain_Idle2,
	Dad_ArcMain_Idle3,
	Dad_ArcMain_Idle4,
	Dad_ArcMain_Left0,
	Dad_ArcMain_Left1,
	Dad_ArcMain_Left2,
	Dad_ArcMain_Down0,
	Dad_ArcMain_Down1,
	Dad_ArcMain_Down2,
	Dad_ArcMain_Up0,
	Dad_ArcMain_Up1,
	Dad_ArcMain_Up2,
	Dad_ArcMain_Right0,
	Dad_ArcMain_Right1,
	Dad_ArcMain_Right2,
	
	Dad_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Dad_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Dad;

//Dad character definitions
static const CharFrame char_dad_frame[] = {
	{Dad_ArcMain_Idle0, {  0,   0, 158, 122}, { 0, 11}}, 
	{Dad_ArcMain_Idle0, {  0, 122, 158, 124}, { 0, 11}},
	{Dad_ArcMain_Idle1, {  0,   0, 160, 126}, { 0, 12}},
	{Dad_ArcMain_Idle1, {  0, 126, 160, 123}, { 0, 11}},
	{Dad_ArcMain_Idle2, {  0,   0, 152, 120}, { 0, 10}},
	{Dad_ArcMain_Idle2, {  0, 120, 152, 116}, { 0, 10}},
	{Dad_ArcMain_Idle3, {  0,   0, 152, 115}, { 0, 10}},
	{Dad_ArcMain_Idle3, {  0, 115, 152, 115}, { 0, 10}},
	{Dad_ArcMain_Idle4, {  0,   0, 154, 114}, { 0, 10}},
	{Dad_ArcMain_Idle4, {  0, 114, 154, 115}, { 0, 10}},
	
	{Dad_ArcMain_Left0, {  0,   0, 148, 146}, { 30, 70}},
	{Dad_ArcMain_Left1, {  0,   0, 150, 148}, { 30, 70}},
	{Dad_ArcMain_Left2, {  0,   0, 150, 153}, { 30, 70}},
	
	{Dad_ArcMain_Down0, {  0,   0, 162, 179}, { 0,  0 + 40}},
	{Dad_ArcMain_Down1, {  0,   0, 162, 180}, {-1, -2 + 40}},
	{Dad_ArcMain_Down2, {  0,   0, 158, 178}, {-5, -5 + 40}},

	{Dad_ArcMain_Up0, {  0,   0, 138, 153}, { 10, 5 + 70}},
	{Dad_ArcMain_Up1, {  0,   0, 140, 148}, { 10, 1 + 70}},
	{Dad_ArcMain_Up2, {  0,   0, 144, 149}, { 12, 0 + 70}},

	{Dad_ArcMain_Right0, {  0,   0, 174, 155}, { 0,  0 + 50}},
	{Dad_ArcMain_Right1, {  0,   0, 174, 153}, { 0,  0 + 50}},
	{Dad_ArcMain_Right2, {  0,   0, 174, 152}, { 0, -2 + 50}},
};

static const Animation char_dad_anim[CharAnim_Max] = {
	{3, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 10, 11, 12, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 16, 17, 18, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 19, 20, 21, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Dad character functions
void Char_Dad_SetFrame(void *user, u8 frame)
{
	Char_Dad *this = (Char_Dad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_dad_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Dad_Tick(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;

	if (this->frame == 0)
		soundcooldown ++;
	
	if (soundcooldown == 8)
	{
		Audio_PlaySound(Sounds[0], 0x1aaa);
		soundcooldown = 0;
	}

	RECT pipe_src = {0, 0, 18, 256};
	RECT_FIXED pipe_dst = {this->character.x + FIXED_DEC(59,1) - stage.camera.x, this->character.y - FIXED_DEC(340,1) - stage.camera.y, FIXED_DEC(18,1), FIXED_DEC(373,1)};

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);

	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Dad_SetFrame);
	Character_Draw(character, &this->tex, &char_dad_frame[this->frame]);

	Stage_DrawTex(&stage.tex_screen, &pipe_src, &pipe_dst, stage.camera.bzoom);
}

void Char_Dad_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Dad_Free(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Dad_New(fixed_t x, fixed_t y)
{
	//Allocate dad object
	Char_Dad *this = Mem_Alloc(sizeof(Char_Dad));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Dad_New] Failed to allocate dad object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Dad_Tick;
	this->character.set_anim = Char_Dad_SetAnim;
	this->character.free = Char_Dad_Free;
	
	Animatable_Init(&this->character.animatable, char_dad_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(-64,1);
	this->character.focus_y = FIXED_DEC(-93,1);
	this->character.focus_zoom = FIXED_DEC(4,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\DAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Dad_ArcMain_Idle0
		"idle1.tim", //Dad_ArcMain_Idle1
		"idle2.tim", //Dad_ArcMain_Idle1
		"idle3.tim", //Dad_ArcMain_Idle1
		"idle4.tim", //Dad_ArcMain_Idle1
		"left0.tim",  //Dad_ArcMain_Left
		"left1.tim",  //Dad_ArcMain_Left
		"left2.tim",  //Dad_ArcMain_Left
		"down0.tim",  //Dad_ArcMain_Down
		"down1.tim",  //Dad_ArcMain_Down
		"down2.tim",  //Dad_ArcMain_Down
		"up0.tim",    //Dad_ArcMain_Up
		"up1.tim",    //Dad_ArcMain_Up
		"up2.tim",    //Dad_ArcMain_Up
		"right0.tim", //Dad_ArcMain_Right
		"right1.tim", //Dad_ArcMain_Right
		"right2.tim", //Dad_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	CdlFILE file;
	IO_FindFile(&file, "\\SOUNDS\\CYMBAL.VAG;1");
	u32 *data = IO_ReadFile(&file);
	Sounds[0] = Audio_LoadVAGData(data, file.size);
	Mem_Free(data);

	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
