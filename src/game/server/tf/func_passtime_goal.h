//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Goal Zone.
//
//=============================================================================//
#ifndef FUNC_PASSTIME_GOAL_H
#define FUNC_PASSTIME_GOAL_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

#define SF_PASSGOAL_SCOREWIN			1
#define SF_PASSGOAL_DONTLETBALLSCORE	2
#define SF_PASSGOAL_CARRYBALLSCORE		4
#define SF_PASSGOAL_SHOWLOCKEDHUD		8

//-----------------------------------------------------------------------------
// Purpose: This class is to get around the fact that DEFINE_FUNCTION doesn't like multiple inheritance
//-----------------------------------------------------------------------------
class CFuncPasstimeGoalShim : public CBaseTrigger
{
	virtual void ShimStartTouch( CBaseEntity *pOther ) = 0;
	virtual void ShimEndTouch( CBaseEntity *pOther ) = 0;

public:
	void StartTouch( CBaseEntity *pOther ) { return ShimStartTouch( pOther ); }
	void EndTouch( CBaseEntity *pOther ) { return ShimEndTouch( pOther ); }
};

//=============================================================================
//
// CTF Passtime Goal Zone class.
//
class CFuncPasstimeGoal : public CFuncPasstimeGoalShim
{
	DECLARE_CLASS( CFuncPasstimeGoal, CBaseTrigger );

public:
	DECLARE_SERVERCLASS();

	void	Spawn();
	virtual void ShimStartTouch( CBaseEntity *pOther );
	virtual void ShimEndTouch( CBaseEntity *pOther );

	bool	IsDisabled( void );
	void	SetDisabled( bool bDisabled );

	// Input handlers
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

	int		UpdateTransmitState( void );

	//void	OnScore( int i );
private:

	bool			m_bTriggerDisabled;		// Enabled/Disabled?
	int				m_iGoalType;

	int				m_iPoints;	// Used in non-CTF maps to identify this capture point

	COutputEvent	m_onScoreAny;	// Fired a flag is captured on this point.
	COutputEvent	m_onScoreRed;	// Fired red team captured a flag on this point.
	COutputEvent	m_onScoreBlu;	// Fired blue team captured a flag on this point.

	DECLARE_DATADESC();
};

#endif // FUNC_PASSTIME_GOAL_H
