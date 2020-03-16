//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase_grenadeproj.h"
#include "tf_gamerules.h"
#include "particle_parse.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "iefx.h"
#include "dlight.h"
#include "tempent.h"
#include "c_te_legacytempents.h"
// Server specific.
#else
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "tf_player.h"
#include "func_break.h"
#include "func_nogrenades.h"
#include "tf_fx.h"
#endif

#ifdef CLIENT_DLL
	extern ConVar lfe_muzzlelight;
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sv_gravity;

//=============================================================================
//
// TF Grenade projectile tables.
//

// Server specific.
#ifdef GAME_DLL
BEGIN_DATADESC( CTFWeaponBaseGrenadeProj )
	DEFINE_THINKFUNC( DetonateThink ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Detonate",	InputDetonate ),
END_DATADESC()

ConVar tf_grenade_show_radius( "tf_grenade_show_radius", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Render radius of grenades" );
ConVar tf_grenade_show_radius_time( "tf_grenade_show_radius_time", "5.0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Time to show grenade radius" );
extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );
extern void SendProxy_Angles( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFWeaponBaseGrenadeProj, DT_TFWeaponBaseGrenadeProj )

LINK_ENTITY_TO_CLASS( tf_weaponbase_grenade_proj, CTFWeaponBaseGrenadeProj );
PRECACHE_REGISTER( tf_weaponbase_grenade_proj );

BEGIN_NETWORK_TABLE( CTFWeaponBaseGrenadeProj, DT_TFWeaponBaseGrenadeProj )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_bTouched ) ),
	RecvPropVector( RECVINFO( m_vInitialVelocity ) ),
	RecvPropBool( RECVINFO( m_bCritical ) ),
	RecvPropEHandle( RECVINFO( m_hLauncher ) ),

	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropQAngles( RECVINFO_NAME( m_angNetworkAngles, m_angRotation ) ),

	RecvPropInt( RECVINFO( m_iDeflected ) ),
	RecvPropEHandle( RECVINFO( m_hDeflectOwner ) ),
#else
	SendPropInt( SENDINFO( m_bTouched ) ),
	SendPropVector( SENDINFO( m_vInitialVelocity ), 20 /*nbits*/, 0 /*flags*/, -3000 /*low value*/, 3000 /*high value*/	),
	SendPropBool( SENDINFO( m_bCritical ) ),
	SendPropEHandle( SENDINFO( m_hLauncher ) ),

	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),

	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_INTEGRAL|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropQAngles	(SENDINFO(m_angRotation), 6, SPROP_CHANGES_OFTEN, SendProxy_Angles ),

	SendPropInt( SENDINFO( m_iDeflected ), 4, SPROP_UNSIGNED ),
	SendPropEHandle( SENDINFO( m_hDeflectOwner ) ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFWeaponBaseGrenadeProj::CTFWeaponBaseGrenadeProj()
{
	m_iDeflected = 0;
	m_hDeflectOwner = NULL;

#ifndef CLIENT_DLL
	m_bUseImpactNormal = false;
	m_vecImpactNormal.Init();

	m_flNextBlipTime = 0.0f;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CTFWeaponBaseGrenadeProj::~CTFWeaponBaseGrenadeProj()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFWeaponBaseGrenadeProj::GetDamageType() 
{ 
	int iDmgType = g_aWeaponDamageTypes[ GetWeaponID() ];
	if ( m_bCritical )
	{
		iDmgType |= DMG_CRITICAL;
	}

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

	return iDmgType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFWeaponBaseGrenadeProj::GetDamageRadius( void )
{
	float flRadius = BaseClass::GetDamageRadius();
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_hLauncher.Get(), flRadius, mult_explosion_radius );
	return flRadius;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	PrecacheTeamParticles( "critical_grenade_%s" );
#endif
	PrecacheScriptSound( "Halloween.HeadlessBossAxeHitWorld" );

	PrecacheParticleSystem( "explosionTrail_seeds_mvm" );
	PrecacheParticleSystem( "fluidSmokeExpl_ring_mvm" );
	PrecacheParticleSystem( "mvm_pow_gold_seq_firework_mid" );
	PrecacheParticleSystem( "deflect_fx" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFWeaponBaseGrenadeProj::GetCustomDamageType() const
{
	return TF_DMG_CUSTOM_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFWeaponBaseGrenadeProj::GetCustomParticleIndex( void ) const
{
	int nHalloweenExplosion = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_hLauncher, nHalloweenExplosion, halloween_pumpkin_explosions );
	if( nHalloweenExplosion == 0 )
		return -1;

	return GetParticleSystemIndex( "halloween_explosion" );
}

//=============================================================================
//
// Client specific functions.
//
#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::Spawn()
{
	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );

	m_iOldTeamNum = m_iTeamNum;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		// Now stick our initial velocity into the interpolation history 
		CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();

		interpolator.ClearHistory();
		float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

		// Add a sample 1 second back.
		Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
		interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

		// Add the current sample.
		vCurOrigin = GetLocalOrigin();
		interpolator.AddToHead( changeTime, &vCurOrigin, false );

		if ( lfe_muzzlelight.GetBool() )
		{
			if ( !IsEffectActive( EF_DIMLIGHT ) )
				AddEffects( EF_DIMLIGHT );
		}
		else
		{
			if ( IsEffectActive( EF_DIMLIGHT ) )
				RemoveEffects( EF_DIMLIGHT );
		}

		CreateLightEffects();
	}
}

void CTFWeaponBaseGrenadeProj::CreateLightEffects( void )
{
	// Handle the dynamic light
	if ( lfe_muzzlelight.GetBool() )
	{
		dlight_t *dl;
		if ( IsEffectActive( EF_DIMLIGHT ) )
		{	
			dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
			dl->origin = GetAbsOrigin();
			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				if ( !m_bCritical ) {
				dl->color.r = 255; dl->color.g = 30; dl->color.b = 10; dl->style = 0; }
				else {
				dl->color.r = 255; dl->color.g = 10; dl->color.b = 10; dl->style = 1; }
				break;

			case TF_TEAM_BLUE:
				if ( !m_bCritical ) {
				dl->color.r = 10; dl->color.g = 30; dl->color.b = 255; dl->style = 0; }
				else {
				dl->color.r = 10; dl->color.g = 10; dl->color.b = 255; dl->style = 1; }
				break;
			}
			dl->radius = 256.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;

			tempents->RocketFlare( GetAbsOrigin() );
		}
	}
}

//=============================================================================
//
// Server specific functions.
//
#else

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFWeaponBaseGrenadeProj *CTFWeaponBaseGrenadeProj::Create( const char *szName, const Vector &position, const QAngle &angles,
	const Vector &velocity, const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon )
{
	CTFWeaponBaseGrenadeProj *pGrenade = static_cast<CTFWeaponBaseGrenadeProj*>( CBaseEntity::Create( szName, position, angles, pOwner ) );
	if ( pGrenade )
	{
		pGrenade->InitGrenade( velocity, angVelocity, pOwner, pWeapon );
		//pGrenade->SetDetonateTimerLength( timer );
	}

	return pGrenade;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::InitGrenade( const Vector &velocity, const AngularImpulse &angVelocity,
	CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon )
{
	// We can't use OwnerEntity for grenades, because then the owner can't shoot them with his hitscan weapons (due to collide rules)
	// Thrower is used to store the person who threw the grenade, for damage purposes.
	SetOwnerEntity( NULL );
	SetThrower( pOwner ); 

	SetupInitialTransmittedGrenadeVelocity( velocity );

	SetGravity( 0.4f/*BaseClass::GetGrenadeGravity()*/ );
	SetFriction( 0.2f/*BaseClass::GetGrenadeFriction()*/ );
	SetElasticity( 0.45f/*BaseClass::GetGrenadeElasticity()*/ );

	SetLauncher( pWeapon );

	CTFWeaponBaseGun *pTFWeapon = dynamic_cast<CTFWeaponBaseGun *>( pWeapon );
	if ( pTFWeapon )
	{
		SetDamage( pTFWeapon->GetProjectileDamage() );
		SetDamageRadius( pTFWeapon->GetTFWpnData().m_flDamageRadius );
		
		int iProjectileType = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pTFWeapon, iProjectileType, override_projectile_type );
		if ( iProjectileType == TF_PROJECTILE_PIPEBOMB_REMOTE_PRACTICE )
			SetModel( "models/weapons/w_models/w_stickybomb2.mdl" );

		if ( pTFWeapon->GetWeaponProjectileType() == TF_PROJECTILE_CANNONBALL )
			SetModel( "models/weapons/w_models/w_cannonball.mdl" );

		float flModelScale = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pTFWeapon, flModelScale, projectile_model_scale );
		SetModelScale( flModelScale );
	}
	else
	{
		SetDamage( 100 );
		SetDamageRadius( 146 );
	}

	ChangeTeam( pOwner->GetTeamNumber() );

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}

	if ( TFGameRules()->IsTFCAllowed() )
		SetAbsVelocity( velocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::InitGrenadeTFC( const Vector &velocity, const AngularImpulse &angVelocity,
	CBaseCombatCharacter *pOwner )
{
	// We can't use OwnerEntity for grenades, because then the owner can't shoot them with his hitscan weapons (due to collide rules)
	// Thrower is used to store the person who threw the grenade, for damage purposes.
	SetOwnerEntity( NULL );
	SetThrower( pOwner ); 

	SetupInitialTransmittedGrenadeVelocity( velocity );

	SetGravity( 0.4f/*BaseClass::GetGrenadeGravity()*/ );
	SetFriction( 0.2f/*BaseClass::GetGrenadeFriction()*/ );
	SetElasticity( 0.45f/*BaseClass::GetGrenadeElasticity()*/ );

	ChangeTeam( pOwner->GetTeamNumber() );

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}

	SetAbsVelocity( velocity );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::Spawn( void )
{
	// Base class spawn.
	BaseClass::Spawn();

	// So it will collide with physics props!
	SetSolidFlags( FSOLID_NOT_STANDABLE );
	SetSolid( SOLID_BBOX );	

	AddEffects( EF_NOSHADOW );

	// Set the grenade size here.
	UTIL_SetSize( this, Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ) );

	// Set the movement type.
	SetCollisionGroup( TF_COLLISIONGROUP_GRENADES );

	// Don't collide with players on the owner's team for the first bit of our life
	m_flCollideWithTeammatesTime = gpGlobals->curtime + GetCollideWithTeammatesDelay();
	m_bCollideWithTeammates = false;

	if ( TFGameRules()->IsTFCAllowed() )
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	else
		VPhysicsInitNormal( SOLID_BBOX, 0, false );

	m_takedamage = DAMAGE_EVENTS_ONLY;

	if ( GetThrower() )
	{
		// Set the team.
		ChangeTeam( GetThrower()->GetTeamNumber() );
	}

	// Set skin based on team ( red = 1, blue = 2 )
	m_nSkin = GetTeamNumber() - 2;

	// Setup the think and touch functions (see CBaseEntity).
	SetThink( &CTFWeaponBaseGrenadeProj::DetonateThink );
	SetNextThink( gpGlobals->curtime + 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::Explode( trace_t *pTrace, int bitsDamageType )
{
	SetModelName( NULL_STRING );//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;

	// Figure out Econ ID.
	int iItemID = -1;
	if ( m_hLauncher.Get() )
	{
		CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( m_hLauncher.Get() );
		if ( pWeapon )
		{
			iItemID = pWeapon->GetItemID();
		}
	}

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );
	}

	// Explosion effect on client
	Vector vecOrigin = GetAbsOrigin();
	CPVSFilter filter( vecOrigin );
	CTFPlayer *pTFAttacker = ToTFPlayer( GetThrower() );

	CSoundEnt::InsertSound( SOUND_COMBAT | SOUND_CONTEXT_EXPLOSION, vecOrigin, BASEGRENADE_EXPLOSION_VOLUME, 0.25, pTFAttacker );

	if ( UseImpactNormal() )
	{
		if ( pTrace->m_pEnt && ( pTrace->m_pEnt->IsPlayer() || pTrace->m_pEnt->IsNPC() ) )
		{
			TE_TFExplosion( filter, 0.0f, vecOrigin, GetImpactNormal(), GetWeaponID(), pTrace->m_pEnt->entindex(), SPECIAL1, GetCustomParticleIndex() );
		}
		else
		{
			TE_TFExplosion( filter, 0.0f, vecOrigin, GetImpactNormal(), GetWeaponID(), -1, SPECIAL1, GetCustomParticleIndex() );
		}
	}
	else
	{
		if ( pTrace->m_pEnt && ( pTrace->m_pEnt->IsPlayer() || pTrace->m_pEnt->IsNPC() ) )
		{
			TE_TFExplosion( filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), pTrace->m_pEnt->entindex(), SPECIAL1, GetCustomParticleIndex() );
		}
		else
		{
			TE_TFExplosion( filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), -1, SPECIAL1, GetCustomParticleIndex() );
		}
	}


	// Use the thrower's position as the reported position
	Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;

	float flRadius = GetDamageRadius();

	if ( tf_grenade_show_radius.GetBool() )
	{
		DrawRadius( flRadius );
	}

	CTakeDamageInfo newInfo( this, GetThrower(), m_hLauncher, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, GetDamageCustom(), &vecReported );
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info = &newInfo;
	radiusInfo.m_vecSrc = vecOrigin;
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_flSelfDamageRadius = 146.0f;

	TFGameRules()->RadiusDamage( radiusInfo );

	// Don't decal players with scorch.
	if ( pTrace->m_pEnt && !pTrace->m_pEnt->IsPlayer() )
	{
		UTIL_DecalTrace( pTrace, "Scorch" );
	}

	int iUseLargeExplosion = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_hLauncher.Get(), iUseLargeExplosion, use_large_smoke_explosion );
	if ( iUseLargeExplosion )
	{
		DispatchParticleEffect( "explosionTrail_seeds_mvm", vecOrigin ,vec3_angle );
		DispatchParticleEffect( "fluidSmokeExpl_ring_mvm", vecOrigin, vec3_angle );
	}

	if ( TFGameRules()->IsHolidayActive( kHoliday_NewYears ) )
		DispatchParticleEffect( "mvm_pow_gold_seq_firework_mid", vecOrigin ,vec3_angle );

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );

	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFWeaponBaseGrenadeProj::OnTakeDamage( const CTakeDamageInfo &info )
{
	CTakeDamageInfo info2 = info;

	// Reduce explosion damage so that we don't get knocked too far
	if ( info.GetDamageType() & DMG_BLAST )
	{
		info2.ScaleDamageForce( 0.05 );
	}

	// We need to skip back to the base entity take damage, because
	// CBaseCombatCharacter doesn't, which prevents us from reacting
	// to physics impact damage.
	return CBaseEntity::OnTakeDamage( info2 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::DetonateThink( void )
{
	if ( !IsInWorld() )
	{
		Remove( );
		return;
	}
	
	BlipSound();

	if( !m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime )
	{
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this );
		m_bHasWarnedAI = true;
	}

	if ( gpGlobals->curtime > m_flCollideWithTeammatesTime && m_bCollideWithTeammates == false )
	{
		m_bCollideWithTeammates = true;
	}

	if ( GetDestroyableHitCount() >= 2 )
		Destroy( false, true );

	if ( gpGlobals->curtime > m_flDetonateTime )
	{
		Detonate();
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::Detonate( void )
{
	// Putting this here just in case we're inside prediction to make sure all effects show up.
	CDisablePredictionFiltering disabler;

	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	SetThink( NULL );

	vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
	Ray_t ray; ray.Init( vecSpot, vecSpot + Vector ( 0, 0, -32 ) );
	UTIL_Portal_TraceRay( ray, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

	Explode( &tr, GetDamageType() );

	if ( GetShakeAmplitude() )
	{
		UTIL_ScreenShake( GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the time at which the grenade will explode.
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::SetDetonateTimerLength( float timer )
{
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_hLauncher.Get(), m_flDetonateTime, fuse_mult );

	m_flDetonateTime = gpGlobals->curtime + timer;
	m_flWarnAITime = gpGlobals->curtime + (timer - 1.5);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
{
	//Assume all surfaces have the same elasticity
	float flSurfaceElasticity = 1.0f;

	//Don't bounce off of players with perfect elasticity
	if ( trace.m_pEnt && ( trace.m_pEnt->IsPlayer() || trace.m_pEnt->IsNPC() ) )
	{
		flSurfaceElasticity = 0.3f;
	}

	float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
	flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

	// NOTE: A backoff of 2.0f is a reflection
	Vector vecAbsVelocity;
	PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
	vecAbsVelocity *= flTotalElasticity;

	// Get the total velocity (player + conveyors, etc.)
	VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
	float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

	// Stop if on ground.
	if ( trace.plane.normal.z > 0.7f )			// Floor
	{
		// Verify that we have an entity.
		CBaseEntity *pEntity = trace.m_pEnt;
		Assert( pEntity );

		SetAbsVelocity( vecAbsVelocity );

		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			if ( pEntity->IsStandable() )
				SetGroundEntity( pEntity );

			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );

			//align to the ground so we're not standing on end
			QAngle angle;
			VectorAngles( trace.plane.normal, angle );

			// rotate randomly in yaw
			angle[1] = random->RandomFloat( 0, 360 );

			// TFTODO: rotate around trace.plane.normal

			SetAbsAngles( angle );			
		}
		else
		{
			Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
			Vector vecBaseDir = GetBaseVelocity();
			VectorNormalize( vecBaseDir );
			float flScale = vecDelta.Dot( vecBaseDir );

			VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
			VectorMA( vecVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity );
			PhysicsPushEntity( vecVelocity, &trace );
		}
	}
	else
	{
		// If we get *too* slow, we'll stick without ever coming to rest because
		// we'll get pushed down by gravity faster than we can escape from the wall.
		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );
		}
		else
		{
			SetAbsVelocity( vecAbsVelocity );
		}
	}

	BounceSound();

	// tell the npc a grenade has bounced
	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 256, 5, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFWeaponBaseGrenadeProj::ShouldNotDetonate( void )
{
	return InNoGrenadeZone( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::RemoveGrenade( bool bBlinkOut )
{
	BaseClass::RemoveGrenade( bBlinkOut );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::SetLauncher( CBaseEntity *pLauncher )
{
	m_hLauncher = pLauncher;
	CBaseProjectile::SetLauncher( pLauncher );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		Vector vecOldVelocity, vecVelocity;

		pPhysicsObject->GetVelocity( &vecOldVelocity, NULL );

		float flSpeed = vecOldVelocity.Length();

		vecVelocity = vecDir;
		vecVelocity *= flSpeed;
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
// Purpose: Increment deflects counter
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::IncremenentDeflected( void )
{
	m_iDeflected++;
}

//-----------------------------------------------------------------------------
// Purpose: This will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
//			Always ignores other grenade projectiles.
//-----------------------------------------------------------------------------
class CTraceFilterCollisionGrenades : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCollisionGrenades );

	CTraceFilterCollisionGrenades( const IHandleEntity *passentity, const IHandleEntity *passentity2 )
		: m_pPassEnt(passentity), m_pPassEnt2(passentity2)
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		if ( pEntity )
		{
			if ( pEntity == m_pPassEnt2 )
				return false;
			if ( pEntity->GetCollisionGroup() == TF_COLLISIONGROUP_GRENADES )
				return false;
			if ( pEntity->GetCollisionGroup() == TFCOLLISION_GROUP_ROCKETS )
				return false;
			if ( pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS )
				return false;
			if ( pEntity->GetCollisionGroup() == COLLISION_GROUP_NONE )
				return false;
			if ( pEntity->GetCollisionGroup() == TFCOLLISION_GROUP_RESPAWNROOMS )
				return false;

			return true;
		}

		return true;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	const IHandleEntity *m_pPassEnt2;
};


const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

//-----------------------------------------------------------------------------
// Purpose: Grenades aren't solid to players, so players don't get stuck on
//			them when they're lying on the ground. We still want thrown grenades
//			to bounce of players though, so manually trace ahead and see if we'd
//			hit something that we'd like the grenade to "collide" with.
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );

	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity( &vel, &angVel );

	Vector start = GetAbsOrigin();

	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGrenades filter( this, GetThrower() );
	trace_t tr;

	Ray_t ray; ray.Init( start, start + vel * gpGlobals->frametime );
	UTIL_Portal_TraceRay( ray, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );

	bool bHitEnemy = false;

	if ( tr.m_pEnt && !InSameTeam( tr.m_pEnt ) )
	{
		bHitEnemy = true;
	}

	if ( tr.m_pEnt )
	{
		CProp_Portal* pPortal = dynamic_cast<CProp_Portal*>( tr.m_pEnt );
		if ( pPortal )
		{
			if ( UTIL_IsBoxIntersectingPortal( GetAbsOrigin(), WorldAlignSize(), pPortal ) && pPortal->IsActivedAndLinked() )
			{
				m_bInSolid = false;
				bHitEnemy = false;
				return;
			}
		}
	}

	if ( tr.startsolid )
	{
		if ( (m_bInSolid == false && m_bCollideWithTeammates == true) || ( m_bInSolid == false  && bHitEnemy == true ) )
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity( &vel, NULL );
		}
		m_bInSolid = true;
		return;
	}

	m_bInSolid = false;

	if ( tr.DidHit() )
	{
		Touch( tr.m_pEnt );
		
		if ( m_bCollideWithTeammates == true || bHitEnemy == true )
		{
			// reflect velocity around normal
			vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;

			// absorb 80% in impact
			vel *= GetElasticity();

			if ( bHitEnemy == true )
			{
				vel *= 0.5f;
			}

			angVel *= -0.5f;
			pPhysics->SetVelocity( &vel, &angVel );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	int otherIndex = !index;
	CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

	if ( !pHitEntity )
		return;

	CProp_Portal* pPortal = dynamic_cast<CProp_Portal*>( pHitEntity );
	if ( pPortal )
	{
		bool bHitPortal = UTIL_IsBoxIntersectingPortal( GetAbsOrigin(), WorldAlignSize(), pPortal );

		if ( bHitPortal && pPortal->IsActivedAndLinked() )
		{
			pEvent->surfaceProps[0] = pEvent->surfaceProps[1] = physprops->GetSurfaceIndex( "default" );
			m_bTouched = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Debug
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::DrawRadius( float flRadius )
{
	Vector pos = GetAbsOrigin();
	int r = 255;
	int g = 0, b = 0;
	float flLifetime = tf_grenade_show_radius_time.GetFloat();
	bool bDepthTest = true;

	Vector edge, lastEdge;
	NDebugOverlay::Line( pos, pos + Vector( 0, 0, 50 ), r, g, b, !bDepthTest, flLifetime );

	lastEdge = Vector( flRadius + pos.x, pos.y, pos.z );
	float angle;
	for( angle=0.0f; angle <= 360.0f; angle += 22.5f )
	{
		edge.x = flRadius * cos( angle ) + pos.x;
		edge.y = pos.y;
		edge.z = flRadius * sin( angle ) + pos.z;

		NDebugOverlay::Line( edge, lastEdge, r, g, b, !bDepthTest, flLifetime );

		lastEdge = edge;
	}

	lastEdge = Vector( pos.x, flRadius + pos.y, pos.z );
	for( angle=0.0f; angle <= 360.0f; angle += 22.5f )
	{
		edge.x = pos.x;
		edge.y = flRadius * cos( angle ) + pos.y;
		edge.z = flRadius * sin( angle ) + pos.z;

		NDebugOverlay::Line( edge, lastEdge, r, g, b, !bDepthTest, flLifetime );

		lastEdge = edge;
	}

	lastEdge = Vector( pos.x, flRadius + pos.y, pos.z );
	for( angle=0.0f; angle <= 360.0f; angle += 22.5f )
	{
		edge.x = flRadius * cos( angle ) + pos.x;
		edge.y = flRadius * sin( angle ) + pos.y;
		edge.z = pos.z;

		NDebugOverlay::Line( edge, lastEdge, r, g, b, !bDepthTest, flLifetime );

		lastEdge = edge;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::InputDetonate( inputdata_t &data )
{
	Detonate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGrenadeProj::Destroy( bool bBlinkOut, bool bBreakRocket )
{
	if ( bBlinkOut )
	{
		RemoveGrenade();
		return;
	}

	if ( bBreakRocket )
	{
		EmitSound( "Halloween.HeadlessBossAxeHitWorld" );

		CPVSFilter filter( GetAbsOrigin() );
		UserMessageBegin( filter, "BreakModelRocketDud" );
			WRITE_SHORT( GetModelIndex() );
			WRITE_VEC3COORD( GetAbsOrigin() );
			WRITE_ANGLES( GetAbsAngles() );
		MessageEnd();
	}

	UTIL_Remove( this );
}

#endif
