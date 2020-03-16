//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_WEAPON_ANTLION_CLAW_H
#define TF_WEAPON_ANTLION_CLAW_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFAntlionClaw C_TFAntlionClaw
#endif

//=============================================================================
//
// Club class.
//
class CTFAntlionClaw : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFAntlionClaw, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFAntlionClaw();
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_ANTLION_CLAW; }

private:

	CTFAntlionClaw( const CTFAntlionClaw & ) {}
};

#endif // TF_WEAPON_ANTLION_CLAW_H
