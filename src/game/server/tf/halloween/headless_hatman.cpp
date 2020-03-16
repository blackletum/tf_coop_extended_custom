//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "headless_hatman.h"
#include "datacache/imdlcache.h"
#include "props_shared.h"
#include "tf_gamerules.h"
#include "team_control_point_master.h"
#include "nav_mesh/tf_nav_area.h"
#include "tf_team.h"
#include "tf_player.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "team_control_point_master.h"
#include "nav_mesh/tf_nav_area.h"
#include "ai_localnavigator.h"
#include "ai_memory.h"
#include "ai_network.h"
#include "ai_pathfinder.h"
#include "ai_navigator.h"
#include "ai_hint.h"
#include "tf_obj.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar tf_halloween_bot_health_base( "tf_halloween_bot_health_base", "3000", FCVAR_CHEAT );
ConVar tf_halloween_bot_health_per_player( "tf_halloween_bot_health_per_player", "200", FCVAR_CHEAT );
ConVar tf_halloween_bot_min_player_count( "tf_halloween_bot_min_player_count", "3", FCVAR_CHEAT );
ConVar tf_halloween_bot_speed( "tf_halloween_bot_speed", "400", FCVAR_CHEAT );
ConVar tf_halloween_bot_attack_range( "tf_halloween_bot_attack_range", "200", FCVAR_CHEAT );
ConVar tf_halloween_bot_speed_recovery_rate( "tf_halloween_bot_speed_recovery_rate", "100", FCVAR_CHEAT, "Movement units/second" );
ConVar tf_halloween_bot_chase_duration( "tf_halloween_bot_chase_duration", "30", FCVAR_CHEAT );
ConVar tf_halloween_bot_chase_range( "tf_halloween_bot_chase_range", "1500", FCVAR_CHEAT );
ConVar tf_halloween_bot_quit_range( "tf_halloween_bot_quit_range", "2000", FCVAR_CHEAT );
ConVar tf_halloween_bot_terrify_radius( "tf_halloween_bot_terrify_radius", "500", FCVAR_CHEAT );

LINK_ENTITY_TO_CLASS( headless_hatman, CHeadlessHatman );

IMPLEMENT_SERVERCLASS_ST( CHeadlessHatman, DT_HeadlessHatman )
END_SEND_TABLE()

#define	HATMAN_MODEL		"models/bots/headless_hatman.mdl"

enum
{
	SCHED_HATMAN_SEARCH_ENEMY = LAST_SHARED_SCHEDULE,
	SCHED_HATMAN_CHASE_ENEMY,
	SCHED_HATMAN_EMERGE
};

enum
{
	TASK_HATMAN_GET_PATH_TO_NEAREST_NODE = LAST_SHARED_TASK,
	TASK_HATMAN_GET_CHASE_PATH_ENEMY_TOLERANCE,
	TASK_HATMAN_EMERGE,
};

int CHeadlessHatman::gm_nMoveXPoseParam = -1;
int CHeadlessHatman::gm_nMoveYPoseParam = -1;

//==================================================
// CHeadlessHatman::m_DataDesc
//==================================================

BEGIN_DATADESC( CHeadlessHatman )

	DEFINE_FIELD( m_hTarget,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_hAimTarget,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_hOldTarget,					FIELD_EHANDLE ),

END_DATADESC()

//==================================================
// CHeadlessHatman
//==================================================

CHeadlessHatman::CHeadlessHatman( void )
{
	m_iMoveX = -1;
	m_iMoveY = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::UpdateOnRemove( void )
{
	TFGameRules()->RemoveBoss( this );
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::Precache( void )
{
	int iMdlIdx = PrecacheModel( "models/bots/headless_hatman.mdl" );
	PrecacheGibsForModel( iMdlIdx );

	// some gamerules checking for doomsday event happens here
	PrecacheModel( "models/weapons/c_models/c_bigaxe/c_bigaxe.mdl" );

	PrecacheScriptSound( "Halloween.HeadlessBossSpawn" );
	PrecacheScriptSound( "Halloween.HeadlessBossSpawnRumble" );
	PrecacheScriptSound( "Halloween.HeadlessBossAttack" );
	PrecacheScriptSound( "Halloween.HeadlessBossAlert" );
	PrecacheScriptSound( "Halloween.HeadlessBossBoo" );
	PrecacheScriptSound( "Halloween.HeadlessBossPain" );
	PrecacheScriptSound( "Halloween.HeadlessBossLaugh" );
	PrecacheScriptSound( "Halloween.HeadlessBossDying" );
	PrecacheScriptSound( "Halloween.HeadlessBossDeath" );
	PrecacheScriptSound( "Halloween.HeadlessBossAxeHitFlesh" );
	PrecacheScriptSound( "Halloween.HeadlessBossAxeHitWorld" );
	PrecacheScriptSound( "Halloween.HeadlessBossFootfalls" );
	PrecacheScriptSound( "Player.IsNowIt" );
	PrecacheScriptSound( "Player.YouAreIt" );
	PrecacheScriptSound( "Player.TaggedOtherIt" );

	PrecacheParticleSystem( "halloween_boss_summon" );
	PrecacheParticleSystem( "halloween_boss_axe_hit_world" );
	PrecacheParticleSystem( "halloween_boss_injured" );
	PrecacheParticleSystem( "halloween_boss_death" );
	PrecacheParticleSystem( "halloween_boss_foot_impact" );
	PrecacheParticleSystem( "halloween_boss_eye_glow" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CHeadlessHatman::GetWeaponModel( void ) const
{	// this isn't even used, but it returns either axe, or mallet, depending on gamerules condition (in doomsday event)
	return "models/weapons/c_models/c_bigaxe/c_bigaxe.mdl";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::Spawn( void )
{
	Precache();

	SetModel( HATMAN_MODEL );

	m_pAxe = (CBaseAnimating *)CreateEntityByName( "prop_dynamic" );
	if ( m_pAxe )
	{
		m_pAxe->SetModel( GetWeaponModel() );
		m_pAxe->FollowEntity( this );
	}

	SetHullType( HULL_MEDIUM_TALL );
	SetSolid( SOLID_BBOX );

	MDLCACHE_CRITICAL_SECTION();

	SetDefaultEyeOffset();

	SetMoveType( MOVETYPE_STEP );
	SetNavType( NAV_GROUND );
	SetBloodColor( DONT_BLEED );

	m_flGroundSpeed = tf_halloween_bot_speed.GetFloat();

	int iHealth = tf_halloween_bot_health_base.GetInt();
	int iNumPlayers = GetGlobalTFTeam( TF_TEAM_RED )->GetNumPlayers() + GetGlobalTFTeam( TF_TEAM_BLUE )->GetNumPlayers();
	int iMinPlayers = tf_halloween_bot_min_player_count.GetInt();
	if (iNumPlayers > iMinPlayers)
		iHealth += tf_halloween_bot_health_per_player.GetInt() * ( iNumPlayers - iMinPlayers );

	SetMaxHealth( iHealth );
	SetHealth( iHealth );

	if ( gm_nMoveXPoseParam == -1 )
	{
		gm_nMoveXPoseParam = LookupPoseParameter( "move_x" );
		gm_nMoveYPoseParam = LookupPoseParameter( "move_y" );
	}

	ClearHintGroup();

	m_attackTimer.Invalidate();
	m_evilCackleTimer.Invalidate();
	m_chaseDuration.Invalidate();
	m_forcedTargetDuration.Invalidate();
	m_shakeTimer.Invalidate();
	m_dyingTimer.Invalidate();

	m_hTarget = NULL;
	m_hAimTarget = NULL;
	m_hOldTarget = NULL;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_SQUAD );
	CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );

	NPCInit();

	BaseClass::Spawn();
	TFGameRules()->RegisterBoss( this );

	SetSchedule( SCHED_HATMAN_EMERGE );

	AddSolidFlags( FSOLID_NOT_SOLID );

	// Do not dissolve
	AddEFlags( EFL_NO_DISSOLVE );

	GetEnemies()->SetEnemyDiscardTime( tf_halloween_bot_chase_duration.GetFloat() );
	GetEnemies()->SetFreeKnowledgeDuration( tf_halloween_bot_chase_duration.GetFloat() * 2 );

	m_terrifyTimer.Start( 20.0f );
	m_vecHome = GetAbsOrigin();
	m_recomputeHomeTimer.Start( 3.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::Activate( void )
{
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Our enemy is unreachable. Select a schedule.
//-----------------------------------------------------------------------------
int CHeadlessHatman::SelectUnreachableSchedule( void )
{
	// Fire that we're unable to reach our target!
	if ( GetEnemy() && GetEnemy()->IsPlayer() )
	{
		m_OnLostPlayer.FireOutput( this, this );
	}

	m_OnLostEnemy.FireOutput( this, this );
	GetEnemies()->MarkAsEluded( GetEnemy() );

	// Move randomly for the moment
	return SCHED_HATMAN_SEARCH_ENEMY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CHeadlessHatman::SelectCombatSchedule( void )
{
	ClearHintGroup();

	// Attack if we can
	if ( HasCondition(COND_CAN_MELEE_ATTACK1) )
		return SCHED_MELEE_ATTACK1;

	// See if we can bark
	if ( HasCondition( COND_ENEMY_UNREACHABLE ) )
		return SelectUnreachableSchedule();

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CHeadlessHatman::SelectSchedule( void )
{
	//if ( !m_emergeTimer.IsElapsed() )
	//	return SCHED_IDLE_STAND;

	//Only do these in combat states
	if ( m_NPCState == NPC_STATE_COMBAT && GetEnemy() )
		return SelectCombatSchedule();

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CHeadlessHatman::MaxYawSpeed( void )
{
	Activity eActivity = GetActivity();

	// Stay still
	if (( eActivity == ACT_TRANSITION ) || 
		( eActivity == ACT_DIESIMPLE ) ||
		( eActivity == ACT_MELEE_ATTACK1 ) )
		return 0.0f;

	return 90.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CHeadlessHatman::HandleAnimEvent( animevent_t *pEvent )
{
	BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
int CHeadlessHatman::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	DispatchParticleEffect( "halloween_boss_injured", info.GetDamagePosition(), GetAbsAngles(), NULL );

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pAttacker - 
//			flDamage - 
//			&vecDir - 
//			*ptr - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
void CHeadlessHatman::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	CTakeDamageInfo info = inputInfo;

	// Bullets are weak against us, buckshot less so
	if ( info.GetDamageType() & DMG_BUCKSHOT )
	{
		info.ScaleDamage( 0.5f );
	}
	else if ( info.GetDamageType() & DMG_BULLET )
	{
		info.ScaleDamage( 0.25f );
	}

	// Make sure we haven't rounded down to a minimal amount
	if ( info.GetDamage() < 1.0f )
	{
		info.SetDamage( 1.0f );
	}

	BaseClass::TraceAttack( info, vecDir, ptr, pAccumulator );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CHeadlessHatman::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_HATMAN_EMERGE:
		{
			m_emergeTimer.Start( 3.0f );

			DispatchParticleEffect( "halloween_boss_summon", GetAbsOrigin(), GetAbsAngles(), NULL );

			m_vecTarget = GetAbsOrigin() + Vector( 0, 0, 10 );
			m_flDistance = 200.0f;

			SetAbsOrigin( m_vecTarget - Vector( 0, 0, 200 ) );
			EmitSound( "Halloween.HeadlessBossSpawnRumble" );

			IGameEvent *event = gameeventmanager->CreateEvent( "pumpkin_lord_summoned" );
			if (event)
				gameeventmanager->FireEvent( event );

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
				SetSchedule( TASK_HATMAN_GET_CHASE_PATH_ENEMY_TOLERANCE );
				TaskComplete();
			}
			break;
		}
	case TASK_HATMAN_GET_PATH_TO_NEAREST_NODE:
		{
			if ( !GetEnemy() )
			{
				TaskFail( FAIL_NO_ENEMY );
				break;
			}

			// Find the nearest node to the enemy
			int node = GetNavigator()->GetNetwork()->NearestNodeToPoint( this, GetEnemy()->GetAbsOrigin(), false );
			CAI_Node *pNode = GetNavigator()->GetNetwork()->GetNode( node );
			if( pNode == NULL )
			{
				TaskFail( FAIL_NO_ROUTE );
				break;
			}

			Vector vecNodePos = pNode->GetPosition( GetHullType() );
			AI_NavGoal_t goal( GOALTYPE_LOCATION, vecNodePos, ACT_RUN );
			if ( GetNavigator()->SetGoal( goal ) )
			{
				GetNavigator()->SetArrivalDirection( GetEnemy() );
				TaskComplete();
				break;
			}


			TaskFail( FAIL_NO_ROUTE );
			break;
		}
		break;

	case TASK_HATMAN_GET_CHASE_PATH_ENEMY_TOLERANCE:
		{
			// Chase the enemy, but allow local navigation to succeed if it gets within the goal tolerance
			GetNavigator()->SetLocalSucceedOnWithinTolerance( true );

			if ( GetNavigator()->SetGoal( GOALTYPE_ENEMY ) )
			{
				TaskComplete();
			}
			else
			{
				RememberUnreachable(GetEnemy());
				TaskFail(FAIL_NO_ROUTE);
			}

			GetNavigator()->SetLocalSucceedOnWithinTolerance( false );
		}
		break;

		case TASK_MELEE_ATTACK1:
		{
			SetLastAttackTime( gpGlobals->curtime );
			if ( !IsRangeGreaterThan( m_hTarget, 100.0f ) && IsAbleToSee( m_hTarget, CBaseCombatCharacter::USE_FOV ) )
			{
				if ( IsRangeLessThan( m_hTarget, tf_halloween_bot_attack_range.GetFloat() ) )
				{
					if ( m_terrifyTimer.IsElapsed() && ( m_hTarget->IsPlayer() || m_hTarget->IsNPC() )  )
					{
						m_terrifyTimer.Reset();
						AddGesture( ACT_MP_GESTURE_VC_HANDMOUTH_ITEM1 );
						m_booDelay.Start( 0.25f );
						m_stunDelay.Start( 0.75f );
						m_actionDuration.Start( 1.25f );
					}

					if ( m_attackDuration.IsElapsed() && m_attackTimer.IsElapsed() )
					{
						AddGesture( ACT_MP_ATTACK_STAND_ITEM1 );

						m_attackTimer.Start( 0.58f );
						EmitSound( "Halloween.HeadlessBossAttack" );
						m_attackDuration.Start( 1.0f );
					}
				}

				UpdateAxeSwing();
			}
			break;
		}

		case TASK_DIE:
		{
			GetNavigator()->StopMoving();	
			ResetActivity();
			SetIdealActivity( ACT_DIESIMPLE );
			m_lifeState = LIFE_DYING;

			break;
		}

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CHeadlessHatman::RunTask( const Task_t *pTask )
{
	BaseClass::RunTask(pTask);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::FoundEnemy( void )
{
	SetState( NPC_STATE_COMBAT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::LostEnemy( void )
{
	SetState( NPC_STATE_ALERT );

	m_OnLostPlayer.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : baseAct - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CHeadlessHatman::NPC_TranslateActivity( Activity baseAct )
{
	if ( ( baseAct == ACT_IDLE ) )
		return (Activity) ACT_MP_STAND_ITEM1;

	if ( ( baseAct == ACT_RUN ) || ( baseAct == ACT_WALK ) )
		return (Activity) ACT_MP_RUN_ITEM1;

	if ( ( baseAct == ACT_MELEE_ATTACK1 ) )
		return (Activity) ACT_MP_STAND_ITEM1;	//return (Activity) ACT_MP_ATTACK_STAND_ITEM1;

	return baseAct;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CHeadlessHatman::GetIdealSpeed( ) const
{
	return tf_halloween_bot_speed.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CHeadlessHatman::GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence )
{
	return tf_halloween_bot_speed.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHeadlessHatman::ShouldWatchEnemy( void )
{
	Activity nActivity = GetActivity();

	if ( ( nActivity == ACT_TRANSITION ) || 
		 ( nActivity == ACT_DIESIMPLE ) )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHeadlessHatman::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
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
void CHeadlessHatman::StartTouch( CBaseEntity *pOther )
{
	if ( pOther && !InSameTeam( pOther ) )
	{
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( pOther );
		if ( pBCC )
		{
			float flDamage = pBCC->GetHealth();
			CTakeDamageInfo info( this, this, flDamage, DMG_CLUB|DMG_SLASH, TF_DMG_CUSTOM_DECAPITATION_BOSS );

			if ( pBCC->ClassMatches( "npc_rollermine" ) || pBCC->ClassMatches( "npc_manhack" ) )
				pBCC->TakeDamage( info );

			CBaseObject *pObj = dynamic_cast<CBaseObject*> ( pBCC );
			if ( pObj )
				pObj->TakeDamage( info ); //pObj->DetonateObject();

			if ( pBCC->IsAlive() )
			{
				m_hTarget = pBCC;
				m_forcedTargetDuration.Start( 3.0f );
			}
		}

		// surprise mother fricker
		/*if ( pOther->ClassMatches( "prop_door_rotating" ) )
			UTIL_Remove( pOther );*/
	}

	BaseClass::StartTouch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	// Don't do anything after death
	if ( m_NPCState == NPC_STATE_DEAD )
	{
		return;
	}

	if ( !m_emergeTimer.IsElapsed() )
	{
		//Vector vec = m_vecTarget + Vector( 0, 0, ( m_emergeTimer.GetRemainingTime() * -m_flDistance ) / m_emergeTimer.GetCountdownDuration() );
		//SetAbsOrigin( vec );

		if ( m_shakeTimer.IsElapsed() )
		{
			m_shakeTimer.Start( 0.25f );
			UTIL_ScreenShake( GetAbsOrigin(), 15.0f, 5.0f, 1.0f, 1000.0f, SHAKE_START );
		}
		return;
	}
	else
	{
		RemoveSolidFlags( FSOLID_NOT_SOLID );
	}

	//if ( IsCurSchedule( SCHED_HATMAN_SEARCH_ENEMY ) )
		SelectVictim();

	RecomputeHomePosition();

	if ( m_evilCackleTimer.IsElapsed() )
	{
		m_evilCackleTimer.Start( RandomFloat( 3.0f, 5.0f ) );

		EmitSound( "Halloween.HeadlessBossLaugh" );

		int rand = RandomInt( 0, 100 );
		if (rand <= 24)
		{
			AddGesture( ACT_MP_GESTURE_VC_FISTPUMP_MELEE );
		}
		else if (rand <= 49)
		{
			AddGesture( ACT_MP_GESTURE_VC_FINGERPOINT_MELEE );
		}
	}

	CBaseCombatCharacter *pVictim = ToBaseCombatCharacter( TFGameRules()->GetIT() );
	if ( pVictim )
	{
		SetEnemy( pVictim );
		UpdateEnemyMemory( pVictim, pVictim->GetAbsOrigin() );

		if ( m_notifyVictimTimer.IsElapsed() )
		{
			CBasePlayer *pPlayer = ToBasePlayer( pVictim );
			if ( pPlayer )
			{
				m_notifyVictimTimer.Start( 7.0f );
				ClientPrint( pPlayer, HUD_PRINTCENTER, "#TF_HALLOWEEN_BOSS_WARN_VICTIM", pPlayer->GetPlayerName() );
			}
		}

		if ( !IsRangeGreaterThan( pVictim, 100.0f ) && IsAbleToSee( pVictim, CBaseCombatCharacter::USE_FOV ) )
		{
			if ( IsRangeLessThan( pVictim, tf_halloween_bot_attack_range.GetFloat() ) )
			{
				if ( m_terrifyTimer.IsElapsed() && ( pVictim->IsPlayer() || pVictim->IsNPC() )  )
				{
					m_terrifyTimer.Reset();
					AddGesture( ACT_MP_GESTURE_VC_HANDMOUTH_ITEM1 );
					m_booDelay.Start( 0.25f );
					m_stunDelay.Start( 0.75f );
					m_actionDuration.Start( 1.25f );
				}

				if ( m_attackDuration.IsElapsed() && m_attackTimer.IsElapsed() )
				{
					AddGesture( ACT_MP_ATTACK_STAND_ITEM1 );

					m_attackTimer.Start( 0.58f );
					EmitSound( "Halloween.HeadlessBossAttack" );
					m_attackDuration.Start( 1.0f );
				}
			}

			UpdateAxeSwing();
		}
	}

	if ( m_hTarget && m_hTarget->IsAlive() )
	{
		SetEnemy( m_hTarget );
		UpdateEnemyMemory( m_hTarget, m_hTarget->GetAbsOrigin() );

		if ( IsRangeLessThan( m_hTarget, tf_halloween_bot_attack_range.GetFloat() ) )
		{
			if ( m_terrifyTimer.IsElapsed() && ( m_hTarget->IsPlayer() || m_hTarget->IsNPC() ) )
			{
				m_terrifyTimer.Reset();
				AddGesture( ACT_MP_GESTURE_VC_HANDMOUTH_ITEM1 );
				m_booDelay.Start( 0.25f );
				m_stunDelay.Start( 0.75f );
				m_actionDuration.Start( 1.25f );
			}

			if ( m_attackDuration.IsElapsed() && m_attackTimer.IsElapsed() )
			{
				AddGesture( ACT_MP_ATTACK_STAND_ITEM1 );

				m_attackTimer.Start( 0.58f );
				EmitSound( "Halloween.HeadlessBossAttack" );
				m_attackDuration.Start( 1.0f );
			}
		}
	}

	UpdateAxeSwing();

	if ( !m_actionDuration.IsElapsed() )
	{
		if ( m_booDelay.HasStarted() && m_booDelay.IsElapsed() )
		{
			m_booDelay.Invalidate();
			EmitSound( "Halloween.HeadlessBossBoo" );
		}

		if ( m_stunDelay.IsElapsed() )
		{
			for ( int iTeam = FIRST_GAME_TEAM; iTeam < TFTeamMgr()->GetTeamCount(); ++iTeam )
			{
				CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
				if ( pTeam )
				{
					for ( int i = 1; i < pTeam->GetNumPlayers(); i++ )
					{
						CTFPlayer *pPlayer = ToTFPlayer( pTeam->GetPlayer( i ) );
						if ( pPlayer && pPlayer->IsAlive() )
						{
							if ( IsRangeLessThan( pPlayer, tf_halloween_bot_terrify_radius.GetFloat() ) )
							{
								if ( !IsAbleToSee( pPlayer, CBaseCombatCharacter::USE_FOV ) )
									continue;

								pPlayer->m_Shared.StunPlayer( 2.0f, 0.0f, 0.0f, TF_STUNFLAGS_GHOSTSCARE, NULL );
							}
						}
					}

					for ( int i = 1; i < pTeam->GetNumNPCs(); i++ )
					{
						CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pTeam->GetNPC( i ) );
						if ( pNPC && pNPC->IsAlive() )	
						{
							if ( IsRangeLessThan( pNPC, tf_halloween_bot_terrify_radius.GetFloat() ) )
							{
								if ( !IsAbleToSee( pNPC, CBaseCombatCharacter::USE_FOV ) )
									continue;

								pNPC->StunNPC( 2.0f, 0.0f, 0.0f, TF_STUNFLAGS_GHOSTSCARE, NULL );
							}
						}
					}
				}
			}
		}
	}

	m_flGroundSpeed = tf_halloween_bot_speed.GetFloat();

	// IM VERY STRONG AND SPOOKY
	CSoundEnt::InsertSound( SOUND_DANGER, WorldSpaceCenter(), 300, 0.1f, this, SOUNDENT_CHANNEL_REPEATED_DANGER );		
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::AttackTarget( CBaseCombatCharacter *victim, float dist )
{
	if ( IsRangeLessThan( victim, dist ) )
	{
		Vector vecFwd;
		GetVectors( &vecFwd, nullptr, nullptr );

		Vector vecToActor = ( victim->WorldSpaceCenter() - WorldSpaceCenter() );
		vecToActor.NormalizeInPlace();

		float flComp;
		float flDist = GetRangeTo( victim );
		if (flDist >= ( dist * 0.5f ))
			flComp = ( ( flDist - ( dist * 0.5f ) ) / ( dist * 0.5f ) ) * 0.27f;
		else
			flComp = 0.0f;

		if ( vecToActor.Dot( vecFwd ) > flComp )
		{
			if ( IsAbleToSee( victim, CBaseCombatCharacter::USE_FOV ) )
			{	// this seems wrong, but it seems victim can only ever be m_hTarget anyway
				float flDamage = m_hTarget->GetMaxHealth() * 0.8f;
				CTakeDamageInfo info( this, this, flDamage, DMG_CLUB|DMG_SLASH, TF_DMG_CUSTOM_DECAPITATION_BOSS );
				CalculateMeleeDamageForce( &info, vecFwd, WorldSpaceCenter(), 5.0f );

				m_hTarget->TakeDamage( info );
				EmitSound( "Halloween.HeadlessBossAxeHitFlesh" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::SelectVictim( void )
{
	ValidateChaseVictim();

	m_hAimTarget = ToBaseCombatCharacter( TFGameRules()->GetIT() );

	if ( !TFGameRules()->GetIT() )
	{
		float flDist1 = FLT_MAX;
		float flDist2 = FLT_MAX;
		CBaseEntity *candidate = nullptr;

		const Vector vecRangeSize = Vector( 3000, 3000, 3000 );

		CBaseEntity *pList[64];
		int count = UTIL_EntitiesInBox( pList, 64, GetAbsOrigin() - vecRangeSize, GetAbsOrigin() + vecRangeSize, 0 );

		for ( int i = 0; i < count; i++ )
		{
			CBaseCombatCharacter *pEntity = ToBaseCombatCharacter( pList[i] );
			if ( pEntity && IsPotentiallyChaseable( pEntity ) )
			{
				float flDistance = GetRangeSquaredTo( pEntity );
				if ( flDist2 > flDistance )
				{
					if ( IsAbleToSee( pEntity, CBaseCombatCharacter::USE_FOV ) )
					{
						flDist2 = flDistance;
						m_hAimTarget = pEntity;
					}

					if ( ( m_vecHome - pEntity->GetAbsOrigin() ).LengthSqr() <= Square( tf_halloween_bot_chase_range.GetFloat() ) )
					{
						if ( flDist1 > flDistance )
						{
							flDist1 = flDistance;
							candidate = pEntity;
						}
					}
				}
			}
		}

		if ( candidate )
		{
			m_chaseDuration.Start( tf_halloween_bot_chase_duration.GetFloat() );
			TFGameRules()->SetIT( candidate );
		}
	}

	if ( !m_hTarget )
	{
		m_hTarget = ToBaseCombatCharacter( TFGameRules()->GetIT() );
		return;
	}

	if ( m_forcedTargetDuration.IsElapsed() )
	{
		m_hTarget = ToBaseCombatCharacter( TFGameRules()->GetIT() );
		return;
	}

	if( !m_hTarget->IsAlive() )
		m_hTarget = ToBaseCombatCharacter( TFGameRules()->GetIT() );
}

CON_COMMAND_F( tf_halloween_force_it, "For testing.", FCVAR_CHEAT )
{
	TFGameRules()->SetIT( UTIL_GetCommandClient() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::ValidateChaseVictim( void )
{
	CBaseCombatCharacter *pIT = ToBaseCombatCharacter( TFGameRules()->GetIT() );
	if ( pIT && (!m_hOldTarget || m_hOldTarget != pIT) )
	{
		m_chaseDuration.Start( tf_halloween_bot_chase_duration.GetFloat() );
		m_hOldTarget = pIT;
	}

	if ( !IsPotentiallyChaseable( pIT ) )
	{
		TFGameRules()->SetIT( nullptr );
		//SetEnemy( nullptr );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHeadlessHatman::IsPotentiallyChaseable( CBaseCombatCharacter *pVictim )
{
	if ( !pVictim )
		return false;

	if ( !pVictim->IsAlive() )
		return false;

	if ( ( GetActivity() == ACT_TRANSITION ) )
		return false;

	if ( InSameTeam( pVictim ) )
		return false;

	if ( pVictim == this )
		return false;

	if ( pVictim->ClassMatches( "npc_bullseye" ) || pVictim->ClassMatches( "npc_sniper" ) || pVictim->ClassMatches( "npc_manhack" ) || pVictim->ClassMatches( "npc_rollermine" ) )
		return false;

	if ( !GetGroundEntity() )
	{
		if (( pVictim->GetAbsOrigin() - m_vecHome ).LengthSqr() > Square( tf_halloween_bot_quit_range.GetFloat() ))
			return false;
	}

	CTFNavArea *area = static_cast<CTFNavArea *>( pVictim->GetLastKnownArea() );
	if ( area )
	{
		if ( area->HasTFAttributes( RED_SPAWN_ROOM|BLUE_SPAWN_ROOM ) )
			return false;

		if ( GetGroundEntity() )
		{
			Vector vecSpot;
			area->GetClosestPointOnArea( pVictim->GetAbsOrigin(), &vecSpot );
			if (( pVictim->GetAbsOrigin() - vecSpot ).Length2DSqr() <= Square( 50.0f ))
			{
				if (( pVictim->GetAbsOrigin() - m_vecHome ).LengthSqr() > Square( tf_halloween_bot_quit_range.GetFloat() ))
					return false;
			}
		}

	}

	CTFPlayer *pPlayer = ToTFPlayer( pVictim );
	if ( pPlayer )
	{
		if ( pPlayer->m_Shared.IsInvulnerable() )
			return false;
	}

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pVictim );
	if ( pNPC )
	{
		if ( pNPC->IsInvulnerable() )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHeadlessHatman::IsSwingingAxe( void )
{
	return !m_attackTimer.IsElapsed();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::UpdateAxeSwing( void )
{
	if ( m_attackTimer.HasStarted() && m_attackTimer.IsElapsed() )
	{
		m_attackTimer.Invalidate();

		AttackTarget( m_hTarget, tf_halloween_bot_attack_range.GetFloat() );

		EmitSound( "Halloween.HeadlessBossAxeHitWorld" );

		UTIL_ScreenShake( GetAbsOrigin(), 150.0f, 5.0f, 1.0f, 1000.0f, SHAKE_START );

		if ( GetWeapon() )
		{
			Vector org; QAngle ang;
			if ( GetWeapon()->GetAttachment( "axe_blade", org, ang ) )
			{
				DispatchParticleEffect( "halloween_boss_axe_hit_world", org, ang, NULL );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::RecomputeHomePosition( void )
{
	if ( m_recomputeHomeTimer.IsElapsed() )
	{
		m_recomputeHomeTimer.Reset();

		if ( g_hControlPointMasters.IsEmpty() )
			return;

		CTeamControlPointMaster *pMaster = g_hControlPointMasters[0];
		for (int i=0; i<pMaster->GetNumPoints(); ++i)
		{
			CTeamControlPoint *pPoint = pMaster->GetControlPoint( i );
			if (pMaster->IsInRound( pPoint ) &&
				 ObjectiveResource()->GetOwningTeam( pPoint->GetPointIndex() ) != TF_TEAM_BLUE &&
				 TFGameRules()->TeamMayCapturePoint( TF_TEAM_BLUE, pPoint->GetPointIndex() ))
			{
				m_vecHome = pPoint->GetAbsOrigin();
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::GatherConditions( void )
{
	BaseClass::GatherConditions();
}

//-----------------------------------------------------------------------------
// Purpose: See if we're able to stand on the ground at this point
// Input  : &vecPos - Position to try
//			*pOut - Result position (only valid if we return true)
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool CHeadlessHatman::CanStandAtPoint( const Vector &vecPos, Vector *pOut )
{
	Vector vecStart = vecPos + Vector( 0, 0, (4*12) );
	Vector vecEnd = vecPos - Vector( 0, 0, (4*12) );
	trace_t	tr;
	bool bTraceCleared = false;

	// Start high and try to go lower, looking for the ground between here and there
	// We do this first because it's more likely to succeed in the typical guard arenas (with open terrain)
	UTIL_TraceHull( vecStart, vecEnd, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.startsolid && !tr.allsolid )
	{
		// We started in solid but didn't end up there, see if we can stand where we ended up
		UTIL_TraceHull( tr.endpos, tr.endpos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
		
		// Must not be in solid
		bTraceCleared = ( !tr.allsolid && !tr.startsolid );
	}
	else
	{
		// Must not be in solid and must have found a floor (otherwise we're potentially hanging over a ledge)
		bTraceCleared = ( tr.allsolid == false && tr.fraction < 1.0f );
	}

	// Either we're clear or we're still unlucky
	if ( bTraceCleared == false )
	{
		return false;
	}

	if ( pOut )
	{
		*pOut = tr.endpos;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CHeadlessHatman::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CHeadlessHatman::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	// Figure out what to do next
	if ( failedSchedule == SCHED_HATMAN_CHASE_ENEMY && HasCondition( COND_ENEMY_UNREACHABLE ) )
		return SelectUnreachableSchedule();

	return BaseClass::SelectFailSchedule( failedSchedule,failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : scheduleType - 
// Output : int
//-----------------------------------------------------------------------------
int CHeadlessHatman::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_CHASE_ENEMY:
		return SCHED_HATMAN_CHASE_ENEMY;
		break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHeadlessHatman::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "Halloween.HeadlessBossDying" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CHeadlessHatman::Event_Killed( const CTakeDamageInfo &info )
{
	CTFPlayer *pTFAttacker = ToTFPlayer( info.GetAttacker() );
	CBaseEntity *pAssister = TFGameRules()->GetAssister( this, pTFAttacker, info.GetInflictor() );

	if ( pTFAttacker )
	{
		CReliableBroadcastRecipientFilter filter;
		UTIL_SayText2Filter( filter, pTFAttacker, false, "#TF_Halloween_Boss_Killers", pTFAttacker->GetPlayerName() );
		if ( pAssister )
		{
			if ( pAssister->IsPlayer() )
			{
				UTIL_SayText2Filter( filter, ToTFPlayer( pAssister ), false, "#TF_Halloween_Boss_Killers", ToTFPlayer( pAssister )->GetPlayerName() );
			}
			else if ( pAssister->IsNPC() )
			{
				CAI_BaseNPC *pNPCAssister = dynamic_cast<CAI_BaseNPC *>( pAssister );
				if ( pNPCAssister )
					UTIL_SayText2Filter( filter, pTFAttacker, false, "#TF_Halloween_Boss_Killers", pNPCAssister->GetLocalizeName() );
			}
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "halloween_boss_killed" );
		if ( event )
		{
			event->SetInt( "boss", 1 );
			event->SetInt( "killer", engine->GetPlayerUserId( pTFAttacker->edict() ) );

			gameeventmanager->FireEvent( event );
		}
	}

	TFGameRules()->SetIT( nullptr );
	SetEnemy( nullptr );

	m_dyingTimer.Start( 3.0f );

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: Don't become a ragdoll until we've finished our death anim
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHeadlessHatman::CanBecomeRagdoll( void )
{
	if ( IsCurSchedule( SCHED_DIE ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &force - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHeadlessHatman::BecomeRagdollOnClient( const Vector &force )
{
	//if ( !m_dyingTimer.IsElapsed() )
	//	return;

	CPVSFilter filter( GetAbsOrigin() );
	UserMessageBegin( filter, "BreakModel" );
		WRITE_SHORT( GetModelIndex() );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_ANGLES( GetAbsAngles() );
		WRITE_SHORT( m_nSkin );
	MessageEnd();

	DispatchParticleEffect( "halloween_boss_death", GetAbsOrigin(), GetAbsAngles(), NULL );
	UTIL_Remove( this );

	IGameEvent *event = gameeventmanager->CreateEvent( "pumpkin_lord_killed" );
	if ( event )
		gameeventmanager->FireEvent( event );

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Handle squad or NPC interactions
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHeadlessHatman::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sender )
{
	return BaseClass::HandleInteraction( interactionType, data, sender );
}

//-----------------------------------------------------------------------------
// Purpose: Cache whatever pose parameters we intend to use
//-----------------------------------------------------------------------------
void CHeadlessHatman::PopulatePoseParameters( void )
{
	BaseClass::PopulatePoseParameters();
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( headless_hatman, CHeadlessHatman )

	//Tasks
	DECLARE_TASK( TASK_HATMAN_GET_PATH_TO_NEAREST_NODE )
	DECLARE_TASK( TASK_HATMAN_GET_CHASE_PATH_ENEMY_TOLERANCE )
	DECLARE_TASK( TASK_HATMAN_EMERGE )

	//=========================================================
	// SCHED_HATMAN_SEARCH_ENEMY
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_HATMAN_SEARCH_ENEMY,

		"	Tasks"
		"		TASK_SET_ROUTE_SEARCH_TIME					3"	// Spend 3 seconds trying to build a path if stuck
		"		TASK_HATMAN_GET_PATH_TO_NEAREST_NODE		500"
		"		TASK_RUN_PATH								0"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_HEAVY_DAMAGE"
	);

	//=========================================================
	// SCHED_HATMAN_CHASE_ENEMY
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_HATMAN_CHASE_ENEMY,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_TOLERANCE_DISTANCE		2000"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	1500"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_FACE_ENEMY					0"
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_TASK_FAILED"
		"		COND_LOST_ENEMY"
	)

	//=========================================================
	// > SCHED_HATMAN_EMERGE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_HATMAN_EMERGE,

		"	Tasks"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_TRANSITION"
		"		TASK_HATMAN_EMERGE			0"
		""
		"	Interrupts"
	)


AI_END_CUSTOM_NPC()