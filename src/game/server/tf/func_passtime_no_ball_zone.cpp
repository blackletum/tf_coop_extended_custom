//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime No Ball Zone.
//
//=============================================================================//
#include "cbase.h"
#include "func_passtime_no_ball_zone.h"

//=============================================================================
//
// CTF Passtime No Ball Zone tables.
//

BEGIN_DATADESC( CFuncPasstimeNoBallZone )
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_passtime_no_ball_zone, CFuncPasstimeNoBallZone );

IMPLEMENT_AUTO_LIST( IFuncPasstimeNoBallZoneAutoList );

//=============================================================================
//
// CTF Passtime No Ball Zone functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncPasstimeNoBallZone::Spawn()
{
	AddSpawnFlags( SF_TRIGGER_ALLOW_ALL | SF_TRIG_TOUCH_DEBRIS );

	BaseClass::Spawn();
}