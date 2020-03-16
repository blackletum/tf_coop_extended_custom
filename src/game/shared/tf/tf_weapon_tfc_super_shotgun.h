//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_SuperShotgun_H
#define TF_WEAPON_SuperShotgun_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_shotgun.h"

#ifdef CLIENT_DLL
#define CTFSuperShotgunTFC C_TFSuperShotgunTFC
#endif

//=============================================================================
//
// TFC Super Shotgun class.
//
class CTFSuperShotgunTFC : public CTFShotgun
{
public:

	DECLARE_CLASS( CTFSuperShotgunTFC, CTFShotgun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFSuperShotgunTFC();
	virtual int			GetWeaponID( void ) const { return TF_WEAPON_TFC_SUPER_SHOTGUN; }
	
private:

	CTFSuperShotgunTFC( const CTFSuperShotgunTFC & ) {}
};

#endif // TF_WEAPON_SuperShotgun_H
