//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "eyeball_boss.h"
#include "npcevent.h"
#include "npc_scanner.h"
#include "IEffects.h"
#include "explode.h"
#include "ai_route.h"
#include "tf_team.h"
#include "tf_player.h"
#include "tf_obj_sentrygun.h"
#include "tf_gamerules.h"
#include "particle_parse.h"
#include "nav_mesh.h"
#include "tf_projectile_rocket.h"
#include "halloween/eyeball_boss/teleport_vortex.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar tf_eyeball_boss_debug( "tf_eyeball_boss_debug", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_eyeball_boss_debug_orientation( "tf_eyeball_boss_debug_orientation", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_eyeball_boss_lifetime( "tf_eyeball_boss_lifetime", "120", FCVAR_CHEAT );
ConVar tf_eyeball_boss_lifetime_spell( "tf_eyeball_boss_lifetime_spell", "8", FCVAR_CHEAT );
ConVar tf_eyeball_boss_speed( "tf_eyeball_boss_speed", "250", FCVAR_CHEAT );
ConVar tf_eyeball_boss_hover_height( "tf_eyeball_boss_hover_height", "200", FCVAR_CHEAT );
ConVar tf_eyeball_boss_acceleration( "tf_eyeball_boss_acceleration", "500", FCVAR_CHEAT );
ConVar tf_eyeball_boss_horiz_damping( "tf_eyeball_boss_horiz_damping", "2", FCVAR_CHEAT );
ConVar tf_eyeball_boss_vert_damping( "tf_eyeball_boss_vert_damping", "1", FCVAR_CHEAT );
ConVar tf_eyeball_boss_attack_range( "tf_eyeball_boss_attack_range", "750", FCVAR_CHEAT );
ConVar tf_eyeball_boss_health_base( "tf_eyeball_boss_health_base", "8000", FCVAR_CHEAT );
ConVar tf_eyeball_boss_health_per_player( "tf_eyeball_boss_health_per_player", "400", FCVAR_CHEAT );
ConVar tf_eyeball_boss_health_at_level_2( "tf_eyeball_boss_health_at_level_2", "17000", FCVAR_CHEAT );
ConVar tf_eyeball_boss_health_per_level( "tf_eyeball_boss_health_per_level", "3000", FCVAR_CHEAT );

extern ConVar tf_halloween_bot_min_player_count;

IMPLEMENT_SERVERCLASS_ST( CEyeBallBoss, DT_EyeBallBoss )
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseEntity", "m_angAbsRotation" ),

	SendPropVector( SENDINFO( m_lookAtSpot ) ),
	SendPropInt( SENDINFO( m_attitude ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( eyeball_boss, CEyeBallBoss );

BEGIN_DATADESC( CEyeBallBoss )
	DEFINE_FIELD( m_flAttackRange,	FIELD_FLOAT ),

	DEFINE_FIELD( m_iAngerPose,			FIELD_INTEGER ),
END_DATADESC()

int CEyeBallBoss::m_level = 0;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static const float EyeBallBossModifyDamage( CTakeDamageInfo const& info )
{
	const float flDamage = info.GetDamage();

	CObjectSentrygun *pSentry = dynamic_cast<CObjectSentrygun *>( info.GetInflictor() );
	if ( pSentry )
		return flDamage / 4;

	CTFProjectile_SentryRocket *pRocket = dynamic_cast<CTFProjectile_SentryRocket *>( info.GetInflictor() );
	if ( pRocket )
		return flDamage / 4;

	CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( info.GetWeapon() );
	if ( pWeapon )
	{
		if ( pWeapon->IsWeapon( TF_WEAPON_MINIGUN ) )
			return flDamage / 4;

		if ( pWeapon->IsWeapon( TF_WEAPON_FLAMETHROWER ) )
			return flDamage / 2;
	}

	return flDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEyeBallBoss::CEyeBallBoss()
{
	m_flAttackRange = tf_eyeball_boss_attack_range.GetFloat();

	m_iOldHealth = -1;
	m_iAngerPose = -1;
	m_lookAtSpot = vec3_origin;
	m_attitude = ATTITUDE_CALM;

	m_desiredSpeed = 0;
	m_desiredAltitude = tf_eyeball_boss_hover_height.GetFloat();
	m_verticalSpeed = 0;
	m_vecMotion = vec3_origin;
	m_wishVelocity = vec3_origin;
	m_localVelocity = vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEyeBallBoss::Precache()
{
	PrecacheModel( "models/props_halloween/halloween_demoeye.mdl" );
	PrecacheModel( "models/props_halloween/eyeball_projectile.mdl" );

	PrecacheScriptSound( "Halloween.EyeballBossIdle" );
	PrecacheScriptSound( "Halloween.EyeballBossBecomeAlert" );
	PrecacheScriptSound( "Halloween.EyeballBossAcquiredVictim" );
	PrecacheScriptSound( "Halloween.EyeballBossStunned" );
	PrecacheScriptSound( "Halloween.EyeballBossStunRecover" );
	PrecacheScriptSound( "Halloween.EyeballBossLaugh" );
	PrecacheScriptSound( "Halloween.EyeballBossBigLaugh" );
	PrecacheScriptSound( "Halloween.EyeballBossDie" );
	PrecacheScriptSound( "Halloween.EyeballBossEscapeSoon" );
	PrecacheScriptSound( "Halloween.EyeballBossEscapeImminent" );
	PrecacheScriptSound( "Halloween.EyeballBossEscaped" );
	PrecacheScriptSound( "Halloween.EyeballBossTeleport" );
	PrecacheScriptSound( "Halloween.HeadlessBossSpawnRumble" );
	PrecacheScriptSound( "Halloween.EyeballBossBecomeEnraged" );
	PrecacheScriptSound( "Halloween.EyeballBossRage" );
	PrecacheScriptSound( "Halloween.EyeballBossCalmDown" );
	PrecacheScriptSound( "Halloween.spell_spawn_boss_disappear" );

	PrecacheParticleSystem( "eyeboss_death" );
	PrecacheParticleSystem( "eyeboss_aura_angry" );
	PrecacheParticleSystem( "eyeboss_aura_grumpy" );
	PrecacheParticleSystem( "eyeboss_aura_calm" );
	PrecacheParticleSystem( "eyeboss_aura_stunned" );
	PrecacheParticleSystem( "eyeboss_tp_normal" );
	PrecacheParticleSystem( "eyeboss_tp_escape" );
	PrecacheParticleSystem( "eyeboss_team_red" );
	PrecacheParticleSystem( "eyeboss_team_blue" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEyeBallBoss::Spawn( void )
{
	Precache();

	SetModel( "models/props_halloween/halloween_demoeye.mdl" );

	BaseClass::Spawn();

	SetHullType( HULL_TINY_CENTERED );
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );

	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );

	m_bloodColor		= DONT_BLEED;
	m_NPCState			= NPC_STATE_NONE;

	SetNavType( NAV_FLY );

	AddFlag( FL_FLY );

	// This entity cannot be dissolved by the combine balls,
	// nor does it get killed by the mega physcannon.
	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL );

	int iHealth;
	if ( m_level <= 1 )
	{
		iHealth = tf_eyeball_boss_health_base.GetInt();
		int iNumPlayers = GetGlobalTFTeam( TF_TEAM_RED )->GetNumPlayers() + GetGlobalTFTeam( TF_TEAM_BLUE )->GetNumPlayers();
		int iMinPlayers = tf_halloween_bot_min_player_count.GetInt();
		if ( iNumPlayers > iMinPlayers )
			iHealth += tf_eyeball_boss_health_per_player.GetInt() * ( iNumPlayers - iMinPlayers );
	}
	else
	{
		int iHealthPerLevel = tf_eyeball_boss_health_per_level.GetInt() * ( m_level - 2 );
		iHealth = tf_eyeball_boss_health_at_level_2.GetInt() + iHealthPerLevel;
	}

	SetMaxHealth( iHealth );
	SetHealth( iHealth );

	const Vector mins( -50.0f, -50.0f, -50.0f ), maxs( 50.0f, 50.0f, 50.0f );
	CollisionProp()->SetSurroundingBoundsType( USE_SPECIFIED_BOUNDS, &mins, &maxs );
	CollisionProp()->SetCollisionBounds( mins, maxs );

	ChangeTeam( TF_TEAM_GREEN );

	AngleVectors( GetLocalAngles(), &m_vCurrentBanking );
	m_fHeadYaw = 0;

	SetCurrentVelocity( vec3_origin );

	m_flSpeed = tf_eyeball_boss_speed.GetFloat();

	CapabilitiesAdd( bits_CAP_MOVE_FLY | bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_SKIP_NAV_GROUND_CHECK );

	NPCInit();

	if ( GetTeamNumber() == TF_TEAM_GREEN )
	{
		m_attitude = ATTITUDE_CALM;
	}
	else
	{
		switch ( GetTeamNumber() )
		{
			case TF_TEAM_RED:
				m_attitude = ATTITUDE_HATEBLUE;
				break;
			case TF_TEAM_BLUE:
				m_attitude = ATTITUDE_HATERED;
				break;
			default:
				break;
		}
	}

	m_lookAtSpot = vec3_origin;

	CBaseEntity *pEntity = NULL;
	while ( ( pEntity = gEntList.FindEntityByClassname( pEntity, "info_target" ) ) != NULL )
	{
		if ( pEntity->NameMatches( "spawn_boss_alt" ) )
			m_hSpawnEnts.AddToTail( pEntity );
	}

	if ( m_hSpawnEnts.IsEmpty() )
		Warning( "No info_target entities named 'spawn_boss_alt' found!\n" );

	SetSchedule( SCHED_EYEBALL_EMERGE );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEyeBallBoss::UpdateEfficiency( bool bInPVS )	
{
	SetEfficiency( ( GetSleepState() != AISS_AWAKE ) ? AIE_DORMANT : AIE_NORMAL ); 
	SetMoveEfficiency( AIME_NORMAL ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEyeBallBoss::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	if ( GetTeamNumber() == TF_TEAM_GREEN )
	{
		const float flTimeLeft = m_lifeTimeDuration.GetRemainingTime();
		if ( flTimeLeft < 10.0f && m_flTimeLeftAlive > 10.0f )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_escape_imminent" );
			if ( event )
			{
				event->SetInt( "time_remaining", 10 );
				event->SetInt( "level", GetLevel() );

				gameeventmanager->FireEvent( event );
			}
		}
		else if ( flTimeLeft < 30.0f && m_flTimeLeftAlive > 30.0f )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_escape_imminent" );
			if ( event )
			{
				event->SetInt( "time_remaining", 30 );
				event->SetInt( "level", GetLevel() );

				gameeventmanager->FireEvent( event );
			}
		}
		else if ( flTimeLeft < 60.0f && m_flTimeLeftAlive > 60.0f )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_escape_imminent" );
			if ( event )
			{
				event->SetInt( "time_remaining", 60 );
				event->SetInt( "level", GetLevel() );

				gameeventmanager->FireEvent( event );
			}		
		}
		
		m_flTimeLeftAlive = flTimeLeft;
	}
	/*else if ( m_lifeTimeDuration.IsElapsed() )
		SetSchedule( SCHED_EYEBALL_ESCAPE );*/

	if ( !m_teleportTimer.IsElapsed() || GetTeamNumber() != TF_TEAM_GREEN )
	{
		SetDesiredAltitude( tf_eyeball_boss_hover_height.GetFloat() );
	}
	else
	{
		SetDesiredAltitude( 0 );

		Vector vecPos = WorldSpaceCenter();
		float flHeight = 0;

		if ( TheNavMesh->GetSimpleGroundHeight( vecPos, &flHeight ) && TheNavMesh->GetNearestNavArea( vecPos, true, 450.0f ) )
		{
			if ( vecPos.z - flHeight < 220.0f )
			{
				//if ( !m_lifeTimeDuration.IsElapsed() )
				//{
					m_teleportTimer.Start( RandomFloat( 10.0f, 15.0f ) );
					SetSchedule( SCHED_EYEBALL_TELEPORT );
				/*}
				else
				{
					SetSchedule( SCHED_EYEBALL_ESCAPE );
				}*/
			}
		}
	}

	if ( IsCurSchedule( TASK_EYEBALL_IDLE ) )
	{
		int iSequence = 0;
		if ( GetTeamNumber() != TF_TEAM_GREEN )
		{
			iSequence = LookupSequence( "lookaround3" );
		}
		else
		{
			if ( GetHealth() < ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) )
			{
				iSequence = LookupSequence( "lookaround3" );
			}
			else
			{
				if ( GetHealth() >= 2 * ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) )
				{
					iSequence = LookupSequence( "lookaround1" );
				}
				else
				{
					iSequence = LookupSequence( "lookaround2" );
				}
			}
		}

		if ( iSequence && ( iSequence != GetSequence() || IsSequenceFinished() ) )
		{
			SetSequence( iSequence );
			SetPlaybackRate( 1.0f );
			SetCycle( 0.0f );
			ResetSequenceInfo();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called just before we are deleted.
//-----------------------------------------------------------------------------
void CEyeBallBoss::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Vector CEyeBallBoss::EyePosition( void )
{
	return GetViewOffset() + GetAbsOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: We'll be off the ground by a few hundred units so search for our last known further away than BaseClass
//-----------------------------------------------------------------------------
void CEyeBallBoss::UpdateLastKnownArea( void )
{
	if ( TheNavMesh->IsGenerating() )
	{
		ClearLastKnownArea();
		return;
	}

	// find the area we are directly standing in
	CNavArea *area = TheNavMesh->GetNearestNavArea( this, GETNAVAREA_CHECK_LOS, 500.0f );
	if ( !area )
		return;

	// make sure we can actually use this area - if not, consider ourselves off the mesh
	if ( !IsAreaTraversable( area ) )
		return;

	if ( area != m_lastNavArea )
	{
		// player entered a new nav area
		if ( m_lastNavArea )
		{
			m_lastNavArea->DecrementPlayerCount( m_registeredNavTeam, entindex() );
			m_lastNavArea->OnExit( this, area );
		}

		m_registeredNavTeam = GetTeamNumber();
		area->IncrementPlayerCount( m_registeredNavTeam, entindex() );
		area->OnEnter( this, m_lastNavArea );

		OnNavAreaChanged( area, m_lastNavArea );

		m_lastNavArea = area;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Gets the appropriate next schedule based on current condition
//			bits.
//-----------------------------------------------------------------------------
int CEyeBallBoss::SelectSchedule(void)
{
	if ( m_NPCState == NPC_STATE_SCRIPT )
		return BaseClass::SelectSchedule();

	if ( m_lookAroundTimer.IsElapsed() )
	{
		m_lookAroundTimer.Start( RandomFloat( 2.0f, 4.0f ) );

		float flRandom = RandomFloat( -M_PI, M_PI );
		Vector vecOrigin = GetAbsOrigin();
		TurnHeadToTarget( 0, vecOrigin + Vector( flRandom * 100.0f, flRandom * 100.0f, 0 ) );

		if ( m_bTaunt )
		{
			m_bTaunt = false;
			return SCHED_EYEBALL_EMOTE;
		}
	}

	if ( GetEnemy() != NULL && GetEnemy()->IsAlive() )
	{
		if ( HasCondition( COND_LOST_ENEMY || HasCondition( COND_ENEMY_DEAD ) ) )
			return SCHED_EYEBALL_IDLE;

		//if ( IsCurSchedule( SCHED_EYEBALL_NOTICE ) )
		//{
			float flRange = tf_eyeball_boss_attack_range.GetFloat();
			if ( GetTeamNumber() != TF_TEAM_GREEN || GetHealth() < ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) )
				flRange *= 2;

			if ( IsRangeLessThan( GetEnemy(), flRange ) )
				return SCHED_EYEBALL_LAUNCH_ROCKET;
		//}

		return SCHED_EYEBALL_NOTICE;
	}

	return SCHED_EYEBALL_IDLE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEyeBallBoss::OnScheduleChange( void )
{
	m_flSpeed = tf_eyeball_boss_speed.GetFloat();

	BaseClass::OnScheduleChange();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : eOldState - 
//			eNewState - 
//-----------------------------------------------------------------------------
void CEyeBallBoss::OnStateChange( NPC_STATE eOldState, NPC_STATE eNewState )
{
	if (( eNewState == NPC_STATE_ALERT ) || ( eNewState == NPC_STATE_COMBAT ))
	{
	}
	else
	{
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CEyeBallBoss::StartTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
	case TASK_EYEBALL_IDLE:
		{
			if ( ShouldPlayIdleSound() )
				IdleSound();

			float flRandom = RandomFloat( 3.0f, 5.0f );
			m_idleTimer.Start( flRandom );

			/*if ( GetTeamNumber() == TF_TEAM_GREEN )
				m_lifeTimeDuration.Start( flRandom + tf_eyeball_boss_lifetime.GetFloat() );
			else
				m_lifeTimeDuration.Start( flRandom + tf_eyeball_boss_lifetime_spell.GetFloat() );*/

			m_iOldHealth = GetHealth();

			m_teleportTimer.Start( RandomFloat( 10.0f, 15.0f ) );

			TaskComplete();
			break;
		}
	case TASK_EYEBALL_EMERGE:
		{
			if ( GetTeamNumber() != TF_TEAM_GREEN )
				TaskComplete();

			int iSequence = LookupSequence( "arrives" );
			if ( iSequence > 0 )
			{
				SetSequence( iSequence );
				SetPlaybackRate( 1.0f );
				ResetSequenceInfo();
			}

			m_emergeTimer.Start( 3.0f );

			DispatchParticleEffect( "halloween_boss_summon", GetAbsOrigin(), GetAbsAngles() );

			m_vecTarget = GetAbsOrigin() + Vector( 0, 0, 100 );
			m_flDistance = 150.0f;

			SetAbsOrigin( m_vecTarget - Vector( 0, 0, 150 ) );
			EmitSound( "Halloween.HeadlessBossSpawnRumble" );

			IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_summoned" );
			if (event)
			{
				event->SetInt( "level", GetLevel() );
				gameeventmanager->FireEvent( event );
			}

			if ( !m_emergeTimer.IsElapsed() )
			{
				Vector vec = m_vecTarget + Vector( 0, 0, ( m_emergeTimer.GetRemainingTime() * -m_flDistance ) / m_emergeTimer.GetCountdownDuration() );
				SetAbsOrigin( vec );

				if ( m_shakeTimer.IsElapsed() )
				{
					m_shakeTimer.Start( 0.25f );
					UTIL_ScreenShake( GetAbsOrigin(), 15.0f, 5.0f, 1.0f, 1000.0f, SHAKE_START );
				}
			}

			if ( IsSequenceFinished() )
			{
				SetSchedule( SCHED_EYEBALL_IDLE );
				TaskComplete();
			}
			break;
		}
	case TASK_EYEBALL_LAUNCH_ROCKET:
		{
			CBaseEntity *pVictim = GetEnemy();
			if ( !pVictim || !pVictim->IsAlive() )
				TaskFail( FAIL_NO_ENEMY );

			m_vecShootAt = pVictim->GetAbsOrigin();
			if ( pVictim->IsBaseObject() )
			{
				if ( dynamic_cast<CObjectSentrygun *>( pVictim ) )
					m_vecShootAt = pVictim->EyePosition();
			}

			int iSequence = 0;
			if ( GetTeamNumber() != TF_TEAM_GREEN ||  GetHealth() < ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) )
			{
				m_iNumRockets = 3;
				m_rocketLaunchDelay.Start( 0.25f );

				EmitSound( "Halloween.EyeballBossRage" );

				iSequence = LookupSequence( "firing3" );
			}
			else if ( GetTeamNumber() != TF_TEAM_GREEN ||GetHealth() < ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) || GetHealth() >= 2 * ( GetMaxHealth() / 3 ) )
			{
				m_iNumRockets = 1;
				m_rocketLaunchDelay.Start( 0.5f );

				iSequence = LookupSequence( "firing1" );
			}
			else
			{
				m_iNumRockets = 3;
				m_rocketLaunchDelay.Start( 0.25f );

				iSequence = LookupSequence( "firing2" );
			}

			if ( iSequence )
			{
				SetSequence( iSequence );
				SetPlaybackRate( 1.0f );
				SetCycle( 0.0f );
				ResetSequenceInfo();
			}

			Vector vecFwd, vecRight;
			GetVectors( &vecFwd, &vecRight, NULL );

			if ( GetTeamNumber() == TF_TEAM_GREEN )
			{
				if ( pVictim->GetFlags() & FL_ONGROUND )
				{
					Vector vecApproach = pVictim->WorldSpaceCenter();
					MoveToTarget( 0, vecApproach + vecRight * 100.0f );
				}
				else
				{
					Vector vecApproach = pVictim->WorldSpaceCenter();
					MoveToTarget( 0, vecApproach + vecRight * -100.0f );
				}
			}

			TurnHeadToTarget( 0, pVictim->GetAbsOrigin() );

			float flSpeed = 330.0f;
			if ( GetTeamNumber() != TF_TEAM_GREEN || GetHealth() < ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) )
			{
				flSpeed = 1100.0f;
			}

			m_iNumRockets--;
			m_refireDelay.Start( 0.3f );

			Vector vecShootAt = m_vecShootAt;

			// If we're angry we suddenly can predict where to fire our rockets
			if ( GetTeamNumber() != TF_TEAM_GREEN || GetHealth() < ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) )
			{
				if ( pVictim && GetRangeTo( pVictim ) > 150.0f )
				{
					float flComp = GetRangeTo( pVictim ) / flSpeed;
					Vector vecAdjustment = pVictim->GetAbsVelocity() * flComp;

					CTraceFilterNoNPCsOrPlayer filter( this, COLLISION_GROUP_NONE );

					trace_t trace;
					UTIL_TraceLine( WorldSpaceCenter(), pVictim->GetAbsOrigin() + vecAdjustment, MASK_SOLID_BRUSHONLY, &filter, &trace );

					if ( trace.DidHit() && ( trace.endpos - vecShootAt ).LengthSqr() > Square( 300.0f ) )
						vecAdjustment = vec3_origin;

					vecShootAt = vecAdjustment + pVictim->GetAbsOrigin();
				}
			}

			QAngle vecAng;
			VectorAngles( vecShootAt - WorldSpaceCenter(), vecAng );

			CTFProjectile_Rocket *pRocket = CTFProjectile_Rocket::Create( this, WorldSpaceCenter(), vecAng, this, this );
			if ( pRocket )
			{
				pRocket->SetModel( "models/props_halloween/eyeball_projectile.mdl" );
				pRocket->EmitSound( "Weapon_RPG.SingleCrit" );

				Vector vecDir;
				AngleVectors( vecAng, &vecDir );
				pRocket->SetAbsVelocity( vecDir * flSpeed );

				pRocket->SetDamage( 50.0f );
				pRocket->SetCritical( GetTeamNumber() == TF_TEAM_GREEN );

				pRocket->ChangeTeam( GetTeamNumber() );
			}

			if ( m_iNumRockets == 0 )
				TaskComplete();

			break;
		}
	case TASK_EYEBALL_ESCAPE:
		{
			int iSequence = LookupSequence( "escape" );
			if ( iSequence )
			{
				SetSequence( iSequence );
				SetPlaybackRate( 1.0f );
				SetCycle( 0.0f );
				ResetSequenceInfo();
			}

			EmitSound( "Halloween.EyeballBossLaugh" );

			if ( IsSequenceFinished() )
			{
				if ( GetTeamNumber() != TF_TEAM_GREEN )
					EmitSound( "Halloween.spell_spawn_boss_disappear" );

				// Commenting this out until missing DispatchParticleEffect method is added
				//DispatchParticleEffect( "eyeboss_tp_escape", GetAbsOrigin(), GetAbsAngles() );

				if ( GetTeamNumber() == TF_TEAM_GREEN )
				{
					IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_escaped" );
					if ( event )
					{
						event->SetInt( "level", GetLevel() );
						gameeventmanager->FireEvent( event );
					}
				}

				UTIL_Remove( this );

				CEyeBallBoss::m_level = 1;

				TaskComplete();
			}
			break;
		}
	case TASK_EYEBALL_TELEPORT:
		{
			int iSequence = LookupSequence( "teleport_out" );
			if ( iSequence )
			{
				SetSequence( iSequence );
				SetPlaybackRate( 1.0f );
				SetCycle( 0.0f );
				ResetSequenceInfo();
			}

			m_iTeleportState = TELEPORT_VANISH;

			if ( IsSequenceFinished() )
			{
				switch ( m_iTeleportState )
				{
					case TELEPORT_VANISH:
					{
						CTeleportVortex *pVortex = (CTeleportVortex *)CBaseEntity::Create( "teleport_vortex", GetAbsOrigin(), vec3_angle );
						if ( pVortex )
							pVortex->SetupVortex( false, false );

						// Commenting this out until missing DispatchParticleEffect method is added
						//DispatchParticleEffect( "eyeboss_tp_normal", GetAbsOrigin(), GetAbsAngles() );
						EmitSound( "Halloween.EyeballBossTeleport" );

						AddEffects( EF_NODRAW|EF_NOINTERP );

						Vector vecNewSpot = PickNewSpawnSpot();
						vecNewSpot.z += 75.0f;

						SetAbsOrigin( vecNewSpot );

						m_iTeleportState = TELEPORT_APPEAR;

						break;
					}
					case TELEPORT_APPEAR:
					{
						// Commenting this out until missing DispatchParticleEffect method is added
						//DispatchParticleEffect( "eyeboss_tp_normal", GetAbsOrigin(), GetAbsAngles() );

						int iSequence = LookupSequence( "teleport_in" );
						if ( iSequence )
						{
							SetSequence( iSequence );
							SetPlaybackRate( 1.0f );
							SetCycle( 0.0f );
							ResetSequenceInfo();
						}

						RemoveEffects( EF_NODRAW|EF_NOINTERP );

						m_iTeleportState = TELEPORT_FINISH;
						break;
					}
					case TELEPORT_FINISH:
						TaskComplete();
				}
			}
			break;
		}
	case TASK_EYEBALL_NOTICE:
		{
			m_chaseDelay.Start( 0.25f );
			EmitSound( "Halloween.EyeballBossBecomeAlert" );
			TurnHeadToTarget( 0, GetEnemy()->GetAbsOrigin() );
			TaskComplete();
			break;
		}
	case TASK_EYEBALL_EMOTE:
		{
			if ( LookupSequence( "laugh" ) )
			{
				SetSequence( LookupSequence( "laugh" ) );
				SetPlaybackRate( 1.0f );
				SetCycle( 0.0f );
				ResetSequenceInfo();
			}

			EmitSound( "Halloween.EyeballBossLaugh" );

			if ( IsSequenceFinished() )
				TaskComplete();

			break;
		}
	case TASK_EYEBALL_STUN:
		{
			m_stunDuration.Start( 5.0f );

			int iSequence = LookupSequence( "stunned" );
			if ( iSequence )
			{
				SetSequence( iSequence );
				SetPlaybackRate( 1.0f );
				SetCycle( 0.0f );
				ResetSequenceInfo();
			}

			EmitSound( "Halloween.EyeballBossStunned" );

			SetDesiredAltitude( 0 );

			int iMaxHealth = GetMaxHealth();
			m_iOldHealth = ( ( 1431655766LL * iMaxHealth ) >> 32 ) - ( iMaxHealth >> 31 );

			if ( m_stunDuration.IsElapsed() )
			{
				BecomeEnraged( 20.0f );
				SetDesiredAltitude( tf_eyeball_boss_hover_height.GetFloat() );
				TaskComplete();
			}
			break;
		}

		case TASK_DIE:
		{
			GetNavigator()->StopMoving();	
			int iSequence = LookupSequence( "death" );
			if ( iSequence )
			{
				SetSequence( iSequence );
				SetPlaybackRate( 1.0f );
				SetCycle( 0.0f );
				ResetSequenceInfo();
			}

			EmitSound( "Halloween.EyeballBossStunned" );

			m_dyingDuration.Start( 10.0f );

			m_lifeState = LIFE_DYING;

			Vector vecGround = WorldSpaceCenter();
			TheNavMesh->GetSimpleGroundHeight( vecGround, &vecGround.z );

			if ( m_dyingDuration.IsElapsed() || ( WorldSpaceCenter().z - vecGround.z ) < 100.0f )
			{
				// Commenting this out until missing DispatchParticleEffect method is added
				//DispatchParticleEffect( "eyeboss_death", GetAbsOrigin(), GetAbsAngles() );

				EmitSound( "Cart.Explode" );
				EmitSound( "Halloween.EyeballBossDie" );

				UTIL_ScreenShake( GetAbsOrigin(), 25.0f, 5.0f, 5.0f, 1000.0f, SHAKE_START );

				if ( GetTeamNumber() == TF_TEAM_GREEN )
				{
					IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_killed" );
					if ( event )
					{
						event->SetInt( "level", GetLevel() );

						gameeventmanager->FireEvent( event );
					}

					CEyeBallBoss::m_level++;
				}

				CTeleportVortex *pVortex = (CTeleportVortex *)CBaseEntity::Create( "teleport_vortex", GetAbsOrigin(), vec3_angle );
				if ( pVortex )
					pVortex->SetupVortex( true, false );

				UTIL_Remove( this );
			}
			break;
		}

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CEyeBallBoss::GetDesiredAltitude( void ) const
{
	return m_desiredAltitude;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CEyeBallBoss::SetDesiredAltitude( float fHeight )
{
	m_desiredAltitude = fHeight;
}

//------------------------------------------------------------------------------
// Purpose: Override to split in two when attacked
//------------------------------------------------------------------------------
int CEyeBallBoss::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	int res = 0;
	CTakeDamageInfo newInfo = info;
	if ( ( info.GetAttacker() != this ) && GetTeamNumber() == TF_TEAM_GREEN )
	{
		int iHealth = GetHealth();
		newInfo.SetDamage( EyeBallBossModifyDamage( info ) );
		res = BaseClass::OnTakeDamage_Alive( newInfo );

		if ( m_iOldHealth >= 0 )
			m_iOldHealth = Max( m_iOldHealth - ( iHealth - GetHealth() ), 0 );
	}

	CTFPlayer *pPlayer = ToTFPlayer( info.GetAttacker() );
	if ( pPlayer )
	{
		if ( !pPlayer->m_purgatoryDuration.HasStarted() || pPlayer->m_purgatoryDuration.IsElapsed() || !m_stunDelay.IsElapsed() )
		{
			if ( info.GetDamageType() & DMG_VEHICLE )
				BecomeEnraged( 5.0f );

			return res;
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_stunned" );
		if ( event )
		{
			event->SetInt( "level", GetLevel() );
			event->SetInt( "player_entindex", pPlayer->entindex() );

			gameeventmanager->FireEvent( event );
		}

		m_stunDelay.Start( 10.0f );
		SetSchedule( SCHED_EYEBALL_STUN );
	}

	SetEnemy( info.GetAttacker() );
	return res;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEyeBallBoss::Event_Killed( const CTakeDamageInfo &info )
{
	// Interrupt whatever schedule I'm on
	SetCondition(COND_SCHEDULE_DONE);

	if ( GetTeamNumber() == TF_TEAM_GREEN )
	{
		CTFPlayer *pTFAttacker = ToTFPlayer( info.GetAttacker() );

		if ( pTFAttacker )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "eyeball_boss_killer" );
			if ( event )
			{
				event->SetInt( "level", GetLevel() );
				event->SetInt( "player_entindex", pTFAttacker->entindex() );

				gameeventmanager->FireEvent( event );
			}
		}
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEyeBallBoss::Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	m_bTaunt = true;
	BaseClass::Event_KilledOther( pVictim, info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CEyeBallBoss::OverrideMove( float flInterval )
{
	MaintainAltitude();

	float flLength = m_localVelocity.NormalizeInPlace();
	m_vecMotion = m_localVelocity;

	m_verticalSpeed = m_localVelocity.z * flLength;

	m_localVelocity.x += ( m_wishVelocity.x - tf_eyeball_boss_horiz_damping.GetFloat() * m_localVelocity.x ) * TICK_INTERVAL;
	m_localVelocity.y += ( m_wishVelocity.y - tf_eyeball_boss_horiz_damping.GetFloat() * m_localVelocity.y ) * TICK_INTERVAL;
	m_localVelocity.z += ( m_wishVelocity.z - tf_eyeball_boss_vert_damping.GetFloat() * m_localVelocity.z ) * TICK_INTERVAL;

	SetAbsVelocity( m_localVelocity );

	CTraceFilterSimpleClassnameList filter( this, COLLISION_GROUP_NONE );
	filter.AddClassnameToIgnore( "eyeball_boss" );
	Vector vecFrameMovement = ( m_localVelocity * TICK_INTERVAL ) + GetAbsOrigin();
	Vector vecAverage = vec3_origin;

	trace_t trace;
	for ( int i = 0; i < 3; i++ )
	{
		UTIL_TraceHull( GetAbsOrigin(), 
						vecFrameMovement, 
						CollisionProp()->OBBMins(), 
						CollisionProp()->OBBMaxs(), 
						GetAITraceMask(), 
						&filter, 
						&trace );

		// clear path, just move there
		if ( !trace.DidHit() )
			break;

		vecAverage += trace.plane.normal;

		Vector vecDifference = vecFrameMovement - GetAbsOrigin();
		if ( !trace.startsolid )
		{
			if ( ( vecFrameMovement - trace.endpos ).LengthSqr() >= 1.0f )
			{
				float flScale = vecDifference.Dot( trace.plane.normal ) * ( 1.0f - trace.fraction );
				vecFrameMovement = vecDifference + GetAbsOrigin() - ( trace.plane.normal * flScale );
				if ( ( vecFrameMovement - trace.endpos ).LengthSqr() >= 1.0f )
					continue;
			}
		}

		vecAverage.NormalizeInPlace();

		// bounce off of surfaces
		float flScale = vecAverage.x * m_localVelocity.x + vecAverage.y * m_localVelocity.y;
		flScale = ( flScale + vecAverage.z * m_localVelocity.z ) + ( flScale + vecAverage.z * m_localVelocity.z );
		m_localVelocity -= ( vecAverage * flScale );
	}

	SetAbsOrigin( trace.endpos );

	m_wishVelocity = vec3_origin;

	SetCurrentVelocity( GetCurrentVelocity() + VelocityToAvoidObstacles(flInterval) );

	LimitSpeed( tf_eyeball_boss_speed.GetFloat() );

	UpdateHead( flInterval );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CEyeBallBoss::MaintainAltitude( void )
{
	if ( IsAlive() )
	{
		CTraceFilterSimpleClassnameList filter( this, COLLISION_GROUP_NONE );
		filter.AddClassnameToIgnore( "eyeball_boss" );
		Vector vecStart = GetAbsOrigin();
		Vector vecEnd = GetAbsOrigin();

		trace_t ceiltrace;
		UTIL_TraceHull(
			vecStart,
			vecEnd + Vector( 0, 0, 1000.0f ),
			WorldAlignMins(),
			WorldAlignMaxs(),
			GetAITraceMask(),
			&filter,
			&ceiltrace
		);

		Vector vecAdditiveVec;

		vecAdditiveVec.x = m_vecMotion.x;
		vecAdditiveVec.y = m_vecMotion.y;
		vecAdditiveVec.z = 0.0f;
		vecAdditiveVec.NormalizeInPlace();

		trace_t floortrace;
		UTIL_TraceHull(
			vecStart + Vector(0, 0, ceiltrace.endpos.z - vecStart.z) + vecAdditiveVec * 50.0f,
			vecEnd + Vector( 0, 0, -2000.0f ) + vecAdditiveVec * 50.0f,
			WorldAlignMins() * 1.25f,
			WorldAlignMaxs() * 1.25f,
			GetAITraceMask(),
			&filter,
			&floortrace
		);

		m_wishVelocity.z += Clamp( GetDesiredAltitude() - ( vecStart.z - floortrace.endpos.z ),
								   -tf_eyeball_boss_acceleration.GetFloat(),
								   tf_eyeball_boss_acceleration.GetFloat() );
	}
	else
	{
		m_wishVelocity = Vector( 0, 0, -300.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CEyeBallBoss::TurnHeadToTarget( float flInterval, const Vector &MoveTarget )
{
	Vector vecTo = MoveTarget - WorldSpaceCenter();
	vecTo.z = 0;

	QAngle vecAng;
	VectorAngles( vecTo, vecAng );

	SetAbsAngles( vecAng );

	m_lookAtSpot = MoveTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &goalPos - 
//			&startPos - 
//			idealRange - 
//			idealHeight - 
// Output : Vector
//-----------------------------------------------------------------------------
Vector CEyeBallBoss::IdealGoalForMovement( const Vector &goalPos, const Vector &startPos, float idealRange, float idealHeightDiff )
{
	Vector	vMoveDir;

	if ( GetGoalDirection( &vMoveDir ) == false )
	{
		vMoveDir = ( goalPos - startPos );
		vMoveDir.z = 0;
		VectorNormalize( vMoveDir );
	}

	// Move up from the position by the desired amount
	Vector vIdealPos = goalPos + Vector( 0, 0, idealHeightDiff ) + ( vMoveDir * -idealRange );

	// Trace down and make sure we can fit here
	trace_t	tr;
	AI_TraceHull( vIdealPos, vIdealPos - Vector( 0, 0, MinGroundDist() ), GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	// Move up otherwise
	if ( tr.fraction < 1.0f )
	{
		vIdealPos.z += ( MinGroundDist() * ( 1.0f - tr.fraction ) );
	}

	return vIdealPos;
}

//-----------------------------------------------------------------------------
// Purpose: Accelerates toward a given position.
// Input  : flInterval - Time interval over which to move.
//			vecMoveTarget - Position to move toward.
//-----------------------------------------------------------------------------
void CEyeBallBoss::MoveToTarget( float flInterval, const Vector &vecMoveTarget )
{
	Vector vecGoal = vecMoveTarget - GetAbsOrigin();
	Vector vecVelocity = vecGoal.Normalized() * tf_eyeball_boss_acceleration.GetFloat();

	m_wishVelocity.x += vecVelocity.x;
	m_wishVelocity.y += vecVelocity.y;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEyeBallBoss::GetVelocity(Vector *vVelocity, AngularImpulse *vAngVelocity)
{
	if ( vVelocity != NULL )
		VectorCopy( m_localVelocity,*vVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CEyeBallBoss::GetGoalDistance( void )
{
	return tf_eyeball_boss_attack_range.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vOut - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CEyeBallBoss::GetGoalDirection( Vector *vOut )
{
	CBaseEntity *pTarget = GetTarget();

	if ( pTarget == NULL )
		return false;

	if ( FClassnameIs( pTarget, "info_hint_air" ) || FClassnameIs( pTarget, "info_target" ) )
	{
		AngleVectors( pTarget->GetAbsAngles(), vOut );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Vector CEyeBallBoss::VelocityToEvade(CBaseCombatCharacter *pEnemy)
{
	if (pEnemy)
	{
		// -----------------------------------------
		//  Keep out of enemy's shooting position
		// -----------------------------------------
		Vector vEnemyFacing = pEnemy->BodyDirection2D( );
		Vector	vEnemyDir   = pEnemy->EyePosition() - GetLocalOrigin();
		VectorNormalize(vEnemyDir);
		float  fDotPr		= DotProduct(vEnemyFacing,vEnemyDir);

		if (fDotPr < -0.9)
		{
			Vector vDirUp(0,0,1);
			Vector vDir;
			CrossProduct( vEnemyFacing, vDirUp, vDir);

			Vector crossProduct;
			CrossProduct(vEnemyFacing, vEnemyDir, crossProduct);
			if (crossProduct.y < 0)
			{
				vDir = vDir * -1;
			}
			return (vDir);
		}
		else if (fDotPr < -0.85)
		{
			Vector vDirUp(0,0,1);
			Vector vDir;
			CrossProduct( vEnemyFacing, vDirUp, vDir);

			Vector crossProduct;
			CrossProduct(vEnemyFacing, vEnemyDir, crossProduct);
			if (random->RandomInt(0,1))
			{
				vDir = vDir * -1;
			}
			return (vDir);
		}
	}
	return vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CEyeBallBoss::DrawDebugTextOverlays(void)
{
	int nOffset = BaseClass::DrawDebugTextOverlays();

	if ( m_debugOverlays & OVERLAY_TEXT_BIT ) 
	{
		Vector vel;
		GetVelocity( &vel, NULL );

		char tempstr[512];
		Q_snprintf( tempstr, sizeof(tempstr), "speed (max): %.2f (%.2f)", vel.Length(), m_flSpeed );
		EntityText( nOffset, tempstr, 0 );
		nOffset++;
	}

	return nOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CEyeBallBoss::GetHeadTurnRate( void ) 
{
	return 800.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
//-----------------------------------------------------------------------------
void CEyeBallBoss::UpdateHead( float flInterval )
{
	Vector vecTo = m_lookAtSpot - WorldSpaceCenter();
	vecTo.NormalizeInPlace();

	Vector vecFwd;
	GetVectors( &vecFwd, NULL, NULL );

	vecFwd += vecTo * 3.0f * TICK_INTERVAL;
	vecFwd.NormalizeInPlace();

	QAngle vecAng;
	VectorAngles( vecFwd, vecAng );

	SetAbsAngles( vecAng );
	if ( tf_eyeball_boss_debug_orientation.GetBool() )
	{
		NDebugOverlay::Line( WorldSpaceCenter(),
							 WorldSpaceCenter() + vecFwd * 150.0f,
							 255,
							 0,
							 205,
							 false,
							 1.0f );
	}

	StudioFrameAdvance();
	DispatchAnimEvents( this );

	if ( m_iAngerPose < 0 )
		m_iAngerPose = LookupPoseParameter( "anger" );

	Assert( m_iAngerPose >= 0 );

	m_attitude = ATTITUDE_CALM;

	if ( GetTeamNumber() == TF_TEAM_GREEN )
	{
		if ( GetHealth() < 2 * ( GetMaxHealth() / 3 ) && ( !m_attitudeTimer.HasStarted() || m_attitudeTimer.IsElapsed() ) )
		{
			if ( GetHealth() < ( GetMaxHealth() / 3 ) )
			{
				m_nSkin = 1; // Red iris
				m_attitude = ATTITUDE_ANGRY;
				SetPoseParameter( m_iAngerPose, 0.0f );

				return;
			}

			m_nSkin = 0; // Default
			m_attitude = ATTITUDE_GRUMPY;
			SetPoseParameter( m_iAngerPose, 0.4f );

			return;
		}

		m_nSkin = 0; // Default
	}
	else if ( GetTeamNumber() != TF_TEAM_GREEN )
	{
		switch ( GetTeamNumber() )
		{
			case TF_TEAM_RED:
				m_nSkin = 2; // Red skin
				m_attitude = ATTITUDE_HATEBLUE;
				break;
			case TF_TEAM_BLUE:
				m_nSkin = 3; // Blu skin
				m_attitude = ATTITUDE_HATERED;
				break;
			default:
				break;
		}
	}

	// Iris should be red when enraged
	if ( ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) && GetTeamNumber() == TF_TEAM_GREEN )
	{
		m_nSkin = 1; // Red iris
	}

	SetPoseParameter( m_iAngerPose, 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &linear - 
//			&angular - 
//-----------------------------------------------------------------------------
void CEyeBallBoss::ClampMotorForces( Vector &linear, AngularImpulse &angular )
{ 
	// limit reaction forces
	linear.x = clamp( linear.x, -500, 500 );
	linear.y = clamp( linear.y, -500, 500 );
	linear.z = clamp( linear.z, -500, 500 );

	// Add in weightlessness
	linear.z += 800;

	angular.z = clamp( angular.z, -GetHeadTurnRate(), GetHeadTurnRate() );
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CEyeBallBoss::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "Halloween.EyeballBossDie" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEyeBallBoss::ShouldPlayIdleSound( void )
{
	return m_idleNoiseTimer.IsElapsed();
}

//-----------------------------------------------------------------------------
// Purpose: Plays sounds while idle or in combat.
//-----------------------------------------------------------------------------
void CEyeBallBoss::IdleSound(void)
{
	if ( GetHealth() < ( GetMaxHealth() / 3 ) || ( m_attitudeTimer.HasStarted() && !m_attitudeTimer.IsElapsed() ) )
	{
		EmitSound( "Halloween.EyeballBossRage" );
		m_idleNoiseTimer.Start( RandomFloat( 1.0f, 2.0f ) );
	}
	else
	{
		EmitSound( "Halloween.EyeballBossIdle" );
		m_idleNoiseTimer.Start( RandomFloat( 3.0f, 5.0f ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Plays a sound when hurt.
//-----------------------------------------------------------------------------
void CEyeBallBoss::PainSound( const CTakeDamageInfo &info )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CEyeBallBoss::BecomeEnraged( float duration )
{
	if ( GetTeamNumber() == TF_TEAM_GREEN && GetHealth() >= ( GetMaxHealth() / 3 ) && ( !m_attitudeTimer.HasStarted() || m_attitudeTimer.IsElapsed() ) )
		EmitSound( "Halloween.EyeballBossBecomeEnraged" );

	m_attitudeTimer.Start( duration );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const Vector& CEyeBallBoss::PickNewSpawnSpot( void ) const
{
	static Vector spot = GetAbsOrigin();

	if ( !m_hSpawnEnts.IsEmpty() )
	{
		CBaseEntity *pSpot = m_hSpawnEnts.Random();
		if ( pSpot )
			spot = pSpot->GetAbsOrigin();
	}

	return spot;
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( eyeball_boss, CEyeBallBoss )

	DECLARE_TASK( TASK_EYEBALL_IDLE )
	DECLARE_TASK( TASK_EYEBALL_LAUNCH_ROCKET )
	DECLARE_TASK( TASK_EYEBALL_NOTICE )
	DECLARE_TASK( TASK_EYEBALL_ESCAPE )
	DECLARE_TASK( TASK_EYEBALL_TELEPORT )
	DECLARE_TASK( TASK_EYEBALL_EMOTE )

	//=========================================================
	// > SCHED_EYEBALL_IDLE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_IDLE,

		"	Tasks"
		"		TASK_EYEBALL_IDLE			0"
		""
		"	Interrupts"
	)

	//=========================================================
	// > SCHED_EYEBALL_EMERGE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_EMERGE,

		"	Tasks"
		"		TASK_EYEBALL_EMERGE			0"
		""
		"	Interrupts"
	)

	//=========================================================
	// > SCHED_EYEBALL_NOTICE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_NOTICE,

		"	Tasks"
		"		TASK_EYEBALL_NOTICE					0"
		""
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_LOST_ENEMY"
	)

	//=========================================================
	// > SCHED_EYEBALL_LAUNCH_ROCKET
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_LAUNCH_ROCKET,

		"	Tasks"
		"		TASK_EYEBALL_LAUNCH_ROCKET			0"
		"		TASK_WAIT							0.1"
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_LOST_ENEMY"
	)

	//=========================================================
	// > SCHED_EYEBALL_TELEPORT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_TELEPORT,

		"	Tasks"
		"		TASK_EYEBALL_TELEPORT					0"
		""
		"	Interrupts"
	)

	//=========================================================
	// > SCHED_EYEBALL_EMOTE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_EMOTE,

		"	Tasks"
		"		TASK_EYEBALL_EMOTE					0"
		""
		"	Interrupts"
	)

	//=========================================================
	// > SCHED_EYEBALL_STUN
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_STUN,

		"	Tasks"
		"		TASK_EYEBALL_STUN					0"
		""
		"	Interrupts"
	)

	//=========================================================
	// > SCHED_EYEBALL_ESCAPE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_EYEBALL_ESCAPE,

		"	Tasks"
		"		TASK_EYEBALL_ESCAPE					0"
		""
		"	Interrupts"
	)

AI_END_CUSTOM_NPC()
