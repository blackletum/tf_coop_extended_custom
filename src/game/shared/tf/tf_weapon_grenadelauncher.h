//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#ifndef TF_WEAPON_GRENADELAUNCHER_H
#define TF_WEAPON_GRENADELAUNCHER_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"
#include "tf_weaponbase_grenadeproj.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFGrenadeLauncher C_TFGrenadeLauncher
#define CTFCannon C_TFCannon
#endif

//=============================================================================
//
// TF Weapon Grenade Launcher.
//
class CTFGrenadeLauncher : public CTFWeaponBaseGun, public ITFChargeUpWeapon
{
public:

	DECLARE_CLASS( CTFGrenadeLauncher, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFGrenadeLauncher();
	~CTFGrenadeLauncher();

	virtual void	Spawn( void );
	virtual int		GetWeaponID( void ) const;

	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual bool	Deploy( void );
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual void	WeaponIdle( void );
	virtual void	WeaponReset( void );
	virtual bool	Reload( void );
	virtual void	Precache( void );
	virtual void	ItemPostFrame( void );

	virtual float	GetProjectileSpeed( void );

	virtual int		GetMaxClip1( void ) const;
	virtual int		GetDefaultClip1( void ) const;

	// ITFChargeUpWeapon
	virtual float	GetChargeBeginTime( void ) { return m_flDetonateTime; }
	virtual float	GetChargeMaxTime( void );
	virtual const char* GetChargeSound( void ) { return "Weapon_LooseCannon.Charge"; }

	int				GetDetonateMode( void ) const;

	CBaseEntity		*FireProjectileInternal( CTFPlayer *pPlayer );
	void			LaunchGrenade( void );

	bool			CanCharge( void );

	virtual bool	IsBlastImpactWeapon( void ) const { return true; }

	/*AddDonkVictim(CBaseEntity const*)
	FireFullClipAtOnce()
	IsDoubleDonk(CBaseEntity const*) const
	ItemPreFrame()
	LaunchGrenade()
	Misfire()
	PostFire()
	ResetDetonateTime()
	SendWeaponAnim(int)()*/

private:
	CNetworkVar( float, m_flDetonateTime );

	CTFGrenadeLauncher( const CTFGrenadeLauncher & ) {}
};

//=============================================================================
//
// TF Weapon Grenade Launcher.
//
class CTFCannon : public CTFGrenadeLauncher
{
public:

	DECLARE_CLASS( CTFCannon, CTFGrenadeLauncher );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFCannon();
	~CTFCannon();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_CANNON; }


private:
	CTFCannon( const CTFGrenadeLauncher & ) {}
};

#endif // TF_WEAPON_GRENADELAUNCHER_H