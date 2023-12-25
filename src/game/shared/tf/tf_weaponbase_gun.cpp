//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: Weapon Base Gun 
//
//=============================================================================

#include "cbase.h"
#include "tf_weaponbase_gun.h"
#include "tf_fx_shared.h"
#include "effect_dispatch_data.h"
#include "takedamageinfo.h"
#include "tf_projectile_nail.h"
#include "tf_projectile_arrow.h"
#include "in_buttons.h"
#include "tf_weapon_jar.h"
#include "tf_weapon_jar_gas.h"
#include "tf_projectile_energy_ball.h"
#include "tf_projectile_energy_ring.h"
#include "tf_projectile_tfc_nail.h"
#include "tf_projectile_tfc_rocket.h"
#if !defined( CLIENT_DLL )	// Server specific.

	#include "tf_gamestats.h"
	#include "tf_player.h"
	#include "tf_fx.h"
	#include "te_effect_dispatch.h"

	#include "tf_projectile_rocket.h"
	#include "tf_weapon_grenade_pipebomb.h"
	#include "tf_projectile_flare.h"
	#include "tf_projectile_dragons_fury.h"
	#include "tf_weapon_grenade_mirv.h"
	#include "te.h"
	#include "soundent.h"
	#include "grenade_frag.h"
	#include "prop_combine_ball.h"
	#include "grenade_ar2.h"
	#include "grenade_spit.h"
	#include "weapon_rpg.h"
	#include "weapon_crossbow.h"
	#include "hl1_basegrenade.h"
	#include "hl1mp_weapon_crossbow.h"
	#include "hl1mp_weapon_rpg.h"

#else	// Client specific.
    #include "c_tf_player.h"
	#include "c_te_effect_dispatch.h"
#endif

//=============================================================================
//
// TFWeaponBase Gun tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFWeaponBaseGun, DT_TFWeaponBaseGun )

BEGIN_NETWORK_TABLE( CTFWeaponBaseGun, DT_TFWeaponBaseGun )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iBurstSize ) ),
	RecvPropInt( RECVINFO( m_iProjectileType ) ),
#else
	SendPropInt( SENDINFO( m_iBurstSize ), -1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iProjectileType ), -1, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFWeaponBaseGun )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_iBurstSize, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

// Server specific.
#if !defined( CLIENT_DLL ) 
BEGIN_DATADESC( CTFWeaponBaseGun )
	DEFINE_THINKFUNC( ZoomOutIn ),
	DEFINE_THINKFUNC( ZoomOut ),
	DEFINE_THINKFUNC( ZoomIn ),
END_DATADESC()
#endif

//=============================================================================
//
// TFWeaponBase Gun functions.
//

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFWeaponBaseGun::CTFWeaponBaseGun()
{
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::ItemPostFrame( void )
{
	int iOldBurstSize = m_iBurstSize;
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
	{
		if ( m_iBurstSize > 0 )
		{
			// Fake the fire button.
			pOwner->m_nButtons |= IN_ATTACK;
		}
	}

	BaseClass::ItemPostFrame();

	// Stop burst if we run out of ammo.
	if ( ( UsesClipsForAmmo1() && m_iClip1 <= 0 ) ||
		( !UsesClipsForAmmo1() && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) ) 
	{
		m_iBurstSize = 0;
	}

	if ( iOldBurstSize > 0 && m_iBurstSize == 0 )
	{
		// Delay the next burst.
		m_flNextPrimaryAttack = gpGlobals->curtime + m_pWeaponInfo->GetWeaponData( TF_WEAPON_PRIMARY_MODE ).m_flBurstDelay;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::PrimaryAttack( void )
{
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );

	// Check for ammunition.
	if ( IsEnergyWeapon() )
	{
		if ( ( m_flEnergy < Energy_GetShotCost() ) )
			return;
	}
	else
	{
		if ( ( m_iClip1 < GetAmmoPerShot() ) && UsesClipsForAmmo1() )
			return;
	}

	if ( pPlayer && !IsEnergyWeapon() )
	{
		// Check for ammunition for single shot/attributes.
		if ( !UsesClipsForAmmo1() && ( m_iWeaponMode == TF_WEAPON_PRIMARY_MODE ) )
		{
			if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) < GetAmmoPerShot() )
				return;
		}
		else if ( !UsesClipsForAmmo2() && ( m_iWeaponMode == TF_WEAPON_SECONDARY_MODE ) )
		{
			if ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) < GetAmmoPerShot() )
				return;
		}
	}

	if ( !CanAttack() )
		return;

	if ( m_pWeaponInfo->GetWeaponData( TF_WEAPON_PRIMARY_MODE ).m_nBurstSize > 0 && m_iBurstSize == 0 )
	{
		// Start the burst.
		m_iBurstSize = m_pWeaponInfo->GetWeaponData( TF_WEAPON_PRIMARY_MODE ).m_nBurstSize;
	}

	if ( m_iBurstSize > 0 )
	{
		m_iBurstSize--;
	}

	CalcIsAttackCritical();
	// Get the player owning the weapon.
	if ( pPlayer )
	{
#ifdef GAME_DLL
		if ( ShouldRemoveInvisibilityOnPrimaryAttack() )
			pPlayer->RemoveInvisibility();

		if ( ShouldRemoveDisguiseOnPrimaryAttack() )
			pPlayer->RemoveDisguise();

		// Minigun has custom handling
		if ( GetWeaponID() != TF_WEAPON_MINIGUN )
		{
			pPlayer->SpeakWeaponFire();
		}
		CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
#endif
	}

	// Set the weapon mode.
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

#if GAME_DLL
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON );
#endif

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	if ( pPlayer )
	{
		pPlayer->SetAnimation( PLAYER_ATTACK1 );
		FireProjectile( pPlayer );
	}

	m_flLastFireTime = gpGlobals->curtime;

	// Set next attack times.
	float flFireDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
	CALL_ATTRIB_HOOK_FLOAT( flFireDelay, mult_postfiredelay );

	if ( pPlayer )
	{
		if ( pPlayer->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
			flFireDelay /= 2;

		if ( pPlayer->m_Shared.InCond( TF_COND_HALLOWEEN_SPEED_BOOST ) )
			flFireDelay /= 1.5;

		if ( pPlayer->m_Shared.InCond( TF_COND_BLASTJUMPING ) )
			CALL_ATTRIB_HOOK_FLOAT( flFireDelay, rocketjump_attackrate_bonus );
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + flFireDelay;

	// Don't push out secondary attack, because our secondary fire
	// systems are all separate from primary fire (sniper zooming, demoman pipebomb detonating, etc)
	//m_flNextSecondaryAttack = gpGlobals->curtime + m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;

	// Set the idle animation times based on the sequence duration, so that we play full fire animations
	// that last longer than the refire rate may allow.
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	AbortReload();
}	

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::SecondaryAttack( void )
{
	// semi-auto behaviour
	if ( m_bInAttack2 )
		return;

	// Get the player owning the weapon.
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

	pPlayer->DoClassSpecialSkill();

	m_bInAttack2 = true;

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
}

CBaseEntity *CTFWeaponBaseGun::FireProjectile( CTFPlayer *pPlayer )
{
	int iProjectile = GetWeaponProjectileType();

	CBaseEntity *pProjectile = NULL;

	string_t strProjectileEntityName = NULL_STRING;
	CALL_ATTRIB_HOOK_STRING( strProjectileEntityName, projectile_entity_name );
	if ( strProjectileEntityName != NULL_STRING )
	{
		pProjectile = CreateEntityByName( STRING( strProjectileEntityName ) );
		if ( pProjectile )
		{
			Vector vecSrc;
			QAngle angForward;
			Vector vecOffset( 23.5f, 12.0f, -3.0f );
			if ( pPlayer->GetFlags() & FL_DUCKING )
				vecOffset.z = 8.0f;

			GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );

			pProjectile->SetAbsOrigin( vecSrc );
			pProjectile->SetOwnerEntity( pPlayer );
			pProjectile->SetAbsAngles( angForward );
#ifdef GAME_DLL
			DispatchSpawn( pProjectile );
#endif
		}
	}
	else
	{
	switch( iProjectile )
	{
	case TF_PROJECTILE_BULLET:
		FireBullet( pPlayer );
		break;

	case TF_PROJECTILE_ROCKET:
	case TF_PROJECTILE_ENERGY_RING:
	case LFE_TFC_PROJECTILE_ROCKET:
	case LFE_TFC_PROJECTILE_IC:
		pProjectile = FireRocket( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_ENERGY_BALL:
		pProjectile = FireEnergyBall( pPlayer, m_bInAttack2 );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_SYRINGE:
		pProjectile = FireNail( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_PIPEBOMB:
	case TF_PROJECTILE_CANNONBALL:
		pProjectile = FirePipeBomb( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_PIPEBOMB_REMOTE:
	case TF_PROJECTILE_PIPEBOMB_REMOTE_PRACTICE:
		pProjectile = FirePipeBomb( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_FLARE:
		pProjectile = FireFlare( pPlayer );
		pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);
		break;

	case TF_PROJECTILE_JAR:
	case TF_PROJECTILE_JAR_MILK:
	case TF_PROJECTILE_FESTITIVE_URINE:
	case TF_PROJECTILE_BREADMONSTER_JARATE:
	case TF_PROJECTILE_BREADMONSTER_MADMILK:
	case TF_PROJECTILE_JAR_GAS:
		pProjectile = FireJar( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_CLEAVER:
	case TF_PROJECTILE_THROWABLE:
		pProjectile = FireJar( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_ARROW:
		pProjectile = FireArrow( pPlayer, TF_PROJECTILETYPE_ARROW );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;
	case TF_PROJECTILE_HEALING_BOLT:
		pProjectile = FireArrow( pPlayer, TF_PROJECTILETYPE_HEALING_BOLT );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;
	case TF_PROJECTILE_BUILDING_REPAIR_BOLT:
		pProjectile = FireArrow( pPlayer, TF_PROJECTILETYPE_BUILDING_BOLT );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;
	case TF_PROJECTILE_FESTITIVE_ARROW:
		pProjectile = FireArrow( pPlayer, TF_PROJECTILETYPE_ARROW_FESTIVE );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;
	case TF_PROJECTILE_FESTITIVE_HEALING_BOLT:
		pProjectile = FireArrow( pPlayer, TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;
	case TF_PROJECTILE_GRAPPLINGHOOK:
		pProjectile = FireArrow( pPlayer, TF_PROJECTILETYPE_HOOK );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_HL2_PROJECTILE_FRAG:
	case LFE_HL1_PROJECTILE_GRENADE:
		pProjectile = FireHLGrenade( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_HL2_PROJECTILE_COMBINEBALL:
		pProjectile = FireHLCombineBall( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_HL2_PROJECTILE_AR2:
		pProjectile = FireHLAR2Grenade( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_HL2_PROJECTILE_SPIT:
		pProjectile = FireHLSpit( pPlayer );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_HL2_PROJECTILE_CROSSBOW_BOLT:
	case LFE_HL1_PROJECTILE_CROSSBOW_BOLT:
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_HL2_PROJECTILE_RPG_MISSILE:
	case LFE_HL1_PROJECTILE_RPG_ROCKET:
		pProjectile = FireHLMissile( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_HL1_PROJECTILE_HORNET:
		pProjectile = FireHLHornet( pPlayer );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case LFE_TFC_PROJECTILE_NAIL:
	case LFE_TFC_PROJECTILE_NAIL_SUPER:
	case LFE_TFC_PROJECTILE_NAIL_TRANQ:
	case LFE_TFC_PROJECTILE_NAIL_RAILGUN:
		pProjectile = FireTFCNail( pPlayer, iProjectile );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		break;

	case TF_PROJECTILE_NONE:
	default:
		// do nothing!
		DevMsg( "Weapon does not have a projectile type set\n" );
		break;
	}
	}

	RemoveAmmo( pPlayer );

#ifdef GAME_DLL
	if ( pProjectile )
	{
		CAttribute_String strCustomMdl;
		if ( GetCustomProjectileModel( &strCustomMdl ) )
		{
			pProjectile->SetModel( strCustomMdl );
		}
	}
#endif

	DoFireEffects();

	UpdatePunchAngles( pPlayer );

	return pProjectile;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::UpdatePunchAngles( CTFPlayer *pPlayer )
{
	// Update the player's punch angle.
	int flPunchAngleMult = 4;
	CALL_ATTRIB_HOOK_FLOAT(flPunchAngleMult, viewpunch_mult);
	QAngle angle = pPlayer->GetPunchAngle();
	float flPunchAngle = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flPunchAngle;

	if ( flPunchAngle > 0 )
	{
		angle.x -= SharedRandomInt( "ShotgunPunchAngle", ( flPunchAngle - 1 ), ( flPunchAngle + 1 ) );
		pPlayer->SetPunchAngle( angle * flPunchAngleMult );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fire a bullet!
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::FireBullet( CTFPlayer *pPlayer )
{
	PlayWeaponShootSound();

	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->EyeAngles() + pPlayer->GetPunchAngle(),
		GetWeaponID(),
		m_iWeaponMode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetWeaponSpread(),
		GetProjectileDamage(),
		IsCurrentAttackACrit() );
}

//-----------------------------------------------------------------------------
// Purpose: Return angles for a projectile reflected by airblast
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::GetProjectileReflectSetup( CTFPlayer *pPlayer, const Vector &vecPos, Vector *vecDeflect, bool bHitTeammates /* = true */, bool bUseHitboxes /* = false */ )
{
	Vector vecForward, vecRight, vecUp;
	AngleVectors( pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp );

	Vector vecShootPos = pPlayer->Weapon_ShootPosition();

	// Estimate end point
	Vector endPos = vecShootPos + vecForward * 2000;

	// Trace forward and find what's in front of us, and aim at that
	trace_t tr;
	int nMask = bUseHitboxes ? MASK_SOLID | CONTENTS_HITBOX : MASK_SOLID;

	Ray_t ray;
	ray.Init( vecShootPos, endPos );
	if ( bHitTeammates )
	{
		CTraceFilterSimple filter( pPlayer, COLLISION_GROUP_NONE );
		UTIL_Portal_TraceRay( ray, nMask, &filter, &tr );
	}
	else
	{
		CTraceFilterIgnoreTeammates filter( pPlayer, COLLISION_GROUP_NONE, pPlayer->GetTeamNumber() );
		UTIL_Portal_TraceRay( ray, nMask, &filter, &tr );
	}

	// vecPos is projectile's current position. Use that to find angles.

	// Find angles that will get us to our desired end point
	// Only use the trace end if it wasn't too close, which results
	// in visually bizarre forward angles
	if ( tr.fraction > 0.1 || bUseHitboxes )
	{
		*vecDeflect = tr.endpos - vecPos;
	}
	else
	{
		*vecDeflect = endPos - vecPos;
	}

	VectorNormalize( *vecDeflect );
}

//-----------------------------------------------------------------------------
// Purpose: Return the origin & angles for a projectile fired from the player's gun
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::GetProjectileFireSetup( CTFPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward, bool bHitTeammates /* = true */, bool bUseHitboxes /* = false */ )
{
	Vector vecForward, vecRight, vecUp;
	AngleVectors( pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp );

	Vector vecShootPos = pPlayer->Weapon_ShootPosition();

	// Estimate end point
	Vector endPos = vecShootPos + vecForward * 2000;	

	// Trace forward and find what's in front of us, and aim at that
	trace_t tr;
	int nMask = bUseHitboxes ? MASK_SOLID | CONTENTS_HITBOX : MASK_SOLID;

	if ( bHitTeammates )
	{
		CTraceFilterSimple filter( pPlayer, COLLISION_GROUP_NONE );
		UTIL_TraceLine( vecShootPos, endPos, nMask, &filter, &tr );
	}
	else
	{
		CTraceFilterIgnoreTeammates filter( pPlayer, COLLISION_GROUP_NONE, pPlayer->GetTeamNumber() );
		UTIL_TraceLine( vecShootPos, endPos, nMask, &filter, &tr );
	}

#ifndef CLIENT_DLL
	// If viewmodel is flipped fire from the other side.
	if ( IsViewModelFlipped() )
		vecOffset.y *= -1.0f;

	int iCenterFire = TF_GL_MODE_REGULAR;
	CALL_ATTRIB_HOOK_INT( iCenterFire, centerfire_projectile );
	if ( iCenterFire )
		vecOffset.y = 0.0f;

	// Offset actual start point
	*vecSrc = vecShootPos + (vecForward * vecOffset.x) + (vecRight * vecOffset.y) + (vecUp * vecOffset.z);
#else
	// If we're seeing another player shooting the projectile, move their start point to the weapon origin
	if ( pPlayer )
	{
		if ( !UsingViewModel() )
		{
			GetAttachment( "muzzle", *vecSrc );
		}
		else
		{
			C_BaseEntity *pViewModel = pPlayer->GetViewModel();

			if ( pViewModel )
			{
				QAngle vecAngles;
				int iMuzzleFlashAttachment = pViewModel->LookupAttachment( "muzzle" );
				pViewModel->GetAttachment( iMuzzleFlashAttachment, *vecSrc, vecAngles );

				Vector vForward;
				AngleVectors( vecAngles, &vForward );

				trace_t trace;	
				UTIL_TraceLine( *vecSrc + vForward * -50, *vecSrc, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace );

				*vecSrc = trace.endpos;
			}
		}
	}
#endif

	// Find angles that will get us to our desired end point
	// Only use the trace end if it wasn't too close, which results
	// in visually bizarre forward angles
	if ( tr.fraction > 0.1 || bUseHitboxes )
	{
		VectorAngles( tr.endpos - *vecSrc, *angForward );
	}
	else
	{
		VectorAngles( endPos - *vecSrc, *angForward );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fire a rocket
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireRocket( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

	// Server only - create the rocket.
#ifdef GAME_DLL

	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( 23.5f, 12.0f, -3.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 8.0f;
	}

	bool bUseHitboxes = ( iType == TF_PROJECTILE_ARROW || iType == TF_PROJECTILE_FESTITIVE_ARROW );

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, bUseHitboxes );

	// Add attribute spread.
	float flSpread = 0;
	CALL_ATTRIB_HOOK_FLOAT( flSpread, projectile_spread_angle );
	if ( flSpread != 0)
	{
		angForward.x += RandomFloat(-flSpread, flSpread);
		angForward.y += RandomFloat(-flSpread, flSpread);
	}

	CTFBaseRocket *pProjectile = NULL;

	switch ( iType )
	{
	case TF_PROJECTILE_ROCKET:
		pProjectile = CTFProjectile_Rocket::Create( this, vecSrc, angForward, pPlayer, pPlayer );
		break;
	case TF_PROJECTILE_ENERGY_RING:
		pProjectile = CTFProjectile_EnergyRing::Create( this, vecSrc, angForward, pPlayer, pPlayer );
		break;
	case LFE_TFC_PROJECTILE_ROCKET:
		pProjectile = CTFRpgRocket::Create( this, vecSrc, angForward, pPlayer );
		break;
	case LFE_TFC_PROJECTILE_IC:
		pProjectile = CTFRpgRocket::Create( this, vecSrc, angForward, pPlayer );
		break;
	default:
		Assert( false );
		break;
	}

	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( GetProjectileDamage() );
	}

	return pProjectile;

#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Fire a projectile nail
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireNail( CTFPlayer *pPlayer, int iSpecificNail )
{
	PlayWeaponShootSound();

	Vector vecSrc;
	QAngle angForward;
	
	GetProjectileFireSetup( pPlayer, Vector(16,6,-8), &vecSrc, &angForward );

	float flSpread = 1.5;
	CALL_ATTRIB_HOOK_FLOAT(flSpread, projectile_spread_angle);


	CTFBaseProjectile *pProjectile = NULL;
	switch( iSpecificNail )
	{
	case TF_PROJECTILE_SYRINGE:
		GetProjectileFireSetup(pPlayer, Vector(16, 6, -8), &vecSrc, &angForward);
		angForward.x += RandomFloat(-flSpread, flSpread);
		angForward.y += RandomFloat(-flSpread, flSpread);
		pProjectile = CTFProjectile_Syringe::Create(vecSrc, angForward, pPlayer, pPlayer, IsCurrentAttackACrit());
		break;
	default:
		Assert(0);
	}

	if ( pProjectile )
	{
		pProjectile->SetWeaponID( GetWeaponID() );
		pProjectile->SetCritical( IsCurrentAttackACrit() );
#ifdef GAME_DLL
		pProjectile->SetDamage( GetProjectileDamage() );
#endif
	}
	
	return pProjectile;
}

//-----------------------------------------------------------------------------
// Purpose: Fire a  pipe bomb
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FirePipeBomb( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL
	int iMode = TF_GL_MODE_REGULAR, iNoSpin = 0;
	int iDetMode = TF_GL_MODE_REGULAR;
	CALL_ATTRIB_HOOK_INT( iMode, set_detonate_mode );
	CALL_ATTRIB_HOOK_INT(iDetMode, set_detonate_mode);

	if ((iType == TF_PROJECTILE_PIPEBOMB_REMOTE) || (iType == TF_PROJECTILE_PIPEBOMB_REMOTE_PRACTICE) || (iType == TF_PROJECTILE_PIPEBOMB) && iDetMode == TF_GL_MODE_REMOTE_DETONATE)
	{
		iMode = TF_GL_MODE_REMOTE_DETONATE;
	}

	Vector vecForward, vecRight, vecUp;
	AngleVectors( pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp );

	// Create grenades here!!
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	vecSrc +=  vecForward * 16.0f + vecRight * 8.0f + vecUp * -6.0f;
	
	Vector vecVelocity = ( vecForward * GetProjectileSpeed() ) + ( vecUp * 200.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +		
		( random->RandomFloat( -10.0f, 10.0f ) * vecUp );

	float flDamageMult = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flDamageMult, mult_dmg );
	
	// Special damage bonus case for stickybombs.
	if ( iMode == TF_GL_MODE_REMOTE_DETONATE )
	{
		float flDamageChargeMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamageChargeMult, stickybomb_charge_damage_increase );
		if ( flDamageChargeMult != 1.0f )
		{
			// If we're a stickybomb with this attribute, we need to calculate out the charge level.
			// Since we know GetProjectileSpeed(), we can use maths to get the charge level percent.
			float flSpeedMult = 1.0f;
			CALL_ATTRIB_HOOK_FLOAT( flSpeedMult, mult_projectile_range );
			float flProjectileSpeedDiffCurrent = GetProjectileSpeed() - ( TF_PIPEBOMB_MIN_CHARGE_VEL * flSpeedMult );
			float flProjectileSpeedDiffMax = ( TF_PIPEBOMB_MAX_CHARGE_VEL - TF_PIPEBOMB_MIN_CHARGE_VEL ) * flSpeedMult;
			float flChargePercent = flProjectileSpeedDiffCurrent / flProjectileSpeedDiffMax;
			
			// Calculate out our additional damage bonus from charging our stickybomb.
			flDamageMult *= ( ( flDamageChargeMult - 1 ) * flChargePercent ) + 1;
			
		}
	}

	CALL_ATTRIB_HOOK_INT ( iNoSpin, grenade_no_spin );

	AngularImpulse spin( 600, 0, 0 );

	if ( iNoSpin == 0 )
	{
		spin = AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );
	}

	CTFGrenadePipebombProjectile *pProjectile = CTFGrenadePipebombProjectile::Create( vecSrc, pPlayer->EyeAngles(), vecVelocity, spin, pPlayer, iMode, flDamageMult, this );
	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( GetProjectileDamage() );
	}
	return pProjectile;

#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Fire a flare
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireFlare( CTFPlayer *pPlayer )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL
	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( 23.5f, 12.0f, -3.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 8.0f;
	}
	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false );

	// Add attribute spread.
	float flSpread = 0;
	CALL_ATTRIB_HOOK_FLOAT( flSpread, projectile_spread_angle );
	if ( flSpread != 0)
	{
		angForward.x += RandomFloat(-flSpread, flSpread);
		angForward.y += RandomFloat(-flSpread, flSpread);
	}

	CTFProjectile_Flare *pProjectile = CTFProjectile_Flare::Create( this, vecSrc, angForward, pPlayer, pPlayer );
	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( GetProjectileDamage() );
	}
	return pProjectile;
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Fire an Arrow
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireArrow( CTFPlayer *pPlayer, ProjectileType_t eType )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL
	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( -16.0f, 10.0f, -6.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 4.0f;
	}

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, true );

	// Add attribute spread.
	float flSpread = 0;
	CALL_ATTRIB_HOOK_FLOAT( flSpread, projectile_spread_angle );
	if ( flSpread != 0)
	{
		angForward.x += RandomFloat(-flSpread, flSpread);
		angForward.y += RandomFloat(-flSpread, flSpread);
	}

	CTFProjectile_Arrow *pProjectile = CTFProjectile_Arrow::Create( vecSrc, angForward, GetProjectileSpeed(), GetProjectileGravity(), eType, pPlayer, this );
	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( GetProjectileDamage() );
	}
	return pProjectile;
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Fire some piss
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireJar( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL
	AngularImpulse spin = AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );

	Vector vecForward, vecRight, vecUp;
	AngleVectors( pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp );

	// Create grenades here!!
	// Set the starting position a bit behind the player so the projectile
	// launches out of the players view
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	vecSrc +=  vecForward * -32.0f + vecRight * 8.0f + vecUp * -6.0f;

	Vector vecVelocity = ( vecForward * GetProjectileSpeed() ) + ( vecUp * 200.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +		
		( random->RandomFloat( -10.0f, 10.0f ) * vecUp );

	//GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );

	CTFWeaponBaseGrenadeProj *pProjectile = NULL;

	switch ( iType )
	{
	case TF_PROJECTILE_JAR:
	case TF_PROJECTILE_FESTITIVE_URINE:
	case TF_PROJECTILE_BREADMONSTER_JARATE:
		pProjectile = CTFProjectile_Jar::Create( this, vecSrc, pPlayer->EyeAngles(), vecVelocity, pPlayer, pPlayer, spin, GetTFWpnData() );
		break;
	case TF_PROJECTILE_JAR_MILK:
	case TF_PROJECTILE_BREADMONSTER_MADMILK:
		pProjectile = CTFProjectile_JarMilk::Create( this, vecSrc, pPlayer->EyeAngles(), vecVelocity, pPlayer, pPlayer, spin, GetTFWpnData() );
		break;
	case TF_PROJECTILE_JAR_GAS:
		pProjectile = CTFProjectile_JarGas::Create( this, vecSrc, pPlayer->EyeAngles(), vecVelocity, pPlayer, pPlayer, spin, GetTFWpnData() );
		break;
	}

	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( GetProjectileDamage() );
	}
	return pProjectile;
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Use this for any old grenades: MIRV, Frag, etc
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireGrenade( CTFPlayer *pPlayer )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL

	Vector vecForward, vecRight, vecUp;
	AngleVectors( pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp );

	// Create grenades here!!
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	vecSrc += vecForward * 16.0f + vecRight * 8.0f + vecUp * -6.0f;

	Vector vecVelocity = ( vecForward * GetProjectileSpeed() ) + ( vecUp * 200.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +
		( random->RandomFloat(-10.0f, 10.0f) * vecUp );

	float flDamageMult = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT(flDamageMult, mult_dmg);
	if ( pPlayer->m_Shared.InCond( TF_COND_RUNE_STRENGTH ) )
		flDamageMult *= 2;

	char szEntName[256];
	V_snprintf( szEntName, sizeof(szEntName), "%s_projectile", WeaponIdToClassname( GetWeaponID() ) );

	CTFWeaponBaseGrenadeProj *pProjectile = CTFWeaponBaseGrenadeProj::Create( szEntName, vecSrc, pPlayer->EyeAngles(), vecVelocity,
		AngularImpulse( 600, random->RandomInt(-1200, 1200), 0 ),
		pPlayer, this /*GetTFWpnData(), 3.0f, flDamageMult*/ );

	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetLauncher( this );
	}
	return pProjectile;

#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Fire a rocket
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireEnergyBall( CTFPlayer *pPlayer, bool bCharged )
{
	PlayWeaponShootSound();

	// Server only - create the rocket.
#ifdef GAME_DLL
	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( 23.5f, 12.0f, -3.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 8.0f;
	}

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );

	CTFBaseRocket *pProjectile = CTFProjectile_EnergyBall::Create( this, vecSrc, angForward, pPlayer, pPlayer );
	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( GetProjectileDamage() );
	}

	return pProjectile;
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: test on tf weapons
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireHLGrenade( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL
	AngularImpulse spin = AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );

	Vector vecForward, vecRight, vecUp;
	AngleVectors( pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp );

	// Create grenades here!!
	// Set the starting position a bit behind the player so the projectile
	// launches out of the players view
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	vecSrc +=  vecForward * -32.0f + vecRight * 8.0f + vecUp * -6.0f;

	Vector vecVelocity = ( vecForward * GetProjectileSpeed() ) + ( vecUp * 200.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +		
		( random->RandomFloat( -10.0f, 10.0f ) * vecUp );

	if ( iType == LFE_HL1_PROJECTILE_GRENADE )
	{
		CHandGrenade *pGrenade = (CHandGrenade*)Create( "grenade_hand", vecVelocity, pPlayer->EyeAngles(), pPlayer );
		if ( pGrenade )
		{
			pGrenade->ShootTimed( pPlayer, vecVelocity, 3.5 );
		}
		return pGrenade;
	}
	else
	{
		return Fraggrenade_Create( vecSrc, vec3_angle, vecVelocity, spin, pPlayer, 3.0f, false );
	}
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireHLCombineBall( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

	// Server only - create the rocket.
#ifdef GAME_DLL

	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( 23.5f, 12.0f, -3.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 8.0f;
	}

	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	Vector vecVelocity = vecAiming * 1000.0f;

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );
	return CreateCombineBall( vecSrc, vecVelocity, 10.0f, 150.0f, 2.0f, pPlayer );
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireHLAR2Grenade( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL
	//pPlayer->RumbleEffect( RUMBLE_357, 0, RUMBLE_FLAGS_NONE );

	float flDamageMult = 100.0f;
	CALL_ATTRIB_HOOK_FLOAT( flDamageMult, mult_dmg );
	if ( pPlayer->m_Shared.InCond( TF_COND_RUNE_STRENGTH ) )
		flDamageMult *= 2;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors( pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow );
	VectorScale( vecThrow, 1000.0f, vecThrow );

	QAngle angles;
	VectorAngles( vecThrow, angles );
	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecSrc, angles, pPlayer );
	pGrenade->SetAbsVelocity( vecThrow );

	pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE ); 
	pGrenade->SetThrower( GetOwner() );
	pGrenade->SetDamage( flDamageMult );
	
	return pGrenade;
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireHLSpit( CTFPlayer *pPlayer )
{
	PlayWeaponShootSound();

#ifdef GAME_DLL
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors( pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow );
	VectorScale( vecThrow, 1000.0f, vecThrow );

	QAngle angles;
	VectorAngles( vecThrow, angles );
	CGrenadeSpit *pGrenade = (CGrenadeSpit*)Create( "grenade_spit", vecSrc, angles, pPlayer );
	pGrenade->SetThrower( GetOwner() );
	pGrenade->ApplyAbsVelocityImpulse( ( vecThrow + RandomVector( -0.035f, 0.035f ) ) /* flVelocity*/ );
	pGrenade->SetSpitSize( random->RandomInt( SPIT_SMALL, SPIT_MEDIUM ) );

	/*QAngle angles( random->RandomFloat( -250, -500 ),
				random->RandomFloat( -250, -500 ),
				random->RandomFloat( -250, -500 ) );*/
		
	//VectorAngles( vecVelocity, angles );
	pGrenade->SetLocalAngularVelocity( angles );

	return pGrenade;
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireHLBolt( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

	// Server only - create the rocket.
#ifdef GAME_DLL
	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( 23.5f, 12.0f, -3.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 8.0f;
	}

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );

	if ( iType == LFE_HL1_PROJECTILE_CROSSBOW_BOLT )
	{
		CCrossbowBoltHL1 *pBolt = CCrossbowBoltHL1::BoltCreate( vecSrc, angForward, pPlayer );
		if ( pBolt )
			pBolt->SetAbsVelocity( vecSrc * HL1_BOLT_AIR_VELOCITY );

		return pBolt;
	}
	else
	{
		CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate( vecSrc, angForward, pPlayer );
		if ( pBolt )
			pBolt->SetAbsVelocity( vecSrc * BOLT_AIR_VELOCITY );

		return pBolt;
	}
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireHLMissile( CTFPlayer *pPlayer, int iType )
{
	PlayWeaponShootSound();

	// Server only - create the rocket.
#ifdef GAME_DLL
	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( 23.5f, 12.0f, -3.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 8.0f;
	}

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );

	if ( iType == LFE_HL1_PROJECTILE_RPG_ROCKET )
	{
		CRpgRocket *pRocket = CRpgRocket::Create( vecSrc, angForward, pPlayer );
		/*if ( pRocket )
		{
			pRocket->m_hOwner = this;
			pRocket->SetAbsVelocity( pRocket->GetAbsVelocity() + vecSrc * DotProduct( pPlayer->GetAbsVelocity(), vecSrc ) );
		}*/
		return pRocket;
	}
	else
	{
		CMissile *pMissile = CMissile::Create( vecSrc, angForward, pPlayer->edict() );
		/*if ( pMissile )
		{
			pMissile->m_hOwner = this;
		}*/
		return pMissile;
	}

#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireHLHornet( CTFPlayer *pPlayer )
{
	PlayWeaponShootSound();

	// Server only - create the rocket.
#ifdef GAME_DLL
	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( 23.5f, 12.0f, -3.0f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 8.0f;
	}

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );

	CBaseEntity *pHornet = CBaseEntity::Create( "hornet", vecSrc, angForward, pPlayer );
	if ( pHornet )
	{
		pHornet->SetAbsVelocity( vecSrc * 300 );
		pHornet->ChangeTeam( pPlayer->GetTeamNumber() );
	}
	
	return pHornet;
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Fire a projectile nail
//-----------------------------------------------------------------------------
CBaseEntity *CTFWeaponBaseGun::FireTFCNail( CTFPlayer *pPlayer, int iSpecificNail )
{
	PlayWeaponShootSound();

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	QAngle angForward = pPlayer->EyeAngles();

	// Fire the nail
	CTFNailgunNail *pProjectile = NULL;
	switch( iSpecificNail )
	{
	case LFE_TFC_PROJECTILE_NAIL:
		pProjectile = CTFNailgunNail::CreateNail( vecSrc, angForward, pPlayer, this, IsCurrentAttackACrit() );
		break;
	case LFE_TFC_PROJECTILE_NAIL_SUPER:
		pProjectile = CTFNailgunNail::CreateSuperNail( vecSrc, angForward, pPlayer, this );
		break;
	case LFE_TFC_PROJECTILE_NAIL_TRANQ:
		pProjectile = CTFNailgunNail::CreateTranqNail( vecSrc, angForward, pPlayer, this );
		break;
	case LFE_TFC_PROJECTILE_NAIL_RAILGUN:
		pProjectile = CTFNailgunNail::CreateRailgunNail( vecSrc, angForward, pPlayer, this );
		break;
	default:
		break;
	}

	return pProjectile;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::PlayWeaponShootSound( void )
{
	if ( IsCurrentAttackACrit() && CanBeCritBoosted() )
	{
		WeaponSound( BURST );
	}
	else
	{
		WeaponSound( SINGLE );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFWeaponBaseGun::GetAmmoPerShot( void ) const
{
	int iAmmoPerShot = 0;
	CALL_ATTRIB_HOOK_INT( iAmmoPerShot, mod_ammo_per_shot );
	if ( iAmmoPerShot )
		return iAmmoPerShot;

	return m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_iAmmoPerShot;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::RemoveAmmo( CTFPlayer *pPlayer )
{
#ifdef GAME_DLL
	if ( IsEnergyWeapon() )
	{
		if ( Energy_HasEnergy() )
			Energy_DrainEnergy();
	}
	else if ( UsesClipsForAmmo1() )
	{
		m_iClip1 -= GetAmmoPerShot();
	}
	else
	{
		if ( m_iWeaponMode == TF_WEAPON_PRIMARY_MODE )
		{
			int nModUseMetalAmmoType = 0;
			CALL_ATTRIB_HOOK_INT( nModUseMetalAmmoType, mod_use_metal_ammo_type );

			if ( nModUseMetalAmmoType )
				pPlayer->RemoveAmmo( GetAmmoPerShot(), TF_AMMO_METAL );
			else
				pPlayer->RemoveAmmo( GetAmmoPerShot(), m_iPrimaryAmmoType );

			if ( 0 < m_iRefundedAmmo )
				pPlayer->GiveAmmo( m_iRefundedAmmo, m_iPrimaryAmmoType );
		}
		else
		{
			pPlayer->RemoveAmmo( GetAmmoPerShot(), m_iSecondaryAmmoType );
			if ( 0 < m_iRefundedAmmo )
				pPlayer->GiveAmmo( m_iRefundedAmmo, m_iSecondaryAmmoType );
		}

		m_iRefundedAmmo = 0;
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Accessor for damage, so sniper etc can modify damage
//-----------------------------------------------------------------------------
float CTFWeaponBaseGun::GetProjectileSpeed( void )
{
	// placeholder for now
	// grenade launcher and pipebomb launcher hook this to set variable pipebomb speed

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Accessor for damage, so sniper etc can modify damage
//-----------------------------------------------------------------------------
float CTFWeaponBaseGun::GetProjectileGravity( void )
{
	return 0.001f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFWeaponBaseGun::GetWeaponSpread( void )
{
	float flSpread = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flSpread;
	CALL_ATTRIB_HOOK_FLOAT( flSpread, mult_spread_scale );

	int iConsecutiveSpread = 0;
	CALL_ATTRIB_HOOK_INT( iConsecutiveSpread, mult_spread_scales_consecutive );
	if ( iConsecutiveSpread > 0 )
		flSpread *= m_iConsecutiveShot;

	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer->m_Shared.InCond( TF_COND_RUNE_PRECISION ) )
		flSpread = 0;

	return flSpread;
}

//-----------------------------------------------------------------------------
// Purpose: Accessor for damage, so sniper etc can modify damage
//-----------------------------------------------------------------------------
float CTFWeaponBaseGun::GetProjectileDamage( void )
{
	float flDamage = (float)m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_nDamage;
	CALL_ATTRIB_HOOK_FLOAT( flDamage, mult_dmg );
	if ( GetTFPlayerOwner() && GetTFPlayerOwner()->m_Shared.InCond( TF_COND_RUNE_STRENGTH ) )
		flDamage *= 2;
	return flDamage;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFWeaponBaseGun::GetWeaponProjectileType( void ) const
{
	int iProjectile = TF_PROJECTILE_NONE;

	CALL_ATTRIB_HOOK_INT( iProjectile, override_projectile_type );

	if ( !iProjectile )
		iProjectile = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_iProjectile;

	return iProjectile;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFWeaponBaseGun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
// Server specific.
#if !defined( CLIENT_DLL )

	// Make sure to zoom out before we holster the weapon.
	ZoomOut();
	SetContextThink( NULL, 0, ZOOM_CONTEXT );

#endif

	// Stop the burst.
	m_iBurstSize = 0;

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::WeaponReset( void )
{
	// Stop the burst.
	m_iBurstSize = 0;

	BaseClass::WeaponReset();
}

//-----------------------------------------------------------------------------
// Purpose:
// NOTE: Should this be put into fire gun
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::DoFireEffects()
{
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

	// Muzzle flash on weapon.
	bool bMuzzleFlash = true;


	if ( pPlayer->IsPlayerClass( TF_CLASS_SNIPER ) )
	{
		if ( pPlayer->IsActiveTFWeapon( TF_WEAPON_COMPOUND_BOW ) )
		{
			bMuzzleFlash = false;
		}
	}

	if ( bMuzzleFlash )
	{
		pPlayer->DoMuzzleFlash();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::ToggleZoom( void )
{
	// Toggle the zoom.
	CBasePlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer )
	{
		if( pPlayer->GetFOV() >= 75 )
		{
			ZoomIn();
		}
		else
		{
			ZoomOut();
		}
	}

	// Get the zoom animation time.
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.2;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::ZoomIn( void )
{
	// The the owning player.
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

	// Set the weapon zoom.
	// TODO: The weapon fov should be gotten from the script file.
	pPlayer->SetFOV( pPlayer, TF_WEAPON_ZOOM_FOV, 0.1f );
	pPlayer->m_Shared.AddCond( TF_COND_ZOOMED );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::ZoomOut( void )
{
	// The the owning player.
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

	if ( pPlayer->m_Shared.InCond( TF_COND_ZOOMED ) )
	{
		// Set the FOV to 0 set the default FOV.
		pPlayer->SetFOV( pPlayer, 0, 0.1f );
		pPlayer->m_Shared.RemoveCond( TF_COND_ZOOMED );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFWeaponBaseGun::ZoomOutIn( void )
{
	//Zoom out, set think to zoom back in.
	ZoomOut();
	SetContextThink( &CTFWeaponBaseGun::ZoomIn, gpGlobals->curtime + ZOOM_REZOOM_TIME, ZOOM_CONTEXT );
}
