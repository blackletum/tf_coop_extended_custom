//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// TF Rocket Launcher
//
//=============================================================================
#ifndef TF_WEAPON_PARTICLE_CANNON_H
#define TF_WEAPON_PARTICLE_CANNON_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"
#include "tf_weaponbase_rocket.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFParticleCannon C_TFParticleCannon
#endif

//=============================================================================
//
// TF Weapon Particle Cannon.
//
class CTFParticleCannon : public CTFWeaponBaseGun, public ITFChargeUpWeapon
{
public:

	DECLARE_CLASS( CTFParticleCannon, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFParticleCannon();
	~CTFParticleCannon();

	virtual void	Precache();
	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_PARTICLE_CANNON; }

	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();
	virtual CBaseEntity *FireProjectile( CTFPlayer *pPlayer );
	virtual void	ItemPostFrame( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual bool	Deploy( void );
	virtual void	WeaponReset( void );

	virtual float	Energy_GetShotCost( void ) const;
	virtual bool	Energy_FullyCharged( void ) const;

	virtual bool	IsEnergyWeapon( void ) const { return true; }

	virtual const char *GetShootSound( int iIndex ) const;
	virtual void	PlayWeaponShootSound( void );

	virtual float	GetChargeBeginTime( void );
	virtual float	GetChargeMaxTime( void );
	virtual const char* GetChargeSound( void ) { return NULL; }

	virtual bool	OwnerCanTaunt( void );

	virtual float	GetProjectileSpeed( void );
	virtual float	GetProjectileGravity( void );

	virtual const char *GetMuzzleFlashParticleEffect( void );

	//virtual float	GetProgress( void ) const { return BaseClass::GetProgress() };

#ifdef GAME_DLL
	void			FireChargedShot( void );
	virtual float	GetAfterburnRateOnHit( void ) const { return 6.0f; }
#else

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	CreateMuzzleFlashEffects( C_BaseEntity *pAttachEnt, int nIndex );
	void			CreateChargeEffect( void );

	virtual const char* GetEffectLabelText( void )	{ return "#TF_MANGLER"; }

	virtual bool	ShouldPlayClientReloadSound( void ) { return true; }
#endif

	/*GetChargeForceReleaseTime()
	GetInitialAfterburnDuration() const
	IsViewModelFlipped()
	ModifyProjectile(CBaseEntity*)
	ShouldPlayFireAnim()*/

/*
#ifdef CLIENT_DLL
	ClientEffectsThink()
	DispatchMuzzleFlash(char const*, C_BaseEntity*)
#endif
*/

private:
	CNetworkVar( float, m_flChargeBeginTime );
	//CNetworkVar( int, m_iChargeEffect );

	CTFParticleCannon( const CTFParticleCannon & ) {}
};


#endif // TF_WEAPON_PARTICLE_CANNON_H