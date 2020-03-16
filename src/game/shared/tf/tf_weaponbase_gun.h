//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: Weapon Base Gun 
//
//=============================================================================

#ifndef TF_WEAPONBASE_GUN_H
#define TF_WEAPONBASE_GUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_shareddefs.h"
#include "tf_weaponbase.h"

#if defined( CLIENT_DLL )
#define CTFWeaponBaseGun C_TFWeaponBaseGun
#endif

#define ZOOM_CONTEXT		"ZoomContext"
#define ZOOM_REZOOM_TIME	1.4f

//=============================================================================
//
// Weapon Base Melee Gun
//
class CTFWeaponBaseGun : public CTFWeaponBase
{
public:

	DECLARE_CLASS( CTFWeaponBaseGun, CTFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#if !defined( CLIENT_DLL ) 
	DECLARE_DATADESC();
#endif

	CTFWeaponBaseGun();

	virtual void ItemPostFrame( void );
	virtual void PrimaryAttack();
	virtual void SecondaryAttack( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void WeaponReset( void );

	// Derived classes call this to fire a bullet.
	//bool TFBaseGunFire( void );

	virtual void DoFireEffects();

	void ToggleZoom( void );

	virtual CBaseEntity *FireProjectile( CTFPlayer *pPlayer );
	virtual void		GetProjectileFireSetup( CTFPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward, bool bHitTeammates = true, bool bUseHitboxes = false );
	void				GetProjectileReflectSetup( CTFPlayer *pPlayer, const Vector &vecPos, Vector *vecDeflect, bool bHitTeammates = true, bool bUseHitboxes = false );

	virtual void FireBullet( CTFPlayer *pPlayer );
	CBaseEntity *FireRocket( CTFPlayer *pPlayer, int iType );
	CBaseEntity *FireNail( CTFPlayer *pPlayer, int iSpecificNail );
	CBaseEntity *FirePipeBomb( CTFPlayer *pPlayer, int iType );
 	CBaseEntity *FireFlare( CTFPlayer *pPlayer );
	CBaseEntity *FireArrow( CTFPlayer *pPlayer, ProjectileType_t eType );
	CBaseEntity *FireJar( CTFPlayer *pPlayer, int iType );
	CBaseEntity *FireEnergyBall( CTFPlayer *pPlayer, bool bCharged );
	CBaseEntity *FireGrenade( CTFPlayer *pPlayer );

	CBaseEntity *FireHLGrenade( CTFPlayer *pPlayer, int iType );
	CBaseEntity *FireHLCombineBall( CTFPlayer *pPlayer, int iType );
	CBaseEntity *FireHLAR2Grenade( CTFPlayer *pPlayer, int iType );
	CBaseEntity *FireHLSpit( CTFPlayer *pPlayer );
	CBaseEntity *FireHLBolt( CTFPlayer *pPlayer, int iType );
	CBaseEntity *FireHLMissile( CTFPlayer *pPlayer, int iType );
	CBaseEntity *FireHLHornet( CTFPlayer *pPlayer );

	CBaseEntity *FireTFCNail( CTFPlayer *pPlayer, int iSpecificNail );

	virtual float GetWeaponSpread( void );
	virtual float GetProjectileSpeed( void );
	virtual float GetProjectileGravity( void );
	virtual float GetProjectileDamage( void );
	virtual int	  GetWeaponProjectileType( void ) const;

	void UpdatePunchAngles( CTFPlayer *pPlayer );

	virtual void ZoomIn( void );
	virtual void ZoomOut( void );
	void ZoomOutIn( void );

	virtual void PlayWeaponShootSound( void );

	virtual int GetAmmoPerShot( void ) const;

	virtual void RemoveAmmo( CTFPlayer *pPlayer );

#ifdef GAME_DLL
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
#endif
private:
	CNetworkVar( int, m_iBurstSize );
	CNetworkVar( int, m_iProjectileType );

	CTFWeaponBaseGun( const CTFWeaponBaseGun & );
};

#endif // TF_WEAPONBASE_GUN_H