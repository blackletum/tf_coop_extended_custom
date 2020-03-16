//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose: TF Caltrop Grenade.
//
//=============================================================================//
#ifndef TF_WEAPON_GRENADE_CALTROP_H
#define TF_WEAPON_GRENADE_CALTROP_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_grenadeproj.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFGrenadeCaltrop C_TFGrenadeCaltrop
#define CTFGrenadeCaltropProjectile C_TFGrenadeCaltropProjectile
#endif

//=============================================================================
//
// TF Caltrop Grenade Projectile
//
class CTFGrenadeCaltropProjectile : public CTFWeaponBaseGrenadeProj
{
public:
	DECLARE_CLASS( CTFGrenadeCaltropProjectile, CTFWeaponBaseGrenadeProj );
	DECLARE_NETWORKCLASS();

	// Unique identifier.
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_GRENADE_CALTROP; }

#ifdef GAME_DLL
	// Creation.
	static CTFGrenadeCaltropProjectile *Create( const Vector &position, const QAngle &angles, const Vector &velocity, 
		                                       const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags = 0 );
	// Overrides.
	virtual void	Spawn();
	virtual void	Precache();
	virtual void	BounceSound( void );
	virtual void	Detonate();
#endif

	virtual void	CaltropTouch( CBaseEntity *pOther );

#ifdef CLIENT_DLL
	virtual void	OnDataChanged(DataUpdateType_t updateType);
#endif

private:

	float m_flDetonateTime;
};

#endif // TF_WEAPON_GRENADE_CALTROP_H
