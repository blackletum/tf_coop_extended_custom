//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_FLAMETHROWER_TFC_H
#define TF_WEAPON_FLAMETHROWER_TFC_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_dragons_fury.h"

#ifdef CLIENT_DLL
#define CTFFlamethrowerTFC C_TFFlamethrowerTFC
#endif

//=============================================================================
//
// TFC Flamethrower class.
//
class CTFFlamethrowerTFC : public CTFWeaponFlameBall
{
public:

	DECLARE_CLASS( CTFFlamethrowerTFC, CTFWeaponFlameBall );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFFlamethrowerTFC();

	virtual int			GetWeaponID( void ) const { return TF_WEAPON_TFC_FLAMETHROWER; }

	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();

	virtual CBaseEntity *FireProjectile( CTFPlayer *pPlayer );

private:

	CTFFlamethrowerTFC( const CTFFlamethrowerTFC & ) {}
};

#endif // TF_WEAPON_FLAMETHROWER_TFC_H
