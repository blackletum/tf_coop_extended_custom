//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. =======
//
// Purpose: A remake of Pyro's Flare Gun from live TF2
//
//=============================================================================
#ifndef TF_WEAPON_FLAREGUN_H
#define TF_WEAPON_FLAREGUN_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

#ifdef CLIENT_DLL
#define CTFFlareGun C_TFFlareGun
//#define CTFFlareGun_Revenge C_TFFlareGun_Revenge
#define CTFProjectile_Flare C_TFProjectile_Flare
#endif

class CTFProjectile_Flare;

class CTFFlareGun : public CTFWeaponBaseGun
{
public:
	DECLARE_CLASS( CTFFlareGun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFFlareGun();

	virtual void	Spawn( void );
	virtual int		GetWeaponID( void ) const { return TF_WEAPON_FLAREGUN; }
	virtual void	ItemPostFrame( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual bool	Deploy( void );

	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();
	virtual void	WeaponReset( void );

	void			AddFlare( CTFProjectile_Flare *pFlare );
	void			DeathNotice( CBaseEntity *pVictim );

#ifdef GAME_DLL
	virtual float	GetAfterburnRateOnHit( void ) const { return 6.0f; }
#else
	virtual bool	ShouldPlayClientReloadSound( void ) { return true; }
#endif

private:
#ifdef GAME_DLL
	CNetworkVar( int,				m_iFlareCount );	
#else
	int				m_iFlareCount;
#endif

	// List of active pipebombs
	typedef CHandle<CTFProjectile_Flare>	FlareHandle;
	CUtlVector<FlareHandle>		m_Flares;

};

#endif // TF_WEAPON_FLAREGUN_H
