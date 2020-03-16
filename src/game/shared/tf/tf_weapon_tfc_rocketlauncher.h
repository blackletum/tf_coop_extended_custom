//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_RPGTFC_H
#define TF_WEAPON_RPGTFC_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_rocketlauncher.h"

#ifdef CLIENT_DLL
#define CTFRPGTFC C_TFRPGTFC
#endif

//=============================================================================
//
// RPGTFC class.
//
class CTFRPGTFC : public CTFRocketLauncher
{
public:

	DECLARE_CLASS( CTFRPGTFC, CTFRocketLauncher );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFRPGTFC();
	virtual int			GetWeaponID( void ) const { return TF_WEAPON_TFC_ROCKETLAUNCHER; }

private:

	CTFRPGTFC( const CTFRPGTFC & ) {}
};

#endif // TF_WEAPON_RPGTFC_H
