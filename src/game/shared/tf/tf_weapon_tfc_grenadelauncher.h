//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_TFC_PIPEBOMBLAUNCHER_H
#define WEAPON_TFC_PIPEBOMBLAUNCHER_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_grenadelauncher.h"


#if defined( CLIENT_DLL )

#define CTFCGrenadeLauncher C_TFCGrenadeLauncher

#endif


// ----------------------------------------------------------------------------- //
// CTFCGrenadeLauncher class definition.
// ----------------------------------------------------------------------------- //

class CTFCGrenadeLauncher : public CTFGrenadeLauncher
{
public:
	DECLARE_CLASS(CTFCGrenadeLauncher, CTFGrenadeLauncher);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCGrenadeLauncher();
	virtual int			GetWeaponID(void) const { return TF_WEAPON_TFC_GRENADELAUNCHER; }

private:

	CTFCGrenadeLauncher(const CTFCGrenadeLauncher &) {}
};

#endif // TF_WEAPON_GRENADELAUNCHER_H