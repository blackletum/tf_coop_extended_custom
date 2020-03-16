//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_SYRINGEGUN_H
#define TF_WEAPON_SYRINGEGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFSyringeGun C_TFSyringeGun
#define	CTFCrossBow C_TFCrossBow
#endif

//=============================================================================
//
// TF Weapon Syringe gun.
//
class CTFSyringeGun : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFSyringeGun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFSyringeGun() {}
	~CTFSyringeGun() {}

	virtual void	Precache();
	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SYRINGEGUN_MEDIC; }
private:

	CTFSyringeGun( const CTFSyringeGun & ) {}
};

class CTFCrossBow : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFCrossBow, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCrossBow() {}
	~CTFCrossBow() {}

	virtual void Precache();
	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_CROSSBOW; }

	virtual void	SecondaryAttack( void );
	virtual void	WeaponRegenerate();

	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	ItemPostFrame( void );

	virtual float	GetProjectileGravity( void ) { return 0.20f; }
	virtual float	GetProjectileSpeed( void ) { return 2600.0f; }

private:

	CTFCrossBow(const CTFCrossBow &) {}
};
#endif // TF_WEAPON_SYRINGEGUN_H
