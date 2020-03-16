//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_TFC_SNIPERRIFLE_H
#define WEAPON_TFC_SNIPERRIFLE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_sniperrifle.h"


#if defined( CLIENT_DLL )

#define CTFCSniperrifle C_TFCSniperrifle

#endif


// ----------------------------------------------------------------------------- //
// TFC SniperRifle class definition.
// ----------------------------------------------------------------------------- //

class CTFCSniperrifle : public CTFSniperRifleClassic
{
public:
	DECLARE_CLASS(CTFCSniperrifle, CTFSniperRifleClassic);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCSniperrifle();
	virtual int			GetWeaponID(void) const { return TF_WEAPON_TFC_SNIPERRIFLE; }

private:

	CTFCSniperrifle(const CTFCSniperrifle &) {}
};

#endif // TF_WEAPON_TFC_SNIPERRIFLE_H