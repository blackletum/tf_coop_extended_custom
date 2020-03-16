//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_WEAPON_TFC_NAILGUN_H
#define TF_WEAPON_TFC_NAILGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

#if defined( CLIENT_DLL )
	#define CTFCNailgun C_TFCNailgun
	#define CTFCRailgun C_TFCRailgun
#endif

// -----------------------------------------------------------------------------//
// CTFCNailgun class definition.
// -----------------------------------------------------------------------------//
class CTFCNailgun : public CTFWeaponBaseGun
{
public:
	DECLARE_CLASS( CTFCNailgun, CTFWeaponBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCNailgun();

	virtual int		GetWeaponID( void ) const { return TF_WEAPON_TFC_NAILGUN; }

private:
	
	CTFCNailgun( const CTFCNailgun & ) {}

};

// -----------------------------------------------------------------------------//
// CTFCNailgun class definition.
// -----------------------------------------------------------------------------//
class CTFCRailgun : public CTFCNailgun
{
public:
	DECLARE_CLASS( CTFCRailgun, CTFCNailgun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCRailgun();

	virtual int		GetWeaponID( void ) const { return TF_WEAPON_TFC_RAILGUN; }

private:
	
	CTFCRailgun( const CTFCRailgun & ) {}

};


#endif // TF_WEAPON_TFC_NAILGUN_H
