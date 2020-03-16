//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_passtime_gun.h"
#include "decals.h"
#include "in_buttons.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_tf_team.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_team.h"
#endif

#ifdef GAME_DLL
ConVar tf_passtime_throwspeed_demoman( "tf_passtime_throwspeed_demoman", "850", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_engineer( "tf_passtime_throwspeed_engineer", "850", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_heavy( "tf_passtime_throwspeed_heavy", "850", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_medic( "tf_passtime_throwspeed_medic", "900", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_pyro( "tf_passtime_throwspeed_pyro", "750", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_scout( "tf_passtime_throwspeed_scout", "700", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_sniper( "tf_passtime_throwspeed_sniper", "900", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_soldier( "tf_passtime_throwspeed_soldier", "800", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_throwspeed_spy( "tf_passtime_throwspeed_spy", "900", FCVAR_NOTIFY | FCVAR_REPLICATED );
#endif

//=============================================================================
//
// Weapon Flag tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( PasstimeGun, DT_PasstimeGun )

BEGIN_NETWORK_TABLE( CPasstimeGun, DT_PasstimeGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CPasstimeGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_passtime_gun, CPasstimeGun );
PRECACHE_WEAPON_REGISTER( tf_weapon_passtime_gun );

//=============================================================================
//
// Weapon Flag functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CPasstimeGun::CPasstimeGun()
{

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CPasstimeGun::~CPasstimeGun()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeGun::Precache( void )
{
	PrecacheScriptSound( "Passtime.Throw" );
	PrecacheScriptSound( "Passtime.TargetLock" );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CPasstimeGun::Deploy( void )
{
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeGun::SecondaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
	{
		pOwner->m_afButtonPressed &= ~IN_ATTACK;
	}

	BaseClass::SecondaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeGun::ItemPostFrame( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !CanAttack() )
		return;

	if ( pOwner->m_nButtons & IN_ATTACK )
	{
		CalcLaunch( pOwner, true );

		if ( pOwner->m_nButtons & IN_ATTACK2 )
		{
			SecondaryAttack();
		}
	}

	if ( pOwner->m_afButtonReleased & IN_ATTACK )
 	{
 		Throw( pOwner );
 	}

	WeaponIdle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeGun::CalcLaunch( CTFPlayer *pOwner, bool bDunno )
{
	SendWeaponAnim( ACT_BALL_VM_THROW_LOOP );

	for ( int i = 0; i < GetGlobalTFTeam( pOwner->GetTeamNumber() )->GetNumPlayers(); i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( GetGlobalTFTeam( pOwner->GetTeamNumber() )->GetPlayer( i ) );
		if ( pPlayer )
		{
			if ( pPlayer->IsEffectActive( EF_NODRAW ) )
				continue;

			if ( !pPlayer->IsAlive() )
				continue;

			Vector vecToPlayer = pPlayer->WorldSpaceCenter() - pOwner->EyePosition();
			vecToPlayer.NormalizeInPlace();

			Vector vecFwd;
			AngleVectors( pOwner->EyeAngles(), &vecFwd );
			vecFwd.NormalizeInPlace();

			float flDistance = ( pOwner->GetAbsOrigin() - pPlayer->GetAbsOrigin() ).Length();

			if ( vecToPlayer.Dot( vecFwd ) > 0.975f || flDistance < 100.0f )
			{
#ifdef GAME_DLL
				CSingleUserRecipientFilter filter( pOwner );
				EmitSound_t params;
				params.m_flSoundTime = 0;
				params.m_pSoundName = "Passtime.TargetLock";
				EmitSound( filter, pOwner->entindex(), params );
#endif
				if ( pOwner->m_Shared.GetPasstimePassTarget() != pPlayer )
					pOwner->m_Shared.SetPasstimePassTarget( pPlayer );
			}
			else
			{
				pOwner->m_Shared.SetPasstimePassTarget( NULL );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeGun::Throw( CTFPlayer *pOwner )
{
/*#ifdef GAME_DLL
	CBaseEntity *pBall = pOwner->GetParent();
	if ( !pBall )
		return;

	if ( !pBall->ClassMatches( "passtime_ball" ) )
		return;

	AngularImpulse spin = AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );

	Vector vecForward, vecRight, vecUp;
	AngleVectors( pOwner->EyeAngles(), &vecForward, &vecRight, &vecUp );

	// Create grenades here!!
	// Set the starting position a bit behind the player so the projectile
	// launches out of the players view
	Vector vecSrc = pOwner->Weapon_ShootPosition();
	vecSrc +=  vecForward * -32.0f + vecRight * 8.0f + vecUp * -6.0f;

	float flThrowSpeed = tf_passtime_throwspeed_soldier.GetFloat();
	switch ( pOwner->GetDesiredPlayerClassIndex() )
	{
		default:
			flThrowSpeed = tf_passtime_throwspeed_soldier.GetFloat();
		case TF_CLASS_ENGINEER:
			flThrowSpeed = tf_passtime_throwspeed_engineer.GetInt();
			break;
		case TF_CLASS_SPY:
			flThrowSpeed = tf_passtime_throwspeed_spy.GetInt();
			break;
		case TF_CLASS_PYRO:
			flThrowSpeed = tf_passtime_throwspeed_pyro.GetInt();
			break;
		case TF_CLASS_HEAVYWEAPONS:
			flThrowSpeed = tf_passtime_throwspeed_heavy.GetInt();
			break;
		case TF_CLASS_MEDIC:
			flThrowSpeed = tf_passtime_throwspeed_medic.GetInt();
			break;
		case TF_CLASS_DEMOMAN:
			flThrowSpeed = tf_passtime_throwspeed_demoman.GetInt();
			break;
		case TF_CLASS_SOLDIER:
			flThrowSpeed = tf_passtime_throwspeed_soldier.GetInt();
			break;
		case TF_CLASS_SNIPER:
			flThrowSpeed = tf_passtime_throwspeed_sniper.GetInt();
			break;
		case TF_CLASS_SCOUT:
			flThrowSpeed = tf_passtime_throwspeed_scout.GetInt();
			break;
	}

	Vector vecVelocity = ( vecForward * flThrowSpeed ) + ( vecUp * 200.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +		
		( random->RandomFloat( -10.0f, 10.0f ) * vecUp );

	pBall->ChangeTeam( pOwner->GetTeamNumber() );

	IPhysicsObject *pPhysicsObject = pBall->VPhysicsGetObject();
	if ( pPhysicsObject )
		pPhysicsObject->AddVelocity( &vecVelocity, &spin );
#endif*/

	SendWeaponAnim( ACT_BALL_VM_THROW_END );
	pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
	EmitSound( "Passtime.Throw" );

	UTIL_Remove( this );
	pOwner->SwitchToNextBestWeapon( NULL );
}
