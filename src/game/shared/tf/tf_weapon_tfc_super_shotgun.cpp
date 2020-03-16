//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_tfc_super_shotgun.h"
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
// Weapon TFC Super Shotgun tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFSuperShotgunTFC, tf_weapon_tfc_super_shotgun )

//=============================================================================
//
// Weapon TFC SuperShotgun functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFSuperShotgunTFC::CTFSuperShotgunTFC()
{
}
