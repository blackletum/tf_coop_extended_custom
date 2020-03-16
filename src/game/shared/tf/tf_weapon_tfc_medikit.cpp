//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tf_weapon_tfc_medikit.h"
#include "tf_gamerules.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_ai_basenpc.h"
#else
#include "tf_player.h"
#include "ai_basenpc.h"
#include "ilagcompensationmanager.h"
#endif


// ----------------------------------------------------------------------------- //
// CTFCMedikit tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( TFCMedikit, DT_WeaponMedikit )

BEGIN_NETWORK_TABLE( CTFCMedikit, DT_WeaponMedikit )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFCMedikit )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_tfc_medikit, CTFCMedikit );
PRECACHE_WEAPON_REGISTER( tf_weapon_tfc_medikit );

// ----------------------------------------------------------------------------- //
// CTFCMedikit implementation.
// ----------------------------------------------------------------------------- //
CTFCMedikit::CTFCMedikit()
{
}

void CTFCMedikit::Smack( void )
{
	trace_t trace;

	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

#if GAME_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, LAG_COMPENSATE_BOUNDS );
#endif

	// We hit, setup the smack.
	if ( DoSwingTrace( trace ) )
	{
		CTFPlayer *pTarget = ToTFPlayer( trace.m_pEnt );
		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( trace.m_pEnt );

		// Hit sound - immediate.
		if( trace.m_pEnt->IsPlayer() || ( trace.m_pEnt->IsNPC() && !trace.m_pEnt->MyNPCPointer()->IsMech() ) )
		{
			WeaponSound( MELEE_HIT );
		}
		else
		{
			WeaponSound( MELEE_HIT_WORLD );
		}

		Vector vecForward; 
		AngleVectors( pPlayer->EyeAngles(), &vecForward );
		Vector vecSwingStart = pPlayer->Weapon_ShootPosition();
		Vector vecSwingEnd = vecSwingStart + vecForward * GetSwingRange();

		// Heal if we're on the same team, Infect if not
		if ( pPlayer->InSameTeam( trace.m_pEnt ) )
		{
			if ( trace.m_pEnt->IsPlayer() )
			{
				// Extinguish Flames
				if ( pTarget->m_Shared.InCond( TF_COND_BURNING ) )
					pTarget->m_Shared.RemoveCond( TF_COND_BURNING );

				// Heal Infection
				if ( pTarget->m_Shared.InCond( TF_COND_BLEEDING ) )
					pTarget->m_Shared.RemoveCond( TF_COND_BLEEDING );

#ifdef GAME_DLL
				// Heal
				if ( pTarget->GetHealth() < pTarget->GetMaxHealth() )
				{
					pTarget->TakeHealth( WEAP_MEDIKIT_HEAL, DMG_GENERIC );		// Heal fully in one hit
				}
				else if ( pTarget->GetHealth() < pTarget->GetMaxHealth() + WEAP_MEDIKIT_OVERHEAL )
				{
					// "Heal" over and above the player's max health
					pTarget->TakeHealth( 5, DMG_GENERIC | DMG_IGNORE_MAXHEALTH );
				}
#endif
			}
			else if ( trace.m_pEnt->IsNPC() )
			{
				// Extinguish Flames
				if ( pNPC->InCond( TF_COND_BURNING ) )
					pNPC->RemoveCond( TF_COND_BURNING );

				// Heal Infection
				if ( pNPC->InCond( TF_COND_BLEEDING ) )
					pNPC->RemoveCond( TF_COND_BLEEDING );

#ifdef GAME_DLL
				// Heal
				if ( pNPC->GetHealth() < pNPC->GetMaxHealth() )
				{
					pNPC->TakeHealth( WEAP_MEDIKIT_HEAL, DMG_GENERIC );		// Heal fully in one hit
				}
				else if ( pNPC->GetHealth() < pNPC->GetMaxHealth() + WEAP_MEDIKIT_OVERHEAL )
				{
					// "Heal" over and above the player's max health
					pNPC->TakeHealth( 5, DMG_GENERIC | DMG_IGNORE_MAXHEALTH );
				}
#endif
			}
		}
		else
		{
#ifdef GAME_DLL
			// Do Damage.
			int iCustomDamage = GetCustomDamageType();
			int iDmgType = DMG_NEVERGIB | DMG_CLUB;
			if ( IsCurrentAttackACrit() )
			{
				// TODO: Not removing the old critical path yet, but the new custom damage is marking criticals as well for melee now.
				iDmgType |= DMG_CRITICAL;
			}
			if ( IsCurrentAttackAMiniCrit() )
			{
				iDmgType |= DMG_MINICRITICAL;
			}
			
			float flDamage = GetMeleeDamage( trace.m_pEnt, iDmgType, iCustomDamage );
			CTakeDamageInfo info( pPlayer, pPlayer, this, flDamage, iDmgType, iCustomDamage );
			CalculateMeleeDamageForce( &info, vecForward, vecSwingEnd, 1.0f / flDamage * GetForceScale() );

			trace.m_pEnt->DispatchTraceAttack( info, vecForward, &trace ); 

			ApplyMultiDamage();

			OnEntityHit( trace.m_pEnt, &info );
#endif
			// Don't impact trace friendly players or objects
			if ( trace.m_pEnt && !trace.m_pEnt->InSameTeam( pPlayer ) )
			{
#ifdef CLIENT_DLL
				UTIL_ImpactTrace( &trace, DMG_CLUB );
#endif
				m_bConnected = true;
			}

			if ( trace.m_pEnt->IsPlayer() )
			{
				// Don't infect if the player's already infected, a medic, or we're still in prematch
				if ( pTarget->m_Shared.InCond( TF_COND_BLEEDING ) || ( pTarget->IsPlayerClass( TF_CLASS_MEDIC ) ) || TFGameRules()->IsInPreMatch() )
					return;

				// Infect
				pTarget->m_Shared.MakeBleed( pPlayer, this, 2.0f, 8 );
			}
			else if ( trace.m_pEnt->IsNPC() )
			{
				// Don't infect if the player's already infected, a medic, or we're still in prematch
				if ( pNPC->InCond( TF_COND_BLEEDING ) || TFGameRules()->IsInPreMatch() )
					return;

				// Infect
				pNPC->MakeBleed( pPlayer, this, 2.0f, 8 );
			}
		}
	}
	else
	{
#ifdef GAME_DLL
		// Do Damage.
		int iCustomDamage = GetCustomDamageType();
		int iDmgType = DMG_NEVERGIB | DMG_CLUB;
		if ( IsCurrentAttackACrit() )
		{
			// TODO: Not removing the old critical path yet, but the new custom damage is marking criticals as well for melee now.
			iDmgType |= DMG_CRITICAL;
		}
		if ( IsCurrentAttackAMiniCrit() )
		{
			iDmgType |= DMG_MINICRITICAL;
		}
		
		float flDamage = GetMeleeDamage( trace.m_pEnt, iDmgType, iCustomDamage );
		CTakeDamageInfo info( pPlayer, pPlayer, this, flDamage, iDmgType, iCustomDamage );
		int iHitSelfOnMiss = 0;
		CALL_ATTRIB_HOOK_INT( iHitSelfOnMiss, hit_self_on_miss );
		if ( iHitSelfOnMiss )
		{
			ApplyMultiDamage();
			pPlayer->TakeDamage( info );
		}
#endif
	}

#ifdef GAME_DLL
	lagcompensation->FinishLagCompensation( pPlayer );
#endif
}