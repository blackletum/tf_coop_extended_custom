//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tf_weapon_tfc_nailgun.h"
#include "tf_projectile_tfc_nail.h"

#if defined( CLIENT_DLL )
	#include "c_tf_player.h"
#else
	#include "tf_player.h"
#endif


CREATE_SIMPLE_WEAPON_TABLE( TFCNailgun, tf_weapon_tfc_nailgun )	// tf_weapon_ng
CREATE_SIMPLE_WEAPON_TABLE( TFCRailgun, tf_weapon_tfc_railgun )

// ----------------------------------------------------------------------------- //
// CTFCNailgun implementation.
// ----------------------------------------------------------------------------- //
CTFCNailgun::CTFCNailgun()
{
}

// ----------------------------------------------------------------------------- //
// CTFCRailgun implementation.
// ----------------------------------------------------------------------------- //
CTFCRailgun::CTFCRailgun()
{
}