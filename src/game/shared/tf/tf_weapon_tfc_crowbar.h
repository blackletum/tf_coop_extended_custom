//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_WEAPON_CROWBAR_H
#define TF_WEAPON_CROWBAR_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFCCrowbar C_TFCCrowbar
#define CTFCUmbrella C_TFCUmbrella
#endif

//=============================================================================
//
// Crowbar class.
//
class CTFCCrowbar : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFCCrowbar, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCCrowbar();
	virtual int			GetWeaponID( void ) const { return TF_WEAPON_TFC_CROWBAR; }
	virtual void	OnSwingHit(trace_t &trace);

private:

	CTFCCrowbar( const CTFCCrowbar & ) {}
};

class CTFCUmbrella : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFCUmbrella, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCUmbrella();
	virtual int			GetWeaponID( void ) const { return TF_WEAPON_TFC_UMBRELLA; }

private:

	CTFCUmbrella( const CTFCUmbrella & ) {}
};

#endif // TF_WEAPON_CROWBAR_H
