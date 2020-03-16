//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_pistol.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"
#include "tf_gamerules.h"
#include "baseobject_shared.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_obj_sentrygun.h"
// Server specific.
#else
#include "tf_player.h"
#include "ai_basenpc.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#include "sceneentity.h"
#endif
#ifdef GAME_DLL
extern ConVar lfe_airblast_physics_force;
#endif

//=============================================================================
//
// Weapon Pistol tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFPistol, tf_weapon_pistol )

//============================

CREATE_SIMPLE_WEAPON_TABLE( TFPistol_Scout, tf_weapon_pistol_scout )

CREATE_SIMPLE_WEAPON_TABLE( TFPistol_ScoutSecondary, tf_weapon_handgun_scout_secondary )
CREATE_SIMPLE_WEAPON_TABLE( TFPistol_ScoutPrimary, tf_weapon_handgun_scout_primary )

//=============================================================================
//
// Weapon Pistol functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPistol::PrimaryAttack( void )
{
#if 0
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if( pOwner )
	{
		// Each time the player fires the pistol, reset the view punch. This prevents
		// the aim from 'drifting off' when the player fires very quickly. This may
		// not be the ideal way to achieve this, but it's cheap and it works, which is
		// great for a feature we're evaluating. (sjb)
		//pOwner->ViewPunchReset();
	}
#endif

	if ( !CanAttack() )
		return;

	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPistol_ScoutPrimary::SecondaryAttack( void )
{
	if ( !CanAttack() )
		return;

	// Set the weapon mode.
	m_iWeaponMode = TF_WEAPON_SECONDARY_MODE;

	Push();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPistol_ScoutPrimary::Push( void )
{
	// Get the player owning the weapon.
	CTFPlayer *pOwner = ToTFPlayer( GetPlayerOwner() );
	if ( !pOwner )
		return;

	if ( m_flNextSecondaryAttack >= gpGlobals->curtime )
		return;

#ifdef GAME_DLL
	WeaponIdle();

	// Let the player remember the usercmd he fired a weapon on. Assists in making decisions about lag compensation.
	pOwner->NoteWeaponFired();
	pOwner->SpeakWeaponFire();
	CTF_GameStats.Event_PlayerFiredWeapon( pOwner, false );

	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pOwner, LAG_COMPENSATE_BOUNDS );
#endif

	pOwner->EmitSound( "Weapon_Hands.Push" );

	int iActivity = ACT_MP_PUSH_STAND_SECONDARY;
	if ( pOwner->GetFlags() & FL_DUCKING )
		iActivity = ACT_MP_PUSH_CROUCH_SECONDARY;
	else if ( pOwner->GetWaterLevel() > WL_Waist )
		iActivity = ACT_MP_PUSH_SWIM_SECONDARY;

	pOwner->DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, iActivity );
	SendWeaponAnim( ACT_SECONDARY_VM_ALTATTACK );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 1.25f;
	pOwner->m_nButtons &= ~IN_ATTACK2;

#ifdef GAME_DLL
	Vector vecDir, vecEnd;
	Vector vecStart = pOwner->Weapon_ShootPosition();
	pOwner->EyeVectors( &vecDir );
	vecEnd = vecStart + vecDir * 48.0;

	trace_t tr;
	CTraceFilterSimple filter( pOwner, COLLISION_GROUP_NONE );
	UTIL_TraceHull( vecStart, vecEnd, Vector( 0, 0, 0 ), Vector( 32, 32, 64 ), MASK_SOLID, &filter, &tr );

	QAngle angPushDir = pOwner->EyeAngles();
	angPushDir[PITCH] = min( -45, angPushDir[PITCH] );
	AngleVectors( angPushDir, &vecDir );

	CBaseEntity *pEntity = tr.m_pEnt;
	if ( !pEntity )
		return;

	if ( pEntity == pOwner )
		return;

	if ( pEntity->InSameTeam( pOwner ) )
		return;

	if ( pEntity->IsPlayer() )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( pEntity );
		if ( pTFPlayer && pTFPlayer->IsAlive() )
		{
			float flPushbackScale = 425.0f;
			// Push enemy
			pTFPlayer->SetGroundEntity( NULL );
			pTFPlayer->SetAbsVelocity( vecDir * flPushbackScale );
			pTFPlayer->EmitSound( "Weapon_Hands.PushImpact" );
			pTFPlayer->SetAirblastState( true );

			CTakeDamageInfo info( pOwner, pOwner, 1.0, DMG_GENERIC );
			pTFPlayer->TakeDamage( info );
		}
	}
	else if ( pEntity->IsNPC() )
	{
		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pEntity );
		if ( pNPC && pNPC->IsAlive() && pNPC->AllowPlayerForces() )
		{
			float flPushbackScale = 425.0f;
			// Push enemy
			pNPC->SetGroundEntity( NULL );
			pNPC->SetAbsVelocity( vecDir * flPushbackScale );
			pNPC->EmitSound( "Weapon_Hands.PushImpact" );
			pNPC->AddCond( TF_COND_KNOCKED_INTO_AIR );

			if ( pNPC->m_takedamage == DAMAGE_YES )
			{
				CTakeDamageInfo info( pOwner, pOwner, 1.0, DMG_GENERIC );
				pNPC->TakeDamage( info );
			}
		}
	}
	else if ( pEntity->VPhysicsGetObject() )
	{
		IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
		if ( pPhys )
		{
			float flPushbackScale = lfe_airblast_physics_force.GetFloat();

			float massFactor = RemapVal( pPhys->GetMass(), 0.5, 15, 0.5, 4 );
			vecDir *= flPushbackScale * massFactor;

			pPhys->Wake();
			pPhys->ApplyForceCenter( vecDir );
			AngularImpulse aVel = RandomAngularImpulse( -10, 10 ) * massFactor;
			pPhys->ApplyTorqueCenter( aVel );

			pEntity->EmitSound( "Weapon_Hands.PushImpact" );

			if ( pEntity->m_takedamage == DAMAGE_YES )
			{
				CTakeDamageInfo info( pOwner, pOwner, 1.0, DMG_GENERIC );
				pEntity->TakeDamage( info );
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPistol_ScoutPrimary::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "Weapon_Hands.Push" );
	PrecacheScriptSound( "Weapon_Hands.PushImpact" );
}