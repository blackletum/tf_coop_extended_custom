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

#include "tf_weapon_pipebomblauncher.h"


#if defined( CLIENT_DLL )

#define CTFCPipebombLauncher C_TFCPipebombLauncher

#endif


// ----------------------------------------------------------------------------- //
// CTFCPipebombLauncher class definition.
// ----------------------------------------------------------------------------- //

class CTFCPipebombLauncher : public CTFPipebombLauncher
{
public:
	DECLARE_CLASS(CTFCPipebombLauncher, CTFPipebombLauncher);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCPipebombLauncher();
	virtual int			GetWeaponID(void) const { return TF_WEAPON_TFC_GRENADELAUNCHER; }

private:

	CTFCPipebombLauncher(const CTFCPipebombLauncher &) {}
};

#endif // TF_WEAPON_TFC_PIPEBOMBLAUNCHER_H