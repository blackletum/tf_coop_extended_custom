//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Goal.
//
//=============================================================================//
#ifndef C_FUNC_PASSTIME_GOAL_H
#define C_FUNC_PASSTIME_GOAL_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_shareddefs.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_FuncPasstimeGoal : public C_BaseEntity
{
	DECLARE_CLASS( C_FuncPasstimeGoal, C_BaseEntity );

public:
	DECLARE_CLIENTCLASS();

	void Spawn( void );
};

#endif // C_FUNC_CAPTURE_ZONE_H