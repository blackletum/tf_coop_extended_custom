//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TF_WEAPON_TFC_SUPER_NAILGUN_H
#define TF_WEAPON_TFC_SUPER_NAILGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_tfc_nailgun.h"

#ifdef CLIENT_DLL
	#define CTFCSuperNailgun C_TFCSuperNailgun
#endif


// ----------------------------------------------------------------------------- //
// CTFCSuperNailgun class definition.
// ----------------------------------------------------------------------------- //
class CTFCSuperNailgun : public CTFCNailgun
{
public:
	DECLARE_CLASS( CTFCSuperNailgun, CTFCNailgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFCSuperNailgun();

	virtual int		GetWeaponID( void ) const { return TF_WEAPON_TFC_SUPER_NAILGUN; }

private:
	
	CTFCSuperNailgun( const CTFCSuperNailgun & ) {}

};


#endif // TF_WEAPON_TFC_SUPER_NAILGUN_H
