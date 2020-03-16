//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: Weapon Knife Class
//
//=============================================================================
#ifndef TF_WEAPON_KNIFE_H
#define TF_WEAPON_KNIFE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFKnife C_TFKnife
#define CTFCKnife C_TFCKnife
#endif

//=============================================================================
//
// Knife class.
//
class CTFKnife : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFKnife, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFKnife();

	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_KNIFE; }

	virtual bool		Deploy( void );
	virtual void		ItemPostFrame( void );
	virtual void		ItemBusyFrame( void );
	virtual void		ItemHolsterFrame( void );
	virtual void		PrimaryAttack( void );

	virtual float		GetMeleeDamage( CBaseEntity *pTarget, int &iDamageType, int &iCustomDamage );

	virtual void		SendPlayerAnimEvent( CTFPlayer *pPlayer );

	bool				IsBehindAndFacingTarget( CBaseEntity *pTarget );

	virtual bool		CalcIsAttackCriticalHelper( void );

	virtual void		DoViewModelAnimation( void );
	virtual bool		SendWeaponAnim( int iActivity );

	void				BackstabVMThink( void );
	void				DisguiseOnKill( void );
	void				BackstabBlocked( void );

#ifdef CLIENT_DLL
	virtual const char* GetEffectLabelText( void )			{ return "#TF_KNIFE"; }
#else
	virtual void		ApplyOnInjuredAttributes( CBaseEntity* pVictim, CTFPlayer* pAttacker, const CTakeDamageInfo &info );
	virtual void		OnPlayerKill( CBaseEntity *pVictim, const CTakeDamageInfo &info );
#endif

	virtual bool		CanDeploy( void );
private:
	EHANDLE				m_hBackstabVictim;
	CNetworkVar( bool, m_bReadyToBackstab );
	CNetworkVar( float, m_flKnifeRegenTime );

	CTFKnife( const CTFKnife & ) {}
};

class CTFCKnife : public CTFKnife
{
public:
	DECLARE_CLASS( CTFCKnife, CTFKnife );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

 	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_TFC_KNIFE; }
};

#endif // TF_WEAPON_KNIFE_H
