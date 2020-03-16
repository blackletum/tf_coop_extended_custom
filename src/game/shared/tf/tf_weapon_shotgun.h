//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_SHOTGUN_H
#define TF_WEAPON_SHOTGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

#if defined( CLIENT_DLL )
#define CTFShotgun C_TFShotgun
#define CTFShotgun_Revenge C_TFShotgun_Revenge
#define CTFShotgun_Soldier C_TFShotgun_Soldier
#define CTFShotgun_HWG C_TFShotgun_HWG
#define CTFShotgun_Pyro C_TFShotgun_Pyro
#define CTFScatterGun C_TFScatterGun
#define CTFSodaPopper C_TFSodaPopper
#define CTFPEPBrawlerBlaster C_TFPEPBrawlerBlaster
#define CTFShotgunBuildingRescue C_TFShotgunBuildingRescue
#define CTFDRGPomson C_TFDRGPomson
#endif

// Reload Modes
enum
{
	TF_WEAPON_SHOTGUN_RELOAD_START = 0,
	TF_WEAPON_SHOTGUN_RELOAD_SHELL,
	TF_WEAPON_SHOTGUN_RELOAD_CONTINUE,
	TF_WEAPON_SHOTGUN_RELOAD_FINISH
};

//=============================================================================
//
// Shotgun class.
//
class CTFShotgun : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFShotgun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFShotgun();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SHOTGUN_PRIMARY; }
	virtual void	PrimaryAttack();

protected:

	void		Fire( CTFPlayer *pPlayer );
	void		UpdatePunchAngles( CTFPlayer *pPlayer );

private:

	CTFShotgun( const CTFShotgun & ) {}
};

// Scout version. Different models, possibly different behaviour later on
class CTFScatterGun : public CTFShotgun
{
public:
	DECLARE_CLASS( CTFScatterGun, CTFShotgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SCATTERGUN; }

	virtual bool    HasKnockback( void );

	virtual bool	Reload( void );
	virtual void	FinishReload( void );

	//virtual void	ApplyPostHitEffects( const CTakeDamageInfo &info, CBaseEntity *pVictim );
	virtual void	Equip( CBaseCombatCharacter *pOwner );

	virtual void	FireBullet( CTFPlayer *pPlayer );

	virtual bool	SendWeaponAnim( int iActivity );
};

//void CanScatterGunKnockBack(CTFWeaponBase* , float distance, float force);

class CTFSodaPopper : public CTFScatterGun
{
public:
	DECLARE_CLASS( CTFSodaPopper, CTFScatterGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SODA_POPPER; }

	virtual void	SecondaryAttack();
	virtual void	ItemBusyFrame( void );

	virtual float	GetEffectBarProgress( void );
	virtual const char *GetEffectLabelText( void ) { return "#TF_Hype"; }
};

class CTFPEPBrawlerBlaster : public CTFScatterGun
{
public:
	DECLARE_CLASS( CTFPEPBrawlerBlaster, CTFScatterGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_PEP_BRAWLER_BLASTER; }

	virtual float	GetEffectBarProgress( void );
	virtual const char *GetEffectLabelText( void ) { return "#TF_Boost"; }
};


class CTFShotgun_Soldier : public CTFShotgun
{
public:
	DECLARE_CLASS( CTFShotgun_Soldier, CTFShotgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SHOTGUN_SOLDIER; }
};

// Secondary version. Different weapon slot, different ammo
class CTFShotgun_HWG : public CTFShotgun
{
public:
	DECLARE_CLASS( CTFShotgun_HWG, CTFShotgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SHOTGUN_HWG; }
};

class CTFShotgun_Pyro : public CTFShotgun
{
public:
	DECLARE_CLASS( CTFShotgun_Pyro, CTFShotgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SHOTGUN_PYRO; }
};


class CTFShotgun_Revenge : public CTFShotgun
{
public:
	DECLARE_CLASS( CTFShotgun_Revenge, CTFShotgun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFShotgun_Revenge();

	virtual void	Precache( void );

#ifdef CLIENT_DLL
	virtual int		GetWorldModelIndex( void );

	virtual void	SetWeaponVisible( bool visible );
#endif

	virtual void	PrimaryAttack( void );
	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SENTRY_REVENGE; }
	virtual int		GetCount( void ) const;

	virtual int		GetCustomDamageType( void ) const;

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchTo );
	virtual void	Detach( void );

	virtual const char *GetEffectLabelText( void ) { return "#TF_Revenge"; }

	virtual void	SentryKilled( int nKills );

	bool			CanGetRevengeCrits( void ) const;

private:

	CNetworkVar( int, m_iRevengeCrits );

	CTFShotgun_Revenge( CTFShotgun_Revenge const& );
};

class CTFShotgunBuildingRescue : public CTFShotgun
{
public:
	DECLARE_CLASS( CTFShotgunBuildingRescue, CTFShotgun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFShotgunBuildingRescue();
	~CTFShotgunBuildingRescue();

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_SHOTGUN_BUILDING_RESCUE; }

	virtual float	GetProjectileGravity( void ) { return 0.20f; }
	virtual float	GetProjectileSpeed( void ) { return 2600.0f; }

private:

	CTFShotgunBuildingRescue( CTFShotgunBuildingRescue const& );
};

class CTFDRGPomson : public CTFShotgun
{
public:
	DECLARE_CLASS( CTFDRGPomson, CTFShotgun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFDRGPomson();
	~CTFDRGPomson();

	virtual void	Precache( void );

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_DRG_POMSON; }

	/*GetIdleParticleEffect()
	GetMuzzleFlashParticleEffect()
	GetProjectileFireSetup(CTFPlayer*, Vector, Vector*, QAngle*, bool, float)*/

private:

	CTFDRGPomson( CTFDRGPomson const& );
};

#endif // TF_WEAPON_SHOTGUN_H
