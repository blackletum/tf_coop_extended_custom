//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose: TF Concussion Grenade.
//
//=============================================================================//
#ifndef TF_WEAPON_GRENADE_CONCUSSION_H
#define TF_WEAPON_GRENADE_CONCUSSION_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_grenadeproj.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFGrenadeConcussionProjectile C_TFGrenadeConcussionProjectile
#endif

class CTFGrenadeConcussionProjectile : public CTFWeaponBaseGrenadeProj
{
public:

	DECLARE_CLASS( CTFGrenadeConcussionProjectile, CTFWeaponBaseGrenadeProj );

	// Unique identifier.
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_GRENADE_CONCUSSION; }

#ifdef GAME_DLL
	// Creation.
	static CTFGrenadeConcussionProjectile *Create( const Vector &position, const QAngle &angles, const Vector &velocity, 
		                                       const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags = 0 );

	// Overrides.
	virtual void	Spawn();
	virtual void	Precache();
	virtual void	BounceSound( void );
	virtual void	Explode( trace_t *pTrace, int bitsDamageType );
	virtual void	Detonate();

private:

	float m_flDetonateTime;
#endif
};

#endif // TF_WEAPON_GRENADE_CONCUSSION_H
