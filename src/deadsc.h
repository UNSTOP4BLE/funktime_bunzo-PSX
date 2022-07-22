/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_DEADSC_H
#define PSXF_GUARD_DEADSC_H

#include "gfx.h"

typedef struct
{
	Gfx_Tex sc0;
	Gfx_Tex sc1;
} DeadSc_Tex;

extern DeadSc_Tex deadsc_tex;

void DeadSc_Init();
void DeadSc_LoadSFX();
void DeadSc_Play(int animcooldown);

#endif
