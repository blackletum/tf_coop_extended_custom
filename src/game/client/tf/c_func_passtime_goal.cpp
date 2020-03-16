//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Goal.
//
//=============================================================================//
#include "cbase.h"
#include "c_func_passtime_goal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_FuncPasstimeGoal, DT_FuncPasstimeGoal, CFuncPasstimeGoal )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Don't draw for friendly players
//-----------------------------------------------------------------------------
void C_FuncPasstimeGoal::Spawn( void )
{
	BaseClass::Spawn();
}