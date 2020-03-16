//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: CTF AmmoPack.
//
//=============================================================================//
#include "cbase.h"
#include "items.h"
#include "tf_gamerules.h"
#include "tf_shareddefs.h"
#include "tf_player.h"
#include "tf_team.h"
#include "engine/IEngineSound.h"
#include "tf_powerup.h"
#include "ai_basenpc.h"
#include "iservervehicle.h"
#include "particle_parse.h"

//=============================================================================
float PackRatios[POWERUP_SIZES] =
{
	0.1,	// TF2C TINY
	0.2,	// SMALL
	0.5,	// MEDIUM
	1.0,	// FULL
};

//=============================================================================
//
// CTF Powerup tables.
//

BEGIN_DATADESC( CTFPowerup )

// Keyfields.
DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),

// Inputs.
DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

// Outputs.

END_DATADESC();

//=============================================================================
//
// CTF Powerup functions.
//

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFPowerup::CTFPowerup()
{
	m_bDisabled = false;
	m_bRespawning = false;

	UseClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerup::Spawn( void )
{
	BaseClass::Precache();
	BaseClass::Spawn();

	BaseClass::SetOriginalSpawnOrigin( GetAbsOrigin() );
	BaseClass::SetOriginalSpawnAngles( GetAbsAngles() );

	VPhysicsDestroyObject();
	SetMoveType( MOVETYPE_NONE );
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );

	if ( m_bDisabled )
	{
		SetDisabled( true );
	}

	m_bRespawning = false;

	ResetSequence( LookupSequence("idle") );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pOther - 
//-----------------------------------------------------------------------------
void CTFPowerup::ItemTouch( CBaseEntity *pOther )
{
	// Vehicles can touch items + pick them up
	if ( pOther->GetServerVehicle() )
	{
		pOther = pOther->GetServerVehicle()->GetPassenger();
		if ( !pOther )
			return;
	}

	// if it's not a thing, ignore
	if ( !pOther->IsCombatCharacter() )
		return;

	CBaseCombatCharacter *pPicker = ToBaseCombatCharacter( pOther );
	if ( !pPicker )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( pPicker );
	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pPicker );

	// Must be a valid pickup scenario (no blocking). Though this is a more expensive
	// check than some that follow, this has to be first Obecause it's the only one
	// that inhibits firing the output OnCacheInteraction.
	// Get our test positions
	Vector vecStartPos;
	IPhysicsObject *pPhysObj = VPhysicsGetObject();
	if ( pPhysObj != NULL )
	{
		// Use the physics hull's center
		QAngle vecAngles;
		pPhysObj->GetPosition( &vecStartPos, &vecAngles );
	}
	else
	{
		// Use the generic bbox center
		vecStartPos = CollisionProp()->WorldSpaceCenter();
	}

	Vector vecEndPos = pPicker->EyePosition();

	// Trace between to see if we're occluded
	/*trace_t tr;
	CTraceFilterSkipTwoEntities filter( pPicker, pItem, COLLISION_GROUP_PLAYER_MOVEMENT );
	UTIL_TraceLine( vecStartPos, vecEndPos, MASK_SOLID, &filter, &tr );
	if ( tr.fraction < 1.0f )
		return;*/

	m_OnCacheInteraction.FireOutput( pOther, this );

	// Can I even pick stuff up?
	if ( !pPicker->IsAllowedToPickupWeapons() )
		return;

	if ( pPicker->IsPlayer() )
	{
		if ( !TFGameRules()->CanHaveItem( pPlayer, this ) )
			return;

		if ( MyTouch( pPlayer ) )
		{
			m_OnPlayerTouch.FireOutput( pOther, this );

			if ( pPlayer->GetTeamNumber() == TF_TEAM_RED )
				m_OnRedPickup.FireOutput( pOther, this );
			else if ( pPlayer->GetTeamNumber() == TF_TEAM_BLUE )
				m_OnBluePickup.FireOutput( pOther, this );
			else if ( pPlayer->GetTeamNumber() == TF_TEAM_GREEN )
				m_OnGreenPickup.FireOutput( pOther, this );
			else if ( pPlayer->GetTeamNumber() == TF_TEAM_YELLOW )
				m_OnYellowPickup.FireOutput( pOther, this );

			if ( m_iszPickupParticle != NULL_STRING )
			{
				CDisablePredictionFiltering disabler;
				DispatchParticleEffect( STRING( m_iszPickupParticle ), GetAbsOrigin(), vec3_angle );
			}

			if ( m_iszPickupSound != NULL_STRING )
				EmitSound( STRING( m_iszPickupSound ) );

			SetTouch( NULL );
			SetThink( NULL );

			// player grabbed the item. 
			TFGameRules()->PlayerGotItem( pPlayer, this );
			if ( TFGameRules()->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
			{
				Respawn(); 
			}
			else
			{
				UTIL_Remove( this );
			}
		}
		/*else if ( gEvilImpulse101 )
		{
			UTIL_Remove( this );
		}*/
	}

	if ( pPicker->IsNPC() )
	{
		if ( NPCTouch( pNPC ) )
		{
			m_OnPlayerTouch.FireOutput( pOther, this );

			if ( pNPC->GetTeamNumber() == TF_TEAM_RED )
				m_OnRedPickup.FireOutput( pOther, this );
			else if ( pNPC->GetTeamNumber() == TF_TEAM_BLUE )
				m_OnBluePickup.FireOutput( pOther, this );
			else if ( pNPC->GetTeamNumber() == TF_TEAM_GREEN )
				m_OnGreenPickup.FireOutput( pOther, this );
			else if ( pNPC->GetTeamNumber() == TF_TEAM_YELLOW )
				m_OnYellowPickup.FireOutput( pOther, this );

			if ( STRING( m_iszPickupParticle ) )
				DispatchParticleEffect( STRING( m_iszPickupParticle ), GetAbsOrigin(), vec3_angle );

			if ( STRING( m_iszPickupSound ) )
				EmitSound( STRING( m_iszPickupSound ) );

			SetTouch( NULL );
			SetThink( NULL );

			// npc grabbed the item.
			if ( TFGameRules()->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
			{
				Respawn(); 
			}
			else
			{
				UTIL_Remove( this );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity* CTFPowerup::Respawn( void )
{
	m_bRespawning = true;
	CBaseEntity *pReturn = BaseClass::Respawn();

	// Override the respawn time
	SetNextThink( gpGlobals->curtime + GetRespawnDelay() );

	return pReturn;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerup::Materialize( void )
{
	if ( !m_bDisabled && IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		EmitSound( "Item.Materialize" );
		RemoveEffects( EF_NODRAW );
	}

	m_bRespawning = false;
	SetTouch( &CItem::ItemTouch );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPowerup::ValidTouch( CBaseEntity *pPicker )
{
	// Is the item enabled?
	if ( IsDisabled() )
		return false;

	// Only touch a live thing.
	if ( !pPicker || !pPicker->IsCombatCharacter() || !pPicker->IsAlive() )
		return false;

	// Team number and does it match?
	int iTeam = GetTeamNumber();
	if ( iTeam && ( pPicker->GetTeamNumber() != iTeam ) )
		return false;

	// Don't collide with the owner for the first portion of our life if we're a lunchbox item
	if ( m_flNextCollideTime > gpGlobals->curtime && pPicker == GetOwnerEntity() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPowerup::MyTouch( CBasePlayer *pPlayer )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPowerup::NPCTouch( CAI_BaseNPC *pNPC )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerup::DropSingleInstance( const Vector &vecVelocity, CBaseCombatCharacter *pOwner, float flUnknown, float flRestTime )
{
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	SetAbsVelocity( vecVelocity );
	SetSolid( SOLID_BBOX );

	if ( flRestTime != 0.0f )
		ActivateWhenAtRest( flRestTime );

	AddSpawnFlags( SF_NORESPAWN );
	
	SetOwnerEntity( pOwner );

	// Remove after 30 seconds.
	SetContextThink( &CBaseEntity::SUB_Remove, gpGlobals->curtime + 30.0f, "PowerupRemoveThink" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPowerup::InputEnable( inputdata_t &inputdata )
{
	SetDisabled( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPowerup::InputDisable( inputdata_t &inputdata )
{
	SetDisabled( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPowerup::IsDisabled( void )
{
	return m_bDisabled;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPowerup::InputToggle( inputdata_t &inputdata )
{
	if ( m_bDisabled )
	{
		SetDisabled( false );
	}
	else
	{
		SetDisabled( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPowerup::SetDisabled( bool bDisabled )
{
	m_bDisabled = bDisabled;

	if ( bDisabled )
	{
		AddEffects( EF_NODRAW );
	}
	else
	{
		// only turn it back on if we're not in the middle of respawning
		if ( !m_bRespawning )
		{
            RemoveEffects( EF_NODRAW );
		}
	}
}


