//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// Purpose: This is the camo version of the combine soldier
//
//=============================================================================
#ifndef	NPC_COMBINEE_H
#define	NPC_COMBINEE_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_combine.h"

//=========================================================
//	>> CNPC_CombineE
//=========================================================
class CNPC_CombineE : public CNPC_Combine
{
	DECLARE_CLASS( CNPC_CombineE, CNPC_Combine );

public:
	void		Spawn( void );
	void		Precache( void );
};

#endif	//NPC_COMBINEE_H
