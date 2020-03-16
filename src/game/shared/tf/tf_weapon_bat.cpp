//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_bat.h"
#include "decals.h"
#include "tf_gamerules.h"
#include "effect_dispatch_data.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_ai_basenpc.h"
#include "in_buttons.h"
#include "particles_new.h"
// Server specific.
#else
#include "tf_player.h"
#include "ai_basenpc.h"
#include "tf_fx.h"
#include "tf_gamestats.h"
#endif

#define TF_STUNBALL_MODEL	  "models/weapons/w_models/w_baseball.mdl"
#define TF_STUNBALL_LIFETIME  15.0f

ConVar tf_scout_stunball_base_duration( "tf_scout_stunball_base_duration", "6.0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Modifies stun duration of stunball" );
ConVar sv_proj_stunball_damage( "sv_proj_stunball_damage", "15", FCVAR_NOTIFY | FCVAR_REPLICATED, "Modifies stunball damage." );
ConVar tf_scout_stunball_regen_rate( "tf_scout_stunball_regen_rate", "15.0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Modifies stunball regen rate." );
ConVar tf_scout_stunball_base_speed( "tf_scout_stunball_base_speed", "3000", FCVAR_NOTIFY | FCVAR_REPLICATED, "Modifies stunball base speed." );
ConVar tf_scout_bat_launch_delay( "tf_scout_bat_launch_delay", "0.1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Modifies delay for stunball launch" );

//=============================================================================
//
// Weapon Bat tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFBat, tf_weapon_bat )

//=============================================================================
//
// Weapon Bat functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFBat::CTFBat()
{
}

//=============================================================================
//
// Weapon Bat Wood tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFBat_Wood, DT_TFWeaponBat_Wood )

BEGIN_NETWORK_TABLE( CTFBat_Wood, DT_TFWeaponBat_Wood )
#ifdef CLIENT_DLL
	RecvPropTime( RECVINFO( m_flNextFireTime ) ),
	RecvPropBool( RECVINFO( m_bFiring ) ),
#else
	SendPropTime( SENDINFO( m_flNextFireTime ) ),
	SendPropBool( SENDINFO( m_bFiring ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBat_Wood )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_bat_wood, CTFBat_Wood );
PRECACHE_WEAPON_REGISTER( tf_weapon_bat_wood );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFBat_Wood::CTFBat_Wood()
{
#ifdef CLIENT_DLL
	// Assume true in case the player is switching to this weapon at resupply
	m_bHasBall = true;
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFBat_Wood::Precache( void )
{
	PrecacheModel( TF_STUNBALL_VIEWMODEL );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Check if we should have the stunball out
//-----------------------------------------------------------------------------
bool CTFBat_Wood::Deploy( void )
{
#ifdef CLIENT_DLL
	// This is here in case the ammo state changes while the weapon is holstered
	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( pOwner )
	{
		// Do we currently have ammo?
		m_bHasBall = pOwner->GetAmmoCount( m_iPrimaryAmmoType ) ? true : false;
	}
#endif

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: Check if we can currently create a new ball
//-----------------------------------------------------------------------------
bool CTFBat_Wood::CanCreateBall( CTFPlayer *pPlayer )
{
	// We need ammo to fire
	if ( !pPlayer )
		return false;

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );

	// Only weapon mode 1 can fire balls
	if ( !iType )
		return false;

	trace_t tr;
	Vector vecStart, vecEnd, vecDir;
	AngleVectors( pPlayer->EyeAngles(), &vecDir );

	vecStart = pPlayer->EyePosition();
	vecEnd = vecStart + (vecDir * 48);

	CTraceFilterIgnorePlayers *pFilter = new CTraceFilterIgnorePlayers( this, COLLISION_GROUP_NONE );

	UTIL_TraceLine( vecStart, vecEnd, MASK_ALL, pFilter, &tr );

#ifdef GAME_DLL
	//NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 255, 0, true, 5.0f );
#endif

	// A wall is stopping our fire
	if ( tr.DidHitWorld() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Pick up a ball on the ground
//-----------------------------------------------------------------------------
bool CTFBat_Wood::PickedUpBall( CTFPlayer *pPlayer )
{
	// Only one ball at a time
	if ( !pPlayer || pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) || GetEffectBarProgress() == 1.0f )
		return false;

#ifdef GAME_DLL
	// Pick up the ball
	pPlayer->GiveAmmo( 1, m_iPrimaryAmmoType, true, TF_AMMO_SOURCE_RESUPPLY );
	pPlayer->SpeakConceptIfAllowed( MP_CONCEPT_GRAB_BALL );
	m_flEffectBarRegenTime = gpGlobals->curtime;
#endif

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Swing the bat
//-----------------------------------------------------------------------------
void CTFBat_Wood::PrimaryAttack( void )
{
	if ( m_flNextFireTime > gpGlobals->curtime )
		return;

	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: Alt. Fire
//-----------------------------------------------------------------------------
void CTFBat_Wood::SecondaryAttack( void )
{
	if ( m_flNextFireTime > gpGlobals->curtime || m_bFiring )
		return;

	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer || !pPlayer->CanAttack() || !CanCreateBall( pPlayer ) || !pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) )
		return;

	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_SECONDARY );
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	m_flNextFireTime = gpGlobals->curtime + 0.25f;
	SetWeaponIdleTime( m_flNextFireTime );

	SetContextThink( &CTFBat_Wood::LaunchBallThink, gpGlobals->curtime + tf_scout_bat_launch_delay.GetFloat(), "LAUNCH_BALL_THINK" );
	m_bFiring = true;

#ifdef GAME_DLL
	if ( pPlayer->m_Shared.InCond( TF_COND_STEALTHED ) )
		pPlayer->RemoveInvisibility();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBat_Wood::ItemPostFrame( void )
{
#ifdef CLIENT_DLL
	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( pOwner )
	{
		// Do we currently have ammo?
		m_bHasBall = pOwner->GetAmmoCount( m_iPrimaryAmmoType ) ? true : false;
	}
#endif

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBat_Wood::LaunchBallThink( void )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	if ( pPlayer )
	{
#ifdef GAME_DLL
		LaunchBall( GetTFPlayerOwner() );
		pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

		pPlayer->SpeakWeaponFire( MP_CONCEPT_BAT_BALL );
		CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
#endif
		StartEffectBarRegen();
	}

	m_bFiring = false;
}

//-----------------------------------------------------------------------------
// Purpose: Launch a ball
//-----------------------------------------------------------------------------
CBaseEntity *CTFBat_Wood::LaunchBall( CTFPlayer *pPlayer )
{
	WeaponSound( SPECIAL2 );

#ifdef GAME_DLL
	AngularImpulse spin = AngularImpulse( random->RandomInt( -1200, 1200 ), 0, 0 );

	Vector vecForward, vecRight, vecUp;
	AngleVectors( pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp );

	// The ball always launches at the player's crouched eye position
	// so that it always passes through the bbox
	Vector vecSrc = pPlayer->GetAbsOrigin() + VEC_DUCK_VIEW_SCALED( pPlayer ) * 1.5;
	vecSrc += vecForward * -64.0f;

	//NDebugOverlay::Line( pPlayer->EyePosition(), vecSrc, 0, 255, 0, true, 5.0f );
	
	Vector vecVelocity = ( vecForward * tf_scout_stunball_base_speed.GetFloat() ) + ( vecUp * 200.0f );

	//GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );

	CTFStunBall *pProjectile = CTFStunBall::Create( this, vecSrc, pPlayer->EyeAngles(), vecVelocity, pPlayer, pPlayer, spin, GetTFWpnData() );
	if ( pProjectile )
	{
		CalcIsAttackCritical();
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( sv_proj_stunball_damage.GetFloat() );
	}

	return pProjectile;
#endif
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Update animations depending on if we do or do not have the ball
//-----------------------------------------------------------------------------
bool CTFBat_Wood::SendWeaponAnim( int iActivity )
{
	int iNewActivity = iActivity;
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	// if we have the ball show it on our viewmodel 
	if ( pPlayer && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) )
	{
		switch ( iActivity )
		{
			case ACT_VM_DRAW:
				iNewActivity = ACT_VM_DRAW_SPECIAL;
				break;
			case ACT_VM_HOLSTER:
				iNewActivity = ACT_VM_HOLSTER_SPECIAL;
				break;
			case ACT_VM_IDLE:
				iNewActivity = ACT_VM_IDLE_SPECIAL;
				break;
			case ACT_VM_PULLBACK:
				iNewActivity = ACT_VM_PULLBACK_SPECIAL;
				break;
			case ACT_VM_PRIMARYATTACK:
			case ACT_VM_SECONDARYATTACK:
				iNewActivity = ACT_VM_PRIMARYATTACK_SPECIAL;
				break;
			case ACT_VM_HITCENTER:
				iNewActivity = ACT_VM_HITCENTER_SPECIAL;
				break;
			case ACT_VM_SWINGHARD:
				iNewActivity = ACT_VM_SWINGHARD_SPECIAL;
				break;
			case ACT_VM_IDLE_TO_LOWERED:
				iNewActivity = ACT_VM_IDLE_TO_LOWERED_SPECIAL;
				break;
			case ACT_VM_IDLE_LOWERED:
				iNewActivity = ACT_VM_IDLE_LOWERED_SPECIAL;
				break;
			case ACT_VM_LOWERED_TO_IDLE:
				iNewActivity = ACT_VM_LOWERED_TO_IDLE_SPECIAL;
				break;
		}
	}

	return BaseClass::SendWeaponAnim( iNewActivity );
}
float CTFBat_Wood::InternalGetEffectBarRechargeTime( void )
{
	return tf_scout_stunball_regen_rate.GetFloat();
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFBat_Wood::CreateMove( float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles )
{
	// Really hacky workaround for primary/secondary interactions
	if ( m_flNextFireTime > gpGlobals->curtime )
	{
		pCmd->buttons &= ~IN_ATTACK;
		pCmd->buttons &= ~IN_ATTACK2;
	}

	if ( pCmd->buttons & IN_ATTACK2 )
	{	
		if ( !CanCreateBall( GetTFPlayerOwner() ) )
		{
			pCmd->buttons &= ~IN_ATTACK2;
		}
		else
		{
			pCmd->buttons &= ~IN_ATTACK;
		}
	}

	BaseClass::CreateMove( flInputSampleTime, pCmd, vecOldViewAngles );
}
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFStunBall, DT_TFStunBall )

BEGIN_NETWORK_TABLE( CTFStunBall, DT_TFStunBall )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bCritical ) ),
#else
	SendPropBool( SENDINFO( m_bCritical ) ),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC( CTFStunBall )
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( tf_projectile_stunball, CTFStunBall );
PRECACHE_REGISTER( tf_projectile_stunball );

CTFStunBall::CTFStunBall()
{
}

CTFStunBall::~CTFStunBall()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

#ifdef GAME_DLL
CTFStunBall *CTFStunBall::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pScorer, const AngularImpulse &angVelocity, const CTFWeaponInfo &weaponInfo )
{
	CTFStunBall *pStunBall = static_cast<CTFStunBall *>( CBaseEntity::CreateNoSpawn( "tf_projectile_stunball", vecOrigin, vecAngles, pOwner ) );

	if ( pStunBall )
	{
		// Set scorer.
		pStunBall->SetScorer( pScorer );

		// Set firing weapon.
		pStunBall->SetLauncher( pWeapon );

		DispatchSpawn( pStunBall );

		pStunBall->InitGrenade( vecVelocity, angVelocity, pOwner, pWeapon );

		pStunBall->ApplyLocalAngularVelocityImpulse( angVelocity );
	}

	return pStunBall;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFStunBall::Precache( void )
{
	PrecacheModel( TF_STUNBALL_MODEL );
	PrecacheScriptSound( "TFPlayer.StunImpactRange" );
	PrecacheScriptSound( "TFPlayer.StunImpact" );

	//PrecacheTeamParticles( "stunballtrail_%s", false );
	PrecacheTeamParticles( "stunballtrail_%s_crit", false );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFStunBall::Spawn( void )
{
	SetModel( TF_STUNBALL_MODEL );
	SetDetonateTimerLength( TF_STUNBALL_LIFETIME );

	BaseClass::Spawn();
	SetTouch( &CTFStunBall::StunBallTouch );

	CreateTrail();

	// Pumpkin Bombs
	AddFlag( FL_GRENADE );

	// Don't collide with anything for a short time so that we never get stuck behind surfaces
	SetCollisionGroup( COLLISION_GROUP_DEBRIS_TRIGGER );

	AddSolidFlags( FSOLID_TRIGGER );

	m_flCreationTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::Explode( trace_t *pTrace, int bitsDamageType )
{
	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );
	}

	CTFWeaponBase *pWeapon = dynamic_cast< CTFWeaponBase * >( m_hLauncher.Get() );

	// Pull out a bit.
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );
	}

	// Damage.
	CTFPlayer *pAttacker = ToTFPlayer( GetThrower() );
	CTFPlayer *pPlayer = ToTFPlayer( m_hEnemy.Get() );
	CAI_BaseNPC *pNPC = dynamic_cast< CAI_BaseNPC * >( m_hEnemy.Get() );

	if ( pAttacker )
	{
		float flAirTime = gpGlobals->curtime - m_flCreationTime;
		float flStunDuration = tf_scout_stunball_base_duration.GetFloat();
		Vector vecDir = GetAbsOrigin();
		VectorNormalize( vecDir );

		CTakeDamageInfo info( this, pAttacker, pWeapon, GetDamage(), GetDamageType(), TF_DMG_CUSTOM_BASEBALL );
		CalculateBulletDamageForce( &info, pWeapon ? pWeapon->GetTFWpnData().iAmmoType : 0, vecDir, GetAbsOrigin() );
		info.SetReportedPosition( pAttacker ? pAttacker->GetAbsOrigin() : vec3_origin );
	
		// Make sure it's is stunnable
		if ( pPlayer && CanStun( pPlayer ) )
		{
			// Do damage.
			pPlayer->DispatchTraceAttack( info, vecDir, pTrace );
			ApplyMultiDamage();

			if ( flAirTime > 0.1f )
			{
				int iBonus = 1;
				if ( flAirTime >= 1.0f )
				{
					// Maximum stun is base duration + 1 second
					flAirTime = 1.0f;
					flStunDuration += 1.0f;

					// 2 points for moonshots
					iBonus++;

					// Big stun
					pPlayer->m_Shared.StunPlayer(flStunDuration * ( flAirTime ), 0.0f, 0.75f, TF_STUNFLAGS_BIGBONK, pAttacker );
				}
				else
				{
					// Small stun
					pPlayer->m_Shared.StunPlayer(flStunDuration * ( flAirTime ), 0.8f, 0.0f, TF_STUNFLAGS_SMALLBONK, pAttacker );
				}

				pAttacker->SpeakConceptIfAllowed( MP_CONCEPT_STUNNED_TARGET );

				// Bonus points.
				IGameEvent *event_bonus = gameeventmanager->CreateEvent( "player_bonuspoints" );
				if ( event_bonus )
				{
					event_bonus->SetInt( "player_entindex", pPlayer->entindex() );
					event_bonus->SetInt( "source_entindex", pAttacker->entindex() );
					event_bonus->SetInt( "points", iBonus );

					gameeventmanager->FireEvent( event_bonus );
				}
				CTF_GameStats.Event_PlayerAwardBonusPoints( pAttacker, pPlayer, iBonus );
			}
		}

		if ( pNPC && CanStun( pNPC ) )
		{
			// Do damage.
			pNPC->DispatchTraceAttack( info, vecDir, pTrace );
			ApplyMultiDamage();

			if ( flAirTime > 0.1f )
			{
				int iBonus = 1;
				if ( flAirTime >= 1.0f )
				{
					// Maximum stun is base duration + 1 second
					flAirTime = 1.0f;
					flStunDuration += 1.0f;

					// 2 points for moonshots
					iBonus++;

					// Big stun
					pNPC->StunNPC(flStunDuration * ( flAirTime ), 0.0f, 0.75f, TF_STUNFLAGS_BIGBONK, pAttacker );
				}
				else
				{
					// Small stun
					pNPC->StunNPC(flStunDuration * ( flAirTime ), 0.8f, 0.0f, TF_STUNFLAGS_SMALLBONK, pAttacker );
				}

				pAttacker->SpeakConceptIfAllowed( MP_CONCEPT_STUNNED_TARGET );

				// Bonus points.
				IGameEvent *event_bonus = gameeventmanager->CreateEvent( "player_bonuspoints" );
				if ( event_bonus )
				{
					event_bonus->SetInt( "player_entindex", pNPC->entindex() );
					event_bonus->SetInt( "source_entindex", pAttacker->entindex() );
					event_bonus->SetInt( "points", iBonus );

					gameeventmanager->FireEvent( event_bonus );
				}
				CTF_GameStats.Event_PlayerAwardBonusPoints( pAttacker, pNPC, iBonus );
			}
		}
	}
	else
	{
		float flAirTime = gpGlobals->curtime - m_flCreationTime;
		float flStunDuration = tf_scout_stunball_base_duration.GetFloat();
		Vector vecDir = GetAbsOrigin();
		VectorNormalize( vecDir );

		CTakeDamageInfo info( this, this, pWeapon, GetDamage(), GetDamageType(), TF_DMG_CUSTOM_BASEBALL );
		CalculateBulletDamageForce( &info, pWeapon ? pWeapon->GetTFWpnData().iAmmoType : 0, vecDir, GetAbsOrigin() );
		info.SetReportedPosition( vec3_origin );
	
		// Make sure it's is stunnable
		if ( pPlayer && CanStun( pPlayer ) )
		{
			// Do damage.
			pPlayer->DispatchTraceAttack( info, vecDir, pTrace );
			ApplyMultiDamage();

			if ( flAirTime > 0.1f )
			{
				int iBonus = 1;
				if ( flAirTime >= 1.0f )
				{
					// Maximum stun is base duration + 1 second
					flAirTime = 1.0f;
					flStunDuration += 1.0f;

					// 2 points for moonshots
					iBonus++;

					// Big stun
					pPlayer->m_Shared.StunPlayer(flStunDuration * ( flAirTime ), 0.0f, 0.75f, TF_STUNFLAGS_BIGBONK, NULL );
				}
				else
				{
					// Small stun
					pPlayer->m_Shared.StunPlayer(flStunDuration * ( flAirTime ), 0.8f, 0.0f, TF_STUNFLAGS_SMALLBONK, NULL );
				}
			}
		}

		if ( pNPC && CanStun( pNPC ) )
		{
			// Do damage.
			pNPC->DispatchTraceAttack( info, vecDir, pTrace );
			ApplyMultiDamage();

			if ( flAirTime > 0.1f )
			{
				int iBonus = 1;
				if ( flAirTime >= 1.0f )
				{
					// Maximum stun is base duration + 1 second
					flAirTime = 1.0f;
					flStunDuration += 1.0f;

					// 2 points for moonshots
					iBonus++;

					// Big stun
					pNPC->StunNPC(flStunDuration * ( flAirTime ), 0.0f, 0.75f, TF_STUNFLAGS_BIGBONK, NULL );
				}
				else
				{
					// Small stun
					pNPC->StunNPC(flStunDuration * ( flAirTime ), 0.8f, 0.0f, TF_STUNFLAGS_SMALLBONK, NULL );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::StunBallTouch( CBaseEntity *pOther )
{
	// Verify a correct "other."
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) )
		return;

	trace_t pTrace;
	Vector velDir = GetAbsVelocity();
	VectorNormalize( velDir );
	Vector vecSpot = GetAbsOrigin() - velDir * 32;
	UTIL_TraceLine( vecSpot, vecSpot + velDir * 64, MASK_SOLID, this, COLLISION_GROUP_NONE, &pTrace );

	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	CAI_BaseNPC *pNPC = dynamic_cast< CAI_BaseNPC * >( pOther );

	// Make us solid once we reach our owner
	if ( GetCollisionGroup() == COLLISION_GROUP_DEBRIS_TRIGGER )
	{
		if ( pOther == GetThrower() )
			SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

		return;
	}

	// Stun the person we hit
	if ( ( gpGlobals->curtime - m_flCreationTime > 0.2f || !InSameTeam( pOther ) ) )
	{
		if ( pPlayer )
		{
			if ( !m_bTouched )
			{
				// Save who we hit for calculations
				m_hEnemy = pOther;

				if ( m_hSpriteTrail.Get() )
					m_hSpriteTrail->SUB_FadeOut();

				Explode( &pTrace, GetDamageType() );

				// Die a little bit after the hit
				SetDetonateTimerLength( 3.0f );
				m_bTouched = true;
			}

			CTFWeaponBase *pWeapon = pPlayer->Weapon_GetWeaponByType( TF_WPN_TYPE_MELEE );
			if ( pWeapon && pWeapon->PickedUpBall( pPlayer ) )
				UTIL_Remove( this );
		}
		else if ( pNPC )
		{
			if ( !m_bTouched )
			{
				pNPC->NPC_TranslateActivity(ACT_MP_STUN_BEGIN);
				// Save who we hit for calculations
				m_hEnemy = pOther;

				if ( m_hSpriteTrail.Get() )
					m_hSpriteTrail->SUB_FadeOut();

				Explode( &pTrace, GetDamageType() );

				// Die a little bit after the hit
				SetDetonateTimerLength( 3.0f );
				m_bTouched = true;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Check if this player can be stunned
//-----------------------------------------------------------------------------
bool CTFStunBall::CanStun( CBaseEntity *pOther )
{
	// Dead players can't be stunned
	if ( !pOther->IsAlive() )
		return false;

	// Don't stun team members
	if ( InSameTeam( pOther ) )
		return false;

	// Don't stun if we can't damage
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	if ( pPlayer )
	{
		if ( pPlayer->m_Shared.IsInvulnerable() || pPlayer->m_Shared.InCond( TF_COND_PHASE ) || pPlayer->m_Shared.InCond( TF_COND_MEGAHEAL ) )
			return false;
	}

	CAI_BaseNPC *pNPC = dynamic_cast< CAI_BaseNPC * >( pOther );
	if ( pNPC )
	{
		if ( !pNPC->AllowStun() )
			return false;

		if ( pNPC->IsInvulnerable() || pNPC->InCond( TF_COND_PHASE ) || pNPC->InCond( TF_COND_MEGAHEAL ) )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::Detonate()
{
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	int otherIndex = !index;
	CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];
	if ( !pHitEntity )
		return;

	// we've touched a surface
	m_bTouched = true;
	
	// Handle hitting skybox (disappear).
	surfacedata_t *pprops = physprops->GetSurfaceData( pEvent->surfaceProps[otherIndex] );
	if ( pprops->game.material == 'X' )
	{
		// uncomment to destroy ball upon hitting sky brush
		//SetThink( &CTFGrenadePipebombProjectile::SUB_Remove );
		//SetNextThink( gpGlobals->curtime );
		return;
	}

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFStunBall::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CTFStunBall::GetAssistant( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		Vector vecOldVelocity, vecVelocity;

		pPhysicsObject->GetVelocity( &vecOldVelocity, NULL );

		float flVel = vecOldVelocity.Length();

		vecVelocity = vecDir;
		vecVelocity *= flVel;
		AngularImpulse angVelocity( ( 600, random->RandomInt( -1200, 1200 ), 0 ) );

		// Now change grenade's direction.
		pPhysicsObject->SetVelocityInstantaneous( &vecVelocity, &angVelocity );
	}

	CBaseCombatCharacter *pBCC = pDeflectedBy->MyCombatCharacterPointer();

	IncremenentDeflected();
	m_hDeflectOwner = pDeflectedBy;
	SetThrower( pBCC );
	ChangeTeam( pDeflectedBy->GetTeamNumber() );

	// Change trail color.
	if ( m_hSpriteTrail.Get() )
	{
		UTIL_Remove( m_hSpriteTrail.Get() );
	}

	CreateTrail();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFStunBall::GetDamageType()
{
	int iDmgType = BaseClass::GetDamageType();

	if ( m_bCritical )
	{
		iDmgType |= DMG_CRITICAL;
	}
	if ( m_iDeflected > 0 )
	{
		iDmgType |= DMG_MINICRITICAL;
	}

	return iDmgType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFStunBall::GetTrailParticleName( void )
{
	return ConstructTeamParticle( "effects/baseballtrail_%s.vmt", GetTeamNumber(), false, g_aTeamNamesShort );
}

// ----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::CreateTrail( void )
{
	CSpriteTrail *pTrail = CSpriteTrail::SpriteTrailCreate( GetTrailParticleName(), GetAbsOrigin(), true );

	if ( pTrail )
	{
		pTrail->FollowEntity( this );
		pTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 128, kRenderFxNone );
		pTrail->SetStartWidth( 9.0f );
		pTrail->SetTextureResolution( 0.01f );
		pTrail->SetLifeTime( 0.4f );
		pTrail->TurnOn();

		pTrail->SetContextThink( &CBaseEntity::SUB_Remove, gpGlobals->curtime + 5.0f, "RemoveThink" );

		m_hSpriteTrail.Set( pTrail );
	}
}

//-----------------------------------------------------------------------------
// Purpose: if scunt misses we can use the ball
//-----------------------------------------------------------------------------
void CTFStunBall::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	if ( reason == PUNTED_BY_CANNON )
	{
		SetDetonateTimerLength( TF_STUNBALL_LIFETIME );
		m_bTouched = false;

		m_hDeflectOwner = pPhysGunUser;
		SetThrower( pPhysGunUser );
		ChangeTeam( pPhysGunUser->GetTeamNumber() );

		// Change trail color.
		if ( m_hSpriteTrail.Get() )
			UTIL_Remove( m_hSpriteTrail.Get() );

		CreateTrail();
	}
}

#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_flCreationTime = gpGlobals->curtime;
		CreateTrails();

		if ( IsEffectActive( EF_DIMLIGHT ) )
			RemoveEffects( EF_DIMLIGHT );
	}

	if ( m_iOldTeamNum && m_iOldTeamNum != m_iTeamNum )
	{
		ParticleProp()->StopEmission();
		CreateTrails();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFStunBall::CreateTrails( void )
{
	if ( IsDormant() )
		return;

	if ( m_bCritical )
	{
		const char *pszEffectName = ConstructTeamParticle( "stunballtrail_%s_crit", GetTeamNumber(), false );
		ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
	/*else
	{
		const char *pszEffectName = ConstructTeamParticle( "stunballtrail_%s", GetTeamNumber(), false );
		ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Don't draw if we haven't yet gone past our original spawn point
//-----------------------------------------------------------------------------
int CTFStunBall::DrawModel( int flags )
{
	if ( gpGlobals->curtime - m_flCreationTime < 0.1f )
		return 0;

	return BaseClass::DrawModel( flags );
}
#endif

//=============================================================================
//
// Weapon Bat Giftwarp tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFBat_Giftwrap, tf_weapon_bat_giftwarp )

//=============================================================================
//
// Weapon Bat Fish tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFBat_Fish, tf_weapon_bat_fish )