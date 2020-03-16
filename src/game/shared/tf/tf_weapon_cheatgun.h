//=============================================================================
//
// Purpose: KITCHEN GUN
//
//=============================================================================
#ifndef TF_WEAPON_CHEATGUN_H
#define TF_WEAPON_CHEATGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFCheatGun C_TFCheatGun
#endif

//=============================================================================
//
// TF Weapon Cheat Gun.
//
class CTFCheatGun : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFCheatGun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS();

	enum
	{
		CHEATMODE_KITCHEN,
		CHEATMODE_MOVE,
		CHEATMODE_SPAWN,
		CHEATMODE_RESIZER,
		CHEATMODE_DECAL,
		CHEATMODE_KILL,
		CHEATMODE_ADDCOND,
		CHEATMODE_COUNT
	};

	CTFCheatGun();
	~CTFCheatGun() {}

	virtual int		GetWeaponID( void ) const	{ return TF_WEAPON_CHEATGUN; }

	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual void	TertiaryAttack( void );
	void			CycleCheatMode( void );
	void			CycleKitchenType( void );

	virtual void	Precache( void );
	virtual void	ItemPostFrame( void );
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );

	void			SetSpawnEnt( CBaseEntity *pTarget );
	void			UpdateTargetTrace( void );
	void			CopyTargetEnt( CBaseEntity *pTarget );
	void			RemoveTarget( void );

	virtual float	GetProjectileDamage( void );
	virtual int		GetWeaponProjectileType( void ) const;
	virtual int		GetActivityWeaponRole( void );

	virtual void	UpdateOnRemove( void );
	bool			OverrideViewAngles( void );
#ifdef CLIENT_DLL
	virtual int		KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );

	void			ParticleTrace( int iCheatMode = CHEATMODE_KITCHEN );
#endif
private:
	CNetworkHandle( CBaseEntity, m_hTraceEnt );
	CNetworkHandle( CBaseEntity, m_hTargetEnt );

	CNetworkVar( int, m_iCheatMode );
	CNetworkVar( int, m_iKitchenProjectile );
	CNetworkVar( float, m_flTraceDistance );

	CNetworkVector( m_vecTraceEnt );

#ifdef CLIENT_DLL
	CNewParticleEffect			*m_pTracer;
#endif
};

#endif // TF_WEAPON_CHEATGUN_H