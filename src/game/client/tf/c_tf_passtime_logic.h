//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Logic.
//
//=============================================================================//
#ifndef C_TF_PASSTIME_LOGIC_H
#define C_TF_PASSTIME_LOGIC_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"

class C_TFPasstimeLogic : public C_BaseEntity
{
	DECLARE_CLASS( C_TFPasstimeLogic, C_BaseEntity );

public:

	DECLARE_CLIENTCLASS();

	C_TFPasstimeLogic();
	~C_TFPasstimeLogic();

	//bool	BCanPlayerPickUpBall(CTFPlayer*) const

	//ClientThink()
	//C_BaseEntity *GetBallReticleTarget(C_BaseEntity**, bool*) const
	//GetTrackPoints(Vector (&) [16])
	//PostDataUpdate(DataUpdateType_t)
	//Spawn()
	//UpdateBeams()
};

#endif // C_TF_PASSTIME_LOGIC_H