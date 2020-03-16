//==== Copyright © 2012, Sandern Corporation, All rights reserved. =========
//
//
//=============================================================================

#include "cbase.h"
#include "npc_archer.h"
#include "tf_player.h"
#include "tf_wearable.h"
#include "particle_parse.h"
#include "props_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	TF_ZOMBIE_MELEE1_RANGE		100.0f

ConVar tf_bot_npc_archer_arrow_damage( "tf_bot_npc_archer_arrow_damage", "75", FCVAR_CHEAT );
ConVar tf_bot_npc_archer_speed( "tf_bot_npc_archer_speed", "100", FCVAR_CHEAT );
ConVar tf_bot_npc_archer_health( "tf_bot_npc_archer_health", "100", FCVAR_CHEAT );

LINK_ENTITY_TO_CLASS( bot_npc_archer, CBotNPCArcher );

BEGIN_DATADESC( CBotNPCArcher )
END_DATADESC()

int CBotNPCArcher::gm_nMoveXPoseParam = -1;
int CBotNPCArcher::gm_nMoveYPoseParam = -1;

CBotNPCArcher::CBotNPCArcher()
{
	m_timeTillDeath.Invalidate();
	m_flAttDamage = 30.0f;
	m_flAttRange = 50.0f;
}

CBotNPCArcher::~CBotNPCArcher()
{
}

void CBotNPCArcher::Precache()
{
	BaseClass::Precache();

	PrecacheModel( "models/player/sniper.mdl" );
	PrecacheModel( "models/weapons/c_models/c_machete/c_machete.mdl" );

	PrecacheScriptSound( "Sniper.Death" );
	PrecacheScriptSound( "Sniper.Taunts05" );
	PrecacheScriptSound( "Sniper.NegativeVocalization03" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::Spawn()
{
	Precache();

	SetModel( "models/player/sniper.mdl" );

	BaseClass::Spawn();

	m_pBow = (CBaseAnimating *)CreateEntityByName( "prop_dynamic" );
	if ( m_pBow )
	{
		m_pBow->SetModel( "models/weapons/c_models/c_machete/c_machete.mdl" );
		m_pBow->FollowEntity( this );
	}

	SetHullType( HULL_HUMAN );
	SetSolid( SOLID_BBOX );
	SetDefaultEyeOffset();

	SetNavType( NAV_GROUND );
	SetBloodColor( BLOOD_COLOR_RED );
	m_NPCState = NPC_STATE_NONE;
	
	m_iHealth = m_iMaxHealth = tf_bot_npc_archer_health.GetFloat();

	m_flFieldOfView		= 0.2;

	SetMoveType( MOVETYPE_STEP );

	SetupGlobalModelData();
	
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_SQUAD );
	CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );

	AddEFlags( EFL_NO_DISSOLVE );

	NPCInit();

	m_specialAttackTimer.Start( RandomFloat( 5.0f, 10.0f ) );

	SetAbsAngles( QAngle( 0, RandomFloat( 0, 360.f ), 0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::Activate()
{
	BaseClass::Activate();

	SetupGlobalModelData();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::SetupGlobalModelData()
{
	if ( gm_nMoveXPoseParam != -1 )
		return;

	gm_nMoveXPoseParam = LookupPoseParameter( "move_x" );
	gm_nMoveYPoseParam = LookupPoseParameter( "move_y" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CBotNPCArcher::GetIdealSpeed( ) const
{
	return tf_bot_npc_archer_speed.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CBotNPCArcher::GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence )
{
	return tf_bot_npc_archer_speed.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity CBotNPCArcher::NPC_TranslateActivity( Activity baseAct )
{
	if ( ( baseAct == ACT_IDLE ) )
		return (Activity) ACT_MP_STAND_MELEE;

	if ((baseAct == ACT_RUN) || (baseAct == ACT_WALK))
		return (Activity)ACT_MP_STAND_MELEE;//ACT_MP_RUN_MELEE;

	if ( ( baseAct == ACT_MELEE_ATTACK1 ) )
		return (Activity) ACT_MP_STAND_MELEE; // ACT_MP_ATTACK_STAND_ITEM1

	return baseAct;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBotNPCArcher::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
{
	// required movement direction
	float flMoveYaw = UTIL_VecToYaw( move.dir );

	// FIXME: move this up to navigator so that path goals can ignore these overrides.
	Vector dir;
	float flInfluence = GetFacingDirection( dir );
	dir = move.facing * (1 - flInfluence) + dir * flInfluence;
	VectorNormalize( dir );

	// ideal facing direction
	float idealYaw = UTIL_AngleMod( UTIL_VecToYaw( dir ) );
		
	// FIXME: facing has important max velocity issues
	GetMotor()->SetIdealYawAndUpdate( idealYaw );	

	// find movement direction to compensate for not being turned far enough
	float flDiff = UTIL_AngleDiff( flMoveYaw, GetLocalAngles().y );

	// Setup the 9-way blend parameters based on our speed and direction.
	Vector2D vCurMovePose( 0, 0 );

	vCurMovePose.x = cos( DEG2RAD( flDiff ) ) * 1.0f; //flPlaybackRate;
	vCurMovePose.y = -sin( DEG2RAD( flDiff ) ) * 1.0f; //flPlaybackRate;

	SetPoseParameter( gm_nMoveXPoseParam, vCurMovePose.x );
	SetPoseParameter( gm_nMoveYPoseParam, vCurMovePose.y );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::PainSound( const CTakeDamageInfo &info )
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBotNPCArcher::DeathSound( const CTakeDamageInfo &info ) 
{
	EmitSound( "Sniper.Death" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::AlertSound( void )
{
	EmitSound( "Sniper.Taunts05" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBotNPCArcher::ShouldPlayIdleSound( void )
{
	if ( random->RandomInt( 0, 25 ) != 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::IdleSound( void )
{
	EmitSound( "Sniper.NegativeVocalization03" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MELEE_ATTACK1:
	{
		SetLastAttackTime( gpGlobals->curtime );
		m_iAttackLayer = AddGesture( ACT_MP_ATTACK_STAND_MELEE );
		break;
	}
	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MELEE_ATTACK1:
	{
		RunAttackTask( pTask->iTask );
		break;
	}
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::RunAttackTask( int task )
{
	AutoMovement( );

	Vector vecEnemyLKP = GetEnemyLKP();

	// If our enemy was killed, but I'm not done animating, the last known position comes
	// back as the origin and makes the me face the world origin if my attack schedule
	// doesn't break when my enemy dies. (sjb)
	if( vecEnemyLKP != vec3_origin )
	{
		if ( ( task == TASK_RANGE_ATTACK1 || task == TASK_RELOAD ) && 
			 ( CapabilitiesGet() & bits_CAP_AIM_GUN ) && 
			 FInAimCone( vecEnemyLKP ) )
		{
			// Arms will aim, so leave body yaw as is
			GetMotor()->SetIdealYawAndUpdate( GetMotor()->GetIdealYaw(), AI_KEEP_YAW_SPEED );
		}
		else
		{
			GetMotor()->SetIdealYawToTargetAndUpdate( vecEnemyLKP, AI_KEEP_YAW_SPEED );
		}
	}

	CAnimationLayer *pPlayer = GetAnimOverlay( m_iAttackLayer );
	if ( pPlayer->m_bSequenceFinished )
	{
		if ( task == TASK_RELOAD && GetShotRegulator() )
		{
			GetShotRegulator()->Reset( false );
		}

		TaskComplete();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	if ( m_timeTillDeath.HasStarted() && m_timeTillDeath.IsElapsed() )
	{
		CTakeDamageInfo info( this, this, 1000, DMG_GENERIC );
		TakeDamage( info );
		return;
	}

	m_flGroundSpeed = tf_bot_npc_archer_speed.GetFloat();

	const Vector vecRangeSize = Vector( 16, 16, 64 );
	CBaseEntity *pList[64];
	int count = UTIL_EntitiesInBox( pList, 64, GetAbsOrigin() - vecRangeSize, GetAbsOrigin() + vecRangeSize, 0 );
	for ( int i = 0; i < count; i++ )
	{
		CBaseCombatCharacter *pEntity = ToBaseCombatCharacter( pList[i] );
		if ( pEntity && pEntity->IsAlive() && !InSameTeam( pEntity ) )
		{
			if ( IsRangeLessThan( pEntity, m_flAttRange ) && ( IsAbleToSee( pEntity, CBaseCombatCharacter::USE_FOV ) ) )
			{
				SetEnemy( pEntity );
				UpdateEnemyMemory( pEntity, pEntity->GetAbsOrigin() );

				if ( m_attackTimer.IsElapsed() )
				{
					m_attackTimer.Start( RandomFloat( 0.8f, 1.2f ) );

					Vector vecDir = pEntity->WorldSpaceCenter() - WorldSpaceCenter();
					vecDir.NormalizeInPlace();

					CTakeDamageInfo info( this, this, m_flAttDamage, DMG_SLASH );
					CalculateMeleeDamageForce( &info, vecDir, WorldSpaceCenter(), 5.0f );
					pEntity->TakeDamage( info );

					CBasePlayer *pPlayer = ToBasePlayer( pEntity );
					if ( pPlayer != NULL )
					{
						//Kick the player angles
						if ( !(pPlayer->GetFlags() & FL_GODMODE ) && pPlayer->GetMoveType() != MOVETYPE_NOCLIP )
						{
							pPlayer->ViewPunch( QAngle( 20.0f, 0.0f, -12.0f ) );

							Vector	dir = pEntity->GetAbsOrigin() - GetAbsOrigin();
							VectorNormalize(dir);

							QAngle angles;
							VectorAngles( dir, angles );
							Vector forward, right;
							AngleVectors( angles, &forward, &right, NULL );

							//Push the target back
							pEntity->ApplyAbsVelocityImpulse( - right * -1.0f - forward * 250.0f );
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::Event_Killed( const CTakeDamageInfo &info )
{
	CTFPlayer *pTFAttacker = ToTFPlayer( info.GetAttacker() );
	if ( pTFAttacker )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "halloween_skeleton_killed" );
		if ( event )
		{
			event->SetInt( "player", engine->GetPlayerUserId( pTFAttacker->edict() ) );
			gameeventmanager->FireEvent( event );
		}
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBotNPCArcher::BecomeRagdollOnClient( const Vector &force )
{
	CPVSFilter filter( GetAbsOrigin() );
	UserMessageBegin( filter, "BreakModel" );
		WRITE_SHORT( GetModelIndex() );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_ANGLES( GetAbsAngles() );
		WRITE_SHORT( m_nSkin );
	MessageEnd();

	UTIL_Remove( this );

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBotNPCArcher::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if ( !IsPlayingGesture( ACT_MP_GESTURE_FLINCH_MELEE ) )
		AddGesture( ACT_MP_GESTURE_FLINCH_MELEE );

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBotNPCArcher::UpdateOnRemove( void )
{
	if ( m_pBow )
		UTIL_Remove( m_pBow );

	BaseClass::UpdateOnRemove();
}

const char *CBotNPCArcher::GetLocalizeName( void )
{
	const char *pszName = BaseClass::GetLocalizeName();
	return pszName;
}
//-------------------------------------------------------------------------------------------------
//
// Schedules
//
//-------------------------------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( bot_npc_archer, CBotNPCArcher )
AI_END_CUSTOM_NPC()
