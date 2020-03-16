//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: Quake Flare Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_flare.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "ai_basenpc.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#endif

#define GRENADE_FLARE_TIMER	8.0f			// seconds

//=============================================================================
//
// TF FLARE Grenade Projectile functions
//

#define GRENADE_MODEL "models/crossbow_bolt.mdl"
#define GRENADE_SOUND "Weapon_Grenade_TFC.Timer"

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_flare, CTFGrenadeFlareProjectile );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_flare );

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeFlareProjectile* CTFGrenadeFlareProjectile::Create( const Vector &position, const QAngle &angles, 
																const Vector &velocity, const AngularImpulse &angVelocity, 
																CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags )
{
	CTFGrenadeFlareProjectile *pGrenade = static_cast<CTFGrenadeFlareProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_flare", position, angles, velocity, angVelocity, pOwner, pWeapon ) );
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
void CTFGrenadeFlareProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	SetDetonateTimerLength( GRENADE_FLARE_TIMER );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeFlareProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheScriptSound( GRENADE_SOUND );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeFlareProjectile::BounceSound( void )
{
	EmitSound( "Weapon_HandGrenade.GrenadeBounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeFlareProjectile::Detonate()
{
	if ( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	// The trace start/end.
	Vector vecStart = GetAbsOrigin() + Vector( 0.0f, 0.0f, 8.0f );
	Vector vecEnd = vecStart + Vector( 0.0f, 0.0f, -32.0f );

	trace_t	trace;
	UTIL_TraceLine ( vecStart, vecEnd, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &trace );

	Explode( &trace, GetDamageType() );
}

extern ConVar tf_grenade_show_radius;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeFlareProjectile::Explode( trace_t *pTrace, int bitsDamageType )
{
	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );

	// Use the thrower's position as the reported position
	Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;

	float flRadius = GetDamageRadius();

	// Explosion sound.
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

	// Explosion damage, using the thrower's position as the report position.
	/*CTFPlayer *pPlayer = ToTFPlayer( GetThrower() );
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info.Set( this, pPlayer, this, GetBlastForce(), GetAbsOrigin(), 0, bitsDamageType, 0, &vecReported );
	radiusInfo.m_vecSrc = GetAbsOrigin();
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_flSelfDamageRadius = flRadius;

	TFGameRules()->RadiusDamage( radiusInfo );*/

	if ( tf_grenade_show_radius.GetBool() )
	{
		DrawRadius( flRadius );
	}

	//EmitSound( "" );

	// Explosion decal.
	UTIL_DecalTrace( pTrace, "Scorch" );

	// Reset.
	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );
	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
}

#endif