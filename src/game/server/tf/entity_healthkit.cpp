//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: CTF HealthKit.
//
//=============================================================================//
#include "cbase.h"
#include "items.h"
#include "tf_shareddefs.h"
#include "tf_player.h"
#include "tf_team.h"
#include "engine/IEngineSound.h"
#include "entity_healthkit.h"
#include "tf_gamestats.h"
#include "ai_basenpc.h"

//=============================================================================
//
// CTF HealthKit defines.
//

#define TF_HEALTHKIT_MODEL			"models/items/healthkit.mdl"
#define TF_HEALTHKIT_PICKUP_SOUND	"HealthKit.Touch"

LINK_ENTITY_TO_CLASS( item_healthkit_full, CHealthKit );
LINK_ENTITY_TO_CLASS( item_healthkit_small, CHealthKitSmall );
LINK_ENTITY_TO_CLASS( item_healthkit_medium, CHealthKitMedium );

//=============================================================================
//
// CTF HealthKit functions.
//

//-----------------------------------------------------------------------------
// Purpose: Spawn function for the healthkit
//-----------------------------------------------------------------------------
void CHealthKit::Spawn( void )
{
	Precache();
	SetModel( GetDefaultPowerupModel() );

	SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( GetDefaultPowerupModel() ) );
	UpdateModelIndexOverrides();

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: vision
//-----------------------------------------------------------------------------
void CHealthKit::UpdateModelIndexOverrides( void ) 
{
	SetModelIndexOverride( VISION_MODE_PYRO, modelinfo->GetModelIndex( "models/items/medkit_large_bday.mdl" ) ); 
	SetModelIndexOverride( VISION_MODE_HALLOWEEN, modelinfo->GetModelIndex( "models/props_halloween/halloween_medkit_large.mdl" ) ); 
}

//-----------------------------------------------------------------------------
// Purpose: Precache function for the healthkit
//-----------------------------------------------------------------------------
void CHealthKit::Precache( void )
{
	PrecacheModel( GetDefaultPowerupModel() );
	PrecacheModel( GetDefaultBirthdayModel() );
	PrecacheModel( GetDefaultHalloweenModel() );

	PrecacheScriptSound( TF_HEALTHKIT_PICKUP_SOUND );
}

//-----------------------------------------------------------------------------
// Purpose: MyTouch function for the healthkit
//-----------------------------------------------------------------------------
bool CHealthKit::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	if ( ValidTouch( pPlayer ) )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
		Assert( pTFPlayer );

		int iHealthToAdd = ceil( pPlayer->GetMaxHealth() * PackRatios[GetPowerupSize()] );
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pTFPlayer, iHealthToAdd, mult_health_frompacks );
		int iHealthRestored = 0;

		// Don't heal the player who dropped this healthkit.
		if ( pTFPlayer != GetOwnerEntity() )
		{
			iHealthRestored = pPlayer->TakeHealth( iHealthToAdd, DMG_GENERIC );

			if ( iHealthRestored )
				bSuccess = true;

			// Restore disguise health.
			if ( pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED ) )
			{
				int iFakeHealthToAdd = ceil( pTFPlayer->m_Shared.GetDisguiseClass() * PackRatios[ GetPowerupSize() ] );
				CTFPlayer *pDisguiseTarget = ToTFPlayer(pTFPlayer->m_Shared.GetDisguiseTarget());
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pDisguiseTarget, iFakeHealthToAdd, mult_health_frompacks );

				if ( pTFPlayer->m_Shared.AddDisguiseHealth( iFakeHealthToAdd, false ) )
					bSuccess = true;
			}

			// Remove any negative conditions whether player got healed or not.
			if ( pTFPlayer->m_Shared.InCond( TF_COND_BURNING ) || pTFPlayer->m_Shared.InCond( TF_COND_BLEEDING ) )
				bSuccess = true;
		}

		if ( bSuccess )
		{
			CSingleUserRecipientFilter user( pPlayer );
			user.MakeReliable();

			UserMessageBegin( user, "ItemPickup" );
			WRITE_STRING( GetClassname() );
			MessageEnd();

			const char *pszSound = TF_HEALTHKIT_PICKUP_SOUND;

			EmitSound( user, entindex(), pszSound );

			CTFPlayer *pTFOwner = ToTFPlayer( GetOwnerEntity() );
			if ( pTFOwner && pTFOwner->InSameTeam( pTFPlayer ) )
			{
				// BONUS DUCKS!
				CTF_GameStats.Event_PlayerAwardBonusPoints( pTFOwner, pPlayer, 1 );
			}

			if ( iHealthRestored )
			{
				pTFPlayer->m_Shared.HealthKitPickupEffects( iHealthRestored );

				// Show healing to the one who dropped the healthkit.
				CBasePlayer *pOwner = ToBasePlayer( GetOwnerEntity() );
				if ( pOwner )
				{
					IGameEvent *event_healed = gameeventmanager->CreateEvent( "player_healed" );
					if ( event_healed )
					{
						event_healed->SetInt( "patient", pPlayer->GetUserID() );
						event_healed->SetInt( "healer", pOwner->GetUserID() );
						event_healed->SetInt( "amount", iHealthRestored );

						gameeventmanager->FireEvent( event_healed );
					}
				}
			}
		}
		else
		{
			// Recharge lunchbox if player's at full health.
			CTFWeaponBase *pLunch = pTFPlayer->Weapon_OwnsThisID( TF_WEAPON_LUNCHBOX );
			if ( pLunch && pLunch->GetEffectBarProgress() < 1.0f )
			{
				pLunch->EffectBarRegenFinished();
				bSuccess = true;
			}
		}
	}

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Purpose: MyTouch function for npc
//-----------------------------------------------------------------------------
bool CHealthKit::NPCTouch( CAI_BaseNPC *pNPC )
{
	bool bSuccess = false;

	if ( !pNPC )
		return false;

	if ( ValidTouch( pNPC ) )
	{
		int iHealthToAdd = pNPC->GetMaxHealth() * PackRatios[GetPowerupSize()];
		int iHealthRestored = 0;

		// Don't heal the npc who dropped this healthkit.
		if ( pNPC != GetOwnerEntity() )
		{
			iHealthRestored = pNPC->TakeHealth( iHealthToAdd, DMG_GENERIC );

			if ( iHealthRestored )
				bSuccess = true;

			// Remove any negative conditions whether npc got healed or not.
			if ( pNPC->InCond( TF_COND_BURNING ) || pNPC->InCond( TF_COND_BLEEDING ) )
				bSuccess = true;
		}

		if ( bSuccess )
		{
			CTFPlayer *pTFOwner = ToTFPlayer( GetOwnerEntity() );
			if ( pTFOwner && pTFOwner->InSameTeam( pNPC ) )
			{
				// BONUS DUCKS!
				CTF_GameStats.Event_PlayerAwardBonusPoints( pTFOwner, pNPC, 1 );
			}

			if ( iHealthRestored )
			{
				pNPC->HealthKitPickupEffects( iHealthRestored );

				// Show healing to the one who dropped the healthkit.
				CBasePlayer *pOwner = ToBasePlayer( GetOwnerEntity() );
				if ( pOwner )
				{
					IGameEvent *event_healed = gameeventmanager->CreateEvent( "npc_healed" );
					if ( event_healed )
					{
						event_healed->SetInt( "patient", pNPC->entindex() );
						event_healed->SetInt( "healer", pOwner->GetUserID() );
						event_healed->SetInt( "amount", iHealthRestored );

						gameeventmanager->FireEvent( event_healed );
					}
				}
			}
		}
	}

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Purpose: vision
//-----------------------------------------------------------------------------
void CHealthKitSmall::UpdateModelIndexOverrides( void ) 
{
	SetModelIndexOverride( VISION_MODE_PYRO, modelinfo->GetModelIndex( "models/items/medkit_small_bday.mdl" ) ); 
	SetModelIndexOverride( VISION_MODE_HALLOWEEN, modelinfo->GetModelIndex( "models/props_halloween/halloween_medkit_small.mdl" ) ); 
}

//-----------------------------------------------------------------------------
// Purpose: vision
//-----------------------------------------------------------------------------
void CHealthKitMedium::UpdateModelIndexOverrides( void ) 
{
	SetModelIndexOverride( VISION_MODE_PYRO, modelinfo->GetModelIndex( "models/items/medkit_medkit_bday.mdl" ) ); 
	SetModelIndexOverride( VISION_MODE_HALLOWEEN, modelinfo->GetModelIndex( "models/props_halloween/halloween_medkit_medium.mdl" ) ); 
}