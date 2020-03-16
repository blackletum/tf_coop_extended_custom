//============ Copyright Valve Corporation, All rights reserved. ===============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "tf_triggers.h"
#include "tf_player.h"
#include "tf_gamerules.h"
#include "tf_gamestats.h"
#include "ai_basenpc.h"
#include "doors.h"
#include "tf_weapon_compound_bow.h"
#include "tf_projectile_arrow.h"
#include "particle_parse.h"
#include "tf_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TF_STUNTYPE_BIGBONK 1
#define TF_STUNTYPE_GHOSTSCARE 2

//=============================================================================
//
// Trigger stun tables.
//

BEGIN_DATADESC( CTriggerStun )

// Keyfields.
DEFINE_KEYFIELD( m_flTriggerDelay, FIELD_FLOAT, "trigger_delay" ),
DEFINE_KEYFIELD( m_flStunDuration, FIELD_TIME, "stun_duration" ),
DEFINE_KEYFIELD( m_flMoveSpeedReduction, FIELD_FLOAT, "move_speed_reduction"),
DEFINE_KEYFIELD( m_iStunType, FIELD_INTEGER, "stun_type"),
DEFINE_KEYFIELD( m_bStunEffects, FIELD_BOOLEAN, "stun_effects"),

// Functions.
DEFINE_FUNCTION( Touch ),

// Inputs.

// Outputs.
DEFINE_OUTPUT( m_outputOnStun, "OnStunPlayer" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_stun, CTriggerStun );

//=============================================================================
//
// Trigger stun functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerStun::Spawn()
{
	BaseClass::Spawn();
	InitTrigger();
	SetNextThink( -1.0f );
	ThinkSet( NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTriggerStun::StunEntity( CBaseEntity *pOther )
{
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	CAI_BaseNPC *pNPC = dynamic_cast< CAI_BaseNPC* >( pOther );

	// Don't stun the player again if they're already stunned
	if ( IsTouching( pOther ) )
	{
		if ( pPlayer && !pPlayer->m_Shared.InCond( TF_COND_STUNNED ) && !pPlayer->m_Shared.InCond( TF_COND_PHASE ) )
		{
			int nStunFlags = 0;

			switch ( m_iStunType )
			{
			case TF_STUNTYPE_BIGBONK:
				nStunFlags = TF_STUNFLAGS_BIGBONK;
				break;
			case TF_STUNTYPE_GHOSTSCARE:
				nStunFlags = TF_STUNFLAGS_GHOSTSCARE;
				break;
			default:
				// No stun specified
				return false;
			}

			if ( !m_bStunEffects )
			{
				// Disable effects
				nStunFlags |= TF_STUNFLAG_NOSOUNDOREFFECT;
			}

			if ( m_flMoveSpeedReduction > 0 )
			{
				nStunFlags |= TF_STUNFLAG_SLOWDOWN;
			}

			pPlayer->m_Shared.StunPlayer( m_flStunDuration, 1.0 - m_flMoveSpeedReduction, 0.0f, nStunFlags, NULL );
			m_outputOnStun.FireOutput( pOther, this );
			m_stunEntities.AddToTail( pOther );

			return true;
		}

		if ( pNPC && !pNPC->InCond( TF_COND_STUNNED ) && !pNPC->InCond( TF_COND_PHASE ) )
		{
			int nStunFlags = 0;

			switch ( m_iStunType )
			{
			case TF_STUNTYPE_BIGBONK:
				nStunFlags = TF_STUNFLAGS_BIGBONK;
				break;
			case TF_STUNTYPE_GHOSTSCARE:
				nStunFlags = TF_STUNFLAGS_GHOSTSCARE;
				break;
			default:
				// No stun specified
				return false;
			}

			if ( !m_bStunEffects )
			{
				// Disable effects
				nStunFlags |= TF_STUNFLAG_NOSOUNDOREFFECT;
			}

			if ( m_flMoveSpeedReduction > 0 )
			{
				nStunFlags |= TF_STUNFLAG_SLOWDOWN;
			}

			pNPC->StunNPC( m_flStunDuration, 1.0 - m_flMoveSpeedReduction, 0.0f, nStunFlags, NULL );
			m_outputOnStun.FireOutput( pOther, this );
			m_stunEntities.AddToTail( pOther );

			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerStun::StunThink()
{
	int iStunCount = 0;

	m_stunEntities.RemoveAll();

	touchlink_t *root = ( touchlink_t * )GetDataObject( TOUCHLINK );
	if ( root )
	{
		for ( touchlink_t *link = root->nextLink; link != root; link = link->nextLink )
		{
			CBaseEntity *pTouch = link->entityTouched;
			if ( pTouch )
			{
				if ( StunEntity( pTouch ) )
				{
					iStunCount++;
				}
			}
		}
	}

	if ( iStunCount > 0 )
	{	
		SetNextThink( gpGlobals->curtime + 0.5 );
	}
	else
	{
		ThinkSet( NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerStun::Touch( CBaseEntity *pOther )
{
	if ( m_pfnThink == NULL )
	{
		SetThink( &CTriggerStun::StunThink );
		SetNextThink( gpGlobals->curtime + m_flTriggerDelay );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerStun::EndTouch( CBaseEntity *pOther )
{
	if ( pOther && IsTouching( pOther ) )
	{
		if ( !m_stunEntities.HasElement( pOther ) )
		{
			StunEntity( pOther );
		}
	}
	BaseClass::EndTouch( pOther );
}

//=============================================================================
//
// Trigger Add Condition functions.
//

BEGIN_DATADESC( CTriggerAddTFPlayerCondition )
	DEFINE_KEYFIELD( m_flDuration, FIELD_FLOAT, "duration" ),
	DEFINE_KEYFIELD( m_nCondition,	FIELD_INTEGER,	"condition" ),

	DEFINE_ENTITYFUNC( StartTouch ),
	DEFINE_ENTITYFUNC( EndTouch ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger_add_tf_player_condition, CTriggerAddTFPlayerCondition );

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerAddTFPlayerCondition::Spawn( void )
{
	BaseClass::Spawn();
	Precache();
	InitTrigger();
}

void CTriggerAddTFPlayerCondition::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerAddTFPlayerCondition::StartTouch( CBaseEntity *pOther )
{
	if ( !m_bDisabled && PassesTriggerFilters( pOther ) )
	{
		if ( pOther->IsPlayer() )
		{
			CTFPlayer *pPlayer = dynamic_cast<CTFPlayer*>( pOther );

			if ( m_flDuration == -1 )
			{
				pPlayer->m_Shared.AddCond( m_nCondition );
				BaseClass::StartTouch( pOther );
			}
			else
			{
				pPlayer->m_Shared.AddCond( m_nCondition, m_flDuration );
				BaseClass::StartTouch( pOther );
			}
		}
		else if ( pOther->IsNPC() )
		{
			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pOther );

			if ( m_flDuration == -1 )
			{
				pNPC->AddCond( m_nCondition );
				BaseClass::StartTouch( pOther );
			}
			else
			{
				pNPC->AddCond( m_nCondition, m_flDuration );
				BaseClass::StartTouch( pOther );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CTriggerAddTFPlayerCondition::EndTouch( CBaseEntity *pOther )
{
	if ( !m_bDisabled && PassesTriggerFilters( pOther ) )
	{
		if ( pOther->IsPlayer() )
		{
			CTFPlayer *pPlayer = dynamic_cast<CTFPlayer*>( pOther );

			if ( m_flDuration == -1 )
			{
				if ( pPlayer )
				{
					if ( !(pPlayer->GetFlags() & FL_DONTTOUCH ) )
						  pPlayer->m_Shared.RemoveCond( m_nCondition );
				}
			}
		}
		else if ( pOther->IsNPC() )
		{
			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>( pOther );

			if ( m_flDuration == -1 )
			{
				if ( pNPC )
				{
					if ( !(pNPC->GetFlags() & FL_DONTTOUCH ) )
						  pNPC->RemoveCond( m_nCondition );
				}
			}
		}
	}

	//BaseClass::EndTouch( pOther );
}

BEGIN_DATADESC( CTriggerRemoveTFPlayerCondition )
	DEFINE_KEYFIELD( m_nCondition,	FIELD_INTEGER,	"condition" ),
	DEFINE_ENTITYFUNC( StartTouch ),
	DEFINE_ENTITYFUNC( EndTouch ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger_remove_tf_player_condition, CTriggerRemoveTFPlayerCondition );

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerRemoveTFPlayerCondition::Spawn( void )
{
	BaseClass::Spawn();
	Precache();
	InitTrigger();
}

void CTriggerRemoveTFPlayerCondition::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerRemoveTFPlayerCondition::StartTouch( CBaseEntity *pOther )
{
	//BaseClass::StartTouch( pOther );

	// If we added him to our list, store the start time
	if ( !m_bDisabled && PassesTriggerFilters( pOther ) )
	{
		if ( pOther->IsPlayer() )
		{
			CTFPlayer *pPlayer = dynamic_cast<CTFPlayer*>( pOther );

			if ( pPlayer )
			{
				if ( !(pPlayer->GetFlags() & FL_DONTTOUCH ) )
					  pPlayer->m_Shared.RemoveCond( m_nCondition );
			}
		}
		else if ( pOther->IsNPC() )
		{
			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>( pOther );

			if ( pNPC )
			{
				if ( !(pNPC->GetFlags() & FL_DONTTOUCH ) )
					  pNPC->RemoveCond( m_nCondition );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CTriggerRemoveTFPlayerCondition::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );
}

//=============================================================================
//
// Trigger Add or Remove Attribute functions.
//

BEGIN_DATADESC( CTriggerAddOrRemoveTFPlayerAttributes )
	DEFINE_KEYFIELD( m_bRemove, FIELD_BOOLEAN, "add_or_remove" ),
	DEFINE_KEYFIELD( m_flDuration, FIELD_FLOAT, "duration" ),
	DEFINE_KEYFIELD( m_flAttributeValue, FIELD_FLOAT, "value" ),
	DEFINE_KEYFIELD( m_iszAttributeName, FIELD_STRING, "attribute_name" ),

	DEFINE_ENTITYFUNC( StartTouch ),
	DEFINE_ENTITYFUNC( EndTouch ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger_add_or_remove_tf_player_attributes, CTriggerAddOrRemoveTFPlayerAttributes );

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerAddOrRemoveTFPlayerAttributes::Spawn( void )
{
	BaseClass::Spawn();
	Precache();
	InitTrigger();
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerAddOrRemoveTFPlayerAttributes::StartTouch( CBaseEntity *pOther )
{
	if ( m_iszAttributeName == NULL_STRING )
		return;

	if ( !m_bDisabled && PassesTriggerFilters( pOther ) )
	{
		if ( pOther->IsPlayer() )
		{
			CTFPlayer *pPlayer = dynamic_cast<CTFPlayer*>( pOther );

			if ( !m_bRemove )
			{
				if ( m_flDuration == -1 )
				{
					pPlayer->AddCustomAttribute( STRING( m_iszAttributeName ), m_flAttributeValue );
					BaseClass::StartTouch( pOther );
				}
				else
				{
					pPlayer->AddCustomAttribute( STRING( m_iszAttributeName ), m_flAttributeValue, m_flDuration );
					BaseClass::StartTouch( pOther );
				}
			}
			else
			{
				pPlayer->RemoveCustomAttribute( STRING( m_iszAttributeName ) );
				BaseClass::StartTouch( pOther );
			}
		}
		else if ( pOther->IsNPC() )
		{
			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pOther );

			if ( !m_bRemove )
			{
				if ( m_flDuration == -1 )
				{
					pNPC->AddCustomAttribute( STRING( m_iszAttributeName ), m_flAttributeValue );
					BaseClass::StartTouch( pOther );
				}
				else
				{
					pNPC->AddCustomAttribute( STRING( m_iszAttributeName ), m_flAttributeValue, m_flDuration );
					BaseClass::StartTouch( pOther );
				}
			}
			else
			{
				pNPC->RemoveCustomAttribute( STRING( m_iszAttributeName ) );
				BaseClass::StartTouch( pOther );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CTriggerAddOrRemoveTFPlayerAttributes::EndTouch( CBaseEntity *pOther )
{
	if ( m_iszAttributeName == NULL_STRING )
		return;

	if ( !m_bDisabled && PassesTriggerFilters( pOther ) )
	{
		if ( pOther->IsPlayer() )
		{
			CTFPlayer *pPlayer = dynamic_cast<CTFPlayer*>( pOther );

			if ( m_flDuration == -1 )
			{
				if ( pPlayer )
				{
					if ( !(pPlayer->GetFlags() & FL_DONTTOUCH ) )
						  pPlayer->RemoveCustomAttribute( STRING( m_iszAttributeName ) );
				}
			}
		}
		else if ( pOther->IsNPC() )
		{
			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>( pOther );

			if ( m_flDuration == -1 )
			{
				if ( pNPC )
				{
					if ( !(pNPC->GetFlags() & FL_DONTTOUCH ) )
						  pNPC->RemoveCustomAttribute( STRING( m_iszAttributeName ) );
				}
			}
		}
	}

	//BaseClass::EndTouch( pOther );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CTriggerTimerDoor )
	DEFINE_KEYFIELD( m_iszDoorName, FIELD_STRING, "door_name" ),
	DEFINE_ENTITYFUNC( StartTouch ),
END_DATADESC();

LINK_ENTITY_TO_CLASS( trigger_timer_door, CTriggerTimerDoor );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTriggerTimerDoor::CTriggerTimerDoor()
{
	/*m_TeamData.SetSize( GetNumberOfTeams() );
	m_bStartTouch = false;
	m_hTrainWatcher = NULL;*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerTimerDoor::Spawn( void )
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerTimerDoor::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch( pOther );
	CBaseDoor *pDoor = dynamic_cast<CBaseDoor *>( gEntList.FindEntityByName( NULL, m_iszDoorName ) );
	if ( pDoor )
	{
		//if ( m_bCapturing )
		//{
			pDoor->DoorGoUp();
		//}
		//else
		//{
		//	pDoor->DoorGoDown();
		//}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerTimerDoor::OnStartCapture( int iTeam )
{
	CBaseDoor *pDoor = dynamic_cast<CBaseDoor *>( gEntList.FindEntityByName( NULL, m_iszDoorName ) );
	if ( pDoor )
	{
		if ( !Q_strnicmp( STRING(gpGlobals->mapname), "mvm_mannhattan", 14 ) )
		{
			TFGameRules()->RandomPlayersSpeakConceptIfAllowed( MP_CONCEPT_MANNHATTAN_GATE_ATK, 1, TF_TEAM_RED );
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerTimerDoor::OnEndCapture( int iTeam )
{
	CBaseDoor *pDoor = dynamic_cast<CBaseDoor *>( gEntList.FindEntityByName( NULL, m_iszDoorName ) );
	if ( pDoor )
	{
		//pDoor->DoorGoDown();

		if ( !Q_strnicmp( STRING(gpGlobals->mapname), "mvm_mannhattan", 14 ) )
		{
			TFGameRules()->RandomPlayersSpeakConceptIfAllowed( MP_CONCEPT_MANNHATTAN_GATE_TAKE, 1, TF_TEAM_RED );
		}
	}
}

//=============================================================================
//
// Trigger ignite arrows tables.
//

BEGIN_DATADESC( CTriggerIgniteArrows )

// Functions.
DEFINE_FUNCTION( Touch ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_ignite_arrows, CTriggerIgniteArrows );

//=============================================================================
//
// Trigger ignite arrows functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerIgniteArrows::Spawn()
{
	BaseClass::Spawn();
	InitTrigger();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerIgniteArrows::Touch( CBaseEntity *pOther )
{
	if ( !m_bDisabled && PassesTriggerFilters( pOther ) )
	{
		CTFPlayer *pPlayer = ToTFPlayer( pOther );
		CTFCompoundBow *pBow = dynamic_cast<CTFCompoundBow *>( pPlayer->GetActiveTFWeapon() );
		if( pBow && !pBow->IsFlameArrow() )
		{
			pBow->SetArrowAlight( true );
		}
	}
}

//=============================================================================
//
// Trigger ignite tables.
//

BEGIN_DATADESC( CTriggerIgnite )
	// Keyfields.
	DEFINE_KEYFIELD( m_flBurnDuration, FIELD_TIME, "burn_duration" ),
	DEFINE_KEYFIELD( m_flDamagePercentPerSecond, FIELD_FLOAT, "damage_percent_per_second" ),
	DEFINE_KEYFIELD( m_iszIgniteParticleName, FIELD_STRING, "ignite_particle_name" ),
	DEFINE_KEYFIELD( m_iszIgniteSoundName, FIELD_STRING, "ignite_sound_name" ),

	// Functions.
	DEFINE_ENTITYFUNC( StartTouch ),
	DEFINE_THINKFUNC( BurnThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_ignite, CTriggerIgnite );

//=============================================================================
//
// Trigger ignite functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerIgnite::Spawn()
{
	BaseClass::Spawn();
	Precache();
	InitTrigger();
	SetThink( &CTriggerIgnite::BurnThink );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

void CTriggerIgnite::Precache( void )
{
	PrecacheParticleSystem( STRING( m_iszIgniteParticleName ) );
	PrecacheScriptSound( STRING( m_iszIgniteSoundName ) );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerIgnite::StartTouch( CBaseEntity *pOther )
{
	if ( !m_bDisabled && PassesTriggerFilters( pOther ) )
	{
		if ( m_iszIgniteParticleName != NULL_STRING )
			DispatchParticleEffect( STRING( m_iszIgniteParticleName ), pOther->GetAbsOrigin(), pOther->GetAbsAngles() );

		if ( m_iszIgniteSoundName != NULL_STRING )
			pOther->EmitSound( STRING( m_iszIgniteSoundName ) );

		BaseClass::StartTouch( pOther );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerIgnite::IgniteEntity( CBaseEntity *pOther )
{
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	CAI_BaseNPC *pNPC = dynamic_cast< CAI_BaseNPC* >( pOther );

	if ( PassesTriggerFilters( pOther ) )
	{
		if ( pPlayer && !pPlayer->m_Shared.InCond( TF_COND_PHASE ) )
		{
			pPlayer->m_Shared.Burn( NULL, NULL, m_flBurnDuration );
			CTakeDamageInfo info( this, this, NULL, m_flDamagePercentPerSecond, DMG_IGNITE | DMG_PREVENT_PHYSICS_FORCE, TF_DMG_CUSTOM_BURNING );
			pPlayer->TakeDamage( info );
		}

		if ( pNPC && !pNPC->InCond( TF_COND_PHASE ) )
		{
			pNPC->Burn( NULL, NULL, m_flBurnDuration );
			CTakeDamageInfo info( this, this, NULL, m_flDamagePercentPerSecond, DMG_IGNITE | DMG_PREVENT_PHYSICS_FORCE, TF_DMG_CUSTOM_BURNING );
			pNPC->TakeDamage( info );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerIgnite::BurnThink()
{
	touchlink_t *root = ( touchlink_t * )GetDataObject( TOUCHLINK );
	if ( root )
	{
		for ( touchlink_t *link = root->nextLink; link != root; link = link->nextLink )
		{
			CBaseEntity *pTouch = link->entityTouched;
			if ( pTouch )
			{
				IgniteEntity( pTouch );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: trigger_coop entity
//-----------------------------------------------------------------------------

const int SF_TRIGGERCOOP_AUTOREMOVE = 0x4000;

LINK_ENTITY_TO_CLASS( trigger_coop, CTriggerCoOp );

BEGIN_DATADESC( CTriggerCoOp )

	// Keyfields.
	DEFINE_KEYFIELD( m_iCountType, FIELD_INTEGER, "CountType" ),
	DEFINE_KEYFIELD( m_iPlayerValue, FIELD_INTEGER, "PlayerValue" ),
	DEFINE_KEYFIELD( m_iUseHud, FIELD_BOOLEAN, "UseHud" ),

	// Outputs
	DEFINE_OUTPUT( m_OnPlayersIn, "OnPlayersIn" ),
	DEFINE_OUTPUT( m_OnPlayersOut, "OnPlayersOut" ),

END_DATADESC()

void CTriggerCoOp::Spawn( void )
{
	InitTrigger();
	
	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS );
}

void CTriggerCoOp::Activate( void )
{
	BaseClass::Activate();
	m_bPlayersOut = false;
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerCoOp::StartTouch( CBaseEntity *pOther )
{
	if ( PassesTriggerFilters( pOther ) && pOther->IsPlayer() )
	{
		EHANDLE hOther;
		hOther = pOther;
		if ( m_iPlayerValue > 0 )
		{
			CTeam *pRedTeam = GetGlobalTeam( TF_STORY_TEAM );
			Assert( pRedTeam );

			if ( m_hTouchingEntities.Find( hOther ) == m_hTouchingEntities.InvalidIndex() )
				m_hTouchingEntities.AddToTail( hOther );

			int nTotalPlayers = pRedTeam->GetNumPlayers();
			int nTouchingPlayers = m_hTouchingEntities.Count();

			if ( m_iCountType == 1 )
			{
				float flTotalPercent = roundf( (float)nTouchingPlayers / (float)nTotalPlayers * 100 );
				float flMinPercent = (float)m_iPlayerValue;
				ConDColorMsg( Color( 77, 116, 85, 255 ), "[TriggerCoOp] Percent: %f/%f\n", flTotalPercent, flMinPercent );
				if ( flTotalPercent >= flMinPercent )
				{
					m_OnPlayersIn.FireOutput( pOther, this );

					for ( int i = 0; i < nTouchingPlayers; i++ )
					{
						CTFPlayer *pTFPlayer = ToTFPlayer( m_hTouchingEntities[i] );
						if ( pTFPlayer )
							CTF_GameStats.Event_PlayerCapturedPoint( pTFPlayer );
					}

					if ( HasSpawnFlags( SF_TRIGGERCOOP_AUTOREMOVE ) )
					{
						UTIL_Remove( this );
					}
				}
			}
			else
			{
				ConDColorMsg( Color( 77, 116, 85, 255 ), "[TriggerCoOp] Count: %i/%i\n", nTouchingPlayers, nTotalPlayers );
				if ( nTouchingPlayers >= nTotalPlayers )
				{
					m_OnPlayersIn.FireOutput( pOther, this );

					for ( int i = 0; i < nTouchingPlayers; i++ )
					{
						CTFPlayer *pTFPlayer = ToTFPlayer( m_hTouchingEntities[i] );
						if ( pTFPlayer )
							CTF_GameStats.Event_PlayerCapturedPoint( pTFPlayer );
					}

					if ( HasSpawnFlags( SF_TRIGGERCOOP_AUTOREMOVE ) )
					{
						UTIL_Remove( this );
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CTriggerCoOp::EndTouch( CBaseEntity *pOther )
{
	if ( IsTouching( pOther ) && pOther->IsPlayer() )
	{
		EHANDLE hOther;
		hOther = pOther;
		m_hTouchingEntities.FindAndRemove( hOther );

		CTeam *pRedTeam = GetGlobalTeam( TF_STORY_TEAM );
		Assert( pRedTeam );
		int nTotalPlayers = pRedTeam->GetNumPlayers();
		if ( m_iCountType == 1 )
		{
			float flTotalPercent = roundf( (float) m_hTouchingEntities.Count() / (float)nTotalPlayers * 100 );
			float flMinPercent = (float)m_iPlayerValue;
			ConDColorMsg( Color( 77, 116, 85, 255 ), "[TriggerCoOp] Percent: %f/%f\n", flTotalPercent, flMinPercent );
			if ( flTotalPercent <= flMinPercent )
			{
				m_OnPlayersOut.FireOutput( pOther, this );
			}
		}
		else
		{
			ConDColorMsg( Color( 77, 116, 85, 255 ), "[TriggerCoOp] Count: %i/%i\n",  m_hTouchingEntities.Count(), nTotalPlayers );
			if ( m_hTouchingEntities.Count() <= nTotalPlayers )
			{
				m_OnPlayersOut.FireOutput( pOther, this );
			}
		}

		// If there are no more entities touching this trigger, fire the lost all touches
		// Loop through the touching entities backwards. Clean out old ones, and look for existing
		bool bFoundOtherTouchee = false;
		int iSize = m_hTouchingEntities.Count();
		for ( int i = iSize-1; i >= 0; i-- )
		{
			EHANDLE hOther;
			hOther = m_hTouchingEntities[i];

			if ( !hOther )
			{
				m_hTouchingEntities.Remove( i );
			}
			else if ( !hOther->IsAlive() )
			{
				m_hTouchingEntities.Remove( i );
			}
			else
			{
				bFoundOtherTouchee = true;
			}
		}

		//FIXME: Without this, triggers fire their EndTouch outputs when they are disabled!
		// Didn't find one?
		if ( !bFoundOtherTouchee /*&& !m_bDisabled*/ )
		{
			m_OnEndTouchAll.FireOutput(pOther, this);
			EndTouchAll();
		}
	}
}
