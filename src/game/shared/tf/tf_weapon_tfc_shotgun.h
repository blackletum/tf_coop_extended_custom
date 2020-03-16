//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_SHOTGUN_TFC_H
#define TF_WEAPON_SHOTGUN_TFC_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_shotgun.h"

#ifdef CLIENT_DLL
#define CTFShotgunTFC C_TFShotgunTFC
#endif

//=============================================================================
//
// TFC Shotgun class.
//
class CTFShotgunTFC : public CTFShotgun
{
public:

	DECLARE_CLASS( CTFShotgunTFC, CTFShotgun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFShotgunTFC();
	virtual int			GetWeaponID(void) const { return TF_WEAPON_TFC_SHOTGUN; }

private:

	CTFShotgunTFC(const CTFShotgunTFC&) {}
};

#endif // TF_WEAPON_SHOTGUN_TFC_H
