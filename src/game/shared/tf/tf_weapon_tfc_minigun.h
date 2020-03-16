//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_TFC_MINIGUN_H
#define WEAPON_TFC_MINIGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_minigun.h"


#if defined( CLIENT_DLL )

#define CTFCMinigun C_TFCMinigun

#endif


// ----------------------------------------------------------------------------- //
// TFC Minigun class definition.
// ----------------------------------------------------------------------------- //

class CTFCMinigun : public CTFMinigun
{
public:
	DECLARE_CLASS(CTFCMinigun, CTFMinigun);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCMinigun();
	virtual int			GetWeaponID(void) const { return TF_WEAPON_TFC_MINIGUN; }

private:

	CTFCMinigun(const CTFCMinigun &) {}
};

#endif // TF_WEAPON_TFC_MINIGUN_H