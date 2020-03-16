//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// TF Ray Gun
//
//=============================================================================
#ifndef TF_WEAPON_RAYGUN_H
#define TF_WEAPON_RAYGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

#ifdef CLIENT_DLL
#define CTFBison C_TFBison
#endif

class CTFBison : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFBison, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFBison() {}
	~CTFBison() {}

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_RAYGUN; }

	virtual float	GetProjectileSpeed( void ) { return 1200.0f; }
	virtual float	GetProjectileGravity( void ) { return 0.0f; }

	virtual float	Energy_GetShotCost( void ) const { return 1.0f; }
	virtual float	Energy_GetRechargeCost( void ) const { return 1.0f; }
private:
	CTFBison( const CTFBison & ) {}
};

/*
// Client specific.
#ifdef CLIENT_DLL
#define CTFRaygun C_TFRaygun
#endif

//=============================================================================
//
// TF Weapon Ray Gun.
//class CTFRaygun : public CTFWeaponBaseGun
{
public:
	DECLARE_CLASS( CTFRaygun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFRaygun();
	~CTFRaygun();

	virtual void	PrimaryAttack( void );

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_RAYGUN; }


	/*Deploy()
	GetDamage()
	GetMuzzleFlashParticleEffect()
	GetProjectileGravity()
	GetProjectileSpeed()
	Holster(CBaseCombatWeapon*)
	IsEnergyWeapon() const
	IsViewModelFlipped()
	ItemPostFrame()
	ModifyProjectile(CBaseEntity*)
	Precache()*/
/*
#ifdef CLIENT_DLL
C_TFRaygun::ClientEffectsThink()
C_TFRaygun::DispatchMuzzleFlash(char const*, C_BaseEntity*)
C_TFRaygun::GetEffectLabelText()
C_TFRaygun::GetIdleParticleEffect()
C_TFRaygun::GetMuzzleFlashParticleEffect()
C_TFRaygun::GetProgress()
C_TFRaygun::ShouldPlayClientReloadSound()
#endif
*/
/*
private:
	CTFRaygun( const CTFRaygun & ) {}
};
*/
#endif // TF_WEAPON_RAYGUN_H