//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Caltrop Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_caltrop.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#endif

#define GRENADE_CALTROP_TIMER			3.0f //Seconds
#define GRENADE_CALTROP_RELEASE_COUNT	6
#define GRENADE_CALTROP_DAMAGE			10

//=============================================================================
//
// TF Caltrop Grenade Projectile functions
//
LINK_ENTITY_TO_CLASS( tf_weapon_grenade_caltrop, CTFGrenadeCaltropProjectile );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_caltrop );

IMPLEMENT_NETWORKCLASS_ALIASED( TFGrenadeCaltropProjectile, DT_TFGrenadeCaltropProjectile )

BEGIN_NETWORK_TABLE( CTFGrenadeCaltropProjectile, DT_TFGrenadeCaltropProjectile )
END_NETWORK_TABLE()

#ifdef GAME_DLL

#define GRENADE_MODEL "models/tfc/caltrop.mdl"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeCaltropProjectile* CTFGrenadeCaltropProjectile::Create( const Vector &position, const QAngle &angles, 
																const Vector &velocity, const AngularImpulse &angVelocity, 
																CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags )
{
	CTFGrenadeCaltropProjectile *pGrenade = static_cast<CTFGrenadeCaltropProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_caltrop", position, angles, velocity, angVelocity, pOwner, pWeapon ) );
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
void CTFGrenadeCaltropProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	SetDetonateTimerLength( GRENADE_CALTROP_TIMER );
	SetTouch( &CTFGrenadeCaltropProjectile::CaltropTouch );

	BaseClass::Spawn();

	// We want to get touch functions called so we can damage enemy players
	AddSolidFlags( FSOLID_TRIGGER );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeCaltropProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeCaltropProjectile::BounceSound( void )
{
	EmitSound( "Weapon_HandGrenade.GrenadeBounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeCaltropProjectile::Detonate()
{
	if ( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	// have the caltrop disappear
	UTIL_Remove( this );
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeCaltropProjectile::CaltropTouch( CBaseEntity *pOther )
{
	if ( ( !pOther->IsPlayer() || !pOther->IsNPC() ) || !( pOther->GetFlags() & FL_ONGROUND ) || !pOther->IsAlive() )
		return;

	// Don't hurt friendlies
	if ( InSameTeam( pOther ) )
		return;

	// Caltrops need to be on the ground. Check to see if we're still moving.
	if ( !TFGameRules()->IsTFCAllowed() )
	{
		Vector vecVelocity;
		VPhysicsGetObject()->GetVelocity( &vecVelocity, NULL );
		if ( vecVelocity.LengthSqr() > (1*1) )
			return;
	}

#ifdef GAME_DLL
	// Do the leg damage to the player
	CTakeDamageInfo info( this, GetThrower(), GRENADE_CALTROP_DAMAGE, /*DMG_LEG_DAMAGE | */ DMG_PREVENT_PHYSICS_FORCE );
	pOther->TakeDamage( info );

	// have the caltrop disappear
	UTIL_Remove( this );
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGrenadeCaltropProjectile::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	
	if ( updateType == DATA_UPDATE_CREATED )
	{
		/*
		SetSolidFlags( FSOLID_NOT_STANDABLE );
		SetSolid( SOLID_BBOX );	

		SetCollisionBounds( Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ) );

		// We want touch calls on the client.
		// So override the collision group, but make it a trigger
		SetCollisionGroup( COLLISION_GROUP_NONE );
		AddSolidFlags( FSOLID_TRIGGER );

		UpdatePartitionListEntry();

		CollisionProp()->UpdatePartition();
		*/
	}
}
#endif