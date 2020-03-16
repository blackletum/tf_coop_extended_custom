//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======//
//
// Purpose: CTF Currency Pack.
//
//=============================================================================//
#ifndef ENTITY_CURRENCYPACK_H
#define ENTITY_CURRENCYPACK_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_powerup.h"
#include "tf_gamerules.h"

//=============================================================================
//
// CTF Currencypack class.
//

DECLARE_AUTO_LIST( ICurrencyPackAutoList )
class CCurrencyPack : public CTFPowerup, public ICurrencyPackAutoList
{
public:
	DECLARE_CLASS( CCurrencyPack, CTFPowerup );
	DECLARE_SERVERCLASS();

	CCurrencyPack();

	void	Spawn( void );
	void	Precache( void );
	//void	UpdateOnRemove( void );

	bool	MyTouch( CBasePlayer *pPlayer );
	bool	NPCTouch( CAI_BaseNPC *pNPC );

	virtual const char *GetDefaultPowerupModel( void ) { return "models/items/currencypack_large.mdl"; }

	//bool	AffectedByRadiusCollection( void ) const;
	//void	BlinkThink( void );
	//void	ComeToRest( void );
	//void	DistributedBy( CBasePlayer *pPlayer );

	//void	SetAmount( float flAmount )

	//virtual int UpdateTransmitState( void );
	//virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo );

private:
	bool	m_bDistributed;
};

class CCurrencyPackSmall : public CCurrencyPack
{
public:
	DECLARE_CLASS( CCurrencyPackSmall, CCurrencyPack );
	powerupsize_t	GetPowerupSize( void ) { return POWERUP_SMALL; }

	virtual const char *GetDefaultPowerupModel( void ) { return "models/items/currencypack_small.mdl"; }
};

class CCurrencyPackMedium : public CCurrencyPack
{
public:
	DECLARE_CLASS( CCurrencyPackMedium, CCurrencyPack );
	powerupsize_t	GetPowerupSize( void ) { return POWERUP_MEDIUM; }

	virtual const char *GetDefaultPowerupModel( void ) { return "models/items/currencypack_medium.mdl"; }
};

#endif // ENTITY_CURRENCYPACK_H


