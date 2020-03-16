//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Napalm Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_napalm.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#include "fire.h"
#endif

//=============================================================================
//
// TF Napalm Grenade Projectile functions (Server specific).
//

#define GRENADE_MODEL "models/tfc/napalm.mdl"
#define GRENADE_NAPALM_TIMER	3.0f
#define GRENADE_SOUND "Weapon_Grenade_TFC.Timer"

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_napalm, CTFGrenadeNapalmProjectile );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_napalm );

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeNapalmProjectile* CTFGrenadeNapalmProjectile::Create( const Vector &position, const QAngle &angles, 
																const Vector &velocity, const AngularImpulse &angVelocity, 
																CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags )
{
	CTFGrenadeNapalmProjectile *pGrenade = static_cast<CTFGrenadeNapalmProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_napalm", position, angles, velocity, angVelocity, pOwner, pWeapon ) );
	if ( pGrenade )
	{
		pGrenade->InitGrenadeTFC( velocity, angVelocity, pOwner );
		pGrenade->ApplyLocalAngularVelocityImpulse( angVelocity );
	}

	return pGrenade;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	SetDetonateTimerLength( GRENADE_NAPALM_TIMER );	
	EmitSound( GRENADE_SOUND );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheScriptSound( GRENADE_SOUND );

	UTIL_PrecacheOther( "env_fire" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::BounceSound( void )
{
	EmitSound( "Weapon_HandGrenade.GrenadeBounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::Detonate()
{
	if ( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	trace_t trace;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector ( 0, 0, -128 ),  MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace );

	// Pull out of the wall a bit
	if ( trace.fraction != 1.0 )
	{
		SetLocalOrigin( trace.endpos + (trace.plane.normal * (m_flDamage - 24) * 0.6) );
	}

	int contents = UTIL_PointContents ( GetAbsOrigin() );
	
	if ( (contents & MASK_WATER) )
	{
		UTIL_Remove( this );
		return;
	}

	// Start some fires
	int i;
	QAngle vecTraceAngles;
	Vector vecTraceDir;
	trace_t firetrace;

	CBaseEntity *pAttacker = GetThrower();
	if ( !pAttacker )
		pAttacker = GetContainingEntity( INDEXENT(0) );

	for( i = 0 ; i < 16 ; i++ )
	{
		// build a little ray
		vecTraceAngles[PITCH]	= random->RandomFloat(45, 135);
		vecTraceAngles[YAW]		= random->RandomFloat(0, 360);
		vecTraceAngles[ROLL]	= 0.0f;

		AngleVectors( vecTraceAngles, &vecTraceDir );

		Vector vecStart, vecEnd;

		vecStart = GetAbsOrigin() + ( trace.plane.normal * 128 );
		vecEnd = vecStart + vecTraceDir * 512;

		UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace );

		Vector	ofsDir = ( firetrace.endpos - GetAbsOrigin() );
		float	offset = VectorNormalize( ofsDir );

		if ( offset > 128 )
			offset = 128;

		//Get our scale based on distance
		float scale	 = 0.1f + ( 0.75f * ( 1.0f - ( offset / 128.0f ) ) );
		float growth = 0.1f + ( 0.75f * ( offset / 128.0f ) );

		if( firetrace.fraction != 1.0 )
		{
			FireSystem_StartFire( firetrace.endpos, scale, growth, 30.0f, (SF_FIRE_START_ON|SF_FIRE_SMOKELESS|SF_FIRE_NO_GLOW), pAttacker, FIRE_NATURAL );
			//CPVSFilter filter( firetrace.endpos );
			//TE_TFExplosion( filter, 0.0f, firetrace.endpos, firetrace.plane.normal, GetWeaponID(), pAttacker->entindex() );
		}
	}

	BaseClass::Detonate();
}

#endif
