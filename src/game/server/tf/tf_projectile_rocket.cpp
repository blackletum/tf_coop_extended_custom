
//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// TF Rocket
//
//=============================================================================
#include "cbase.h"
#include "tf_projectile_rocket.h"
#include "tf_player.h"
#include "tf_gamerules.h"

//=============================================================================
//
// TF Rocket functions (Server specific).
//
#define ROCKET_MODEL "models/weapons/w_models/w_rocket.mdl"
#define ROCKET_MODEL_AIRSTRIKE "models/weapons/w_models/w_rocket_airstrike/w_rocket_airstrike.mdl"

LINK_ENTITY_TO_CLASS( tf_projectile_rocket, CTFProjectile_Rocket );
PRECACHE_REGISTER( tf_projectile_rocket );

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Rocket, DT_TFProjectile_Rocket )
BEGIN_NETWORK_TABLE( CTFProjectile_Rocket, DT_TFProjectile_Rocket )
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFProjectile_Rocket *CTFProjectile_Rocket::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pScorer )
{
	CTFProjectile_Rocket *pRocket = static_cast<CTFProjectile_Rocket*>( CTFBaseRocket::Create( pWeapon, "tf_projectile_rocket", vecOrigin, vecAngles, pOwner ) );

	if ( pRocket )
	{
		pRocket->SetScorer( pScorer );
	}

	return pRocket;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Rocket::Spawn()
{
	int iMiniRocket = 0;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_hLauncher.Get(), iMiniRocket, mini_rockets );
	if ( iMiniRocket != 0 )
	{
		SetModel( ROCKET_MODEL_AIRSTRIKE );
	}
	else
	{
		SetModel( ROCKET_MODEL );
	}

	string_t strModelOverride = NULL_STRING;
	CALL_ATTRIB_HOOK_STRING_ON_OTHER( m_hLauncher.Get(), strModelOverride, custom_projectile_model );
	if ( strModelOverride != NULL_STRING )
	{
		SetModel( STRING( strModelOverride ) );
	}

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Rocket::Precache()
{
	PrecacheModel( ROCKET_MODEL );
	PrecacheModel( ROCKET_MODEL_AIRSTRIKE );

	PrecacheTeamParticles( "critical_rocket_%s" );
	PrecacheParticleSystem( "rockettrail" );
	PrecacheParticleSystem( "eyeboss_projectile" );
	PrecacheParticleSystem( "halloween_rockettrail" );
	PrecacheParticleSystem( "rockettrail_underwater" );
	PrecacheParticleSystem( "pyrovision_rockettrail" );

	BaseClass::Precache();
}
