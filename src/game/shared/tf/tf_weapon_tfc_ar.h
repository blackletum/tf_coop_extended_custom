//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#ifndef TF_WEAPON_TFC_ARTFC_H
#define TF_WEAPON_TFC_ARTFC_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_smg.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFARTFC C_TFARTFC
#endif

//=============================================================================
//
// TFC AR gun.
//
class CTFARTFC : public CTFSMG
{
public:

	DECLARE_CLASS( CTFARTFC, CTFSMG );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFARTFC() {}
	~CTFARTFC() {}

	virtual int		GetWeaponID( void ) const { return TF_WEAPON_TFC_AUTOMATICRIFLE; }

private:

	CTFARTFC( const CTFARTFC & ) {}
};

#endif // TF_WEAPON_TFC_ARTFC_H