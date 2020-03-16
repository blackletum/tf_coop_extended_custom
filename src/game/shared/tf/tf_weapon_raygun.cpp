//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// TF Ray Gun
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_raygun.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#else
#include "tf_player.h"
#include "ai_basenpc.h"
#include "ilagcompensationmanager.h"
#endif


CREATE_SIMPLE_WEAPON_TABLE( TFBison, tf_weapon_raygun )

/*
//=============================================================================
//
// Weapon Ray Gun tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFRayGun, DT_WeaponRaygun )

BEGIN_NETWORK_TABLE( CTFRayGun, DT_WeaponRaygun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFRayGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_raygun, CTFRayGun );
PRECACHE_WEAPON_REGISTER( tf_weapon_raygun );

//=============================================================================
//
// Weapon Ray Gun functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFRaygun::CTFRaygun()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFRaygun::~CTFRaygun()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFRaygun::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();
}*/