//============ Copyright Valve Corporation, All rights reserved. ===============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TF_TRIGGER_CATAPULT_H
#define TF_TRIGGER_CATAPULT_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "triggers.h"

class CTriggerCatapult : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerCatapult, CBaseTrigger );
	DECLARE_DATADESC();

	void	Spawn( void );
	void	StartTouch( CBaseEntity *pOther );

	//CalculateLaunchVector(CBaseEntity*, CBaseEntity*);
	//CalculateLaunchVectorPreserve(Vector, CBaseEntity*, CBaseEntity*, bool);
	//void		LaunchByDirection(CBaseEntity*);
	//void		LaunchByTarget(CBaseEntity*, CBaseEntity*);
	//void		LaunchThink();

	void	InputSetPlayerSpeed( inputdata_t &inputdata );
	void	InputSetPhysicsSpeed( inputdata_t &inputdata );
	void	InputSetLaunchTarget( inputdata_t &inputdata );
	void	InputSetExactVelocityChoiceType( inputdata_t &inputdata );

	COutputEvent	m_OnCatapulted;

private:
	float	m_flPlayerVelocity;
	float	m_flPhysicsVelocity;
	Vector	m_vecLaunchAngles;
	string_t m_strLaunchTarget;
	bool	m_bUseThresholdCheck;
	bool	m_bUseExactVelocity;
	float	m_flLowerThreshold;
	float	m_flUpperThreshold;
	int		m_ExactVelocityChoice;
	bool	m_bOnlyVelocityCheck;
	bool	m_bApplyAngularImpulse;
	float	m_flEntryAngleTolerance;
	float	m_flAirControlSupressionTime;
	bool	m_bDirectionSuppressAirControl;
	EHANDLE m_hLaunchTarget;
	float	m_flRefireDelay;
	EHANDLE	m_hAbortedLaunchees;
};

#endif