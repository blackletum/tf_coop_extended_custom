//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_WEAPON_ZOMBIE_CLAW_H
#define TF_WEAPON_ZOMBIE_CLAW_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFZombieClaw C_TFZombieClaw
#endif

//=============================================================================
//
// Zombie Claw class.
//
class CTFZombieClaw : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFZombieClaw, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFZombieClaw();
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_ZOMBIE_CLAW; }
	virtual void		Precache( void );
	virtual void		SecondaryAttack( void );

	virtual bool		HasChargeBar( void );
	virtual float		InternalGetEffectBarRechargeTime( void ) { return 4.0f; }
	virtual const char	*GetEffectLabelText( void ) { return "#LFE_Leap"; }
private:

	CTFZombieClaw( const CTFZombieClaw & ) {}
};

#endif // TF_WEAPON_ZOMBIE_CLAW_H
