//=============================================================================//
//
// Purpose: Crocs lives here!
//
//=============================================================================//
#ifndef FUNC_CROC_H
#define FUNC_CROC_H

#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"
#include "tf_player.h"

//=============================================================================
// func_croc-odile
//=============================================================================
class CFuncCroc : public CBaseTrigger
{
public:
	DECLARE_CLASS( CFuncCroc, CBaseTrigger );
	DECLARE_DATADESC();

	CFuncCroc();

	void	Spawn( void );
	void	Precache( void );

	virtual int		UpdateTransmitState( void );
	virtual int		ShouldTransmit( const CCheckTransmitInfo *pInfo );
	virtual bool	ShouldCollide( int collisionGroup, int contentsMask ) const;

	virtual void	StartTouch( CBaseEntity *pOther );

	void			FireOutputs( CBaseEntity *pActivator );
private:
	COutputEvent	m_OnEat;
	COutputEvent	m_OnEatBlue;
	COutputEvent	m_OnEatRed;
};

//=============================================================================
// the actual crocodile
//=============================================================================
class CEntityCroc : public CBaseAnimating
{
public:
	DECLARE_CLASS( CEntityCroc, CBaseAnimating );

	void	Spawn( void );
	void	InitCroc( void );
	void	Think ( void );
	void	CrocAttack ( void );

	DECLARE_DATADESC();
};

#endif // FUNC_CROC_H



