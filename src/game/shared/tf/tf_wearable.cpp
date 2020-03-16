#include "cbase.h"
#include "tf_wearable.h"

#ifdef GAME_DLL
#include "tf_player.h"
#include "ai_basenpc.h"
#else
#include "c_tf_player.h"
#include "c_ai_basenpc.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( TFWearable, DT_TFWearable );

BEGIN_NETWORK_TABLE( CTFWearable, DT_TFWearable )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_wearable, CTFWearable );
PRECACHE_REGISTER( tf_wearable );

IMPLEMENT_NETWORKCLASS_ALIASED( TFWearableVM, DT_TFWearableVM );

BEGIN_NETWORK_TABLE( CTFWearable, DT_TFWearableVM )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_wearable_vm, CTFWearableVM );
PRECACHE_REGISTER( tf_wearable_vm );

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWearable::Equip( CBaseEntity *pEntity )
{
	BaseClass::Equip( pEntity );
	UpdateModelToClass();

	// player_bodygroups
	UpdatePlayerBodygroups();
}

//---------------------------------------------------------------------------- -
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWearable::UpdateModelToClass( void )
{
	if ( m_bExtraWearable && GetItem()->GetStaticData() )
	{
		SetModel( GetItem()->GetStaticData()->extra_wearable );
		string_t strUnusual = NULL_STRING;
		CALL_ATTRIB_HOOK_STRING( strUnusual, set_attached_particle_name );
		if ( strUnusual != NULL_STRING )
			SetParticle( STRING( strUnusual ) );
	}
	else
	{
		CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
		if ( pOwner )
		{
			const char *pszModel = GetItem()->GetPlayerDisplayModel( pOwner->GetPlayerClass()->GetClassIndex() );
			if ( pszModel[0] != '\0' )
			{
				SetModel( pszModel );
			string_t strUnusual = NULL_STRING;
			CALL_ATTRIB_HOOK_STRING( strUnusual, set_attached_particle_name );
			if ( strUnusual != NULL_STRING )
				SetParticle( STRING( strUnusual ) );
			}
		}
	}
}

#else

int C_TFWearable::InternalDrawModel( int flags )
{
	C_TFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	bool bNotViewModel = ( ( pOwner && !pOwner->IsLocalPlayer() ) || C_BasePlayer::ShouldDrawLocalPlayer() );
	bool bUseInvulnMaterial = ( bNotViewModel && pOwner && pOwner->m_Shared.InCond( TF_COND_INVULNERABLE ) );
	if ( bUseInvulnMaterial )
		modelrender->ForcedMaterialOverride( *pOwner->GetInvulnMaterialRef() );

	int ret = BaseClass::InternalDrawModel( flags );

	if ( bUseInvulnMaterial )
		modelrender->ForcedMaterialOverride( NULL );

	return ret;
}

#endif
