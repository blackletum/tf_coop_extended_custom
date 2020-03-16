//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose: TF Emp Grenade.
//
//=============================================================================//
#ifndef TF_WEAPON_GRENADE_EMP_H
#define TF_WEAPON_GRENADE_EMP_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_grenadeproj.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFGrenadeEmpProjectile C_TFGrenadeEmpProjectile
#endif

//=============================================================================
//
// TF Emp Grenade Projectile
//

class CTFGrenadeEmpProjectile : public CTFWeaponBaseGrenadeProj
{
public:

	DECLARE_CLASS( CTFGrenadeEmpProjectile, CTFWeaponBaseGrenadeProj );

	// Unique identifier.
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_GRENADE_EMP; }

#ifdef GAME_DLL
	// Creation.
	static CTFGrenadeEmpProjectile *Create( const Vector &position, const QAngle &angles, const Vector &velocity, 
		                                       const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags = 0 );

	// Overrides.
	virtual void	Spawn();
	virtual void	Precache();
	virtual void	BounceSound( void );
	virtual void	Detonate();
	void			DetonateThink( void );

	DECLARE_DATADESC();

private:

	bool			m_bPlayedLeadIn;
#endif
};

#endif // TF_WEAPON_GRENADE_EMP_H
