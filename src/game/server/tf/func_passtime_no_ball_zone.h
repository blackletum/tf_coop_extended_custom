//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime No Ball Zone.
//
//=============================================================================//
#ifndef FUNC_PASSTIME_NO_BALL_ZONE_H
#define FUNC_PASSTIME_NO_BALL_ZONE_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

//=============================================================================
//
// CTF Passtime No Ball Zone class.
//
DECLARE_AUTO_LIST( IFuncPasstimeNoBallZoneAutoList )
class CFuncPasstimeNoBallZone : public CBaseTrigger, public IFuncPasstimeNoBallZoneAutoList
{
	DECLARE_CLASS( CFuncPasstimeNoBallZone, CBaseTrigger );
public:

	void	Spawn();

private:

	DECLARE_DATADESC();
};

#endif // FUNC_PASSTIME_GOAL_H
