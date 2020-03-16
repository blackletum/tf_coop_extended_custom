//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. ========//
//
// Purpose: Flare used by the flaregun.
//
//=============================================================================//
#include "cbase.h"
#include "tf_projectile_energy_ball.h"
#include "tf_gamerules.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "particles_new.h"
#include "iefx.h"
#include "dlight.h"
#include "tempent.h"
#include "c_te_legacytempents.h"
#else
#include "tf_player.h"
#include "tf_fx.h"
#include "ai_basenpc.h"
#include "collisionutils.h"
#include "tf_team.h"
#include "props.h"
#endif

#ifdef CLIENT_DLL
	extern ConVar lfe_muzzlelight;
#endif

extern ConVar	tf_rocket_show_radius;

#define TF_WEAPON_ENERGYBALL_MODEL	"models/empty.mdl"

//=============================================================================
//
// Dragon's Fury Projectile
//

BEGIN_DATADESC( CTFProjectile_EnergyBall )
END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_projectile_energy_ball, CTFProjectile_EnergyBall );
PRECACHE_REGISTER( tf_projectile_energy_ball );

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_EnergyBall, DT_TFProjectile_EnergyBall )
BEGIN_NETWORK_TABLE( CTFProjectile_EnergyBall, DT_TFProjectile_EnergyBall )
#ifdef GAME_DLL
	SendPropBool( SENDINFO( m_bCritical ) ),
#else
	RecvPropBool( RECVINFO( m_bCritical ) ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFProjectile_EnergyBall::CTFProjectile_EnergyBall()
{

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFProjectile_EnergyBall::~CTFProjectile_EnergyBall()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#else
	m_bCollideWithTeammates = false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::SetChargedShot( bool bState )
{
	m_bChargedShot = bState;

#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
	CreateTrails();
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::Precache()
{
	PrecacheModel( TF_WEAPON_ENERGYBALL_MODEL );

	PrecacheParticleSystem( "drg_cow_rockettrail_normal" );
	PrecacheParticleSystem( "drg_cow_rockettrail_normal_blue" );
	PrecacheParticleSystem( "drg_cow_rockettrail_charged" );
	PrecacheParticleSystem( "drg_cow_rockettrail_charged_blue" );

	PrecacheParticleSystem( "drg_cow_explosioncore_normal" );
	PrecacheParticleSystem( "drg_cow_explosioncore_normal_blue" );
	PrecacheParticleSystem( "drg_cow_explosioncore_charged" );
	PrecacheParticleSystem( "drg_cow_explosioncore_charged_blue" );

	PrecacheScriptSound( "Weapon_CowMangler.Explode" );
	PrecacheScriptSound( "Weapon_CowMangler.ExplodeCharged" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::Spawn()
{
	SetModel( TF_WEAPON_ENERGYBALL_MODEL );
	BaseClass::Spawn();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CTFProjectile_EnergyBall::GetScorer( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFProjectile_EnergyBall::GetDamageType() 
{ 
	int iDmgType = BaseClass::GetDamageType();

	// Buff banner mini-crit calculations
	CTFWeaponBase *pWeapon = ( CTFWeaponBase * )m_hLauncher.Get();
	if ( pWeapon )
	{
		pWeapon->CalcIsAttackMiniCritical();
		if ( pWeapon->IsCurrentAttackAMiniCrit() )
		{
			iDmgType |= DMG_MINICRITICAL;
		}
	}

	if ( m_bCritical )
	{
		iDmgType |= DMG_MINICRITICAL;
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
void CTFProjectile_EnergyBall::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
{
	// Get rocket's speed.
	float flVel = GetAbsVelocity().Length();

	QAngle angForward;
	VectorAngles( vecDir, angForward );

	// Now change rocket's direction.
	SetAbsAngles( angForward );
	SetAbsVelocity( vecDir * flVel );

	// And change owner.
	IncremenentDeflected();
	SetOwnerEntity( pDeflectedBy );
	ChangeTeam( pDeflectedBy->GetTeamNumber() );
	SetScorer( pDeflectedBy );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFProjectile_EnergyBall::GetDamage( void )
{
	return BaseClass::GetDamage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFProjectile_EnergyBall::GetDamageCustom( void )
{
	if ( m_bChargedShot )
		return TF_DMG_CUSTOM_PLASMA_CHARGED;

 	return TF_DMG_CUSTOM_PLASMA;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::Explode( trace_t *pTrace, CBaseEntity *pOther )
{
	// Save this entity as enemy, they will take 100% damage.
	m_hEnemy = pOther;

	// Invisible.
	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Figure out Econ ID.
	int iItemID = -1;
	if ( m_hLauncher.Get() )
	{
		CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( m_hLauncher.Get() );
		if ( pWeapon )
			iItemID = pWeapon->GetItemID();
	}

	// Pull out a bit.
	if ( pTrace->fraction != 1.0 )
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );

	CBaseEntity *pAttacker = GetOwnerEntity();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>( pAttacker );
	if ( pScorerInterface )
		pAttacker = pScorerInterface->GetScorer();

	int iDamageType = GetDamageType();

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	CPVSFilter filter( vecOrigin );

	const char *pszEffectName = m_bChargedShot ? "drg_cow_explosioncore_charged" : "drg_cow_explosioncore_normal";
	if ( GetTeamNumber() != TF_TEAM_RED )
		pszEffectName = m_bChargedShot ? "drg_cow_explosioncore_charged_blue" : "drg_cow_explosioncore_normal_blue";

	QAngle vecAngles;
	VectorAngles( pTrace->plane.normal, vecAngles );

	TE_TFParticleEffect( filter, 0.0, pszEffectName, vecOrigin, vecAngles, NULL, PATTACH_CUSTOMORIGIN );

	EmitSound( m_bChargedShot ? "Weapon_CowMangler.ExplodeCharged" : "Weapon_CowMangler.Explode" );
	CSoundEnt::InsertSound( SOUND_COMBAT | SOUND_CONTEXT_EXPLOSION, vecOrigin, BASEGRENADE_EXPLOSION_VOLUME, 0.25, pAttacker );

	int iMiniCritOnAirborne = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_hLauncher.Get(), iMiniCritOnAirborne, mini_crit_airborne );
	if ( iMiniCritOnAirborne != 0 && ( ( pOther->IsPlayer() || pOther->IsNPC() ) && pOther->GetGroundEntity() == NULL ) )
		iDamageType |= DMG_MINICRITICAL;

	if ( m_bChargedShot )
		iDamageType |= DMG_MINICRITICAL | DMG_IGNITE;

	// Damage.
	float flRadius = GetRadius();

	CTakeDamageInfo newInfo( this, pAttacker, m_hLauncher, vec3_origin, vecOrigin, GetDamage(), iDamageType, GetDamageCustom() );
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info = &newInfo;
	radiusInfo.m_vecSrc = vecOrigin;
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_flSelfDamageRadius = GetSelfDamageRadius();
	radiusInfo.m_bStockSelfDamage = UseStockSelfDamage();

	TFGameRules()->RadiusDamage( radiusInfo );

	if ( m_bChargedShot )
		UTIL_ScreenShake( vecOrigin, 25.0f, 150.0f, 1.0f, 750.0f, SHAKE_START );

	// Debug!
	if ( tf_rocket_show_radius.GetBool() )
		DrawRadius( flRadius );

	// Don't decal players with scorch.
	if ( !pOther->IsPlayer() )
		UTIL_DecalTrace( pTrace, "Scorch" );

	int iUseLargeExplosion = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_hLauncher.Get(), iUseLargeExplosion, use_large_smoke_explosion );
	if ( iUseLargeExplosion )
	{
		DispatchParticleEffect( "explosionTrail_seeds_mvm", vecOrigin ,vec3_angle );
		DispatchParticleEffect( "fluidSmokeExpl_ring_mvm", vecOrigin, vec3_angle );
	}

	if ( TFGameRules()->IsHolidayActive( kHoliday_NewYears ) )
		DispatchParticleEffect( "mvm_pow_gold_seq_firework_mid", vecOrigin ,vec3_angle );

	// Remove the rocket.
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFProjectile_EnergyBall *CTFProjectile_EnergyBall::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pScorer )
{
	CTFProjectile_EnergyBall *pEnergyBall = static_cast<CTFProjectile_EnergyBall*>( CBaseEntity::CreateNoSpawn( "tf_projectile_energy_ball", vecOrigin, vecAngles, pOwner ) );
	if ( pEnergyBall )
	{
		pEnergyBall->InitEnergyBall( vecOrigin, vecAngles, pOwner, pScorer );
		pEnergyBall->SetScorer( pScorer );
		DispatchSpawn( pEnergyBall );
	}

	return pEnergyBall;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::InitEnergyBall( const Vector &velocity, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pWeapon )
{
	// Set team.
	ChangeTeam( pOwner->GetTeamNumber() );

	// Set firing weapon.
	SetLauncher( pWeapon );

	// Setup the initial velocity.
	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

	float flVelocity = 1100.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flVelocity, mult_projectile_speed );

	Vector vecVelocity = vecForward * flVelocity;
	SetAbsVelocity( vecVelocity );
	SetupInitialTransmittedGrenadeVelocity( vecVelocity );

	// Setup the initial angles.
	QAngle angles;
	VectorAngles( vecVelocity, angles );
	SetAbsAngles( angles );
		
	float flGravity = 0.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flGravity, mod_rocket_gravity );
	if ( flGravity )
	{
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
		SetGravity( flGravity );
	}
}

#else

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		CreateTrails();
		CreateLightEffects();
	}

	// Watch team changes and change trail accordingly.
	if ( m_iOldTeamNum && m_iOldTeamNum != m_iTeamNum )
	{
		ParticleProp()->StopEmission();
		CreateTrails();
		CreateLightEffects();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::CreateTrails( void )
{
	if ( IsDormant() )
		return;

	if ( enginetrace->GetPointContents( GetAbsOrigin() ) & MASK_WATER )
	{
		ParticleProp()->Create( "rockettrail_waterbubbles", PATTACH_POINT_FOLLOW, "empty" );
	}
	else
	{
		const char *pszEffectName = ( GetTeamNumber() == TF_TEAM_RED ) ? "drg_cow_rockettrail_normal" : "drg_cow_rockettrail_normal_blue";
		ParticleProp()->Create( pszEffectName, PATTACH_POINT_FOLLOW, "empty" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_EnergyBall::CreateLightEffects( void )
{
	// Handle the dynamic light
	if ( lfe_muzzlelight.GetBool() )
	{
		AddEffects( EF_DIMLIGHT );

		dlight_t *dl;
		if ( IsEffectActive( EF_DIMLIGHT ) )
		{	
			dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
			dl->origin = GetAbsOrigin();
			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				if ( !m_bCritical )
				{	dl->color.r = 255; dl->color.g = 30; dl->color.b = 10; }
				else
				{	dl->color.r = 255; dl->color.g = 10; dl->color.b = 10; }
				break;

			case TF_TEAM_BLUE:
				if ( !m_bCritical )
				{	dl->color.r = 10; dl->color.g = 30; dl->color.b = 255; }
				else
				{	dl->color.r = 10; dl->color.g = 10; dl->color.b = 255; }
				break;
			}
			dl->radius = 256.0f;
			dl->die = gpGlobals->curtime + 0.1;

			tempents->RocketFlare( GetAbsOrigin() );
		}
	}
}
#endif