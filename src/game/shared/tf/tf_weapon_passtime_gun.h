//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_WEAPON_PASSTIME_GUN_H
#define TF_WEAPON_PASSTIME_GUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

#ifdef CLIENT_DLL
#define CPasstimeGun C_PasstimeGun
#endif

//=============================================================================
//
// PASS Time Gun class.
//
class CPasstimeGun : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CPasstimeGun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CPasstimeGun();
	~CPasstimeGun();

	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_PASSTIME_GUN; }
	virtual void		SecondaryAttack();

	virtual void		Precache( void );

	bool				Deploy( void );
	virtual void		ItemPostFrame( void );

	void				CalcLaunch( CTFPlayer *pOwner, bool bDunno );
	void				Throw( CTFPlayer *pOwner );

	virtual bool		CanBeSelected( void )				{ return false; }
	virtual bool		CanDeploy( void )					{ return true; }
	virtual bool		CanHolster( void ) const			{ return false; }
	virtual bool		VisibleInWeaponSelection( void )	{ return false; }

	/*ActivityList(int&)
	BValidPassTarget(CTFPlayer*, CTFPlayer*, HudNotification_t*)
	CanBeSelected()
	CanCharge()
	EThrowState const& CNetworkVarBase<EThrowState, NetworkVar_m_eThrowState>::operator=<EThrowState>(EThrowState const&)
	Equip(CBaseCombatCharacter*)
	GetChargeBeginTime()
	GetChargeMaxTime()
	GetCurrentCharge()
	GetDrawActivity()
	GetServerClass()
	GetWorldModel() const
	HasPrimaryAmmo()
	Holster(CBaseCombatWeapon*)
	IsEnergyWeapon() const
	ItemHolsterFrame()
	SendWeaponAnim(int)
	Spawn()
	UpdateOnRemove()
	VisibleInWeaponSelection()
	WeaponReset()*/

	// enum EThrowState
private:

	//CNetworkVar( EThrowState, m_eThrowState );
	//CNetworkVar( float, m_fChargeBeginTime );

	CPasstimeGun( const CPasstimeGun & ) {}
};

#endif // TF_WEAPON_PASSTIME_GUN_H
