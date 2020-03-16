//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_tfc_grenadelauncher.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Club tables.
//
CREATE_SIMPLE_WEAPON_TABLE(TFCGrenadeLauncher, tf_weapon_tfc_grenadelauncher)

//=============================================================================
//
// Weapon Club functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFCGrenadeLauncher::CTFCGrenadeLauncher()
{
}
