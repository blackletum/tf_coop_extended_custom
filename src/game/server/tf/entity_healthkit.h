//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: CTF HealthKit.
//
//=============================================================================//
#ifndef ENTITY_HEALTHKIT_H
#define ENTITY_HEALTHKIT_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_powerup.h"
#include "tf_gamerules.h"

//=============================================================================
//
// CTF HealthKit class.
//

class CHealthKit : public CTFPowerup
{
public:
	DECLARE_CLASS( CHealthKit, CTFPowerup );

	void	Spawn( void );
	void	Precache( void );
	bool	MyTouch( CBasePlayer *pPlayer );
	bool	NPCTouch( CAI_BaseNPC *pNPC );

	virtual const char *GetDefaultPowerupModel( void )
	{
		if ( TFGameRules()->IsBirthday() )
			return GetDefaultBirthdayModel();
		else if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) )
			return GetDefaultHalloweenModel();

		return "models/items/medkit_large.mdl";
	}

	virtual const char *GetDefaultBirthdayModel( void ) { return "models/items/medkit_large_bday.mdl"; }
	virtual const char *GetDefaultHalloweenModel( void ) { return "models/props_halloween/halloween_medkit_large.mdl"; }
	void	UpdateModelIndexOverrides( void );

	powerupsize_t	GetPowerupSize( void ) { return POWERUP_FULL; }
};

class CHealthKitSmall : public CHealthKit
{
public:
	DECLARE_CLASS( CHealthKitSmall, CHealthKit );
	powerupsize_t	GetPowerupSize( void ) { return POWERUP_SMALL; }

	virtual const char *GetDefaultPowerupModel( void )
	{
		if ( TFGameRules()->IsBirthday() )
			return GetDefaultBirthdayModel();
		else if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) )
			return GetDefaultHalloweenModel();

		return "models/items/medkit_small.mdl";
	}

	virtual const char *GetDefaultBirthdayModel( void ) { return "models/items/medkit_small_bday.mdl"; }
	virtual const char *GetDefaultHalloweenModel( void ) { return "models/props_halloween/halloween_medkit_small.mdl"; }
	void	UpdateModelIndexOverrides( void );
};

class CHealthKitMedium : public CHealthKit
{
public:
	DECLARE_CLASS( CHealthKitMedium, CHealthKit );
	powerupsize_t	GetPowerupSize( void ) { return POWERUP_MEDIUM; }

	virtual const char *GetDefaultPowerupModel( void )
	{
		if ( GetOwnerEntity() == NULL )
		{
			if ( TFGameRules()->IsBirthday() )
				return GetDefaultBirthdayModel();
			else if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) )
				return GetDefaultHalloweenModel();
		}

		return "models/items/medkit_medium.mdl";
	}

	virtual const char *GetDefaultBirthdayModel( void ) { return "models/items/medkit_medium_bday.mdl"; }
	virtual const char *GetDefaultHalloweenModel( void ) { return "models/props_halloween/halloween_medkit_medium.mdl"; }
	void	UpdateModelIndexOverrides( void );
};

#endif // ENTITY_HEALTHKIT_H


