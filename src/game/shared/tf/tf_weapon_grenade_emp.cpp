//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Emp Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weaponbase_grenadeproj.h"
#include "tf_weapon_grenade_emp.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "ai_basenpc.h"
#include "tf_obj.h"
#include "items.h"
#include "soundent.h"
#include "KeyValues.h"
#include "particle_parse.h"
#include "beam_shared.h"
#endif

#define GRENADE_EMP_TIMER	3.0f //Seconds
#define	GRENADE_EMP_LEADIN	2.0f 

//=============================================================================
//
// TF Emp Grenade Projectile functions (Server specific).
//

#define GRENADE_MODEL "models/tfc/emp_grenade.mdl"
#define GRENADE_SOUND "Weapon_Grenade_TFC.Timer"

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_emp, CTFGrenadeEmpProjectile );
PRECACHE_REGISTER( tf_weapon_grenade_emp );

#ifdef GAME_DLL

BEGIN_DATADESC( CTFGrenadeEmpProjectile )
DEFINE_THINKFUNC( DetonateThink ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeEmpProjectile* CTFGrenadeEmpProjectile::Create( const Vector &position, const QAngle &angles, 
																const Vector &velocity, const AngularImpulse &angVelocity, 
																CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags )
{
	CTFGrenadeEmpProjectile *pGrenade = static_cast<CTFGrenadeEmpProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_emp", position, angles, velocity, angVelocity, pOwner, pWeapon ) );
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
void CTFGrenadeEmpProjectile::Spawn()
{
	Precache();
	SetModel( GRENADE_MODEL );
	SetDetonateTimerLength( GRENADE_EMP_TIMER );
	EmitSound( GRENADE_SOUND );

	BaseClass::Spawn();

	m_bPlayedLeadIn = false;

	SetThink( &CTFGrenadeEmpProjectile::DetonateThink );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeEmpProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheScriptSound( GRENADE_SOUND );
	PrecacheScriptSound( "Weapon_Grenade_Emp.LeadIn" );
	PrecacheModel( "sprites/physcannon_bluelight1b.vmt" );
	PrecacheParticleSystem( "emp_shockwave" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeEmpProjectile::BounceSound( void )
{
	EmitSound( "Weapon_HandGrenade.GrenadeBounce" );
}

extern ConVar tf_grenade_show_radius;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeEmpProjectile::Detonate()
{
	if ( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	// Explosion effect on client
	// SendDispatchEffect();

	float flRadius = 180;
	float flDamage = 1;

	if ( tf_grenade_show_radius.GetBool() )
	{
		DrawRadius( flRadius );
	}

	// Apply some amount of EMP damage to every entity in the radius. They will calculate 
	// their own damage based on how much ammo they have or some other wacky calculation.

	CTakeDamageInfo info( this, GetThrower(), vec3_origin, GetAbsOrigin(), flDamage, /* DMG_EMP |*/ DMG_PREVENT_PHYSICS_FORCE );

	CBaseEntity *pEntityList[100];
	int nEntityCount = UTIL_EntitiesInSphere( pEntityList, 100, GetAbsOrigin(), flRadius, 0 );
	int iEntity;
	for ( iEntity = 0; iEntity < nEntityCount; ++iEntity )
	{
		CBaseEntity *pEntity = pEntityList[iEntity];

		if ( pEntity == this )
			continue;

		if ( pEntity && pEntity->IsPlayer() )
			continue;

		if ( pEntity && ( pEntity->m_takedamage == DAMAGE_YES || pEntity->m_takedamage == DAMAGE_EVENTS_ONLY ) && !InSameTeam( GetThrower() ) )
		{
			pEntity->TakeDamage( info );

			if ( pEntity->IsPlayer() )
			{
				CTFPlayer *pPlayer = ToTFPlayer( pEntity );
				if ( pPlayer )
				{
					if ( !TFGameRules()->IsTFCAllowed() )
					{
						CBeam *pBeam = CBeam::BeamCreate( "sprites/physcannon_bluelight1b.vmt", 3.0 );
						if ( !pBeam )
							return;

						pBeam->PointsInit( GetAbsOrigin(), pEntity->WorldSpaceCenter() );
						pBeam->SetColor( 255, 255, 255 );
						pBeam->SetBrightness( 128 );
						pBeam->SetNoise( 12.0f );
						pBeam->SetEndWidth( 3.0f );
						pBeam->SetWidth( 3.0f );
						pBeam->LiveForTime( 0.5f );	// Fail-safe
						pBeam->SetFrameRate( 25.0f );
						pBeam->SetFrame( random->RandomInt( 0, 2 ) );

						if ( pPlayer->GetActiveTFWeapon() )
							pPlayer->RemoveAmmo( pPlayer->GetActiveTFWeapon()->Clip1(), pPlayer->GetActiveTFWeapon()->GetPrimaryAmmoType() );
					}
					else
					{
						if ( pPlayer->GetActiveTFWeapon() )
							pPlayer->RemoveAmmo( pPlayer->GetActiveTFWeapon()->Clip1(), pPlayer->GetActiveTFWeapon()->GetPrimaryAmmoType() );
					}
				}
			}
			else if ( pEntity->IsNPC() )
			{
				CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pEntity );
				if ( pNPC )
				{
					if ( !TFGameRules()->IsTFCAllowed() )
					{
						CBeam *pBeam = CBeam::BeamCreate( "sprites/physcannon_bluelight1b.vmt", 3.0 );
						if ( !pBeam )
							return;

						pBeam->PointsInit( GetAbsOrigin(), pEntity->WorldSpaceCenter() );
						pBeam->SetColor( 255, 255, 255 );
						pBeam->SetBrightness( 128 );
						pBeam->SetNoise( 12.0f );
						pBeam->SetEndWidth( 3.0f );
						pBeam->SetWidth( 3.0f );
						pBeam->LiveForTime( 0.5f );	// Fail-safe
						pBeam->SetFrameRate( 25.0f );
						pBeam->SetFrame( random->RandomInt( 0, 2 ) );

						if ( pNPC->GetActiveWeapon() )
							pNPC->RemoveAmmo( 8, pNPC->GetActiveWeapon()->GetPrimaryAmmoType() );
					}
					else
					{
						if ( pNPC->GetActiveWeapon() )
							pNPC->RemoveAmmo( 8, pNPC->GetActiveWeapon()->GetPrimaryAmmoType() );
					}
				}
			}
			else if ( pEntity->IsBaseObject() )
			{
				CBaseObject *pObject = dynamic_cast<CBaseObject *>( pEntity );
				if ( pObject )
				{
					if ( !TFGameRules()->IsTFCAllowed() )
					{
						CBeam *pBeam = CBeam::BeamCreate( "sprites/physcannon_bluelight1b.vmt", 3.0 );
						if ( !pBeam )
							return;

						pBeam->PointsInit( GetAbsOrigin(), pEntity->WorldSpaceCenter() );
						pBeam->SetColor( 255, 255, 255 );
						pBeam->SetBrightness( 128 );
						pBeam->SetNoise( 12.0f );
						pBeam->SetEndWidth( 3.0f );
						pBeam->SetWidth( 3.0f );
						pBeam->LiveForTime( 0.5f );	// Fail-safe
						pBeam->SetFrameRate( 25.0f );
						pBeam->SetFrame( random->RandomInt( 0, 2 ) );
					}
					else
					{
						pObject->RemoveAmmo( 20, "TF_AMMO_PRIMARY" );
					}
				}
			}
			else if ( pEntity->ClassMatches( "item_ammo*" ) || pEntity->ClassMatches( "tf_ammo_pack" ) )
			{
				if ( !TFGameRules()->IsTFCAllowed() )
				{
					CBeam *pBeam = CBeam::BeamCreate( "sprites/physcannon_bluelight1b.vmt", 3.0 );
					if ( !pBeam )
						return;

					pBeam->PointsInit( GetAbsOrigin(), pEntity->WorldSpaceCenter() );
					pBeam->SetColor( 255, 255, 255 );
					pBeam->SetBrightness( 128 );
					pBeam->SetNoise( 12.0f );
					pBeam->SetEndWidth( 3.0f );
					pBeam->SetWidth( 3.0f );
					pBeam->LiveForTime( 0.5f );	// Fail-safe
					pBeam->SetFrameRate( 25.0f );
					pBeam->SetFrame( random->RandomInt( 0, 2 ) );

					// reduces to atom
					pEntity->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
				}
				else
				{
					UTIL_Remove( pEntity );
				}
			}
		}
	}

	DispatchParticleEffect( "emp_shockwave", GetAbsOrigin(), vec3_angle );

	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeEmpProjectile::DetonateThink( void )
{
	if ( !m_bPlayedLeadIn && gpGlobals->curtime > GetDetonateTime() - GRENADE_EMP_LEADIN )
	{
		Vector soundPosition = GetAbsOrigin() + Vector( 0, 0, 5 );
		CPASAttenuationFilter filter( soundPosition );

		EmitSound( filter, entindex(), "Weapon_Grenade_Emp.LeadIn" );
		m_bPlayedLeadIn = true;
	}

	BaseClass::DetonateThink();
}

#endif
