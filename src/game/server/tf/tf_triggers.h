//===== NOT Copyright © 1996-2005, Valve Corporation, All rights reserved. =====//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TE_TFTRIGGER_H
#define TE_TFTRIGGER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "triggers.h"
#include "trigger_area_capture.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTriggerStun : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerStun, CBaseTrigger );

public:
	void	Spawn( void );
	void	Touch( CBaseEntity *pOther );
	void	EndTouch( CBaseEntity *pOther );

	bool	StunEntity( CBaseEntity *pOther );
	void	StunThink( void );

private:
	DECLARE_DATADESC();

	COutputEvent	m_outputOnStun;	// Fired a stun

	float			m_flTriggerDelay;
	float			m_flStunDuration;
	float			m_flMoveSpeedReduction;
	int				m_iStunType;
	bool			m_bStunEffects;

	CUtlVector< CBaseEntity *>	m_stunEntities;
	
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTriggerAddTFPlayerCondition : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerAddTFPlayerCondition, CBaseTrigger );
public:
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );

	virtual void StartTouch( CBaseEntity *pOther );
	virtual void EndTouch( CBaseEntity *pOther );

private:

	float			m_flDuration;
	int				m_nCondition;
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTriggerRemoveTFPlayerCondition : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerRemoveTFPlayerCondition, CBaseTrigger );
public:
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );

	virtual void StartTouch( CBaseEntity *pOther );
	virtual void EndTouch( CBaseEntity *pOther );

private:
	int				m_nCondition;
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTriggerAddOrRemoveTFPlayerAttributes : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerAddOrRemoveTFPlayerAttributes, CBaseTrigger );
public:
	DECLARE_DATADESC();

	void Spawn( void );

	virtual void StartTouch( CBaseEntity *pOther );
	virtual void EndTouch( CBaseEntity *pOther );

private:
	bool			m_bRemove;
	float			m_flDuration;
	float			m_flAttributeValue;
	string_t		m_iszAttributeName;
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTriggerTimerDoor : public CTriggerAreaCapture
{
	DECLARE_CLASS( CTriggerTimerDoor, CTriggerAreaCapture );
public:
	CTriggerTimerDoor();

	// A team has finished capturing the zone.
	virtual void OnEndCapture( int iTeam );
	virtual void OnStartCapture( int iTeam );

	virtual void Spawn( void );

	virtual void StartTouch(CBaseEntity *pOther) OVERRIDE;
private:
	string_t			m_iszDoorName;
	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTriggerIgniteArrows : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerIgniteArrows, CBaseTrigger );

public:
	void	Spawn( void );
	void	Touch( CBaseEntity *pOther );

private:
	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTriggerIgnite : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerIgnite, CBaseTrigger );

public:
	virtual void	Spawn( void );
	virtual void	Precache( void );

	virtual void	StartTouch( CBaseEntity *pOther );

	void			IgniteEntity( CBaseEntity *pOther );
	void			BurnThink( void );
	//void			BurnEntities( void );

private:
	DECLARE_DATADESC();

	float			m_flBurnDuration;
	float			m_flDamagePercentPerSecond;
	string_t		m_iszIgniteParticleName;
	string_t		m_iszIgniteSoundName;
	
};

//-----------------------------------------------------------------------------
// trigger_coop header part
//-----------------------------------------------------------------------------
class CTriggerCoOp : public CBaseTrigger
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CTriggerCoOp, CBaseTrigger );

	virtual void	Spawn( void );
	virtual void	Activate( void );

	virtual void	StartTouch( CBaseEntity *pOther );
	virtual void	EndTouch( CBaseEntity *pOther );

private:
	int				m_iCountType;
	int				m_iPlayerValue;
	bool			m_iUseHud;

	bool			m_bPlayersOut;

	// Outputs
	COutputEvent	m_OnPlayersIn;
	COutputEvent	m_OnPlayersOut;
};

#endif