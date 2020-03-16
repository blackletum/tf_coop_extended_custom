//============== Copyright LFE-TEAM Not All rights reserved. ==================//
//
// Purpose: medic become super useful
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "tf_revive.h"

#ifdef CLIENT_DLL
#include "tf_hud_mediccallers.h"
#include "iefx.h"
#include "dlight.h"
#else
#include "tf_gamestats.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#endif

#ifdef CLIENT_DLL
	extern ConVar lfe_muzzlelight;
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFReviveMarker, DT_TFReviveMarker )

BEGIN_NETWORK_TABLE( CTFReviveMarker, DT_TFReviveMarker )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iMarkerHealth ) ),
	RecvPropInt( RECVINFO( m_iMarkerMaxHealth ) ),
	RecvPropEHandle( RECVINFO( m_hOwner ) ),
#else
	SendPropInt( SENDINFO( m_iMarkerHealth ), -1, SPROP_VARINT | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_iMarkerMaxHealth ), 13 ),
	SendPropEHandle( SENDINFO( m_hOwner ) ),
#endif
END_NETWORK_TABLE()

// Server specific.
#ifdef GAME_DLL
BEGIN_DATADESC( CTFReviveMarker )
	DEFINE_THINKFUNC( ReviveThink ),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( entity_revive_marker, CTFReviveMarker );

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFReviveMarker::CTFReviveMarker()
{
#ifdef GAME_DLL
	m_iHealth = 1;
	m_iMarkerHealth = 1;
	m_iMarkerMaxHealth = 150;
#endif
	m_hOwner = NULL;

	UseClientSideAnimation();
}
//-----------------------------------------------------------------------------
// Purpose: Deconstructor.
//-----------------------------------------------------------------------------
CTFReviveMarker::~CTFReviveMarker()
{
	m_hOwner = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFReviveMarker::Spawn()
{
	SetModel( TF_REVIVEMARKER_MODEL );

	BaseClass::Spawn();

	AddEffects( EF_NOSHADOW );
	ResetSequence( LookupSequence( "idle" ) );

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	SetCollisionGroup( COLLISION_GROUP_DEBRIS_TRIGGER ); //
	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_TRIGGER /*| FSOLID_NOT_SOLID */ );
	SetBlocksLOS( false );

	/*VPhysicsInitNormal( SOLID_VPHYSICS, FSOLID_TRIGGER, false );
	if ( VPhysicsGetObject() )
		VPhysicsGetObject()->SetMass( 25.0f );*/

	m_iMarkerHealth = 1;

#ifdef GAME_DLL
	SetElasticity( 0.0f );
	SetGravity( 1.0f );
#endif

	m_takedamage = DAMAGE_EVENTS_ONLY;

	// Setup the think function.
	SetThink( &CTFReviveMarker::ReviveThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFReviveMarker::Precache()
{
	// Precache the models.
	PrecacheModel( TF_REVIVEMARKER_MODEL );

	// Precache the sounds.
	PrecacheScriptSound( "MVM.PlayerRevived" );

	// Precache the particles.
	PrecacheParticleSystem( "speech_revivecall" );
	PrecacheParticleSystem( "speech_revivecall_medium" );
	PrecacheParticleSystem( "speech_revivecall_hard" );
					 
	BaseClass::Precache();
}
#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFReviveMarker::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_FULLCHECK );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFReviveMarker::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

CTFReviveMarker *CTFReviveMarker::Create( CTFPlayer *pOwner )
{
	CTFReviveMarker *pReviveMarker = static_cast<CTFReviveMarker *>( CreateEntityByName( "entity_revive_marker" ) );
	if ( pReviveMarker )
	{
		UTIL_SetOrigin( pReviveMarker, pOwner->GetAbsOrigin() + Vector(0, 0, 32) );
		QAngle vecAngles = pOwner->GetAbsAngles();
		vecAngles[PITCH] = 0.0;
		pReviveMarker->SetAbsAngles( vecAngles );
		DispatchSpawn( pReviveMarker );
		pReviveMarker->SetOwner( pOwner );

		pReviveMarker->m_iMarkerMaxHealth = pOwner->GetMaxHealth() / 2;

	}

	return pReviveMarker;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFReviveMarker::AddMarkerHealth( float flAmount )
{
	float flHealFraction = 4;

	flHealFraction += gpGlobals->frametime * flAmount;

	int nHealthToAdd = (int)flHealFraction;
	if ( nHealthToAdd > 0 )
	{
		flHealFraction -= nHealthToAdd;

		if ( m_iMarkerHealth < m_iMarkerMaxHealth )
			m_iMarkerHealth += nHealthToAdd;
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFReviveMarker::ReviveOwner( void )
{
#ifdef GAME_DLL
	if ( !m_hOwner )
	{
		UTIL_Remove( this );
		return;
	}

	m_hOwner->AllowInstantSpawn();
	m_hOwner->ForceRespawn();

	m_hOwner->SpeakConceptIfAllowed( MP_CONCEPT_RESURRECTED );

	/*IGameEvent *event = gameeventmanager->CreateEvent( "revive_player_complete" );
	if ( event )
	{
		event->SetInt( "entindex", entindex() ); // the healer
		gameeventmanager->FireEvent( event );
	}*/
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFReviveMarker::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	switch ( collisionGroup )
	{
	case COLLISION_GROUP_NPC:
	case COLLISION_GROUP_PROJECTILE:
	case COLLISION_GROUP_WEAPON:
	case COLLISION_GROUP_DOOR_BLOCKER:
	case COLLISION_GROUP_PASSABLE_DOOR:
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFReviveMarker::ReviveThink( void )
{
#ifdef GAME_DLL
	// no owner, delete
	if ( !m_hOwner )
	{
		UTIL_Remove(this);
		return;
	}

	// no team, delete
	if (m_hOwner && m_hOwner->GetTeamNumber() != TF_STORY_TEAM && m_hOwner->GetTeamNumber() != TF_COMBINE_TEAM)
	{
		UTIL_Remove(this);
		return;
	}

	/*if ( m_hOwner->GetDeathTime() > 7 )
		DispatchParticleEffect( "speech_revivecall_hard", PATTACH_POINT_FOLLOW, this, "mediccall", true );
	else if ( m_hOwner->GetDeathTime() > 3 )
		DispatchParticleEffect( "speech_revivecall_medium", PATTACH_POINT_FOLLOW, this, "mediccall", true );
	else
		DispatchParticleEffect( "speech_revivecall", PATTACH_POINT_FOLLOW, this, "mediccall", true );

	if ( something )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "revive_player_stopped" );
		if ( event )
		{
			event->SetInt( "entindex", entindex() ); // marker, medic or the owner?
			gameeventmanager->FireEvent( event );
		}
	}*/
			
#endif

	/*if ( IsReviveInProgress() )
		PromptOwner();*/

	if ( GetHealth() >= GetMaxHealth() )
		ReviveOwner();


	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFReviveMarker::IsReviveInProgress( void )
{
	int iOldHealth = m_iMarkerHealth;
	if ( m_iMarkerHealth != iOldHealth )
		return true;

	return false;
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFReviveMarker::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		/*C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( m_hOwner && pLocalPlayer && InSameTeam( pLocalPlayer ) )
		{
			if ( pLocalPlayer && pLocalPlayer->IsPlayerClass( TF_CLASS_MEDIC ) && pLocalPlayer->IsAlive() )
			{
				Vector vecPos;
				if ( GetAttachmentLocal( LookupAttachment( "mediccall" ), vecPos ) )
				{
					vecPos += Vector(0,0,18);	// Particle effect is 18 units above the attachment
					CTFMedicCallerPanel::AddMedicCaller( this, 5.0, vecPos, TF_CALL_REVIVE_EASY );
				}
			}
		}*/

		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	if ( lfe_muzzlelight.GetBool() )
	{
		dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
		dl->origin = GetAbsOrigin();
		switch ( GetTeamNumber() )
		{
		case TF_TEAM_RED:
			dl->color.r = 255; dl->color.g = 30; dl->color.b = 10; dl->style = 0;
			break;

		case TF_TEAM_BLUE:
			dl->color.r = 10; dl->color.g = 30; dl->color.b = 255; dl->style = 0;
			break;
		}
		dl->die = gpGlobals->curtime + 0.1f;
		dl->radius = 256.0f;
		dl->decay = 512.0f;
	}

	BaseClass::OnDataChanged( updateType );
}
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFReviveMarker::SetOwner( CTFPlayer *pOwner )
{
	m_hOwner = pOwner;
	ChangeTeam( pOwner->GetTeamNumber() );

	if ( GetTeamNumber() == TF_TEAM_BLUE )
		m_nSkin = 1;
	else
		m_nSkin = 0;

	SetBodygroup( FindBodygroupByName( "class" ), pOwner->GetDesiredPlayerClassIndex() - 1 );
}
#endif