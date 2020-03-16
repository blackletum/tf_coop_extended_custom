//====== Copyright � 1996-2018, Valve Corporation, All rights reserved. =======
//
// Purpose: A remake of Throwable Jar(s) from live TF2
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_jar.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"
#include "datacache/imdlcache.h"
#include "effect_dispatch_data.h"
#include "engine/IEngineSound.h"
#include "tf_gamerules.h"
#include "eventlist.h"
#include "tf_viewmodel.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "particles_new.h"
// Server specific.
#else
#include "tf_player.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "tf_fx.h"
#include "tf_gamestats.h"
#endif

#define URINE_MODEL "models/weapons/c_models/urinejar.mdl"
#define MILK_MODEL "models/weapons/c_models/c_madmilk/c_madmilk.mdl"
#define GAS_MODEL "models/weapons/c_models/c_gascan/c_gascan.mdl"

#define TF_JAR_DAMAGE 0.0f
#define TF_JAR_VEL 1000.0f
#define TF_JAR_GRAV 1.0f

extern ConVar tf_grenade_show_radius;
//=============================================================================
//
// TF Projectile Jar tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Jar, DT_TFProjectile_Jar )

BEGIN_NETWORK_TABLE( CTFProjectile_Jar, DT_TFProjectile_Jar )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bCritical ) ),
#else
	SendPropBool( SENDINFO( m_bCritical ) ),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC( CTFProjectile_Jar )
END_DATADESC()
#endif

ConVar tf_jar_show_radius( "tf_jar_show_radius", "0", FCVAR_REPLICATED | FCVAR_CHEAT /*| FCVAR_DEVELOPMENTONLY*/, "Render jar radius." );

LINK_ENTITY_TO_CLASS( tf_projectile_jar, CTFProjectile_Jar );
PRECACHE_REGISTER( tf_projectile_jar );

CTFProjectile_Jar::CTFProjectile_Jar()
{
}

CTFProjectile_Jar::~CTFProjectile_Jar()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

#ifdef GAME_DLL
CTFProjectile_Jar *CTFProjectile_Jar::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pScorer, const AngularImpulse &angVelocity, const CTFWeaponInfo &weaponInfo )
{
	CTFProjectile_Jar *pJar = static_cast<CTFProjectile_Jar *>( CBaseEntity::CreateNoSpawn( "tf_projectile_jar", vecOrigin, vecAngles, pOwner ) );

	if ( pJar )
	{
		// Set scorer.
		pJar->SetScorer( pScorer );

		// Set firing weapon.
		pJar->SetLauncher( pWeapon );

		DispatchSpawn( pJar );

		pJar->InitGrenade( vecVelocity, angVelocity, pOwner, pWeapon );

		pJar->ApplyLocalAngularVelocityImpulse( angVelocity );
	}

	return pJar;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::Precache( void )
{
	PrecacheModel( URINE_MODEL );

	PrecacheTeamParticles( "peejar_trail_%s", false, g_aTeamNamesShort );
	PrecacheParticleSystem( "peejar_impact" );

	PrecacheScriptSound( "Jar.Explode" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::Spawn( void )
{
	switch ( GetEffectCondition() )
	{
	case TF_COND_URINE:
		SetModel( URINE_MODEL );
		break;
	case TF_COND_MAD_MILK:
		SetModel( MILK_MODEL );
		break;
	case TF_COND_GAS:
		SetModel( GAS_MODEL );
		break;
	default:
		break;
	}

	SetDetonateTimerLength( 10.0f );

	BaseClass::Spawn();
	SetTouch( &CTFProjectile_Jar::JarTouch );

	// Pumpkin Bombs
	AddFlag( FL_GRENADE );

	// Don't collide with anything for a short time so that we never get stuck behind surfaces
	SetCollisionGroup( TF_COLLISIONGROUP_GRENADES );

	AddSolidFlags( FSOLID_TRIGGER );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::Explode( trace_t *pTrace, int bitsDamageType )
{
	// Invisible.
	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );
	}

	CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( m_hLauncher.Get() );

	// Pull out a bit.
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );
	}

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	EmitSound( "Jar.Explode" );

	const char *pszEffectName = "peejar_impact";
	switch ( GetEffectCondition() )
	{
	case TF_COND_URINE:
		pszEffectName = "peejar_impact";
		break;
	case TF_COND_MAD_MILK:
		pszEffectName = "peejar_impact_milk";
		break;
	default:
		break;
	}

	DispatchParticleEffect( pszEffectName, vecOrigin, vec3_angle );

	// Damage.
	CBaseEntity *pAttacker = NULL;
	if ( pWeapon )
		pAttacker = pWeapon->GetOwnerEntity();
	else
		pAttacker = GetContainingEntity( INDEXENT(0) );

	float flRadius = GetDamageRadius();

	CTakeDamageInfo newInfo( this, pAttacker, m_hLauncher, vec3_origin, vecOrigin, GetDamage(), GetDamageType() );
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info = &newInfo;
	radiusInfo.m_vecSrc = vecOrigin;
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_flSelfDamageRadius = 121.0f; // Original rocket radius?

	// If we extinguish a friendly player reduce our recharge time by 20%
	if ( TFGameRules()->RadiusJarEffect( radiusInfo, GetEffectCondition() ) && m_iDeflected == 0 && pWeapon ) 
	{
		float flCooldownReduction = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flCooldownReduction, "extinguish_reduces_cooldown" );
		pWeapon->SetEffectBarProgress( pWeapon->GetEffectBarProgress() * flCooldownReduction );
	}

	// Debug!
	if ( tf_jar_show_radius.GetBool() )
	{
		DrawRadius( flRadius );
	}

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );
	
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::JarTouch( CBaseEntity *pOther )
{
	if ( pOther == GetThrower() )
	{
		// Make us solid if we're not already
		if ( GetCollisionGroup() == TF_COLLISIONGROUP_GRENADES )
		{
			SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
		}
		return;
	}

	// Verify a correct "other."
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) )
		return;

	// Handle hitting skybox (disappear).
	trace_t pTrace;
	Vector velDir = GetAbsVelocity();
	VectorNormalize( velDir );
	Vector vecSpot = GetAbsOrigin() - velDir * 32;
	Ray_t ray; ray.Init( vecSpot, vecSpot + velDir * 64 );
	UTIL_Portal_TraceRay( ray, MASK_SOLID, this, COLLISION_GROUP_NONE, &pTrace );

	if ( pTrace.fraction < 1.0 && pTrace.surface.flags & SURF_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	// Blow up if we hit this
	if ( pOther->IsCombatCharacter() )
	{
		Explode( &pTrace, GetDamageType() );
	}
	// We should bounce off of certain surfaces (resupply cabinets, spawn doors, etc.)
	else
	{
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::Detonate()
{
	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	SetThink( NULL );

	vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
	Ray_t ray; ray.Init( vecSpot, vecSpot + Vector ( 0, 0, -32 ) );
	UTIL_Portal_TraceRay( ray, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

	Explode( &tr, GetDamageType() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	int otherIndex = !index;
	CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

	if ( !pHitEntity )
	{
		return;
	}

	// Blow up
	SetThink( &CTFProjectile_Jar::Detonate );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CTFProjectile_Jar::GetAssistant( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
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
}

//-----------------------------------------------------------------------------
// Purpose: only change only when launch in case of timer expire in while hold
//-----------------------------------------------------------------------------
void CTFProjectile_Jar::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason )
{
	if ( Reason == PUNTED_BY_CANNON )
	{
		m_hDeflectOwner = pPhysGunUser;
		SetThrower( pPhysGunUser );
		ChangeTeam( pPhysGunUser->GetTeamNumber() );
	}
}

#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Jar::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_flCreationTime = gpGlobals->curtime;
		CreateTrails();
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
void C_TFProjectile_Jar::CreateTrails( void )
{
	if ( IsDormant() )
		return;

	const char *pszEffectName = ConstructTeamParticle( "peejar_trail_%s", GetTeamNumber(), false, g_aTeamNamesShort );

	ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
}

//-----------------------------------------------------------------------------
// Purpose: Don't draw if we haven't yet gone past our original spawn point
//-----------------------------------------------------------------------------
int CTFProjectile_Jar::DrawModel( int flags )
{
	if ( gpGlobals->curtime - m_flCreationTime < 0.1f )
		return 0;

	return BaseClass::DrawModel( flags );
}
#endif

//=============================================================================
//
// Weapon JarMilk
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_JarMilk, DT_TFProjectile_JarMilk )

BEGIN_NETWORK_TABLE( CTFProjectile_JarMilk, DT_TFProjectile_JarMilk )
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC( CTFProjectile_JarMilk )
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( tf_projectile_jar_milk, CTFProjectile_JarMilk );
PRECACHE_REGISTER( tf_projectile_jar_milk );

#ifdef GAME_DLL
CTFProjectile_JarMilk *CTFProjectile_JarMilk::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pScorer, const AngularImpulse &angVelocity, const CTFWeaponInfo &weaponInfo )
{
	CTFProjectile_JarMilk *pJar = static_cast<CTFProjectile_JarMilk *>( CBaseEntity::CreateNoSpawn( "tf_projectile_jar_milk", vecOrigin, vecAngles, pOwner ) );

	if ( pJar )
	{
		// Set scorer.
		pJar->SetScorer( pScorer );

		// Set firing weapon.
		pJar->SetLauncher( pWeapon );

		DispatchSpawn( pJar );

		pJar->InitGrenade( vecVelocity, angVelocity, pOwner, pWeapon );

		pJar->ApplyLocalAngularVelocityImpulse( angVelocity );
	}

	return pJar;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_JarMilk::Precache( void )
{
	PrecacheModel( MILK_MODEL );
	PrecacheParticleSystem( "peejar_impact_milk" );

	BaseClass::Precache();
}
#endif

//=============================================================================
//
// Weapon PISS tables.
//


IMPLEMENT_NETWORKCLASS_ALIASED( TFJar, DT_WeaponJar )

BEGIN_NETWORK_TABLE( CTFJar, DT_WeaponJar )
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFJar )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_jar, CTFJar );
PRECACHE_WEAPON_REGISTER( tf_weapon_jar );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFJar )
END_DATADESC()
#endif

CTFJar::CTFJar()
{
	m_flEffectBarRegenTime = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFJar::Precache( void )
{
	BaseClass::Precache();
	PrecacheParticleSystem( "peejar_drips" );
}

// ---------------------------------------------------------------------------- -
// Purpose: 
//-----------------------------------------------------------------------------
void CTFJar::PrimaryAttack( void )
{
	// Get the player owning the weapon.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CanAttack() )
		return;

	if ( !HasAmmo() )
		return;

	CalcIsAttackCritical();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	FireProjectile( pPlayer );

#if !defined( CLIENT_DLL ) 
	pPlayer->SpeakWeaponFire( MP_CONCEPT_JARATE_LAUNCH );
	CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
#endif

	// Set next attack times.
	float flDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
	CALL_ATTRIB_HOOK_FLOAT( flDelay, mult_postfiredelay );
	m_flNextPrimaryAttack = gpGlobals->curtime + flDelay;

	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_flEffectBarRegenTime = gpGlobals->curtime + InternalGetEffectBarRechargeTime();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFJar::GetProjectileDamage( void )
{
	return TF_JAR_DAMAGE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFJar::GetProjectileSpeed( void )
{
	return TF_JAR_VEL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFJar::GetProjectileGravity( void )
{
	return TF_JAR_GRAV;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFJar::CalcIsAttackCriticalHelper( void )
{
	// No random critical hits.
	return false;
}

//=============================================================================
//
// Weapon JarMilk
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFJarMilk, DT_WeaponJarMilk )

BEGIN_NETWORK_TABLE( CTFJar, DT_WeaponJarMilk )
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFJarMilk )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_jar_milk, CTFJarMilk );
PRECACHE_WEAPON_REGISTER( tf_weapon_jar_milk );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFJarMilk )
END_DATADESC()
#endif

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Kill splash particles 
//-----------------------------------------------------------------------------
bool C_TFJarMilk::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	C_TFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer && pPlayer->IsLocalPlayer() )
	{
		C_BaseViewModel *vm = pPlayer->GetViewModel();
		if ( vm )
		{
			pPlayer->StopViewModelParticles( vm );
		}
	}

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: Override energydrink particles
//-----------------------------------------------------------------------------
const char* C_TFJarMilk::ModifyEventParticles( const char* token )
{
	if ( !V_stricmp( token, "energydrink_splash" ) )
	{
		CEconItemDefinition *pStatic = GetItem()->GetStaticData();
		if ( pStatic )
		{
			PerTeamVisuals_t *pVisuals = pStatic->GetVisuals( GetTeamNumber() );
			if ( pVisuals )
			{
				const char *pszCustomEffectName = pVisuals->custom_particlesystem;
				if ( pszCustomEffectName[0] != '\0' )
				{
					return pszCustomEffectName;
				}
			}
		}
	}

	return token;
}
#endif