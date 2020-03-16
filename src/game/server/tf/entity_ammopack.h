//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: CTF AmmoPack.
//
//=============================================================================//
#ifndef ENTITY_AMMOPACK_H
#define ENTITY_AMMOPACK_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_powerup.h"
#include "tf_gamerules.h"

#define TF_AMMOPACK_PICKUP_SOUND	"AmmoPack.Touch"

//=============================================================================
//
// CTF AmmoPack class.
//

class CAmmoPack : public CTFPowerup
{
public:
	DECLARE_CLASS( CAmmoPack, CTFPowerup );

	void	Spawn( void );
	void	Precache( void );
	bool	MyTouch( CBasePlayer *pPlayer );

	powerupsize_t	GetPowerupSize( void ) { return POWERUP_FULL; }

	virtual const char *GetDefaultPowerupModel( void ) { return TFGameRules()->IsBirthday() ? "models/items/ammopack_large_bday.mdl" : "models/items/ammopack_large.mdl"; }
	void	UpdateModelIndexOverrides( void );
};

class CAmmoPackSmall : public CAmmoPack
{
public:
	DECLARE_CLASS( CAmmoPackSmall, CAmmoPack );
	powerupsize_t	GetPowerupSize( void ) { return POWERUP_SMALL; }

	virtual const char *GetDefaultPowerupModel( void ) { return TFGameRules()->IsBirthday() ? "models/items/ammopack_small_bday.mdl" : "models/items/ammopack_small.mdl"; }
	void	UpdateModelIndexOverrides( void );
};

class CAmmoPackMedium : public CAmmoPack
{
public:
	DECLARE_CLASS( CAmmoPackMedium, CAmmoPack );
	powerupsize_t	GetPowerupSize( void ) { return POWERUP_MEDIUM; }

	virtual const char *GetDefaultPowerupModel( void ) { return TFGameRules()->IsBirthday() ? "models/items/ammopack_small_bday.mdl" : "models/items/ammopack_small.mdl"; }
	void	UpdateModelIndexOverrides( void );
};

#endif // ENTITY_AMMOPACK_H


