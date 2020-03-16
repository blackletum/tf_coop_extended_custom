//=============================================================================//
//
// Purpose: Crocs lives here!
//
//=============================================================================//

#include "cbase.h"
#include "func_croc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CFuncCroc )

	// Outputs
	DEFINE_OUTPUT( m_OnEat, "OnEat" ),
	DEFINE_OUTPUT( m_OnEatRed, "OnEatRed" ),
	DEFINE_OUTPUT( m_OnEatBlue, "OnEatBlue" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( func_croc, CFuncCroc );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFuncCroc::CFuncCroc()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncCroc::Spawn( void )
{
	Precache();
	BaseClass::Spawn();
	InitTrigger();

	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS | SF_TRIGGER_ALLOW_NPCS );
	AddEffects( EF_NODRAW );
	
	SetCollisionGroup( TFCOLLISION_GROUP_RESPAWNROOMS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFuncCroc::Precache( void )
{
	UTIL_PrecacheOther( "entity_croc" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncCroc::StartTouch( CBaseEntity *pOther )
{
	if ( m_bDisabled || !pOther )
		return;

	BaseClass::StartTouch( pOther );

	if ( IsTouching( pOther ) && !pOther->InSameTeam( this ) )
	{
		FireOutputs( pOther );

		CEntityCroc *pCroc = (CEntityCroc*) CreateEntityByName( "entity_croc" );
		pCroc->SetAbsOrigin( pOther->GetAbsOrigin() );
		pCroc->SetAbsAngles( pOther->GetAbsAngles() );
		DispatchSpawn( pCroc );
		pCroc->CrocAttack();

		CTakeDamageInfo info( this, this, 1000, DMG_SLASH | DMG_ALWAYSGIB, TF_DMG_CUSTOM_CROC );
		pOther->TakeDamage( info );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncCroc::FireOutputs( CBaseEntity *pActivator )
{
	if ( !pActivator )
		return;

	m_OnEat.FireOutput( pActivator, this );

	if ( pActivator->GetTeamNumber() == TF_TEAM_RED )
		m_OnEatRed.FireOutput( pActivator, this );
	else if ( pActivator->GetTeamNumber() == TF_TEAM_BLUE )
		m_OnEatBlue.FireOutput( pActivator, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFuncCroc::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: Only transmit this entity to clients that aren't in our team
//-----------------------------------------------------------------------------
int CFuncCroc::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	if ( m_bDisabled )
	{
		if ( GetTeamNumber() != TEAM_UNASSIGNED )
		{
			// Only transmit to enemy players
			CBaseEntity *pRecipientEntity = CBaseEntity::Instance( pInfo->m_pClientEnt );
			if ( pRecipientEntity->GetTeamNumber() > LAST_SHARED_TEAM && !InSameTeam(pRecipientEntity) )
				return FL_EDICT_ALWAYS;
		}
	}

	return FL_EDICT_DONTSEND;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFuncCroc::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup == COLLISION_GROUP_NPC )
	{
		switch( GetTeamNumber() )
		{
		case TF_TEAM_BLUE:
			if ( !(contentsMask & CONTENTS_BLUETEAM) )
				return false;
			break;

		case TF_TEAM_RED:
			if ( !(contentsMask & CONTENTS_REDTEAM) )
				return false;
			break;

		case TF_TEAM_GREEN:
			if ( !(contentsMask & CONTENTS_GREENTEAM ) )
				return false;
			break;

		case TF_TEAM_YELLOW:
			if ( !(contentsMask & CONTENTS_YELLOWTEAM ) )
				return false;
			break;
		}

		return true;
	}

	return false;
}

BEGIN_DATADESC( CEntityCroc )

	// Function Pointers
	DEFINE_FUNCTION( Think ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( entity_croc, CEntityCroc );

void CEntityCroc::InitCroc( void )
{
	PrecacheModel( "models/props_island/crocodile/crocodile.mdl" );

	PrecacheScriptSound( "Crocs.JumpOut" );
	PrecacheScriptSound( "Crocs.JumpBite" );
	PrecacheScriptSound( "Crocs.JumpIn" );
	PrecacheParticleSystem( "water_splash_croc_spawn" );
}

void CEntityCroc::Spawn( void )
{
	InitCroc();
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );

	SetModel( "models/props_island/crocodile/crocodile.mdl" );
	UseClientSideAnimation();
	ResetSequence( LookupSequence( "ref" ) );

	SetThink ( NULL );
	SetNextThink( gpGlobals->curtime );
}

void CEntityCroc::Think( void )
{
	UTIL_Remove( this );
	SetThink( NULL );
	SetNextThink( gpGlobals->curtime );
}

void CEntityCroc::CrocAttack( void )
{
	//EmitSound( "" ); // no need because both particle and sound are already set in the animation itself

	SetSequence( LookupSequence( "attack" ) );

	SetTouch ( NULL );
	SetThink ( &CEntityCroc::Think );
	SetNextThink( gpGlobals->curtime + 1.0f );
}