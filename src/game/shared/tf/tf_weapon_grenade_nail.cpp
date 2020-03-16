//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Nail Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_nail.h"
#include "tf_gamerules.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#include "tf_projectile_nail.h"
#include "tf_projectile_tfc_nail.h"
#include "physics_saverestore.h"
#include "phys_controller.h"
#endif

#define GRENADE_NAIL_TIMER	3.0f //Seconds

#define GRENADE_MODEL "models/tfc/ngrenade.mdl"
#define GRENADE_SOUND "Weapon_Grenade_TFC.Timer"

//=============================================================================
//
// TF Nail Grenade Projectile functions (Server specific).
//
#ifdef GAME_DLL

BEGIN_DATADESC( CTFGrenadeNailProjectile )
	DEFINE_THINKFUNC( EmitNails ),
	DEFINE_EMBEDDED( m_GrenadeController ),
	DEFINE_PHYSPTR( m_pMotionController ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_nail, CTFGrenadeNailProjectile );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_nail );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeNailProjectile* CTFGrenadeNailProjectile::Create( const Vector &position, const QAngle &angles, 
																const Vector &velocity, const AngularImpulse &angVelocity, 
																CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags )
{
	// Nail grenades are always thrown like discs
	QAngle vecCustomAngles = angles;
	vecCustomAngles.x = clamp( vecCustomAngles.x, -10,10 );
	vecCustomAngles.z = clamp( vecCustomAngles.x, -10,10 );
	Vector vecCustomAngVelocity = vec3_origin;
	vecCustomAngVelocity.z = RandomFloat( -600, 600 );
	CTFGrenadeNailProjectile *pGrenade = static_cast<CTFGrenadeNailProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_nail", position, vecCustomAngles, velocity, vecCustomAngVelocity, pOwner, pWeapon ) );
	if ( pGrenade )
	{
		pGrenade->InitGrenadeTFC( velocity, angVelocity, pOwner );
	}

	return pGrenade;
}

CTFGrenadeNailProjectile::~CTFGrenadeNailProjectile()
{
	if ( m_pMotionController != NULL )
	{
		physenv->DestroyMotionController( m_pMotionController );
		m_pMotionController = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNailProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );
	SetDetonateTimerLength( GRENADE_NAIL_TIMER );
	EmitSound( GRENADE_SOUND );

	m_pMotionController = NULL;

	UseClientSideAnimation();
		
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNailProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheScriptSound( GRENADE_SOUND );
	PrecacheScriptSound( "Weapon_Grenade_Nail.Launch" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNailProjectile::BounceSound( void )
{
	EmitSound( "Weapon_HandGrenade.GrenadeBounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNailProjectile::Detonate()
{
	if ( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	StartEmittingNails();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFGrenadeNailProjectile::OnTakeDamage( const CTakeDamageInfo &info )
{
	if ( m_pMotionController != NULL )
	{
		// motioncontroller is animating us, dont take hits that will disorient us
		return 0;
	}

	return BaseClass::OnTakeDamage( info );
}

void CTFGrenadeNailProjectile::StartEmittingNails( void )
{
	// 0.4 seconds later, emit nails
	if ( !TFGameRules()->IsTFCAllowed() )
	{
		IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

		if ( pPhysicsObject )
		{
			m_pMotionController = physenv->CreateMotionController( &m_GrenadeController );
			m_pMotionController->AttachObject( pPhysicsObject, true );

			pPhysicsObject->EnableGravity( false );

			pPhysicsObject->Wake();
		}

		QAngle ang(0,0,0);
		Vector pos = GetAbsOrigin();
		pos.z += 32;
		m_GrenadeController.SetDesiredPosAndOrientation( pos, ang );
	}
	else
	{
		SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );

		QAngle ang(0,0,0);
		Vector pos = GetAbsOrigin();
		pos.z += 32;

		Vector currentPos = GetAbsOrigin();
		QAngle currentAng = GetAbsAngles();

		Vector vecVel;
		AngularImpulse angVel;
		GetVelocity( &vecVel, &angVel );

		Vector linear;
		AngularImpulse angular;

		// not at the right height yet, keep moving up
		linear.z =  50 * ( pos.z - currentPos.z );

		if ( currentPos.z > pos.z )
		{
			// lock into position
			if ( vecVel.Length() > 1.0 )
			{
				AngularImpulse nil( 0,0,0 );
				SetAbsVelocity( vec3_origin );
				ApplyLocalAngularVelocityImpulse( nil );

				// For now teleport to the proper orientation
				currentAng.x = 0;
				currentAng.y = 0;
				currentAng.z = 0;
				SetAbsOrigin( currentPos );
				SetAbsAngles( currentAng );
			}
		}

		// Start rotating in the right direction
		// we'll lock angles once we reach proper height to stop the oscillating
		Vector m_worldGoalAxis(0,0,1);

		// Get the alignment axis in object space
		Vector currentLocalTargetAxis;
		VectorIRotate( m_worldGoalAxis, EntityToWorldTransform(), currentLocalTargetAxis );

		float invDeltaTime = (1/1);
		float m_angularLimit = 10;

		angular = ComputeRotSpeedToAlignAxes( m_worldGoalAxis, currentLocalTargetAxis, angVel, 1.0, invDeltaTime * invDeltaTime, m_angularLimit * invDeltaTime );
	}

	m_flNailAngle = 0;
	m_iNumNailBurstsLeft = 40;

	int animDesired = SelectWeightedSequence( ACT_RANGE_ATTACK1 );
	ResetSequence( animDesired );
	SetPlaybackRate( 1.0 );

	Vector soundPosition = GetAbsOrigin() + Vector( 0, 0, 5 );
	CPASAttenuationFilter filter( soundPosition );
	EmitSound( filter, entindex(), "Weapon_Grenade_Nail.Launch" );

#ifdef GAME_DLL
	SetThink( &CTFGrenadeNailProjectile::EmitNails );
	SetNextThink( gpGlobals->curtime + 0.4 );
#endif
}

void CTFGrenadeNailProjectile::EmitNails( void )
{
	m_iNumNailBurstsLeft--;

	if ( m_iNumNailBurstsLeft < 0 )
	{
		BaseClass::Detonate();
		return;
	}

	Vector forward, up;
	float flAngleToAdd = random->RandomFloat( 30, 40 );

	CBaseEntity *pAttacker = GetThrower();
	if ( !pAttacker )
		pAttacker = GetContainingEntity( INDEXENT(0) );

	// else release some nails
	for ( int i=0; i < 4 ;i++ )
	{
		m_flNailAngle = UTIL_AngleMod( m_flNailAngle + flAngleToAdd );

		QAngle angNail( random->RandomFloat( -3, 3 ), m_flNailAngle, 0 );

		// Emit a nail
		if ( TFGameRules()->IsTFCAllowed() )
		{
			CTFNailgunNail *pNail = CTFNailgunNail::CreateNail( GetAbsOrigin(), angNail, pAttacker, this, false );
			if ( pNail )
			{
				pNail->SetDamage( 18 );
			}
		}
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

IMotionEvent::simresult_e CNailGrenadeController::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	// Try to get to m_vecDesiredPosition

	// Try to orient ourselves to m_angDesiredOrientation

	Vector currentPos;
	QAngle currentAng;

	pObject->GetPosition( &currentPos, &currentAng );

	Vector vecVel;
	AngularImpulse angVel;
	pObject->GetVelocity( &vecVel, &angVel );

	linear.Init();
	angular.Init();

	if ( m_bReachedPos )
	{
		// Lock at this height
		if ( vecVel.Length() > 1.0 )
		{
			AngularImpulse nil( 0,0,0 );
			pObject->SetVelocityInstantaneous( &vec3_origin, &nil );

			// For now teleport to the proper orientation
			currentAng.x = 0;
			currentAng.y = 0;
			currentAng.z = 0;
			pObject->SetPosition( currentPos, currentAng, true );
		}
	}
	else
	{
		// not at the right height yet, keep moving up
		linear.z =  50 * ( m_vecDesiredPosition.z - currentPos.z );

		if ( currentPos.z > m_vecDesiredPosition.z )
		{
			// lock into position
			m_bReachedPos = true;
		}

		// Start rotating in the right direction
		// we'll lock angles once we reach proper height to stop the oscillating
		matrix3x4_t matrix;
		// get the object's local to world transform
		pObject->GetPositionMatrix( &matrix );

		Vector m_worldGoalAxis(0,0,1);

		// Get the alignment axis in object space
		Vector currentLocalTargetAxis;
		VectorIRotate( m_worldGoalAxis, matrix, currentLocalTargetAxis );

		float invDeltaTime = (1/deltaTime);
		float m_angularLimit = 10;

		angular = ComputeRotSpeedToAlignAxes( m_worldGoalAxis, currentLocalTargetAxis, angVel, 1.0, invDeltaTime * invDeltaTime, m_angularLimit * invDeltaTime );
	}

	return SIM_GLOBAL_ACCELERATION;
}

void CNailGrenadeController::SetDesiredPosAndOrientation( Vector pos, QAngle orientation )
{
	m_vecDesiredPosition = pos;
	m_angDesiredOrientation = orientation;

	m_bReachedPos = false;
	m_bReachedOrientation = false;
}

BEGIN_SIMPLE_DATADESC( CNailGrenadeController )
END_DATADESC()

#endif
