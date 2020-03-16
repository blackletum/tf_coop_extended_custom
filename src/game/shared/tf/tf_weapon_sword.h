//====== Copyright ? 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_SWORD_H
#define TF_WEAPON_SWORD_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_decapitation.h"

#ifdef CLIENT_DLL
#define CTFSword C_TFSword
#define CTFKatana C_TFKatana
#endif


class CTFSword : public CTFDecapitationMeleeWeaponBase
{
	DECLARE_CLASS( CTFSword, CTFWeaponBaseMelee )
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFSword();
	virtual ~CTFSword();

	virtual void	Precache( void );
	virtual bool	Deploy( void );
	virtual int		GetSwingRange( void ) const;
	virtual int		GetSwordHealthMod( void );
	virtual float	GetSwordSpeedMod( void );

	virtual void	OnDecapitation( CBaseEntity *pVictim );

#ifdef CLIENT_DLL
	virtual const char* GetEffectLabelText( void ) { return "#TF_Berzerk"; }

	int				GetCount( void );

	virtual void	WeaponIdle( void );
#endif
	virtual void	WeaponReset( void );
private:

	CTFSword( const CTFSword& ) {}
};

class CTFKatana : public CTFDecapitationMeleeWeaponBase
{
	DECLARE_CLASS( CTFKatana, CTFWeaponBaseMelee )
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFKatana();
	virtual ~CTFKatana();

	virtual bool	Deploy( void );
	virtual bool	CanHolster( void ) const;
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );

	virtual void	OnDecapitation( CBaseEntity *pVictim );

	virtual float	GetMeleeDamage( CBaseEntity *pTarget, int &iDamageType, int &iCustomDamage );
	virtual int		GetSkinOverride() const;

	virtual int		GetActivityWeaponRole( void );
private:

#ifdef GAME_DLL
	CNetworkVar( bool, m_bIsBloody );
#else
	bool m_bIsBloody;
#endif

	CTFKatana( const CTFKatana& ) {}
};

#endif // TF_WEAPON_SWORD_H