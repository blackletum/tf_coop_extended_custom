//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_ICTFC_H
#define TF_WEAPON_ICTFC_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_flaregun.h"

#ifdef CLIENT_DLL
#define CTFICTFC C_TFICTFC
#endif

//=============================================================================
//
// ICTFC class.
//
class CTFICTFC : public CTFFlareGun
{
public:

	DECLARE_CLASS( CTFICTFC, CTFFlareGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFICTFC();
	virtual int			GetWeaponID( void ) const { return TF_WEAPON_TFC_INCENDIARYCANNON; }

private:

	CTFICTFC( const CTFICTFC & ) {}
};

#endif // TF_WEAPON_ICTFC_H
