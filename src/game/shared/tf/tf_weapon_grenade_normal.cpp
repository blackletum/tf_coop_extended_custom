//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Normal Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_normal.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#endif

#define GRENADE_TIMER	3.0f //Seconds

//=============================================================================
//
// TF Normal Grenade Projectile functions (Server specific).
//

#define GRENADE_MODEL "models/w_grenade.mdl"
#define GRENADE_SOUND "Weapon_Grenade_TFC.Timer"

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_normal, CTFGrenadeNormalProjectile );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_normal );

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeNormalProjectile* CTFGrenadeNormalProjectile::Create( const Vector &position, const QAngle &angles, 
																const Vector &velocity, const AngularImpulse &angVelocity, 
																CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags )
{
	CTFGrenadeNormalProjectile *pGrenade = static_cast<CTFGrenadeNormalProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_normal", position, angles, velocity, angVelocity, pOwner, pWeapon ) );
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
void CTFGrenadeNormalProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	SetDetonateTimerLength( GRENADE_TIMER );
	EmitSound( GRENADE_SOUND );

	BaseClass::Spawn();

	m_flDamage = 100;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNormalProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheScriptSound( GRENADE_SOUND );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNormalProjectile::BounceSound( void )
{
	EmitSound( "Weapon_HandGrenade.GrenadeBounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNormalProjectile::Detonate()
{
	if ( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	BaseClass::Detonate();
}

#endif
