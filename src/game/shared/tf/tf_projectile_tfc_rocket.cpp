//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		RPG
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "tf_projectile_tfc_rocket.h"
#include "effect_dispatch_data.h"
#include "tf_gamerules.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "model_types.h"
#include "beamdraw.h"
#include "fx_line.h"
#include "view.h"
#else
#include "tf_player.h"
#include "rope.h"
#include "soundent.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "shake.h"
#include "props_shared.h"
#include "debugoverlay_shared.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
void TE_BeamFollow(IRecipientFilter& filter, float delay,
	int iEntIndex, int modelIndex, int haloIndex, float life, float width, float endWidth,
	float fadeLength, float r, float g, float b, float a);
#endif

//=============================================================================
// RPG Rocket
//=============================================================================
IMPLEMENT_NETWORKCLASS_ALIASED( TFRpgRocket, DT_TFRpgRocket )

BEGIN_NETWORK_TABLE( CTFRpgRocket, DT_TFRpgRocket )
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC(CTFRpgRocket)
	DEFINE_FIELD( m_flIgniteTime, FIELD_TIME ),
	//DEFINE_FIELD( m_iTrail,			FIELD_INTEGER ),

	// Function Pointers
	DEFINE_ENTITYFUNC( RocketTouch ),
	DEFINE_THINKFUNC( IgniteThink ),
	DEFINE_THINKFUNC( SeekThink ),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( tf_rpg_rocket, CTFRpgRocket );

CTFRpgRocket::CTFRpgRocket()
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CTFRpgRocket::Precache( void )
{
	PrecacheModel( "models/rpgrocket.mdl" );
	PrecacheScriptSound( "Weapon_RPG.RocketIgnite" );
	m_iTrail = PrecacheModel( "sprites/smoke.vmt" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CTFRpgRocket::Spawn( void )
{
	Precache();

#ifdef GAME_DLL
	SetSolid( SOLID_BBOX );
	SetModel( "models/rpgrocket.mdl" );

	UTIL_SetSize( this, -Vector(0, 0, 0), Vector(0, 0, 0) );

	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	AddEFlags( EFL_NO_WATER_VELOCITY_CHANGE );
	SetCollisionGroup( TFCOLLISION_GROUP_ROCKETS );

	// Setup attributes.
	m_takedamage = DAMAGE_NO;
	SetGravity( 0.0f );

	m_flDamage = 60;
	//m_DmgRadius = m_flDamage * 2.5;

	// Setup the touch and think functions.
	SetTouch( &CTFRpgRocket::RocketTouch );
	SetThink( &CTFRpgRocket::IgniteThink );
	SetNextThink( gpGlobals->curtime /*+ 0.4f*/ );

	// Don't collide with players on the owner's team for the first bit of our life
	m_flCollideWithTeammatesTime = gpGlobals->curtime + 0.25;
	m_bCollideWithTeammates = TFGameRules()->IsMPFriendlyFire() ? true : false;
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRpgRocket::UpdateOnRemove( void )
{
	StopSound( "Weapon_RPG.RocketIgnite" );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFRpgRocket::GetRadius( void )
{
	return BaseClass::GetRadius();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CTFRpgRocket::RocketTouch( CBaseEntity *pOther )
{
	// Verify a correct "other."
	Assert( pOther );
	if ( pOther->IsSolidFlagSet( FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS ) )
		return;

	// Handle hitting skybox (disappear).
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	if ( pTrace->surface.flags & SURF_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	if ( ( pTrace->contents & CONTENTS_WATER ) )
	{
		// Splash!
		CEffectData data;
		data.m_fFlags = 0;
		data.m_vOrigin = pTrace->endpos;
		data.m_vNormal = Vector( 0, 0, 1 );
		data.m_flScale = 8.0f;

		DispatchEffect( "watersplash", data );
	}

	StopSound( "Weapon_RPG.RocketIgnite" );

	trace_t trace;
	memcpy( &trace, pTrace, sizeof( trace_t ) );
	Explode( &trace, pOther );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRpgRocket::IgniteThink( void )
{
	AddEffects( EF_DIMLIGHT );

	//EmitSound( "Weapon_RPG.RocketIgnite" );

	SetThink( &CTFRpgRocket::SeekThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	CBroadcastRecipientFilter filter;
	TE_BeamFollow(filter, 0.0,
		entindex(),
		m_iTrail,
		0,
		4,
		5,
		5,
		0,
		224,
		224,
		255,
		255);

	m_flIgniteTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRpgRocket::SeekThink( void )
{
	if ( gpGlobals->curtime > m_flCollideWithTeammatesTime && m_bCollideWithTeammates == false )
		m_bCollideWithTeammates = true;

	Vector vecTarget;
	Vector vecFwd;
	Vector vecDir;
	trace_t tr;

	AngleVectors(GetAbsAngles(), &vecFwd);

	vecTarget = vecFwd;

	QAngle vecAng;
	VectorAngles(vecTarget, vecAng);
	SetAbsAngles(vecAng);

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = GetAbsVelocity().Length();
	if (gpGlobals->curtime - m_flIgniteTime < 1.0)
	{
		SetAbsVelocity(GetAbsVelocity() * 0.2 + vecTarget * (flSpeed * 0.8 + 400));
		if (GetWaterLevel() == 3)
		{
			// go slow underwater
			if (GetAbsVelocity().Length() > 300)
			{
				Vector vecVel = GetAbsVelocity();
				VectorNormalize(vecVel);
				SetAbsVelocity(vecVel * 300);
			}

			UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1, GetAbsOrigin(), 4);
		}
		else
		{
			if (GetAbsVelocity().Length() > 2000)
			{
				Vector vecVel = GetAbsVelocity();
				VectorNormalize(vecVel);
				SetAbsVelocity(vecVel * 2000);
			}
		}
	}
	else
	{
		if (IsEffectActive(EF_DIMLIGHT))
		{
			ClearEffects();
		}

		SetAbsVelocity(GetAbsVelocity() * 0.2 + vecTarget * flSpeed * 0.798);
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CTFRpgRocket::Explode( trace_t *pTrace, CBaseEntity *pOther )
{
	StopSound( "Weapon_RPG.RocketIgnite" );
	BaseClass::Explode( pTrace, pOther );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : &vecOrigin - 
//			&vecAngles - 
//			NULL - 
//
// Output : CTFRpgRocket
//-----------------------------------------------------------------------------
CTFRpgRocket *CTFRpgRocket::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner )
{
	CTFRpgRocket *pRocket = static_cast<CTFRpgRocket*>( CBaseEntity::CreateNoSpawn( "tf_rpg_rocket", vecOrigin, vecAngles, pOwner ) );
	if ( !pRocket )
		return NULL;

	// Set firing weapon.
	pRocket->SetLauncher( pWeapon );

	// Spawn.
	DispatchSpawn( pRocket );

	float flGravity = 0.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flGravity, mod_rocket_gravity );
	if ( flGravity )
	{
		pRocket->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
		pRocket->SetGravity( flGravity );
	}

	// Setup the initial velocity.
	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

	float flSpeed = pRocket->GetRocketSpeed();
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flSpeed, mult_projectile_speed );

	Vector vecVelocity = vecForward * flSpeed;
	pRocket->SetAbsVelocity( vecVelocity );
	pRocket->SetupInitialTransmittedGrenadeVelocity( vecVelocity );

	// Setup the initial angles.
	QAngle angles;
	VectorAngles( vecVelocity, angles );
	pRocket->SetAbsAngles( angles );

	// Set team.
	pRocket->ChangeTeam( pOwner->GetTeamNumber() );

	return pRocket;
}
#endif

//=============================================================================
//
//=============================================================================
IMPLEMENT_NETWORKCLASS_ALIASED( TFRpgIC, DT_TFRpgIC )

BEGIN_NETWORK_TABLE( CTFRpgIC, DT_TFRpgIC )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_ic_rocket, CTFRpgIC );

CTFRpgIC::CTFRpgIC()
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CTFRpgIC::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CTFRpgIC::Spawn( void )
{
	BaseClass::Spawn();

#ifdef GAME_DLL
	m_flDamage = 60;
#endif
}

#ifdef GAME_DLL
void CTFRpgIC::Explode( trace_t *pTrace, CBaseEntity *pOther )
{
	BaseClass::Explode( pTrace, pOther );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : &vecOrigin - 
//			&vecAngles - 
//			NULL - 
//
// Output : CTFRpgIC
//-----------------------------------------------------------------------------
CTFRpgIC *CTFRpgIC::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner )
{
	CTFRpgIC *pRocket = static_cast<CTFRpgIC*>( CBaseEntity::CreateNoSpawn( "tf_ic_rocket", vecOrigin, vecAngles, pOwner ) );
	if ( !pRocket )
		return NULL;

	// Set firing weapon.
	pRocket->SetLauncher( pWeapon );

	// Spawn.
	DispatchSpawn( pRocket );

	float flGravity = 0.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flGravity, mod_rocket_gravity );
	if ( flGravity )
	{
		pRocket->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
		pRocket->SetGravity( flGravity );
	}

	// Setup the initial velocity.
	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

	float flSpeed = pRocket->GetRocketSpeed();
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flSpeed, mult_projectile_speed );

	Vector vecVelocity = vecForward * flSpeed;
	pRocket->SetAbsVelocity( vecVelocity );
	pRocket->SetupInitialTransmittedGrenadeVelocity( vecVelocity );

	// Setup the initial angles.
	QAngle angles;
	VectorAngles( vecVelocity, angles );
	pRocket->SetAbsAngles( angles );

	// Set team.
	pRocket->ChangeTeam( pOwner->GetTeamNumber() );

	return pRocket;
}
#endif