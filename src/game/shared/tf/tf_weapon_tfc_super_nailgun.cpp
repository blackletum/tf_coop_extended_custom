//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tf_weapon_tfc_super_nailgun.h"
#include "tf_projectile_tfc_nail.h"

#if defined( CLIENT_DLL )
	#include "c_tf_player.h"
#else
	#include "tf_player.h"
#endif

// ----------------------------------------------------------------------------- //
// CTFCSuperNailgun tables.
// ----------------------------------------------------------------------------- //
CREATE_SIMPLE_WEAPON_TABLE( TFCSuperNailgun, tf_weapon_tfc_super_nailgun )	// tf_weapon_superng

// ----------------------------------------------------------------------------- //
// CTFCSuperNailgun implementation.
// ----------------------------------------------------------------------------- //
CTFCSuperNailgun::CTFCSuperNailgun()
{
}