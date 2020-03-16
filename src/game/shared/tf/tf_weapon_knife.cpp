//====== Copyright ? 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: Weapon Knife.
//
//=============================================================================
#include "cbase.h"
#include "tf_gamerules.h"
#include "tf_weapon_knife.h"
#include "decals.h"
#include "ai_basenpc_shared.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "ilagcompensationmanager.h"
#include "tf_player.h"
#include "tf_gamestats.h"
#include "npc_antlion.h"
#endif

//=============================================================================
//
// Weapon Knife tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFKnife, DT_TFWeaponKnife )

BEGIN_NETWORK_TABLE( CTFKnife, DT_TFWeaponKnife )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bReadyToBackstab ) ),
#else
	SendPropBool( SENDINFO( m_bReadyToBackstab ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFKnife )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_bReadyToBackstab, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_knife, CTFKnife );
PRECACHE_WEAPON_REGISTER( tf_weapon_knife );

//=============================================================================
//
// Weapon Knife functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFKnife::CTFKnife()
{
	m_flKnifeRegenTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Change idle anim to raised if we're ready to backstab.
//-----------------------------------------------------------------------------
bool CTFKnife::Deploy( void )
{
	if ( BaseClass::Deploy() )
	{
		m_bReadyToBackstab = false;
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFKnife::ItemPostFrame( void )
{
	BackstabVMThink();

	if ( m_flEffectBarRegenTime >= m_flKnifeRegenTime )
		m_flEffectBarRegenTime = m_flKnifeRegenTime;

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFKnife::ItemBusyFrame( void )
{
	if ( m_flEffectBarRegenTime >= m_flKnifeRegenTime )
		m_flEffectBarRegenTime = m_flKnifeRegenTime;

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFKnife::ItemHolsterFrame( void )
{
	if ( m_flEffectBarRegenTime >= m_flKnifeRegenTime )
		m_flEffectBarRegenTime = m_flKnifeRegenTime;

	BaseClass::ItemHolsterFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Set stealth attack bool
//-----------------------------------------------------------------------------
void CTFKnife::PrimaryAttack( void )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CanAttack() )
		return;

	// Set the weapon usage mode - primary, secondary.
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

#ifdef GAME_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, LAG_COMPENSATE_BOUNDS );
#endif

	trace_t trace;
	if ( DoSwingTrace( trace ) == true )
	{
		// we will hit something with the attack
		if( trace.m_pEnt && ( trace.m_pEnt->IsPlayer() || trace.m_pEnt->IsNPC() ) )
		{
			CBaseCombatCharacter *pTarget = trace.m_pEnt->MyCombatCharacterPointer();

			if ( pTarget && pTarget->GetTeamNumber() != pPlayer->GetTeamNumber() )
			{
				// Deal extra damage to players when stabbing them from behind
				if ( IsBehindAndFacingTarget( pTarget ) )
				{
					// this will be a backstab, do the strong anim
					m_iWeaponMode = TF_WEAPON_SECONDARY_MODE;

					// store the victim to compare when we do the damage
					m_hBackstabVictim = pTarget;
				}
			}
		}
	}

#ifdef GAME_DLL
	pPlayer->RemoveInvisibility();
	pPlayer->RemoveDisguise();

	lagcompensation->FinishLagCompensation( pPlayer );

	// Reset "backstab ready" state after each attack.
	m_bReadyToBackstab = false;
#endif

	// Swing the weapon.
	Swing( pPlayer );
	
	// And hit instantly.
	Smack();
	m_flSmackTime = -1.0f;

#ifdef GAME_DLL
	pPlayer->SpeakWeaponFire();
	CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACritical() );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
bool CTFKnife::CanDeploy( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return false;

	if ( ( m_flKnifeRegenTime > 0.0f ) && GetEffectBarProgress() < m_flKnifeRegenTime )
		return false;

	return true;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
void CTFKnife::ApplyOnInjuredAttributes( CBaseEntity* pVictim, CTFPlayer* pAttacker, const CTakeDamageInfo &info )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( pOwner != pVictim )
		return;

	float flMeltInFire = 0.0f;
	CALL_ATTRIB_HOOK_FLOAT( flMeltInFire, melts_in_fire );
	if ( ( flMeltInFire > 0.0f ) && info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING )
	{
		m_flKnifeRegenTime = flMeltInFire;
 		m_flEffectBarRegenTime = gpGlobals->curtime + flMeltInFire;
		pOwner->EmitSound( "Icicle.Melt" );
		pOwner->Weapon_Switch( pOwner->Weapon_GetSlot( LOADOUT_POSITION_SECONDARY ) );
	}

	BaseClass::ApplyOnInjuredAttributes( pVictim, pAttacker, info );
}

//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
void CTFKnife::OnPlayerKill( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

	if ( !pVictim )
		return;

	if ( info.GetDamageCustom() != TF_DMG_CUSTOM_BACKSTAB )
		return;

	int nDisguiseOnBackstab = 0;
	CALL_ATTRIB_HOOK_INT( nDisguiseOnBackstab, set_disguise_on_backstab );
	if ( nDisguiseOnBackstab != 0 )
	{
		if ( pPlayer->HasTheFlag() )
		{
			pPlayer->RemoveDisguise();
			return;
		}

		//SetContextThink( &CTFKnife::DisguiseOnKill, 0.2f, "DisguiseOnKill" );
	}

	int nSanguisuge = 0;
	CALL_ATTRIB_HOOK_INT( nSanguisuge, sanguisuge );
	if ( nSanguisuge != 0 )
	{
		int iHealthToAdd = pVictim->GetMaxHealth() * 1.1;
		int iBoostMax = pPlayer->m_Shared.GetMaxBuffedHealth();
		iHealthToAdd = clamp( iHealthToAdd, 0, iBoostMax - pPlayer->GetHealth() );
		if ( pPlayer->TakeHealth( iHealthToAdd, DMG_IGNORE_MAXHEALTH ) )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
			if ( event )
			{
				event->SetInt( "amount", iHealthToAdd );
				event->SetInt( "entindex", pPlayer->entindex() );

				gameeventmanager->FireEvent( event );
			}
			pPlayer->m_Shared.HealthKitPickupEffects( iHealthToAdd );
		}
	}

	BaseClass::OnPlayerKill( pVictim, info );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Do backstab damage
//-----------------------------------------------------------------------------
float CTFKnife::GetMeleeDamage( CBaseEntity *pTarget, int &iDamageType, int &iCustomDamage )
{
	float flBaseDamage = BaseClass::GetMeleeDamage( pTarget, iDamageType, iCustomDamage );

	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( pOwner && pTarget && ( pTarget->IsPlayer() || pTarget->IsNPC() ) )
	{
		// Since Swing and Smack are done in the same frame now we don't need to run additional checks anymore.
		if ( m_iWeaponMode == TF_WEAPON_SECONDARY_MODE && m_hBackstabVictim.Get() == pTarget )
		{
			// this will be a backstab, do the strong anim.
			// Do twice the target's health so that random modification will still kill him.
			flBaseDamage = 1000;

			// Declare a backstab.
			iCustomDamage = TF_DMG_CUSTOM_BACKSTAB;
		}
	}

	return flBaseDamage;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFKnife::IsBehindAndFacingTarget( CBaseEntity *pTarget )
{
	Assert( pTarget );

	// Only certain NPCs can be backstabbed
	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pTarget );
	if ( pNPC )
	{
#ifdef GAME_DLL
		CNPC_Antlion *pAntlion = dynamic_cast<CNPC_Antlion *>( pNPC );
		if ( pAntlion && pAntlion->IsFlipped() )
			return true;
#endif

		if ( !pNPC->AllowBackstab() )
			return false;
	}

	// Get the forward view vector of the target, ignore Z
	Vector vecVictimForward;
	AngleVectors( pTarget->EyeAngles(), &vecVictimForward );
	vecVictimForward.z = 0.0f;
	vecVictimForward.NormalizeInPlace();

	// Get a vector from my origin to my targets origin
	Vector vecToTarget;
	vecToTarget = pTarget->WorldSpaceCenter() - GetOwner()->WorldSpaceCenter();
	vecToTarget.z = 0.0f;
	vecToTarget.NormalizeInPlace();

	// Get a forward vector of the attacker.
	Vector vecOwnerForward;
	AngleVectors( GetOwner()->EyeAngles(), &vecOwnerForward );
	vecOwnerForward.z = 0.0f;
	vecOwnerForward.NormalizeInPlace();

	float flDotOwner = DotProduct( vecOwnerForward, vecToTarget );
	float flDotVictim = DotProduct( vecVictimForward, vecToTarget );

	// Make sure they're actually facing the target.
	// This needs to be done because lag compensation can place target slightly behind the attacker.
	if ( flDotOwner > 0.5 )
		return ( flDotVictim > -0.1 );

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFKnife::CalcIsAttackCriticalHelper( void )
{
	// Always crit from behind, never from front
	return ( m_iWeaponMode == TF_WEAPON_SECONDARY_MODE );
}

//-----------------------------------------------------------------------------
// Purpose: Allow melee weapons to send different anim events
// Input  :  - 
//-----------------------------------------------------------------------------
void CTFKnife::SendPlayerAnimEvent( CTFPlayer *pPlayer )
{
	if ( m_iWeaponMode == TF_WEAPON_SECONDARY_MODE )
	{
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, ACT_MP_ATTACK_STAND_SECONDARYFIRE );
	}
	else
	{
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFKnife::DoViewModelAnimation( void )
{
	// Overriding so it doesn't do backstab animation on crit.
	Activity act = ( m_iWeaponMode == TF_WEAPON_PRIMARY_MODE ) ? ACT_VM_HITCENTER : ACT_VM_SWINGHARD;

	SendWeaponAnim( act );
}

//-----------------------------------------------------------------------------
// Purpose: Change idle anim to raised if we're ready to backstab.
//-----------------------------------------------------------------------------
bool CTFKnife::SendWeaponAnim( int iActivity )
{
	switch( iActivity )
	{
	case ACT_VM_IDLE:
	case ACT_MELEE_VM_IDLE:
	case ACT_ITEM1_VM_IDLE:
	case ACT_ITEM2_VM_IDLE:
		if ( m_bReadyToBackstab )
			iActivity = ACT_BACKSTAB_VM_IDLE;
		break;
	case ACT_BACKSTAB_VM_UP:
	case ACT_ITEM1_BACKSTAB_VM_UP:
	case ACT_ITEM2_BACKSTAB_VM_UP:
		m_bReadyToBackstab = true;
		break;
	case ACT_BACKSTAB_VM_DOWN:
	case ACT_ITEM1_BACKSTAB_VM_DOWN:
	case ACT_ITEM2_BACKSTAB_VM_DOWN:
	default:
		m_bReadyToBackstab = false;
		break;
	}
	return BaseClass::SendWeaponAnim( iActivity );
}

//-----------------------------------------------------------------------------
// Purpose: Check for knife raise conditions.
//-----------------------------------------------------------------------------
void CTFKnife::BackstabVMThink( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;
	if ( GetActivity() == ACT_VM_IDLE ||
		GetActivity() == ACT_MELEE_VM_IDLE ||
		GetActivity() == ACT_BACKSTAB_VM_IDLE ||
		GetActivity() == ACT_ITEM1_VM_IDLE  ||
		GetActivity() == ACT_ITEM1_BACKSTAB_VM_IDLE ||
		GetActivity() == ACT_ITEM2_VM_IDLE  ||
		GetActivity() == ACT_ITEM2_BACKSTAB_VM_IDLE )
	{
		trace_t tr;
		if ( CanAttack() && DoSwingTrace( tr ) &&
			( tr.m_pEnt->IsPlayer() || tr.m_pEnt->IsNPC() ) && tr.m_pEnt->GetTeamNumber() != pOwner->GetTeamNumber() &&
			IsBehindAndFacingTarget( tr.m_pEnt ) )
		{
			if ( !m_bReadyToBackstab )
			{
				m_bReadyToBackstab = true;
				SendWeaponAnim( ACT_BACKSTAB_VM_UP );
			}
		}
		else
		{
			if ( m_bReadyToBackstab )
			{
				m_bReadyToBackstab = false;
				SendWeaponAnim( ACT_BACKSTAB_VM_DOWN );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFKnife::DisguiseOnKill( void )
{
	if ( !m_hBackstabVictim )
		return;

	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return;

	int iTeamNum = m_hBackstabVictim->GetTeamNumber();

	int iClassIdx = TF_CLASS_SCOUT;
	if ( m_hBackstabVictim->IsPlayer() )
	{
		CTFPlayer *pTFVictim = ToTFPlayer( m_hBackstabVictim );
		if ( m_hBackstabVictim )
			iClassIdx = pTFVictim->GetPlayerClass()->GetClassIndex();
	}

	if ( m_hBackstabVictim->IsNPC() )
	{
		#ifdef GAME_DLL
		if ( m_hBackstabVictim->ClassMatches( "npc_combine_s" ) || m_hBackstabVictim->ClassMatches( "npc_metropolice" ) )
			iClassIdx = TF_CLASS_COMBINE;
		if ( m_hBackstabVictim->ClassMatches( "npc_zombie" ) || m_hBackstabVictim->ClassMatches( "npc_poisonzombie" ) || m_hBackstabVictim->ClassMatches( "npc_zombine" ) || m_hBackstabVictim->ClassMatches( "npc_fastzombie" ) )
			iClassIdx = TF_CLASS_ZOMBIEFAST;
		if ( m_hBackstabVictim->ClassMatches( "npc_antlion" ) )
			iClassIdx = TF_CLASS_ANTLION;
		#endif
	}

	pOwner->m_Shared.Disguise( iTeamNum, iClassIdx, m_hBackstabVictim );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFKnife::BackstabBlocked( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	m_flNextPrimaryAttack = gpGlobals->curtime + 2.0f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 2.0f;

	SendWeaponAnim( ACT_MELEE_VM_PULLBACK );
}


CREATE_SIMPLE_WEAPON_TABLE( TFCKnife, tf_weapon_tfc_knife )