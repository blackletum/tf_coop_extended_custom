//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. =======
//
// Purpose: A remake of Heavy's Sandvich from live TF2
//
//=============================================================================
#ifndef TF_WEAPON_LUNCHBOX_H
#define TF_WEAPON_LUNCHBOX_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase.h"

#ifdef CLIENT_DLL
#define CTFLunchBox C_TFLunchBox
#define CTFLunchBox_Drink C_TFLunchBox_Drink
#endif

class CTFLunchBox : public CTFWeaponBase
{
public:
	DECLARE_CLASS( CTFLunchBox, CTFWeaponBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFLunchBox();

	virtual int GetWeaponID() const { return TF_WEAPON_LUNCHBOX; }

	virtual bool	ShouldBlockPrimaryFire( void ) { return true; }

	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );

	virtual void	DepleteAmmo( void );

	virtual bool	HasChargeBar( void ) { return true; }
	virtual float	InternalGetEffectBarRechargeTime( void );
	virtual const char	*GetEffectLabelText( void ) { return "#TF_Sandwich"; }
	virtual void	SwitchBodyGroups( void );


#ifdef GAME_DLL
	virtual void	Precache( void );
	virtual void	ApplyBiteEffects( bool bHurt );
#endif

private:
#ifdef GAME_DLL
	EHANDLE m_hDroppedLunch;
#endif

	bool	m_bBitten;
};

class CTFLunchBox_Drink : public CTFLunchBox
{
public:
	DECLARE_CLASS( CTFLunchBox_Drink, CTFLunchBox );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual bool	ShouldBlockPrimaryFire( void ) { return true; }

	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );

	virtual void	DepleteAmmo( void );
	virtual bool	HasChargeBar( void ) { return true; }
	virtual float	InternalGetEffectBarRechargeTime( void ) { return 22.2f; }
	virtual const char	*GetEffectLabelText( void ) { return "#TF_EnergyDrink"; }

#ifdef CLIENT_DLL
	virtual bool		Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual const char* ModifyEventParticles( const char* token );
#endif
};

#endif // TF_WEAPON_LUNCHBOX_H
