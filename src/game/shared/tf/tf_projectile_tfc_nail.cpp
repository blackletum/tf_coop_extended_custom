//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tf_projectile_tfc_nail.h"
#include "tf_weaponbase_gun.h"
#ifdef CLIENT_DLL
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"
#include "c_te_effect_dispatch.h"
#include "c_tf_player.h"
#include "cliententitylist.h"
#else
#include "tf_player.h"
#include "effect_dispatch_data.h"
#endif

#define NAILGUN_MODEL "models/tfc/nail.mdl"
#define TFC_NAIL_DISPATCH_EFFECT		"ClientProjectile_TFCNail"

LINK_ENTITY_TO_CLASS( tf_nailgun_nail, CTFNailgunNail );
PRECACHE_REGISTER( tf_nailgun_nail );

#ifdef GAME_DLL
BEGIN_DATADESC( CTFNailgunNail )
	DEFINE_ENTITYFUNC( NailTouch )
END_DATADESC()
#endif

short g_sModelIndexTFCNail;
void PrecacheTFCNail( void *pUser )
{
	g_sModelIndexTFCNail = modelinfo->GetModelIndex( NAILGUN_MODEL );
}

PRECACHE_REGISTER_FN( PrecacheTFCNail );


CTFNailgunNail::CTFNailgunNail()
{
}

CTFNailgunNail::~CTFNailgunNail()
{
}

CTFNailgunNail *CTFNailgunNail::CreateNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pWeapon, bool bCritical )
{
	CTFNailgunNail *pNail = static_cast<CTFNailgunNail*>( CTFBaseProjectile::Create( "tf_nailgun_nail", vecOrigin, vecAngles, pOwner, CTFNailgunNail::GetInitialVelocity(), g_sModelIndexTFCNail, TFC_NAIL_DISPATCH_EFFECT, pOwner, bCritical ) );
#ifdef GAME_DLL
	CTFWeaponBaseGun *pTFWeapon = dynamic_cast<CTFWeaponBaseGun *>( pWeapon );
	if ( pTFWeapon )
	{
		pNail->SetDamage( pTFWeapon->GetProjectileDamage() );
	}
	else
	{
		pNail->SetDamage( 9 );
	}
#endif
	return pNail;
}

CTFNailgunNail *CTFNailgunNail::CreateSuperNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pWeapon, bool bCritical )
{
	CTFNailgunNail *pNail = CreateNail( vecOrigin, vecAngles, pOwner, pWeapon );
#ifdef GAME_DLL
	CTFWeaponBaseGun *pTFWeapon = dynamic_cast<CTFWeaponBaseGun *>( pWeapon );
	if ( pTFWeapon )
	{
		pNail->SetDamage( pTFWeapon->GetProjectileDamage() * 1.3 );
	}
	else
	{
		pNail->SetDamage( 12 );
	}
#endif
	return pNail;
}

CTFNailgunNail *CTFNailgunNail::CreateTranqNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pWeapon, bool bCritical )
{
	CTFNailgunNail *pNail = CreateNail( vecOrigin, vecAngles, pOwner, pWeapon );
#ifdef GAME_DLL
	CTFWeaponBaseGun *pTFWeapon = dynamic_cast<CTFWeaponBaseGun *>( pWeapon );
	if ( pTFWeapon )
	{
		pNail->SetDamage( pTFWeapon->GetProjectileDamage() );
	}
	else
	{
		pNail->SetDamage( 18 );
	}
#endif
	return pNail;
}

CTFNailgunNail *CTFNailgunNail::CreateRailgunNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pWeapon, bool bCritical )
{
	CTFNailgunNail *pNail = CreateNail( vecOrigin, vecAngles, pOwner, pWeapon );
#ifdef GAME_DLL
	CTFWeaponBaseGun *pTFWeapon = dynamic_cast<CTFWeaponBaseGun *>( pWeapon );
	if ( pTFWeapon )
	{
		pNail->SetDamage( pTFWeapon->GetProjectileDamage() );
	}
	else
	{
		pNail->SetDamage( 23 );
	}
#endif
	return pNail;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFNailgunNail::Spawn()
{
#ifdef CLIENT_DLL
	BaseClass::Spawn();
#else
	// Precache.
	Precache();

	SetModel( GetProjectileModelName() );

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	AddEFlags( EFL_NO_WATER_VELOCITY_CHANGE );

	UTIL_SetSize( this, -Vector( 1.0f, 1.0f, 1.0f ), Vector( 1.0f, 1.0f, 1.0f ) );
	//SetSize( Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	// Setup attributes.
	SetGravity( GetGravity() );
	m_takedamage = DAMAGE_NO;

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	// Setup the touch and think functions.
	SetTouch( &CTFNailgunNail::NailTouch );
	SetThink( &CTFNailgunNail::FlyThink );
	SetNextThink( gpGlobals->curtime );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFNailgunNail::Precache()
{
	PrecacheModel( NAILGUN_MODEL );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CTFNailgunNail::GetProjectileModelName( void )
{
	return NAILGUN_MODEL;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFNailgunNail::NailTouch( CBaseEntity *pOther )
{
	// Verify a correct "other."
	Assert( pOther );
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) )
		return;

	// Handle hitting skybox (disappear).
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	trace_t *pNewTrace = const_cast<trace_t*>( pTrace );

	if( pTrace->surface.flags & SURF_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	// pass through ladders
	if( pTrace->surface.flags & CONTENTS_LADDER )
		return;

	if ( pOther->IsWorld() )
	{
		SetAbsVelocity( vec3_origin	);
		AddSolidFlags( FSOLID_NOT_SOLID );

		// Remove immediately. Clientside projectiles will stick in the wall for a bit.
		UTIL_Remove( this );
		return;
	}

	CProp_Portal *pPortal = dynamic_cast<CProp_Portal*>( pOther ); 
	if ( pPortal )
		return;

	// determine the inflictor, which is the weapon which fired this projectile
	CBaseEntity *pInflictor = NULL;
	CBaseEntity *pOwner = GetOwnerEntity();
	if ( pOwner )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( pOwner );
		if ( pTFPlayer )
		{
			pInflictor = pTFPlayer->Weapon_OwnsThisID( GetWeaponID() );
		}
	}

	CTakeDamageInfo info;
	info.SetAttacker( GetOwnerEntity() );		// the player who operated the thing that emitted nails
	info.SetInflictor( pInflictor );	// the weapon that emitted this projectile
	info.SetWeapon( pInflictor );
	info.SetDamage( GetDamage() );
	info.SetDamageForce( GetDamageForce() );
	info.SetDamagePosition( GetAbsOrigin() );
	info.SetDamageType( GetDamageType() );

	Vector dir;
	AngleVectors( GetAbsAngles(), &dir );

	pOther->DispatchTraceAttack( info, dir, pNewTrace );
	ApplyMultiDamage();

	UTIL_Remove( this );
}
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientsideProjectileTFCNailCallback( const CEffectData &data )
{
	C_TFPlayer *pPlayer = dynamic_cast<C_TFPlayer*>( ClientEntityList().GetBaseEntityFromHandle( data.m_hEntity ) );
	if ( pPlayer )
	{
		C_LocalTempEntity *pNail = ClientsideProjectileCallback( data, 0.5f );
		if ( pNail )
		{
			pNail->AddEffects( EF_NOSHADOW );
			pNail->flags |= FTENT_USEFASTCOLLISIONS;
		}
	}
}

DECLARE_CLIENT_EFFECT( TFC_NAIL_DISPATCH_EFFECT, ClientsideProjectileTFCNailCallback );

#endif
