//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Concussion Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_concussion.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "ai_basenpc.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#include "beam_flags.h"
#endif

#define GRENADE_CONCUSSION_TIMER	3.0f			// seconds

//=============================================================================
//
// TF Concussion Grenade Projectile functions
//

#define GRENADE_MODEL "models/tfc/conc_grenade.mdl"
#define GRENADE_SOUND "Weapon_Grenade_TFC.Timer"

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_concussion, CTFGrenadeConcussionProjectile );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_concussion );

#ifdef GAME_DLL
// For our ring explosion
int s_nConcussionTexture = -1;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeConcussionProjectile* CTFGrenadeConcussionProjectile::Create( const Vector &position, const QAngle &angles, 
																const Vector &velocity, const AngularImpulse &angVelocity, 
																CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags )
{
	CTFGrenadeConcussionProjectile *pGrenade = static_cast<CTFGrenadeConcussionProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_concussion", position, angles, velocity, angVelocity, pOwner, pWeapon ) );
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
void CTFGrenadeConcussionProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	SetDetonateTimerLength( GRENADE_CONCUSSION_TIMER );
	EmitSound( GRENADE_SOUND );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeConcussionProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheScriptSound( GRENADE_SOUND );
	PrecacheScriptSound( "Weapon_QuakeRPG.Single" );

	s_nConcussionTexture = PrecacheModel( "sprites/lgtning.vmt" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeConcussionProjectile::BounceSound( void )
{
	EmitSound( "Weapon_HandGrenade.GrenadeBounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeConcussionProjectile::Detonate()
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

	// Explode (concuss).
	Explode( &trace, GetDamageType() );

	// Screen shake.
	if ( GetShakeAmplitude() )
	{
		UTIL_ScreenShake( GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START );
	}
}

extern ConVar tf_grenade_show_radius;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeConcussionProjectile::Explode( trace_t *pTrace, int bitsDamageType )
{
	// Invisible.
	SetModelName( NULL_STRING );	
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );

	// Use the thrower's position as the reported position
	Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;

	float flRadius = GetDamageRadius();

	//Shockring
	CBroadcastRecipientFilter filter;
	te->BeamRingPoint( filter, 0, GetAbsOrigin(),	//origin
		0,	//start radius
		flRadius * 1.2,		//end radius
		s_nConcussionTexture, //texture
		0,			//halo index
		0,			//start frame
		1,			//framerate
		0.3f,		//life
		30,			//width
		0,			//spread
		0,			//amplitude
		255,		//r
		255,		//g
		225,		//b
		64,			//a
		0,			//speed
		FBEAM_FADEOUT
		);

	// Explosion sound.
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

	// Explosion damage, using the thrower's position as the report position.
	CTFPlayer *pPlayer = ToTFPlayer( GetThrower() );
	CTFRadiusDamageInfo radiusInfo;

	CTakeDamageInfo info( this, pPlayer, this, GetBlastForce(), GetAbsOrigin(), 0, bitsDamageType, 0, &vecReported );

	CTFRadiusDamageInfo radius;
	radius.info = &info;
	radiusInfo.m_vecSrc = GetAbsOrigin();
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_flSelfDamageRadius = flRadius;

	TFGameRules()->RadiusDamage( radiusInfo );

	// Concussion.
	CBaseEntity *pEntityList[64];
	int nEntityCount = UTIL_EntitiesInSphere( pEntityList, 64, GetAbsOrigin(), flRadius, FL_CLIENT );
	for ( int iEntity = 0; iEntity < nEntityCount; ++iEntity )
	{
		CBaseEntity *pEntity = pEntityList[iEntity];
		CTFPlayer *pTestPlayer = ToTFPlayer( pEntity );
		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pEntity );

		// You can concuss yourself.
		if ( !pEntity->InSameTeam( pPlayer ) )
		{
			if ( pTestPlayer || ( pPlayer == pTestPlayer ) )
			{
				pTestPlayer->m_Shared.Concussion( pPlayer, flRadius );
			}
			else if ( pNPC )
			{
				pNPC->Concussion( pPlayer, flRadius );
			}
		}
	}

	if ( tf_grenade_show_radius.GetBool() )
	{
		DrawRadius( flRadius );
	}

	EmitSound( "Weapon_QuakeRPG.Single" );

	// Explosion decal.
	//UTIL_DecalTrace( pTrace, "Scorch" );

	// Reset.
	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );
	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
}

#endif