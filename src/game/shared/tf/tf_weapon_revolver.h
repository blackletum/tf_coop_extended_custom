//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#ifndef TF_WEAPON_REVOLVER_H
#define TF_WEAPON_REVOLVER_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFRevolver C_TFRevolver
#endif

//=============================================================================
//
// TF Weapon Revolver.
//
class CTFRevolver : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFRevolver, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFRevolver() {}
	~CTFRevolver() {}

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_REVOLVER; }

	virtual void	PrimaryAttack( void );

	virtual int		GetDamageType() const;
	virtual bool	CanFireCriticalShot( CBaseEntity *pEntity, bool bIsHeadshot = false );

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchTo );
	virtual void	Detach( void );

#ifdef CLIENT_DLL
	virtual int		GetCount( void ) const;

	virtual const char *GetEffectLabelText( void ) { return "#TF_CRITS"; }
#endif
private:
	float flDistanceToTarget;

	CTFRevolver( const CTFRevolver & ) {}
};

#endif // TF_WEAPON_REVOLVER_H