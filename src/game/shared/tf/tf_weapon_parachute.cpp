//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_parachute.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "tf_wearable.h"
#include "tf_projectile_arrow.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_team.h"
#endif

//=============================================================================
//
// Weapon Parachute tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFParachute, DT_TFParachute )

BEGIN_NETWORK_TABLE( CTFParachute, DT_TFParachute )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFParachute )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_parachute, CTFParachute );
PRECACHE_WEAPON_REGISTER( tf_weapon_parachute );

//=============================================================================
//
// Weapon Parachute functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFParachute::CTFParachute()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFParachute::~CTFParachute()
{
}

//-----------------------------------------------------------------------------
// Purpose: Create the banner addon for the backpack
//-----------------------------------------------------------------------------
void CTFParachute::CreateBanner( int iBuffType )
{
#ifdef CLIENT_DLL
	C_TFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner || !pOwner->IsAlive() )
		return;

	for ( int i = 0; i < pOwner->GetNumWearables(); i++ )
	{
		CTFWearable *pWearable = static_cast<CTFWearable *>( pOwner->GetWearable( i ) );
		if ( !pWearable )
			continue;

		if ( !pWearable->IsExtraWearable() )
			continue;

		// here's the arrow for banner
		C_TFProjectile_Arrow *pBanner = new C_TFProjectile_Arrow();
		if ( pBanner )
		{
			pBanner->InitializeAsClientEntity( "models/workshop/weapons/c_models/c_paratooper_pack/c_paratrooper_parachute.mdl", RENDER_GROUP_OPAQUE_ENTITY );

			// Attach the flag to the backpack
			int bone = pWearable->LookupBone( "bip_spine_3" );
			if ( bone != -1 )
				pBanner->AttachEntityToBone( pWearable, bone );
		}
	}
#endif
}

#ifdef CLIENT_DLL
void CTFParachute::ClientThink( void )
{
	BaseClass::ClientThink();
	ParachuteAnimThink();
}

void CTFParachute::ParachuteAnimThink( void )
{
}
#endif

CREATE_SIMPLE_WEAPON_TABLE( TFParachute_Primary, tf_weapon_parachute_primary )
CREATE_SIMPLE_WEAPON_TABLE( TFParachute_Secondary, tf_weapon_parachute_secondary )
