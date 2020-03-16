//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_WEAPON_MEDIKIT_TFC_H
#define TF_WEAPON_MEDIKIT_TFC_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#if defined( CLIENT_DLL )
	#define CTFCMedikit C_TFCMedikit
#endif

// ----------------------------------------------------------------------------- //
// CTFCMedikit class definition.
// ----------------------------------------------------------------------------- //
class CTFCMedikit : public CTFWeaponBaseMelee
{
public:
	DECLARE_CLASS( CTFCMedikit, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFCMedikit();

	virtual int	GetWeaponID( void ) const			{ return TF_WEAPON_TFC_MEDIKIT; }
	virtual void		Smack( void );

private:
	CTFCMedikit( const CTFCMedikit & ) {}
};


#endif // TF_WEAPON_MEDIKIT_TFC
