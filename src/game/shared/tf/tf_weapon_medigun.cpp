﻿//======== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose:			The Medic's Medikit weapon
//					
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "tf_gamerules.h"
#include "ai_basenpc_shared.h"
#include "tf_revive.h"
#include "effect_dispatch_data.h"
#include "tf_weaponbase_rocket.h"
#include "tf_weaponbase_grenadeproj.h"

#if defined( CLIENT_DLL )
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "particles_simple.h"
#include "c_tf_player.h"
#include "soundenvelope.h"
#include "iefx.h"
#include "dlight.h"
#include "tempent.h"
#else
#include "ndebugoverlay.h"
#include "tf_player.h"
#include "tf_team.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#endif

#include "tf_weapon_medigun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *g_pszMedigunHealSounds[TF_MEDIGUN_COUNT] =
{
	"WeaponMedigun.Healing",
	"WeaponMedigun.Healing",
	"Weapon_Quick_Fix.Healing",
	"WeaponMedigun_Vaccinator.Healing"
};

// Buff ranges
ConVar weapon_medigun_damage_modifier( "weapon_medigun_damage_modifier", "1.5", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Scales the damage a player does while being healed with the medigun." );
ConVar weapon_medigun_construction_rate( "weapon_medigun_construction_rate", "10", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Constructing object health healed per second by the medigun." );
ConVar weapon_medigun_charge_rate( "weapon_medigun_charge_rate", "40", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Amount of time healing it takes to fully charge the medigun." );
ConVar weapon_medigun_chargerelease_rate( "weapon_medigun_chargerelease_rate", "8", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Amount of time it takes the a full charge of the medigun to be released." );

#if defined (CLIENT_DLL)
ConVar tf_medigun_autoheal( "tf_medigun_autoheal", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_USERINFO, "Setting this to 1 will cause the Medigun's primary attack to be a toggle instead of needing to be held down." );
#else
ConVar tf_medigun_lagcomp(  "tf_medigun_lagcomp", "1", FCVAR_DEVELOPMENTONLY );
#endif

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RecvProxy_HealingTarget( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CWeaponMedigun *pMedigun = ((CWeaponMedigun*)(pStruct));
	if ( pMedigun != NULL )
	{
		pMedigun->ForceHealingTargetUpdate();
	}

	RecvProxy_IntToEHandle( pData, pStruct, pOut );
}
#endif

LINK_ENTITY_TO_CLASS( tf_weapon_medigun, CWeaponMedigun );
PRECACHE_WEAPON_REGISTER( tf_weapon_medigun );

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMedigun, DT_WeaponMedigun )

BEGIN_NETWORK_TABLE( CWeaponMedigun, DT_WeaponMedigun )
#if !defined( CLIENT_DLL )
	SendPropFloat( SENDINFO( m_flChargeLevel ), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
	SendPropFloat( SENDINFO( m_flEnergyLevel ), 11, 0, 0.0f, 100.0f ),
	SendPropEHandle( SENDINFO( m_hHealingTarget ) ),
	SendPropBool( SENDINFO( m_bHealing ) ),
	SendPropBool( SENDINFO( m_bAttacking ) ),
	SendPropBool( SENDINFO( m_bChargeRelease ) ),
	SendPropBool( SENDINFO( m_bHolstered ) ),
	SendPropInt( SENDINFO( m_nChargeResistType ) ),
#else
	RecvPropFloat( RECVINFO( m_flChargeLevel ) ),
	RecvPropFloat( RECVINFO( m_flEnergyLevel ) ),
	RecvPropEHandle( RECVINFO( m_hHealingTarget ), RecvProxy_HealingTarget ),
	RecvPropBool( RECVINFO( m_bHealing ) ),
	RecvPropBool( RECVINFO( m_bAttacking ) ),
	RecvPropBool( RECVINFO( m_bChargeRelease ) ),
	RecvPropBool( RECVINFO( m_bHolstered ) ),
	RecvPropInt( RECVINFO( m_nChargeResistType ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponMedigun  )

	DEFINE_PRED_FIELD( m_bHealing, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bAttacking, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bHolstered, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hHealingTarget, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),

	DEFINE_FIELD( m_flHealEffectLifetime, FIELD_FLOAT ),

	DEFINE_PRED_FIELD( m_flChargeLevel, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flEnergyLevel, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bChargeRelease, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD( m_nChargeResistType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

//	DEFINE_PRED_FIELD( m_bPlayingSound, FIELD_BOOLEAN ),
//	DEFINE_PRED_FIELD( m_bUpdateHealingTargets, FIELD_BOOLEAN ),

END_PREDICTION_DATA()
#endif

#define PARTICLE_PATH_VEL				140.0
#define NUM_PATH_PARTICLES_PER_SEC		300.0f
#define NUM_MEDIGUN_PATH_POINTS		8

extern ConVar tf_max_health_boost;
extern ConVar lfe_muzzlelight;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponMedigun::CWeaponMedigun( void )
{
	WeaponReset();

	SetPredictionEligible( true );
}

CWeaponMedigun::~CWeaponMedigun()
{
#ifdef CLIENT_DLL
	if ( m_pChargedSound )
		CSoundEnvelopeController::GetController().SoundDestroy( m_pChargedSound );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::WeaponReset( void )
{
	BaseClass::WeaponReset();

	m_flHealEffectLifetime = 0;

	m_bHealing = false;
	m_bAttacking = false;
	m_bHolstered = true;
	m_bChargeRelease = false;

	m_bCanChangeTarget = true;

	m_flNextBuzzTime = 0;
	m_flReleaseStartedAt = 0;

	m_nChargeResistType = TF_RESIST_BULLET;

	if ( TFGameRules()->State_Get() == GR_STATE_RND_RUNNING )
	{
		int iPreserveUbercharge = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( GetTFPlayerOwner(), iPreserveUbercharge, preserve_ubercharge );
		m_flChargeLevel = Min( iPreserveUbercharge / 100.0f, m_flChargeLevel.Get() );
	}
	else
	{
		m_flChargeLevel = 0.0f;
	}

	m_flEnergyLevel = 0.0f;

	RemoveHealingTarget( true );

#if defined( CLIENT_DLL )
	m_bPlayingSound = false;
	m_bUpdateHealingTargets = false;
	m_bOldChargeRelease = false;

	UpdateEffects();
	ManageChargeEffect();

	m_pChargeEffect = NULL;
	m_pChargedSound = NULL;

#else
	m_hShield = NULL;
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::Precache()
{
	BaseClass::Precache();

	int iType = GetMedigunType();

	PrecacheScriptSound( g_pszMedigunHealSounds[iType] );

	PrecacheScriptSound( "WeaponMedigun.NoTarget" );
	PrecacheScriptSound( "WeaponMedigun.Charged" );
	PrecacheScriptSound( "WeaponMedigun_Vaccinator.Toggle" );

	PrecacheTeamParticles( "medicgun_invulnstatus_fullcharge_%s" );
	PrecacheTeamParticles( "medicgun_beam_%s" );
	PrecacheTeamParticles( "medicgun_beam_%s_invun" );

	// Precache charge sounds.
	for ( int i = 0; i < TF_CHARGE_COUNT; i++ )
	{
		PrecacheScriptSound( g_MedigunEffects[i].sound_enable );
		PrecacheScriptSound( g_MedigunEffects[i].sound_disable );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMedigun::Deploy( void )
{
	if ( BaseClass::Deploy() )
	{
		m_bHolstered = false;

#ifdef GAME_DLL
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( m_bChargeRelease && pOwner )
			pOwner->m_Shared.RecalculateChargeEffects();
#endif

#ifdef CLIENT_DLL
		ManageChargeEffect();
#endif

		m_flNextTargetCheckTime = gpGlobals->curtime;

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMedigun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	RemoveHealingTarget( true );
	m_bAttacking = false;
	m_bHolstered = true;

#ifdef GAME_DLL
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
		pOwner->m_Shared.RecalculateChargeEffects( true );
#endif

#ifdef CLIENT_DLL
	UpdateEffects();
	ManageChargeEffect();
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::UpdateOnRemove( void )
{
	RemoveHealingTarget( true );
	m_bAttacking = false;
	m_bChargeRelease = false;
	m_bHolstered = true;

#ifndef CLIENT_DLL
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
		pOwner->m_Shared.RecalculateChargeEffects( true );

	if ( m_hShield )
		UTIL_Remove( m_hShield );
#else
	if ( m_bPlayingSound )
	{
		m_bPlayingSound = false;
		StopHealSound();
	}

	UpdateEffects();
	ManageChargeEffect();
#endif

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CWeaponMedigun::GetTargetRange( void )
{
	return (float)m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flRange;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CWeaponMedigun::GetStickRange( void )
{
	return ( GetTargetRange() * 1.2 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CWeaponMedigun::GetHealRate( void )
{
	float flHealRate = (float)m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_nDamage;
	CALL_ATTRIB_HOOK_FLOAT( flHealRate, mult_medigun_healrate );

	float flHealMult = 1.0f;
	if ( m_hHealingTarget && ( m_hHealingTarget->IsPlayer() || m_hHealingTarget->IsNPC() ) )
	{
		CBaseCombatCharacter *pCombat = ToBaseCombatCharacter( m_hHealingTarget );
		if ( pCombat && pCombat->GetActiveWeapon() )
		{
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pCombat->GetActiveWeapon(), flHealMult, mult_healing_from_medics );
			if ( flHealMult )
				flHealRate *= flHealMult;
		}
	}

	float flHealingMastery = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flHealingMastery, healing_mastery );
	if ( flHealingMastery )
		flHealRate *= flHealingMastery;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && pOwner->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		flHealRate *= 2.0f;

	if ( pOwner && pOwner->m_Shared.InCond( TF_COND_MEGAHEAL ) )
		flHealRate *= 3.0f;


	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pOwner, flHealRate, mult_medigun_healrate_on_wearer);

	return flHealRate;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CWeaponMedigun::GetMinChargeAmount( void ) const
{
	float flMinAmount = 0.5f;

	if ( GetMedigunType() == TF_MEDIGUN_VACCINATOR )
		flMinAmount = 0.25f;

	return flMinAmount;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CWeaponMedigun::GetMedigunType( void ) const
{
	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );

	if ( iType >= 0 && iType < TF_MEDIGUN_COUNT )
		return iType;

	AssertMsg( 0, "Invalid medigun type!\n" );
	return TF_MEDIGUN_STOCK;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CWeaponMedigun::GetResistType( void ) const
{
	int m_nChargeResistType = 0;
	if ( m_nChargeResistType >= 0 && m_nChargeResistType < TF_RESIST_COUNT )
		return m_nChargeResistType;

	AssertMsg( 0, "Invalid resist type!\n" );
	return TF_RESIST_BULLET;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
medigun_charge_types CWeaponMedigun::GetChargeType( void ) const
{
	int iChargeType = TF_CHARGE_INVULNERABLE;
	CALL_ATTRIB_HOOK_INT( iChargeType, set_charge_type );

	if ( iChargeType > TF_CHARGE_NONE && iChargeType < TF_CHARGE_COUNT )
		return (medigun_charge_types)iChargeType;

	if ( GetMedigunType() == TF_MEDIGUN_VACCINATOR )
	{
		int nChargeResistType = TF_CHARGE_BULLET_RESIST;

		if ( m_nChargeResistType == TF_RESIST_BULLET )
			nChargeResistType = TF_CHARGE_BULLET_RESIST;

		if ( m_nChargeResistType == TF_RESIST_BLAST )
			nChargeResistType = TF_CHARGE_BLAST_RESIST;

		if ( m_nChargeResistType == TF_RESIST_FIRE )
			nChargeResistType = TF_CHARGE_FIRE_RESIST;

		return (medigun_charge_types)nChargeResistType;
	}

	AssertMsg( 0, "Invalid charge type!\n" );
	return TF_CHARGE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CWeaponMedigun::GetHealSound( void )
{
	return g_pszMedigunHealSounds[GetMedigunType()];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::CycleResistType( void )
{
#ifdef CLIENT_DLL
	CLocalPlayerFilter filter;
	EmitSound( filter, entindex(), "WeaponMedigun_Vaccinator.Toggle" );
#endif

	m_nChargeResistType += 1;
	if ( m_nChargeResistType <= TF_RESIST_COUNT )
		m_nChargeResistType = TF_RESIST_BULLET;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Zhis wall is robot death!
//-----------------------------------------------------------------------------
void CWeaponMedigun::CreateMedigunShield( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	CTFMedigunShield *pShield = CTFMedigunShield::Create( pOwner );
	if ( pShield )
		m_hShield = pShield;

	pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_HEAL_SHIELD );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMedigun::HealingTarget( CBaseEntity *pTarget )
{
	if ( pTarget == m_hHealingTarget )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMedigun::AllowedToHealTarget( CBaseEntity *pTarget )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pTarget );
	if ( pTFPlayer )
	{
		bool bStealthed = pTFPlayer->m_Shared.IsStealthed();
		bool bDisguised = pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED );
 
		// We can heal teammates and enemies that are disguised as teammates
		if ( !bStealthed &&
			( pTFPlayer->InSameTeam( pOwner ) ||
			( bDisguised && pTFPlayer->m_Shared.GetDisguiseTeam() == pOwner->GetTeamNumber() ) ) )
		{
			return true;
		}
	}
	else if ( pTarget->IsNPC() )	// See if this is NPC.
 	{
		CAI_BaseNPC *pNPC = assert_cast<CAI_BaseNPC *>( pTarget );

		if ( !pNPC->CanBeHealed() )
			return false;

		// We can heal teammates only
		if ( pNPC->InSameTeam( pOwner ) )
		{
			return true;
		}
 	}
	else if ( pTarget->IsCombatItem() )	// See if this is Revive Marker.
 	{
#if GAME_DLL
		if ( !pTarget->ClassMatches( "entity_revive_marker" ) )
			return false;
#endif

		CTFReviveMarker *pMarker = assert_cast<CTFReviveMarker *>( pTarget );
		// We can heal teammates only
		if ( pMarker->InSameTeam( pOwner ) )
		{
			return true;
		}
 	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMedigun::CouldHealTarget( CBaseEntity *pTarget )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return false;

	if ( (pTarget->IsPlayer() || pTarget->IsNPC() ) && pTarget->IsAlive() && !HealingTarget(pTarget) )
		return AllowedToHealTarget( pTarget );

	if ( pTarget->IsCombatItem() && !HealingTarget( pTarget ) )
		return AllowedToHealTarget( pTarget );

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::MaintainTargetInSlot()
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	CBaseEntity *pTarget = m_hHealingTarget;
	Assert( pTarget );

	// Make sure the guy didn't go out of range.
	bool bLostTarget = true;
	Vector vecSrc = pOwner->Weapon_ShootPosition( );
	Vector vecTargetPoint = pTarget->WorldSpaceCenter();
	Vector vecPoint;

	// If it's brush built, use absmins/absmaxs
	pTarget->CollisionProp()->CalcNearestPoint( vecSrc, &vecPoint );

	float flDistance = (vecPoint - vecSrc).Length();
	if ( flDistance < GetStickRange() )
	{
		if ( m_flNextTargetCheckTime > gpGlobals->curtime )
			return;

		m_flNextTargetCheckTime = gpGlobals->curtime + 1.0f;

		trace_t tr;
		CMedigunFilter drainFilter( pOwner );

		Vector vecAiming;
		pOwner->EyeVectors( &vecAiming );

		Vector vecEnd = vecSrc + vecAiming * GetTargetRange();

		UTIL_TraceLine( vecSrc, vecEnd, (MASK_SHOT & ~CONTENTS_HITBOX), pOwner, COLLISION_GROUP_VEHICLE, &tr );

		// Still visible?
		if ( tr.m_pEnt == pTarget )
			return;

		Ray_t ray2;
		ray2.Init( vecSrc, vecTargetPoint );
		UTIL_Portal_TraceRay( ray2, MASK_SHOT, &drainFilter, &tr );

		// Still visible?
		if (( tr.fraction == 1.0f) || (tr.m_pEnt == pTarget))
			return;

		// If we failed, try the target's eye point as well
		Ray_t ray3; ray3.Init( vecSrc, pTarget->EyePosition() );
		UTIL_Portal_TraceRay( ray3, MASK_SHOT, &drainFilter, &tr );

		if (( tr.fraction == 1.0f) || (tr.m_pEnt == pTarget))
			return;
	}

	// We've lost this guy
	if ( bLostTarget )
	{
		RemoveHealingTarget();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::FindNewTargetForSlot()
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	Vector vecSrc = pOwner->Weapon_ShootPosition( );
	if ( m_hHealingTarget )
	{
		RemoveHealingTarget();
	}

	// In Normal mode, we heal players under our crosshair
	Vector vecAiming;
	pOwner->EyeVectors( &vecAiming );

	// Find a player in range of this player, and make sure they're healable.
	Vector vecEnd = vecSrc + vecAiming * GetTargetRange();
	trace_t tr;

	UTIL_TraceLine( vecSrc, vecEnd, (MASK_SHOT & ~CONTENTS_HITBOX), pOwner, COLLISION_GROUP_VEHICLE, &tr );
	if ( tr.fraction != 1.0 && tr.m_pEnt )
	{
		if ( CouldHealTarget( tr.m_pEnt ) )
		{
#ifdef GAME_DLL
			pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_STARTEDHEALING );
			if ( tr.m_pEnt->IsPlayer() )
			{
				CTFPlayer *pTarget = ToTFPlayer( tr.m_pEnt );
				pTarget->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_STARTEDHEALING );
			}

			// Start the heal target thinking.
			SetContextThink( &CWeaponMedigun::HealTargetThink, gpGlobals->curtime, s_pszMedigunHealTargetThink );
#endif

			m_hHealingTarget.Set( tr.m_pEnt );
			m_flNextTargetCheckTime = gpGlobals->curtime + 1.0f;
		}			
	}
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponMedigun::HealTargetThink( void )
{	
	// Verify that we still have a valid heal target.
	CBaseEntity *pTarget = m_hHealingTarget;
	if ( !pTarget || !pTarget->IsAlive() )
	{
		SetContextThink( NULL, 0, s_pszMedigunHealTargetThink );
		return;
	}

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	CTFReviveMarker *pMarker = dynamic_cast<CTFReviveMarker *>( pTarget );
	if ( pMarker )
		pMarker->AddMarkerHealth( GetHealRate() );

	float flTime = gpGlobals->curtime - pOwner->GetTimeBase();
	if ( flTime > 5.0f || !AllowedToHealTarget(pTarget) )
	{
		RemoveHealingTarget( false );
	}

	SetNextThink( gpGlobals->curtime + 0.2f, s_pszMedigunHealTargetThink );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CWeaponMedigun::IsAttachedToBuilding( void ) const
{
	if ( m_hHealingTarget )
		return m_hHealingTarget->IsBaseObject();

	return false;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to a healable target
//-----------------------------------------------------------------------------
bool CWeaponMedigun::FindAndHealTargets( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return false;

	bool bFound = false;

	// Maintaining beam to existing target?
	CBaseEntity *pTarget = m_hHealingTarget;
	if ( pTarget && pTarget->IsAlive() )
	{
		MaintainTargetInSlot();
	}
	else if ( pTarget && pTarget->IsCombatItem() )
	{
		MaintainTargetInSlot();
	}
	else
	{	
		FindNewTargetForSlot();
	}

	CBaseEntity *pNewTarget = m_hHealingTarget;
	if ( pNewTarget )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( pNewTarget );
		CAI_BaseNPC *pNPC = pNewTarget->MyNPCPointer();

#ifdef GAME_DLL
		CTFReviveMarker *pMarker = dynamic_cast<CTFReviveMarker *>( pNewTarget );
		if ( pTFPlayer && pTFPlayer->IsAlive() )
		{
			if ( pTarget != pNewTarget && pNewTarget->IsPlayer() )
			{
				pTFPlayer->m_Shared.Heal( pOwner, GetHealRate() );
				pOwner->TeamFortress_SetSpeed();
			}

			pTFPlayer->m_Shared.RecalculateChargeEffects( false );
		}
		else if ( pNPC && pNPC->IsAlive() )
		{
			if ( pTarget != pNewTarget && pNewTarget->IsNPC() )
			{
				pNPC->Heal( pOwner, GetHealRate() );
			}

			pNPC->RecalculateChargeEffects( false );
		}
		else if ( pMarker )
		{
			if ( pTarget != pNewTarget && pNewTarget->IsCombatItem() )
			{
				pMarker->AddMarkerHealth( GetHealRate() );
				//CTF_GameStats.Event_PlayerRevived( pOwner );
			}
		}

		if ( m_flReleaseStartedAt && m_flReleaseStartedAt < (gpGlobals->curtime + 0.2) )
		{
			if ( GetMedigunType() == TF_MEDIGUN_STOCK )
			{
				// When we start the release, everyone we heal rockets to full health
				pNewTarget->TakeHealth( pNewTarget->GetMaxHealth(), DMG_GENERIC );
			}
		}
#endif
	
		bFound = true;

		// Charge up our power if we're not releasing it, and our target
		// isn't receiving any benefit from our healing.
		if ( !m_bChargeRelease )
		{
			if ( pTFPlayer )
			{
				int iBoostMax = floor( pTFPlayer->m_Shared.GetMaxBuffedHealth() * 0.95);

				if ( weapon_medigun_charge_rate.GetFloat() )
				{
					float flChargeAmount = gpGlobals->frametime / weapon_medigun_charge_rate.GetFloat();
					CALL_ATTRIB_HOOK_FLOAT( flChargeAmount, mult_medigun_uberchargerate );

					if ( TFGameRules() && TFGameRules()->InSetup() )
					{
						// Build charge at triple rate during setup
						flChargeAmount *= 3.0f;
					}
					else if ( pNewTarget->GetHealth() >= iBoostMax )
					{
						// Reduced charge for healing fully healed guys
						flChargeAmount *= 0.5f;
					}
					else if ( pOwner->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
					{
						flChargeAmount *= 2.0f;
					}

					int iTotalHealers = pTFPlayer->m_Shared.GetNumHealers();
					if ( iTotalHealers > 1 )
					{
						flChargeAmount /= (float)iTotalHealers;
					}

					float flNewLevel = min( m_flChargeLevel + flChargeAmount, 1.0 );
#ifdef GAME_DLL
					if ( flNewLevel >= 1.0 && m_flChargeLevel < 1.0 )
					{
						pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_CHARGEREADY );

						if ( pTFPlayer )
						{
							pTFPlayer->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_CHARGEREADY );
						}
					}
#endif
					m_flChargeLevel = flNewLevel;
				}
			}
			else if ( pNPC )
			{
				int iBoostMax = floor( pNPC->GetMaxBuffedHealth() * 0.95);

				if ( weapon_medigun_charge_rate.GetFloat() )
				{
					float flChargeAmount = gpGlobals->frametime / weapon_medigun_charge_rate.GetFloat();
					CALL_ATTRIB_HOOK_FLOAT( flChargeAmount, mult_medigun_uberchargerate );

					if ( TFGameRules() && TFGameRules()->InSetup() )
					{
						// Build charge at triple rate during setup
						flChargeAmount *= 3.0f;
					}
					else if ( pNewTarget->GetHealth() >= iBoostMax )
					{
						// Reduced charge for healing fully healed guys
						flChargeAmount *= 0.5f;
					}
					else if ( pOwner->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
					{
						flChargeAmount *= 2.0f;
					}

					int iTotalHealers = pNPC->GetNumHealers();
					if ( iTotalHealers > 1 )
					{
						flChargeAmount /= (float)iTotalHealers;
					}

					float flNewLevel = min( m_flChargeLevel + flChargeAmount, 1.0 );
#ifdef GAME_DLL
					if ( flNewLevel >= 1.0 && m_flChargeLevel < 1.0 )
					{
						pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_CHARGEREADY );
					}
#endif
					m_flChargeLevel = flNewLevel;
				}
			}
		}
	}

	return bFound;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::AddCharge( float flAmount )
{
	float flChargeRate = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flChargeRate, mult_medigun_uberchargerate );
	if ( !flChargeRate ) // Can't earn uber.
		return;

	float flNewLevel = min( m_flChargeLevel + flAmount, 1.0 );

#ifdef GAME_DLL
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	CTFPlayer *pHealingTarget = ToTFPlayer( m_hHealingTarget );

	if ( !m_bChargeRelease && flNewLevel >= 1.0 && m_flChargeLevel < 1.0 )
	{
		if ( pPlayer )
		{
			pPlayer->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_CHARGEREADY );
		}

		if ( pHealingTarget )
		{
			pHealingTarget->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_CHARGEREADY );
		}
	}
#endif

	m_flChargeLevel = flNewLevel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::SubtractCharge( float flAmount )
{
	float flChargeRate = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flChargeRate, mult_medigun_uberchargerate );
	if ( !flChargeRate ) // Can't earn uber, don't bother.
		return;

	// Never let us fall under 0%.
	if ( m_flChargeLevel > 0 )
	{
		float flNewLevel = max( m_flChargeLevel - ( flAmount / 100), 0.0 );

		m_flChargeLevel = flNewLevel;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::SubtractChargeAndUpdateDeployState( float flAmount, bool bSomething )
{
	SubtractCharge( flAmount );
#ifdef CLIENT_DLL
	ManageChargeEffect();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::SetCharge( float flAmount )
{
#ifdef GAME_DLL
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	CTFPlayer *pHealingTarget = ToTFPlayer( m_hHealingTarget );

	if ( !m_bChargeRelease && m_flChargeLevel >= 1.0 && m_flChargeLevel < 1.0 )
	{
		if ( pPlayer )
		{
			pPlayer->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_CHARGEREADY );
		}

		if ( pHealingTarget )
		{
			pHealingTarget->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_CHARGEREADY );
		}
	}
#endif

	m_flChargeLevel = flAmount;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponMedigun::SetEnergyMeter( float flAmount )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
 	if ( !pPlayer )
		return;

	if ( flAmount >= 0.0f )
		m_flEnergyLevel = min( m_flEnergyLevel + flAmount, 100.0f );
	else
		m_flEnergyLevel = flAmount;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::ItemHolsterFrame( void )
{
	BaseClass::ItemHolsterFrame();

	DrainCharge();

#ifdef GAME_DLL
	if ( m_hShield != NULL )
	{
		m_hShield->UpdateShieldPosition();

		if ( m_flEnergyLevel > 0.0f )
		{
			m_flEnergyLevel -= ( 100.0f / 10 ) * gpGlobals->frametime;
		}
		else
		{
			m_flEnergyLevel = 0.0f;
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::DrainCharge( void )
{
	// If we're in charge release mode, drain our charge
	if ( m_bChargeRelease )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( !pOwner )
			return;

		float flUberTime = weapon_medigun_chargerelease_rate.GetFloat();
		float flAddUberTime = 0.0f;
		CALL_ATTRIB_HOOK_FLOAT( flAddUberTime, add_uber_time );

		float flChargeAmount = gpGlobals->frametime / flUberTime+flAddUberTime;
		m_flChargeLevel = max( m_flChargeLevel - flChargeAmount, 0.0 );
		if ( !m_flChargeLevel )
		{
			m_bChargeRelease = false;
			m_flReleaseStartedAt = 0;

#ifdef GAME_DLL
			pOwner->m_Shared.RecalculateChargeEffects();
#endif
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Overloaded to handle the hold-down healing
//-----------------------------------------------------------------------------
void CWeaponMedigun::ItemPostFrame( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	// If we're lowered, we're not allowed to fire
	if ( CanAttack() == false )
	{
		RemoveHealingTarget( true );
		return;
	}

#if !defined( CLIENT_DLL )
	if ( AppliesModifier() )
	{
		m_DamageModifier.SetModifier( weapon_medigun_damage_modifier.GetFloat() );
	}
#endif

	// Try to start healing
	m_bAttacking = false;
	if ( pOwner->GetMedigunAutoHeal() )
	{
		if ( pOwner->m_nButtons & IN_ATTACK )
		{
			if ( m_bCanChangeTarget )
			{
				RemoveHealingTarget();
#if defined( CLIENT_DLL )
				m_bPlayingSound = false;
				StopHealSound();
#endif
				// can't change again until we release the attack button
				m_bCanChangeTarget = false;
			}
		}
		else
		{
			m_bCanChangeTarget = true;
		}

		if ( m_bHealing || ( pOwner->m_nButtons & IN_ATTACK ) )
		{
			PrimaryAttack();
			m_bAttacking = true;
		}
	}
	else
	{
		if ( /*m_bChargeRelease || */ pOwner->m_nButtons & IN_ATTACK )
		{
			PrimaryAttack();
			m_bAttacking = true;
		}
 		else if ( m_bHealing )
 		{
 			// Detach from the player if they release the attack button.
 			RemoveHealingTarget();
 		}
	}

	if ( pOwner->m_nButtons & IN_ATTACK2 )
	{
		SecondaryAttack();
	}

	if ( pOwner->m_afButtonPressed & IN_RELOAD )
	{
		if ( GetMedigunType() == TF_MEDIGUN_VACCINATOR )
			CycleResistType();
	}

#ifdef GAME_DLL
	if ( ( pOwner->m_afButtonPressed & IN_ATTACK3 ) && ( m_flEnergyLevel >= 100.0f ) && m_hShield == NULL )
	{
		CreateMedigunShield();
	}

	if ( m_hShield != NULL )
	{
		m_hShield->UpdateShieldPosition();

		if ( m_flEnergyLevel > 0.0f )
		{
			m_flEnergyLevel -= ( 100.0f / 10 ) * gpGlobals->frametime;
		}
		else
		{
			m_flEnergyLevel = 0.0f;
		}
	}
#endif

	WeaponIdle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMedigun::Lower( void )
{
	// Stop healing if we are
	if ( m_bHealing )
	{
		RemoveHealingTarget( true );
		m_bAttacking = false;

#ifdef CLIENT_DLL
		UpdateEffects();
#endif
	}

	return BaseClass::Lower();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::RemoveHealingTarget( bool bSilent )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

#ifdef GAME_DLL
	if ( m_hHealingTarget )
	{
		if ( m_hHealingTarget->IsPlayer() )
		{
			CTFPlayer *pTFPlayer = ToTFPlayer( m_hHealingTarget );
			pTFPlayer->m_Shared.StopHealing( pOwner );
			pTFPlayer->m_Shared.RecalculateChargeEffects( false );

			if ( !bSilent )
			{
				pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_STOPPEDHEALING, pTFPlayer->IsAlive() ? "healtarget:alive" : "healtarget:dead" );
				pTFPlayer->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_STOPPEDHEALING );
			}
		}
		else if ( m_hHealingTarget->IsNPC() )
		{
			CAI_BaseNPC *pNPC = m_hHealingTarget->MyNPCPointer();
			pNPC->StopHealing( pOwner );
			pNPC->RecalculateChargeEffects( false );

			pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_STOPPEDHEALING, pNPC->IsAlive() ? "healtarget:alive" : "healtarget:dead" );
		}
		else if ( m_hHealingTarget->IsCombatItem() )
		{
			CTFReviveMarker *pMarker = dynamic_cast<CTFReviveMarker *>( m_hHealingTarget.Get() );
			if ( pMarker )
				pMarker->AddMarkerHealth( 0 );
		}
	}

	// Stop thinking - we no longer have a heal target.
	SetContextThink( NULL, 0, s_pszMedigunHealTargetThink );
#endif

	m_hHealingTarget.Set( NULL );

	// Stop the welding animation
	if ( m_bHealing && !bSilent )
	{
		SendWeaponAnim( ACT_MP_ATTACK_STAND_POSTFIRE );
		pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_POST );
	}

#ifndef CLIENT_DLL
	m_DamageModifier.RemoveModifier();
#endif
	m_bHealing = false;
	pOwner->TeamFortress_SetSpeed();
}


//-----------------------------------------------------------------------------
// Purpose: Attempt to heal any player within range of the medikit
//-----------------------------------------------------------------------------
void CWeaponMedigun::PrimaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !CanAttack() )
		return;

#if !defined (CLIENT_DLL)
	if ( tf_medigun_lagcomp.GetBool() )
		lagcompensation->StartLagCompensation( pOwner, LAG_COMPENSATE_BOUNDS );
#endif

	if ( FindAndHealTargets() )
	{
		// Start the animation
		if ( !m_bHealing )
		{
#ifdef GAME_DLL
			pOwner->SpeakWeaponFire();
#endif
			SendWeaponAnim( ACT_MP_ATTACK_STAND_PREFIRE );
			pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRE );
		}

		m_bHealing = true;
	}
	else
	{
		RemoveHealingTarget();
	}
	
#if !defined (CLIENT_DLL)
	if ( tf_medigun_lagcomp.GetBool() )
		lagcompensation->FinishLagCompensation( pOwner );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Burn charge level to generate invulnerability
//-----------------------------------------------------------------------------
void CWeaponMedigun::SecondaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !CanAttack() )
		return;

	float flFullCharge = 1.0f;
	if ( GetMedigunType() == TF_MEDIGUN_VACCINATOR )
		flFullCharge = 0.25f;

	// Ensure they have a full charge and are not already in charge release mode
	if ( m_flChargeLevel < 1.0 || m_bChargeRelease )
	{
#ifdef CLIENT_DLL
		// Deny, flash
		if ( !m_bChargeRelease && m_flFlashCharge <= 0 )
		{
			m_flFlashCharge = 10;
			pOwner->EmitSound( "Player.DenyWeaponSelection" );
		}
#endif
		return;
	}

	if ( pOwner->HasTheFlag() )
	{
#ifdef GAME_DLL
		CSingleUserRecipientFilter filter( pOwner );
		TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_NO_INVULN_WITH_FLAG );
#endif
		pOwner->EmitSound( "Player.DenyWeaponSelection" );
		return;
	}

	// Start super charge
	m_bChargeRelease = true;
	m_flReleaseStartedAt = 0;//gpGlobals->curtime;

#ifdef GAME_DLL
	CTF_GameStats.Event_PlayerInvulnerable( pOwner );
	pOwner->m_Shared.RecalculateChargeEffects();

	pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_CHARGEDEPLOYED );

	if ( m_hHealingTarget )
	{
		if ( m_hHealingTarget->IsPlayer() )
		{
			CTFPlayer *pTFPlayer = ToTFPlayer( m_hHealingTarget );
			pTFPlayer->m_Shared.RecalculateChargeEffects();
			pTFPlayer->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_CHARGEDEPLOYED );
		}
		else if ( m_hHealingTarget->IsNPC() )
		{
			CAI_BaseNPC *pNPC = m_hHealingTarget->MyNPCPointer();
			pNPC->RecalculateChargeEffects();
		}
	}

	IGameEvent * event = gameeventmanager->CreateEvent( "player_chargedeployed" );
	if ( event )
	{
		event->SetInt( "userid", pOwner->GetUserID() );

		gameeventmanager->FireEvent( event, true );	// don't send to clients
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Idle tests to see if we're facing a valid target for the medikit
//			If so, move into the "heal-able" animation. 
//			Otherwise, move into the "not-heal-able" animation.
//-----------------------------------------------------------------------------
void CWeaponMedigun::WeaponIdle( void )
{
	if ( HasWeaponIdleTimeElapsed() )
	{
		// Loop the welding animation
		if ( m_bHealing )
		{
			SendWeaponAnim( ACT_VM_PRIMARYATTACK );
			return;
		}

		return BaseClass::WeaponIdle();
	}
}

#if defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::StopHealSound( bool bStopHealingSound, bool bStopNoTargetSound )
{
	if ( bStopHealingSound )
	{
		StopSound( GetHealSound() );
	}

	if ( bStopNoTargetSound )
	{
		StopSound( "WeaponMedigun.NoTarget" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::ManageChargeEffect( void )
{
	bool bOwnerTaunting = false;

	if ( GetTFPlayerOwner() && GetTFPlayerOwner()->m_Shared.InCond( TF_COND_TAUNTING ) == true )
	{
		bOwnerTaunting = true;
	}

	if ( GetTFPlayerOwner() && bOwnerTaunting == false && m_bHolstered == false && ( m_flChargeLevel >= 1.0f || m_bChargeRelease == true ) )
	{
		C_BaseEntity *pEffectOwner = GetWeaponForEffect();

		if ( pEffectOwner && m_pChargeEffect == NULL )
		{
			const char *pszEffectName = ConstructTeamParticle( "medicgun_invulnstatus_fullcharge_%s", GetTFPlayerOwner()->GetTeamNumber() );

			m_pChargeEffect = pEffectOwner->ParticleProp()->Create( pszEffectName, PATTACH_POINT_FOLLOW, "muzzle" );
			m_hChargeEffectHost = pEffectOwner;
		}

		if ( m_pChargedSound == NULL )
		{
			CLocalPlayerFilter filter;

			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

			m_pChargedSound = controller.SoundCreate( filter, entindex(), "WeaponMedigun.Charged" );
			controller.Play( m_pChargedSound, 1.0, 100 );
		}
	}
	else
	{
		C_BaseEntity *pEffectOwner = m_hChargeEffectHost.Get();

		if ( m_pChargeEffect != NULL )
		{
			if ( pEffectOwner )
			{
				pEffectOwner->ParticleProp()->StopEmission( m_pChargeEffect );
				m_hChargeEffectHost = NULL;
			}

			m_pChargeEffect = NULL;
		}

		if ( m_pChargedSound != NULL )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pChargedSound );
			m_pChargedSound = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void CWeaponMedigun::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( m_bUpdateHealingTargets )
	{
		UpdateEffects();
		m_bUpdateHealingTargets = false;
	}

	// Think?
	if ( m_bHealing )
	{
		ClientThinkList()->SetNextClientThink( GetClientHandle(), CLIENT_THINK_ALWAYS );
	}
	else
	{
		ClientThinkList()->SetNextClientThink( GetClientHandle(), CLIENT_THINK_NEVER );
		m_bPlayingSound = false;
		StopHealSound( true, false );

		// Are they holding the attack button but not healing anyone? Give feedback.
		if ( IsActiveByLocalPlayer() && GetOwner() && GetOwner()->IsAlive() && m_bAttacking && GetOwner() == C_BasePlayer::GetLocalPlayer() && CanAttack() == true )
		{
			if ( gpGlobals->curtime >= m_flNextBuzzTime )
			{
				CLocalPlayerFilter filter;
				EmitSound( filter, entindex(), "WeaponMedigun.NoTarget" );
				m_flNextBuzzTime = gpGlobals->curtime + 0.5f;	// only buzz every so often.
			}
		}
		else
		{
			StopHealSound( false, true );	// Stop the "no target" sound.
		}
	}

	ManageChargeEffect();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::ClientThink()
{
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalPlayer )
		return;

	// Don't show it while the player is dead. Ideally, we'd respond to m_bHealing in OnDataChanged,
	// but it stops sending the weapon when it's holstered, and it gets holstered when the player dies.
	CTFPlayer *pFiringPlayer = GetTFPlayerOwner();
	if ( !pFiringPlayer || pFiringPlayer->IsPlayerDead() || pFiringPlayer->IsDormant() )
	{
		ClientThinkList()->SetNextClientThink( GetClientHandle(), CLIENT_THINK_NEVER );
		m_bPlayingSound = false;
		StopHealSound();
		return;
	}
		
	// If the local player is the guy getting healed, let him know 
	// who's healing him, and their charge level.
	if( m_hHealingTarget != NULL )
	{
		if ( pLocalPlayer == m_hHealingTarget )
		{
			pLocalPlayer->SetHealer( pFiringPlayer, m_flChargeLevel );
		}

		// Handle the dynamic light
		if ( lfe_muzzlelight.GetBool() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
			dl->origin = pFiringPlayer->Weapon_ShootPosition();
			switch ( pFiringPlayer->GetTeamNumber() )
			{
			case TF_TEAM_RED:
				dl->color.r = 255;
				dl->color.g = 80;
				dl->color.b = 10;
				break;

			case TF_TEAM_BLUE:
				dl->color.r = 10;
				dl->color.g = 80;
				dl->color.b = 255;
				break;
			}
			dl->die = gpGlobals->curtime + 0.01f;
			dl->radius = 128.f;
			dl->decay = 512.0f;
			dl->style = 1;
		}

		if ( !m_bPlayingSound )
		{
			m_bPlayingSound = true;
			CLocalPlayerFilter filter;
			EmitSound( filter, entindex(), GetHealSound() );
		}
	}

	if ( m_bOldChargeRelease != m_bChargeRelease )
	{
		m_bOldChargeRelease = m_bChargeRelease;
		ForceHealingTargetUpdate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMedigun::UpdateEffects( void )
{
	CTFPlayer *pFiringPlayer = GetTFPlayerOwner();
	if ( !pFiringPlayer )
		return;

	C_BaseEntity *pEffectOwner = m_hHealingTargetEffect.hOwner.Get();

	// Remove all the effects
	if ( m_hHealingTargetEffect.pEffect && pEffectOwner )
	{
		pEffectOwner->ParticleProp()->StopEmission( m_hHealingTargetEffect.pEffect );
		pEffectOwner->ParticleProp()->StopEmission( m_hHealingTargetEffect.pCustomEffect );
	}

	m_hHealingTargetEffect.hOwner = NULL;
	m_hHealingTargetEffect.pTarget = NULL;
	m_hHealingTargetEffect.pEffect = NULL;
	m_hHealingTargetEffect.pCustomEffect = NULL;

	pEffectOwner = GetWeaponForEffect();

	// Don't add targets if the medic is dead
	if ( !pEffectOwner || pFiringPlayer->IsPlayerDead() || !pFiringPlayer->IsPlayerClass( TF_CLASS_MEDIC ) || pFiringPlayer->m_Shared.InCond( TF_COND_TAUNTING ) )
		return;

	// Add our targets
	// Loops through the healing targets, and make sure we have an effect for each of them
	if ( m_hHealingTarget )
	{
		if ( m_hHealingTargetEffect.pTarget == m_hHealingTarget )
			return;

		const char *pszFormat = IsReleasingCharge() ? "medicgun_beam_%s_invun" : "medicgun_beam_%s";
		const char *pszEffectName = ConstructTeamParticle( pszFormat, GetTeamNumber() );

		CNewParticleEffect *pEffect = pEffectOwner->ParticleProp()->Create( pszEffectName, PATTACH_POINT_FOLLOW, "muzzle" );
		if ( !m_hHealingTarget->IsCombatItem() )
			pEffectOwner->ParticleProp()->AddControlPoint( pEffect, 1, m_hHealingTarget, PATTACH_ABSORIGIN_FOLLOW, NULL, Vector(0,0,50) );
		else
			pEffectOwner->ParticleProp()->AddControlPoint( pEffect, 1, m_hHealingTarget, PATTACH_POINT_FOLLOW, "healbeam" );

		CEconItemDefinition *pStatic = GetItem()->GetStaticData();
		if ( pStatic )
		{
			PerTeamVisuals_t *pVisuals =	pStatic->GetVisuals( GetTeamNumber() );
			if ( pVisuals )
			{
				const char *pszCustomEffectName = pVisuals->custom_particlesystem;
				if ( pszCustomEffectName[0] != '\0' )
				{
					CNewParticleEffect *pCustomEffect = pEffectOwner->ParticleProp()->Create( pszCustomEffectName, PATTACH_POINT_FOLLOW, "muzzle" );
					if ( !m_hHealingTarget->IsCombatItem() )
						pEffectOwner->ParticleProp()->AddControlPoint( pCustomEffect, 1, m_hHealingTarget, PATTACH_ABSORIGIN_FOLLOW, NULL, Vector(0,0,50) );
					else
						pEffectOwner->ParticleProp()->AddControlPoint( pCustomEffect, 1, m_hHealingTarget, PATTACH_POINT_FOLLOW, "healbeam" );
 					m_hHealingTargetEffect.pCustomEffect = pCustomEffect;
				}
			}
		}

		m_hHealingTargetEffect.hOwner = pEffectOwner;
		m_hHealingTargetEffect.pTarget = m_hHealingTarget;
		m_hHealingTargetEffect.pEffect = pEffect;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CWeaponMedigun::GetEffectBarProgress( void )
{
	int nRageOnHeal = 0;
	CALL_ATTRIB_HOOK_INT( nRageOnHeal, generate_rage_on_heal );
	if ( nRageOnHeal )
		return GetEnergyLevel() / 100.0f;

	return 0.0f;
}





IMPLEMENT_NETWORKCLASS_ALIASED( TFMedigunShield, DT_TFMedigunShield )

BEGIN_NETWORK_TABLE( CTFMedigunShield, DT_TFMedigunShield )
END_NETWORK_TABLE()

// Server specific.
#ifdef GAME_DLL
BEGIN_DATADESC( CTFMedigunShield )
	DEFINE_THINKFUNC( ShieldThink ),
	DEFINE_ENTITYFUNC( ShieldTouch ),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( entity_medigun_shield, CTFMedigunShield );

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFMedigunShield::CTFMedigunShield()
{
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor.
//-----------------------------------------------------------------------------
CTFMedigunShield::~CTFMedigunShield()
{
#ifdef CLIENT_DLL
	if ( m_pShieldSound )
		CSoundEnvelopeController::GetController().SoundDestroy( m_pShieldSound );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::Spawn()
{
	Precache();

	SetModel( "models/props_mvm/mvm_player_shield.mdl" );

	BaseClass::Spawn();

	AddEffects( EF_NOSHADOW | EF_ITEM_BLINK );

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE ); // 
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	SetSolidFlags( FSOLID_TRIGGER /*| FSOLID_NOT_SOLID */ );
	SetBlocksLOS( false );

	m_takedamage = DAMAGE_EVENTS_ONLY;

#ifdef GAME_DLL
	SetTouch( &CTFMedigunShield::ShieldTouch );
	SetThink( &CTFMedigunShield::ShieldThink );
	SetNextThink( gpGlobals->curtime );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::Precache()
{
	BaseClass::Precache();

	PrecacheModel( "models/props_mvm/mvm_player_shield.mdl" );
	PrecacheModel( "models/props_mvm/mvm_player_shield2.mdl" );

	PrecacheScriptSound( "WeaponMedi_Shield.Protection" );
	PrecacheScriptSound( "WeaponMedi_Shield.Deploy" );
	PrecacheScriptSound( "WeaponMedi_Shield.Retract" );
	PrecacheScriptSound( "WeaponMedi_Shield.Burn" );
	PrecacheScriptSound( "WeaponMedi_Shield.Burn_lp" );

	PrecacheParticleSystem( "mvm_shield_impact" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFMedigunShield::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	switch (collisionGroup)
	{
	case TFCOLLISION_GROUP_ROCKETS:
	case COLLISION_GROUP_PROJECTILE:
	case COLLISION_GROUP_WEAPON:
		return true;
	}

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::UpdateShieldPosition( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	Vector vecForward;
	QAngle angAngs;

	angAngs = pOwner->GetAbsAngles();
	AngleVectors( angAngs, &vecForward );

	SetAbsOrigin( pOwner->GetAbsOrigin() + vecForward * 200 );
	SetAbsAngles( angAngs );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
unsigned int CTFMedigunShield::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::ClientThink( void )
{
	BaseClass::ClientThink();
}
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::ShieldThink( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	CWeaponMedigun *pMedigun = pOwner->GetMedigun();
	if ( pMedigun )
	{
		if ( pMedigun->IsEffectActive( EF_NODRAW ) )
		{
			if ( !IsEffectActive( EF_NODRAW ) )
				AddEffects( EF_NODRAW );
		}
		else
		{
			if ( IsEffectActive( EF_NODRAW ) )
			{
				RemoveEffects( EF_NODRAW );
				EmitSound( "WeaponMedi_Shield.Deploy" );
			}
		}

		if ( pMedigun->GetEnergyLevel() > 40.0f )
		{
			m_nRenderFX = kRenderFxNone;
		}
		else if ( pMedigun->GetEnergyLevel() == 40.0f )
		{
			m_nRenderFX = kRenderFxStrobeSlow;
			EmitSound( "WeaponMedi_Shield.Retract" );
		}
		else if ( pMedigun->GetEnergyLevel() < 40.0f )
		{
			m_nRenderFX = kRenderFxStrobeSlow;
		}

		if ( pMedigun->GetEnergyLevel() == 0.0f )
			UTIL_Remove( this );
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::StartTouch( CBaseEntity *pOther )
{
	Assert( pOther );

	if ( InSameTeam( pOther ) )
		return;

	if ( IsEffectActive( EF_NODRAW ) )
		return;

	/*if ( m_pShieldSound == NULL )
	{
		CPVSFilter filter( GetAbsOrigin() );
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		m_pShieldSound = controller.SoundCreate( filter, entindex(), "WeaponMedi_Shield.Burn_lp" );
		controller.Play( m_pShieldSound, 1.0, 100 );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::ShieldTouch( CBaseEntity *pOther )
{
	Assert( pOther );

	if ( InSameTeam( pOther ) )
		return;

	if ( IsEffectActive( EF_NODRAW ) )
		return;

	CBaseGrenade *pGrenade = dynamic_cast<CBaseGrenade *>( pOther );
	if ( pGrenade )
		pGrenade->ExplodeTouch( this );

	CTFBaseRocket *pRocket = dynamic_cast<CTFBaseRocket *>( pOther );
	if ( pRocket )
		pRocket->RocketTouch( this );

	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( pOwner )
	{
		CWeaponMedigun *pMedigun = pOwner->GetMedigun();
		if ( pMedigun )
		{
			CTakeDamageInfo info( this, pOwner, pMedigun, pOther->GetAbsOrigin(), pOther->GetAbsOrigin(), 2.0f, DMG_DISSOLVE, TF_DMG_CUSTOM_PLASMA );
			pOther->TakeDamage( info );
			EmitSound( "WeaponMedi_Shield.Burn" );
		}
	}
	else
	{
		CTakeDamageInfo info( this, this, vec3_origin, pOther->GetAbsOrigin(), 2.0f, DMG_DISSOLVE, TF_DMG_CUSTOM_PLASMA );
		pOther->TakeDamage( info );
		EmitSound( "WeaponMedi_Shield.Burn" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedigunShield::EndTouch( CBaseEntity *pOther )
{
	Assert( pOther );

	if ( InSameTeam( pOther ) )
		return;

	if ( IsEffectActive( EF_NODRAW ) )
		return;

	/*if ( m_pShieldSound != NULL )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pShieldSound );
		m_pShieldSound = NULL;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFMedigunShield::OnTakeDamage( const CTakeDamageInfo &info )
{
	if ( !IsEffectActive( EF_NODRAW ) )
	{
		DispatchParticleEffect( "mvm_shield_impact", info.GetDamagePosition(), GetAbsAngles(), NULL );
		EmitSound( "WeaponMedi_Shield.Protection" );
	}

	return BaseClass::OnTakeDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMedigunShield::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	if ( IsEffectActive( EF_NODRAW ) )
		return;

	if ( !IsEffectActive( EF_NODRAW ) )
	{
		DispatchParticleEffect( "mvm_shield_impact", info.GetDamagePosition(), GetAbsAngles(), NULL );
		EmitSound( "WeaponMedi_Shield.Protection" );
	}

	// something EnergyShieldImpact?
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFMedigunShield *CTFMedigunShield::Create( CTFPlayer *pOwner )
{
	CTFMedigunShield *pShield = static_cast<CTFMedigunShield *>( CreateEntityByName( "entity_medigun_shield" ) );
	if ( pShield )
	{
		UTIL_SetOrigin( pShield, pOwner->GetAbsOrigin() + Vector(0, 200, 0) );
		pShield->SetOwnerEntity( pOwner );
		pShield->ChangeTeam( pOwner->GetTeamNumber() );

		if ( pShield->GetTeamNumber() == TF_TEAM_BLUE )
			pShield->m_nSkin = 1;
		else
			pShield->m_nSkin = 0;

		CWeaponMedigun *pMedigun = pOwner->GetMedigun();
		if ( pMedigun )
		{
			int nRageOnHeal = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pMedigun, nRageOnHeal, generate_rage_on_heal );
			if ( nRageOnHeal >= 2 )
				pShield->SetModel( "models/props_mvm/mvm_player_shield2.mdl" );
		}

		pShield->UpdateShieldPosition();
		pShield->Spawn();
		DispatchSpawn( pShield );
		pShield->EmitSound( "WeaponMedi_Shield.Deploy" );
	}

	return pShield;
}

#endif