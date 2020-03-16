//==== Copyright © 2012, Sandern Corporation, All rights reserved. =========
//
//
//=============================================================================

#include "cbase.h"
#include "zombie.h"
#include "tf_player.h"
#include "tf_wearable.h"
#include "particle_parse.h"
#include "props_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	TF_ZOMBIE_MELEE1_RANGE		100.0f

ConVar tf_halloween_zombie_damage( "tf_halloween_zombie_damage", "10", FCVAR_CHEAT );
ConVar tf_halloween_zombie_speed( "tf_halloween_zombie_speed", "300", FCVAR_CHEAT );

LINK_ENTITY_TO_CLASS( tf_zombie, CTFZombie );

IMPLEMENT_SERVERCLASS_ST( CTFZombie, DT_TFZombie )
	SendPropInt( SENDINFO( m_nSkeletonType ) ),
	SendPropInt( SENDINFO( m_nZombieClass ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CTFZombie )
	DEFINE_FIELD( m_hZombieOwner,		FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_nSkeletonType,	FIELD_INTEGER, "zombie_type" ),
END_DATADESC()

int CTFZombie::gm_nMoveXPoseParam = -1;
int CTFZombie::gm_nMoveYPoseParam = -1;

CTFZombie::CTFZombie()
{
	m_nSkeletonType = SKELETON_NORMAL;
	m_nZombieClass = TF_CLASS_UNDEFINED;

	m_timeTillDeath.Invalidate();
	m_flAttDamage = 30.0f;
	m_flAttRange = 50.0f;
}

CTFZombie::~CTFZombie()
{
}

void CTFZombie::Precache()
{
	BaseClass::Precache();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	int iMdlIdx = PrecacheModel( "models/bots/skeleton_sniper/skeleton_sniper.mdl" );
	PrecacheGibsForModel( iMdlIdx );

	iMdlIdx = PrecacheModel( "models/bots/skeleton_sniper_boss/skeleton_sniper_boss.mdl" );
	PrecacheGibsForModel( iMdlIdx );

	PrecacheModel( "models/player/items/demo/crown.mdl" );

	PrecacheModel( "models/player/items/scout/scout_zombie.mdl" );
	PrecacheModel( "models/player/items/sniper/sniper_zombie.mdl" );
	PrecacheModel( "models/player/items/soldier/soldier_zombie.mdl" );
	PrecacheModel( "models/player/items/demo/demo_zombie.mdl" );
	PrecacheModel( "models/player/items/medic/medic_zombie.mdl" );
	PrecacheModel( "models/player/items/heavy/heavy_zombie.mdl" );
	PrecacheModel( "models/player/items/pyro/pyro_zombie.mdl" );
	PrecacheModel( "models/player/items/spy/spy_zombie.mdl" );
	PrecacheModel( "models/player/items/engineer/engineer_zombie.mdl" );

	PrecacheParticleSystem( "bomibomicon_ring" );
	PrecacheParticleSystem( "spell_pumpkin_mirv_goop_red" );
	PrecacheParticleSystem( "spell_pumpkin_mirv_goop_blue" );
	PrecacheParticleSystem( "spell_skeleton_goop_green" );

	PrecacheScriptSound( "Halloween.skeleton_break" );
	PrecacheScriptSound( "Halloween.skeleton_laugh_small" );
	PrecacheScriptSound( "Halloween.skeleton_laugh_medium" );
	PrecacheScriptSound( "Halloween.skeleton_laugh_giant" );

	CBaseEntity::SetAllowPrecache( allowPrecache );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::Spawn()
{
	Precache();

	SetModel( "models/bots/skeleton_sniper/skeleton_sniper.mdl" );

	BaseClass::Spawn();

	SetHullType( HULL_HUMAN );
	SetSolid( SOLID_BBOX );
	SetDefaultEyeOffset();

	SetNavType( NAV_GROUND );
	SetBloodColor( BLOOD_COLOR_ZOMBIE );
	m_NPCState = NPC_STATE_NONE;
	
	m_iHealth = m_iMaxHealth = 50;

	m_flFieldOfView		= 0.2;

	SetMoveType( MOVETYPE_STEP );

	SetupGlobalModelData();
	
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_SQUAD );
	CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );

	m_hZombieOwner = NULL;
	SetSkeletonType( m_nSkeletonType );

	m_evilCackleTimer.Invalidate();

	AddEFlags( EFL_NO_DISSOLVE );

	NPCInit();

	m_specialAttackTimer.Start( RandomFloat( 5.0f, 10.0f ) );

	SetAbsAngles( QAngle( 0, RandomFloat( 0, 360.f ), 0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::Activate()
{
	BaseClass::Activate();

	SetupGlobalModelData();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::SetupGlobalModelData()
{
	if ( gm_nMoveXPoseParam != -1 )
		return;

	gm_nMoveXPoseParam = LookupPoseParameter( "move_x" );
	gm_nMoveYPoseParam = LookupPoseParameter( "move_y" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFZombie::GetIdealSpeed( ) const
{
	return tf_halloween_zombie_speed.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFZombie::GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence )
{
	return tf_halloween_zombie_speed.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity CTFZombie::NPC_TranslateActivity( Activity baseAct )
{
	if ( ( baseAct == ACT_IDLE ) )
		return (Activity) ACT_MP_STAND_MELEE;

	if ( ( baseAct == ACT_RUN ) || ( baseAct == ACT_WALK ) )
		return (Activity) ACT_MP_RUN_MELEE;

	if ( ( baseAct == ACT_MELEE_ATTACK1 ) )
		return (Activity) ACT_MP_STAND_MELEE; // ACT_MP_ATTACK_STAND_ITEM1

	return baseAct;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFZombie::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
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
void CTFZombie::PainSound( const CTakeDamageInfo &info )
{
	//EmitSound( "Zombie.Pain" );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CTFZombie::DeathSound( const CTakeDamageInfo &info ) 
{
	if ( m_nSkeletonType <= SKELETON_KING )
	{
		EmitSound( "Halloween.skeleton_break" );
	}
	else
	{
		EmitSound( "TFPlayer.Decapitated" );
		/*CTFPlayer *pOwner = ToTFPlayer( m_hZombieOwner );
		if ( !pOwner )
			return;
		
		TFPlayerClassData_t *pData = pOwner->GetPlayerClass()->GetData();
		if ( !pData )
			return;

		CPASAttenuationFilter filter( GetAbsOrigin() );

		EmitSound_t ep;
		ep.m_nChannel = CHAN_VOICE;
		ep.m_flVolume = 1.0f;
		ep.m_nPitch = 0.8;
		ep.m_SoundLevel = SNDLVL_NORM;

		if ( info.GetDamageType() & DMG_BLAST )
			ep.m_pSoundName = pData->m_szExplosionDeathSound;
		else if ( info.GetDamageType() & DMG_CRITICAL )
			ep.m_pSoundName = pData->m_szCritDeathSound;
		else if ( info.GetDamageType() & (DMG_CLUB | DMG_SLASH) )
			ep.m_pSoundName = pData->m_szMeleeDeathSound;
		else
			ep.m_pSoundName = pData->m_szDeathSound;
		
		EmitSound( filter, entindex(), ep );*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::AlertSound( void )
{
	//EmitSound( "Zombie.Alert" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::StartTask( const Task_t *pTask )
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
void CTFZombie::RunTask( const Task_t *pTask )
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
void CTFZombie::RunAttackTask( int task )
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
void CTFZombie::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	if ( m_timeTillDeath.HasStarted() && m_timeTillDeath.IsElapsed() )
	{
		CTakeDamageInfo info( this, this, 1000, DMG_GENERIC );
		TakeDamage( info );
		return;
	}

	if ( m_nSkeletonType == SKELETON_ZOMBIE_FAST )
		m_flGroundSpeed = tf_halloween_zombie_speed.GetFloat();
	else if ( m_nSkeletonType == SKELETON_ZOMBIE_POISON )
		m_flGroundSpeed = tf_halloween_zombie_speed.GetFloat() * 1.2;
	else
		m_flGroundSpeed = tf_halloween_zombie_speed.GetFloat() / 1.2;

	if ( m_evilCackleTimer.IsElapsed() )
	{
		if ( m_nSkeletonType == SKELETON_NORMAL )
		{
			m_evilCackleTimer.Start( RandomFloat( 2.0, 3.0 ) );
			EmitSound( "Halloween.skeleton_laugh_small" );
		}
		else if ( m_nSkeletonType == SKELETON_NORMAL )
		{
			EmitSound( "Halloween.skeleton_laugh_medium" );
			m_evilCackleTimer.Start( RandomFloat( 6.0, 7.0 ) );
		}
		else if ( m_nSkeletonType == SKELETON_KING )
		{
			EmitSound( "Halloween.skeleton_laugh_giant" );
			m_evilCackleTimer.Start( RandomFloat( 4.0, 5.0 ) );
		}
	}

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
void CTFZombie::Event_Killed( const CTakeDamageInfo &info )
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
bool CTFZombie::BecomeRagdollOnClient( const Vector &force )
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
int CTFZombie::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if ( !IsPlayingGesture( ACT_MP_GESTURE_FLINCH_ITEM1 ) )
		AddGesture( ACT_MP_GESTURE_FLINCH_ITEM1 );

	char const *pszDamageEffect = "spell_skeleton_goop_green";
	if ( m_nSkeletonType <= SKELETON_KING )
	{
		switch ( GetTeamNumber() )
		{
			case TF_TEAM_RED:
				pszDamageEffect = "spell_pumpkin_mirv_goop_red";
				break;
			case TF_TEAM_BLUE:
				pszDamageEffect = "spell_pumpkin_mirv_goop_blue";
				break;
			default:
				break;
		}
	}

	DispatchParticleEffect( pszDamageEffect, info.GetDamagePosition(), GetAbsAngles() );

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::UpdateOnRemove( void )
{
	if ( m_pHat )
		UTIL_Remove( m_pHat );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::SetZombieOwner( CBaseEntity *pOwner, CBaseEntity *pKiller )
{
	if ( !pOwner )
		return;

	if ( !pKiller )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( pOwner );
	if ( !pPlayer )
		return;

	if ( pKiller->ClassMatches( "npc_headcrab" ) || pKiller->ClassMatches( "monster_headcrab" ) )
		SetSkeletonType( SKELETON_ZOMBIE );
	else if ( pKiller->ClassMatches( "npc_headcrab_fast" ) )
		SetSkeletonType( SKELETON_ZOMBIE_FAST );
	else if ( pKiller->ClassMatches( "npc_headcrab_poison" ) )
		SetSkeletonType( SKELETON_ZOMBIE_POISON );

	UTIL_Remove( pKiller );
	m_hZombieOwner =  pPlayer;

	const char *pszModel = "models/bots/skeleton_sniper/skeleton_sniper.mdl";
	int nSkin = 0;

	m_nZombieClass = pPlayer->GetDesiredPlayerClassIndex();
	switch ( m_nZombieClass )
	{
	case TF_CLASS_SCOUT:
		pszModel = "models/player/scout.mdl";
		break;

	case TF_CLASS_SNIPER:
		pszModel = "models/player/sniper.mdl";
		break;

	case TF_CLASS_SOLDIER:
		pszModel = "models/player/soldier.mdl";
		break;

	case TF_CLASS_DEMOMAN:
		pszModel = "models/player/demo.mdl";
		break;

	case TF_CLASS_MEDIC:
		pszModel = "models/player/medic.mdl";
		break;

	case TF_CLASS_HEAVYWEAPONS:
		pszModel = "models/player/heavy.mdl";
		break;

	case TF_CLASS_PYRO:
		pszModel = "models/player/pyro.mdl";
		break;

	case TF_CLASS_SPY:
		pszModel = "models/player/spy.mdl";
		nSkin += 18;
		break;

	case TF_CLASS_ENGINEER:
		pszModel = "models/player/engineer.mdl";
		break;

	default:
		pszModel = "models/bots/skeleton_sniper/skeleton_sniper.mdl";
		break;
	}

	SetModel( pszModel );

	if ( pOwner->GetTeamNumber() == TF_TEAM_RED )
		nSkin += 4;
	else
		nSkin += 5;

	m_nSkin = nSkin;

	if ( m_nSkeletonType == SKELETON_ZOMBIE_FAST )
		m_iHealth = m_iMaxHealth = pPlayer->GetMaxHealth() / 1.75;
	else if ( m_nSkeletonType == SKELETON_ZOMBIE_POISON )
		m_iHealth = m_iMaxHealth = pPlayer->GetMaxHealth();
	else
		m_iHealth = m_iMaxHealth = pPlayer->GetMaxHealth() / 1.5;

	//AddHat( STRING( pKiller->GetModelName() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombie::SetSkeletonType( int nType )
{
	switch ( nType )
	{
	case SKELETON_NORMAL:
		SetActivity( ACT_TRANSITION );
		m_flAttRange = 50.0f;
		m_flAttDamage = 30.0f;
		break;

	case SKELETON_SMALL:
		SetModelScale( 0.5f );
		SetActivity( ACT_TRANSITION );
		m_flAttRange = 40.0f;
		m_flAttDamage = 20.0f;
		break;

	case SKELETON_KING:
		SetModel( "models/bots/skeleton_sniper_boss/skeleton_sniper_boss.mdl" );
		SetModelScale( 2.0f );
		SetActivity( ACT_TRANSITION );
		AddHat( "models/player/items/demo/crown.mdl" );
		m_iHealth = m_iMaxHealth = 1000;
		m_flAttRange = 100.0f;
		m_flAttDamage = 100.0f;
		break;

	case SKELETON_ZOMBIE:
	case SKELETON_ZOMBIE_FAST:
	case SKELETON_ZOMBIE_POISON:
		m_flAttDamage = tf_halloween_zombie_damage.GetFloat();
		break;
	}

	m_nSkeletonType = nType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFZombie *CTFZombie::CreateZombie( const Vector &vecOrigin, CBaseEntity *pOwner, CBaseEntity *pKiller, SkeletonType_t eType, float flLifeTime )
{
	CTFZombie *pZombie = (CTFZombie *)CreateEntityByName( "tf_zombie" );
	if ( pZombie )
	{
		pZombie->SetAbsOrigin( vecOrigin );
		pZombie->Spawn();
		DispatchSpawn( pZombie );
		pZombie->SetSkeletonType( eType );
		pZombie->SetZombieOwner( pOwner, pKiller );
		pZombie->Teleport( &vecOrigin, NULL, NULL );
		if ( flLifeTime > 0.0f )
			pZombie->m_timeTillDeath.Start( flLifeTime );
	}

	return pZombie;
}

//-----------------------------------------------------------------------------
// Purpose: crown or headcrab?
//-----------------------------------------------------------------------------
void CTFZombie::AddHat( const char *pModel )
{
	int headBone = LookupBone( "bip_head" );
	if ( headBone == -1 )
		return;

	m_pHat = (CBaseAnimating *)CreateEntityByName( "prop_dynamic" );
	if ( m_pHat )
	{
		m_pHat->SetModel( pModel );

		Vector vecOrigin; QAngle vecAngles;
		GetBonePosition( headBone, vecOrigin, vecAngles );
		m_pHat->SetAbsOrigin(vecOrigin);
		m_pHat->SetAbsAngles(vecAngles);

		m_pHat->FollowEntity( this );
	}
}

const char *CTFZombie::GetLocalizeName( void )
{
	const char *pszName = BaseClass::GetLocalizeName();
	/*if ( m_hZombieOwner != NULL )
	{
		char szName[64];
		V_snprintf( szName, sizeof( szName ), "%s (%s)", BaseClass::GetLocalizeName(), m_hZombieOwner->GetPlayerName() );
		pszName = szName;
	}*/

	return pszName;
}
//-------------------------------------------------------------------------------------------------
//
// Schedules
//
//-------------------------------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( tf_zombie, CTFZombie )
AI_END_CUSTOM_NPC()


//-----------------------------------------------------------------------------
// vintage skeleton spawner
//-----------------------------------------------------------------------------
class CZombieSpawner : public CPointEntity
{
	DECLARE_CLASS( CZombieSpawner, CPointEntity );
public:
	CZombieSpawner();
	virtual ~CZombieSpawner() {}

	DECLARE_DATADESC()

	virtual void Spawn( void );
	virtual void Think( void );

	void InputEnable( inputdata_t &input );
	void InputDisable( inputdata_t &input );
	void InputSetMaxActiveZombies( inputdata_t &input );

private:
	float m_flZombieLifeTime;
	int m_nMaxActiveZombies;
	bool m_bEnabled;
	bool m_bInfiniteZombies;
	CTFZombie::SkeletonType_t m_nSkeletonType;
	int m_iNumSpawned;
	CUtlVector< CHandle<CTFZombie> > m_hZombies;
};

LINK_ENTITY_TO_CLASS( tf_zombie_spawner, CZombieSpawner );

BEGIN_DATADESC( CZombieSpawner )
	DEFINE_KEYFIELD( m_flZombieLifeTime, FIELD_FLOAT, "zombie_lifetime" ),
	DEFINE_KEYFIELD( m_nMaxActiveZombies, FIELD_INTEGER, "max_zombies" ),
	DEFINE_KEYFIELD( m_bInfiniteZombies, FIELD_BOOLEAN, "infinite_zombies" ),
	DEFINE_KEYFIELD( m_nSkeletonType, FIELD_INTEGER, "skeleton_type" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetMaxActiveZombies", InputSetMaxActiveZombies ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CZombieSpawner::CZombieSpawner()
{
	m_flZombieLifeTime = 0;
	m_bEnabled = true;
	m_bInfiniteZombies = false;
	m_iNumSpawned = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CZombieSpawner::Spawn( void )
{
	BaseClass::Spawn();
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CZombieSpawner::Think( void )
{
	CBaseHandle null;
	m_hZombies.FindAndFastRemove( null );

	if ( !m_bEnabled )
	{
		SetNextThink( gpGlobals->curtime + 0.2f );
		return;
	}

	if ( m_bInfiniteZombies )
	{
		if ( m_hZombies.Count() > m_nMaxActiveZombies )
		{
			SetNextThink( gpGlobals->curtime + 0.2f );
			return;
		}
	}
	else if ( m_iNumSpawned >= m_nMaxActiveZombies )
	{
		SetNextThink( gpGlobals->curtime + 0.2f );
		return;
	}

	CTFZombie *pZombie = CTFZombie::CreateZombie( GetAbsOrigin(), NULL, NULL, m_nSkeletonType, m_flZombieLifeTime );
	if ( pZombie )
	{
		++m_iNumSpawned;

		CHandle<CTFZombie> hndl( pZombie );
		m_hZombies.AddToTail( hndl );
	}

	SetNextThink( gpGlobals->curtime + RandomFloat( 1.5, 3.0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CZombieSpawner::InputEnable( inputdata_t &data )
{
	m_bEnabled = true;

	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CZombieSpawner::InputDisable( inputdata_t &data )
{
	m_bEnabled = false;
	m_iNumSpawned = 0;
	m_hZombies.RemoveAll();

	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CZombieSpawner::InputSetMaxActiveZombies( inputdata_t &data )
{
	m_nMaxActiveZombies = data.value.Int();
}