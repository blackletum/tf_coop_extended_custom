//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//========================================================================//

#include "cbase.h"
#include "econ_wearable.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_NETWORKCLASS_ALIASED( EconWearable, DT_EconWearable )

BEGIN_NETWORK_TABLE( CEconWearable, DT_EconWearable )
#ifdef GAME_DLL
	SendPropBool( SENDINFO( m_bExtraWearable ) ),
#else
	RecvPropBool( RECVINFO( m_bExtraWearable ) ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconWearable::Spawn( void )
{
	InitializeAttributes();

	Precache();

	if ( m_bExtraWearable && GetItem()->GetStaticData() )
	{
		SetModel( GetItem()->GetStaticData()->extra_wearable );
	}
	else
	{
		SetModel( GetItem()->GetPlayerDisplayModel() );
	}

	BaseClass::Spawn();

	AddEffects( EF_BONEMERGE );
	AddEffects( EF_BONEMERGE_FASTCULL );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	SetBlocksLOS( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CEconWearable::GetSkin( void )
{
	if ( HasItemDefinition() && GetItem() && GetItem()->GetSkin( GetTeamNumber(), false ) > -1 )
		return GetItem()->GetSkin( GetTeamNumber(), false );

	switch ( GetTeamNumber() )
	{
	case TF_TEAM_RED:
		return 0;

	case TF_TEAM_BLUE:
		return 1;

	case TF_TEAM_GREEN:
		return 0;
		
	case TF_TEAM_YELLOW:
		return 0;

	default:
		return 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconWearable::UpdateWearableBodyGroups( CBaseEntity *pEntity )
{
	PerTeamVisuals_t *visual = GetItem()->GetStaticData()->GetVisuals( GetTeamNumber() );
	for ( unsigned int i = 0; i < visual->player_bodygroups.Count(); i++ )
	{
		const char *szBodyGroupName = visual->player_bodygroups.GetElementName( i );

		if ( szBodyGroupName )
		{
			if ( pEntity->IsPlayer() )
			{
				CBasePlayer *pPlayer = ToBasePlayer( pEntity );
				if ( pPlayer )
				{
					int iBodyGroup = pPlayer->FindBodygroupByName( szBodyGroupName );
					int iBodyGroupValue = visual->player_bodygroups.Element( i );

					pPlayer->SetBodygroup( iBodyGroup, iBodyGroupValue );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconWearable::GiveTo( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

#ifdef GAME_DLL
	CBaseCombatCharacter *pBCC = pEntity->MyCombatCharacterPointer();
	if ( pBCC )
		pBCC->EquipWearable( this );
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconWearable::Equip( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	FollowEntity( pEntity, true );
	SetOwnerEntity( pEntity );
	ChangeTeam( pEntity->GetTeamNumber() );

	// Extra wearables don't provide attribute bonuses
	if ( !IsExtraWearable() )
		ReapplyProvision();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconWearable::UnEquip( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	StopFollowingEntity();

	SetOwnerEntity( NULL );
	ReapplyProvision();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconWearable::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	if ( pPlayer )
	{
		//m_OnPlayerUse.FireOutput( pActivator, pCaller );

		GiveTo( pPlayer );
	}
}
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconWearable::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ShadowType_t CEconWearable::ShadowCastType( void )
{
	if ( ShouldDraw() )
	{
		return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
	}

	return SHADOWS_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEconWearable::ShouldDraw( void )
{
	if ( !GetOwnerEntity() )
		return false;

	CBaseCombatCharacter *pOwner = GetOwnerEntity()->MyCombatCharacterPointer();
	if ( pOwner )
	{
		if ( pOwner->IsPlayer() )
		{
			CBasePlayer *pPlayer = ToBasePlayer( pOwner );
			if ( pPlayer && !pPlayer->ShouldDrawThisPlayer() )
				return false;
		}

		if ( !pOwner->IsAlive() )
			return false;
	}

	return BaseClass::ShouldDraw();
}

#endif
