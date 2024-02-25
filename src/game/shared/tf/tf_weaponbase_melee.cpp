//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weaponbase_melee.h"
#include "effect_dispatch_data.h"
#include "tf_gamerules.h"
#include "ai_basenpc_shared.h"
#include "baseobject_shared.h"
#include "ihasbuildpoints.h"
#include "particle_parse.h"

// Server specific.
#if !defined( CLIENT_DLL )
#include "tf_player.h"
#include "tf_gamestats.h"
#include "tf_obj.h"
#include "ilagcompensationmanager.h"
// Client specific.
#else
#include "c_tf_player.h"
#endif

//=============================================================================
//
// TFWeaponBase Melee tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFWeaponBaseMelee, DT_TFWeaponBaseMelee )

BEGIN_NETWORK_TABLE( CTFWeaponBaseMelee, DT_TFWeaponBaseMelee )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFWeaponBaseMelee )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weaponbase_melee, CTFWeaponBaseMelee );

// Server specific.
#if !defined( CLIENT_DLL ) 
BEGIN_DATADESC( CTFWeaponBaseMelee )
	DEFINE_THINKFUNC( Smack )
END_DATADESC()
#endif

#ifdef GAME_DLL
ConVar tf_meleeattackforcescale( "tf_meleeattackforcescale", "80.0", FCVAR_CHEAT | FCVAR_GAMEDLL | FCVAR_DEVELOPMENTONLY );
extern ConVar tf_debug_damage;
#endif

ConVar tf_weapon_criticals_melee( "tf_weapon_criticals_melee", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Controls random crits for melee weapons.\n0 - Melee weapons do not randomly crit. \n1 - Melee weapons can randomly crit only if tf_weapon_criticals is also enabled. \n2 - Melee weapons can always randomly crit regardless of the tf_weapon_criticals setting.", true, 0, true, 2 );
extern ConVar tf_weapon_criticals;
extern ConVar tf_debug_criticals;

//=============================================================================
//
// TFWeaponBase Melee functions.
//

// -----------------------------------------------------------------------------
// Purpose: Constructor.
// -----------------------------------------------------------------------------
CTFWeaponBaseMelee::CTFWeaponBaseMelee()
{
	WeaponReset();
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
void CTFWeaponBaseMelee::WeaponReset( void )
{
	BaseClass::WeaponReset();

	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;
	m_flSmackTime = -1.0f;
	m_bConnected = false;
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
void CTFWeaponBaseMelee::Precache()
{
	PrecacheScriptSound( "Powerup.Knockout_Melee_Hit" );
	PrecacheParticleSystem( "taunt_scout_bat_trail" );

	BaseClass::Precache();
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
void CTFWeaponBaseMelee::Spawn()
{
	Precache();

	// Get the weapon information.
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( GetClassname() );
	Assert( hWpnInfo != GetInvalidWeaponInfoHandle() );
	CTFWeaponInfo *pWeaponInfo = dynamic_cast< CTFWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );
	Assert( pWeaponInfo && "Failed to get CTFWeaponInfo in melee weapon spawn" );
	m_pWeaponInfo = pWeaponInfo;
	Assert( m_pWeaponInfo );

	// No ammo.
	m_iClip1 = -1;

	BaseClass::Spawn();
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
bool CTFWeaponBaseMelee::CanHolster( void ) const
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && ( pOwner->m_Shared.InCond( TF_COND_CANNOT_SWITCH_FROM_MELEE ) || pOwner->m_Shared.InCond( TF_COND_RUNE_KNOCKOUT ) ) )
		return false;

	return BaseClass::CanHolster();
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
bool CTFWeaponBaseMelee::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_flSmackTime = -1.0f;
	if ( GetPlayerOwner() )
	{
		GetPlayerOwner()->m_flNextAttack = gpGlobals->curtime + 0.5;
		GetTFPlayerOwner()->m_Shared.SetNextMeleeCrit( kCritType_None );
	}

	return BaseClass::Holster( pSwitchingTo );
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
void CTFWeaponBaseMelee::PrimaryAttack()
{
	// Get the current player.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CanAttack() )
		return;

	// Set the weapon usage mode - primary, secondary.
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;
	m_bConnected = false;

#ifdef GAME_DLL
	pPlayer->EndClassSpecialSkill();
#endif

	// Swing the weapon.
	Swing( pPlayer );

	m_bCurrentAttackIsMiniCrit = pPlayer->m_Shared.GetNextMeleeCrit() != kCritType_None;

#if !defined( CLIENT_DLL ) 
	pPlayer->RemoveInvisibility();
	pPlayer->RemoveDisguise();

	pPlayer->SpeakWeaponFire();
	CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACritical() );
#endif
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
void CTFWeaponBaseMelee::SecondaryAttack()
{
	// semi-auto behaviour
	if ( m_bInAttack2 )
		return;

	// Get the current player.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, epicsword );

	pPlayer->DoClassSpecialSkill();

	#ifdef GAME_DLL
	if ( iType == 1 && !pPlayer->m_Shared.InCond( TF_COND_ROCKETPACK ) )
	{
		Vector vecPushDir = pPlayer->EyeDirection3D();
		QAngle angPushDir = pPlayer->EyeAngles();
		angPushDir[PITCH] = min( -25, angPushDir[PITCH] );
		AngleVectors( angPushDir, &vecPushDir );
		pPlayer->SetGroundEntity( NULL );
		pPlayer->SetAbsVelocity( vecPushDir * 800 );
		pPlayer->m_Shared.AddCond( TF_COND_SHIELD_CHARGE );
		pPlayer->m_Shared.AddCond( TF_COND_ROCKETPACK );
		//EmitSound( "InfinityBlade.Jump" );
	}
	#endif

	m_bInAttack2 = true;

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CTFWeaponBaseMelee::Swing( CTFPlayer *pPlayer )
{
	CalcIsAttackCritical();
	CalcIsAttackMiniCritical();

	// Play the melee swing and miss (whoosh) always.
	SendPlayerAnimEvent( pPlayer );

	DoViewModelAnimation();

	// Set next attack times.
	float flFireDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
	CALL_ATTRIB_HOOK_FLOAT( flFireDelay, mult_postfiredelay );

	if ( pPlayer->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		flFireDelay /= 2;

	if ( pPlayer->m_Shared.InCond( TF_COND_HALLOWEEN_SPEED_BOOST ) )
		flFireDelay /= 1.5;

	m_flNextPrimaryAttack = gpGlobals->curtime + flFireDelay;

	SetWeaponIdleTime( m_flNextPrimaryAttack + m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeIdleEmpty );

	if ( IsCurrentAttackACrit() )
	{
		WeaponSound( BURST );
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}

	m_flSmackTime = gpGlobals->curtime + m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flSmackDelay;
	CALL_ATTRIB_HOOK_FLOAT(m_flSmackTime, mult_smackdelay);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseMelee::DoViewModelAnimation( void )
{
	Activity act = ( m_iWeaponMode == TF_WEAPON_PRIMARY_MODE ) ? ACT_VM_HITCENTER : ACT_VM_SWINGHARD;

	if ( IsCurrentAttackACritical() )
		act = ACT_VM_SWINGHARD;

	if ( IsCurrentAttackAMiniCrit() )
		act = ACT_VM_SWINGHARD;

	SendWeaponAnim( act );

#ifdef CLIENT_DLL
	C_TFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer )
	{
		if ( !m_pSwingEffect && !pPlayer->IsLocalPlayer() )
			m_pSwingEffect = pPlayer->ParticleProp()->Create( "taunt_scout_bat_trail", PATTACH_POINT_FOLLOW, "effect_hand_R" );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Allow melee weapons to send different anim events
// Input  :  - 
//-----------------------------------------------------------------------------
void CTFWeaponBaseMelee::SendPlayerAnimEvent( CTFPlayer *pPlayer )
{
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFWeaponBaseMelee::GetSwingRange( void ) const
{
	CBasePlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer == nullptr )
		return 48;

	float flSwingRangeMult = 1.0f;
	if ( pPlayer->GetModelScale() > 1.0f )
		flSwingRangeMult *= pPlayer->GetModelScale();

	CALL_ATTRIB_HOOK_FLOAT( flSwingRangeMult, melee_range_multiplier );
	return 48 * flSwingRangeMult;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseMelee::ItemPostFrame()
{
	// Check for smack.
	if ( m_flSmackTime > 0.0f && gpGlobals->curtime > m_flSmackTime )
	{
		Smack();
		m_flSmackTime = -1.0f;
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFWeaponBaseMelee::DoSwingTrace( trace_t &tr )
{
	return DoSwingTraceInternal( tr, false, NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFWeaponBaseMelee::DoSwingTraceInternal( trace_t &trace, bool bCleave, MeleePartitionVector *enumResults )
{
	// Setup a volume for the melee weapon to be swung - approx size, so all melee behave the same.
	static Vector vecSwingMins( -18, -18, -18 );
	static Vector vecSwingMaxs( 18, 18, 18 );

	float flBoundsMult = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flBoundsMult, melee_bounds_multiplier );

	// Get the current player.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return false;

	// Setup the swing range.
	Vector vecForward; 
	AngleVectors( pPlayer->EyeAngles(), &vecForward );
	Vector vecSwingStart = pPlayer->Weapon_ShootPosition();
	Vector vecSwingEnd = vecSwingStart + vecForward * GetSwingRange();
	CTraceFilterIgnoreTeammates filterFriendlies( this, COLLISION_GROUP_NONE, GetTeamNumber() );

	if ( bCleave )
	{
		Ray_t ray; CBaseEntity *pList[256];
		ray.Init( vecSwingStart, vecSwingEnd, vecSwingMins * flBoundsMult, vecSwingMaxs * flBoundsMult );

		int nCount = UTIL_EntitiesAlongRay( pList, sizeof pList, ray, FL_CLIENT|FL_OBJECT );
		if ( nCount > 0 )
		{
			int nHit = 0;
			for ( int i=0; i < nCount; ++i )
			{
				if ( pList[ i ] == pPlayer )
					continue;

				if ( pList[ i ]->GetTeamNumber() == pPlayer->GetTeamNumber() )
					continue;

				if ( enumResults )
				{
					trace_t trace;
					UTIL_TraceModel( vecSwingStart, vecSwingEnd, 
									 vecSwingMins * flBoundsMult, 
									 vecSwingMaxs * flBoundsMult, 
									 pList[i], COLLISION_GROUP_NONE, &trace );

					enumResults->Element( enumResults->AddToTail() ) = trace;
				}
			}

			return nHit > 0;
		}

		return false;
	}
	else
	{
		Ray_t ray;
		ray.Init( vecSwingStart, vecSwingEnd  );

		// This takes priority over anything else we hit
		int nDamagesSappers = 0;
		CALL_ATTRIB_HOOK_INT( nDamagesSappers, set_dmg_apply_to_sapper );
		if ( nDamagesSappers != 0 )
		{
			CTraceFilterIgnorePlayers filterPlayers( NULL, COLLISION_GROUP_NONE );
			UTIL_Portal_TraceRay( ray, MASK_SOLID, &filterPlayers, &trace );
			if ( trace.fraction >= 1.0 )
				UTIL_TraceHull( vecSwingStart, vecSwingEnd, vecSwingMins * flBoundsMult, vecSwingMaxs * flBoundsMult, MASK_SOLID, &filterPlayers, &trace );

			// Ensure valid target
			if ( trace.m_pEnt )
			{
				if ( trace.m_pEnt->IsBaseObject() )
				{
					CBaseObject *pObject = static_cast<CBaseObject *>( trace.m_pEnt );
					if ( pPlayer->InSameTeam( pObject ) && pObject->HasSapper() )
						return ( trace.fraction < 1.0 );
				}
				else if ( trace.m_pEnt->IsNPC() )
				{
					CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( trace.m_pEnt );
					if ( pNPC && pNPC->HasSapper() && pNPC->InSameTeam( pPlayer ) )
					{
						pNPC->RemoveCond( TF_COND_SAPPED );
						return ( trace.fraction < 1.0 );
					}
				}
			}
		}

		// See if we hit anything.
		CTraceFilterIgnoreFriendlyCombatItems filterCombatItem( this, COLLISION_GROUP_NONE, pPlayer->GetTeamNumber() );
		UTIL_Portal_TraceRay( ray, MASK_SOLID, &filterCombatItem, &trace );
		if ( trace.fraction >= 1.0 )
		{
			UTIL_TraceHull( vecSwingStart, vecSwingEnd, vecSwingMins * flBoundsMult, vecSwingMaxs * flBoundsMult, MASK_SOLID, &filterCombatItem, &trace );
			if ( trace.fraction < 1.0 )
			{
				// Calculate the point of intersection of the line (or hull) and the object we hit
				// This is and approximation of the "best" intersection
				CBaseEntity *pHit = trace.m_pEnt;
				if ( !pHit || pHit->IsBSPModel() )
				{
					// Why duck hull min/max?
					FindHullIntersection( vecSwingStart, trace, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, pPlayer );
				}

				// This is the point on the actual surface (the hull could have hit space)
				vecSwingEnd = trace.endpos;	
			}
		}

		return ( trace.fraction < 1.0f );
	}
}

// -----------------------------------------------------------------------------
// Purpose:
// Note: Think function to delay the impact decal until the animation is finished 
//       playing.
// -----------------------------------------------------------------------------
void CTFWeaponBaseMelee::Smack( void )
{
	CUtlVector<trace_t> results;
	trace_t trace;

	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer == nullptr )
		return;

#if !defined (CLIENT_DLL)
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, LAG_COMPENSATE_BOUNDS );
#endif

	int nMeleeCleaves = 0;
	CALL_ATTRIB_HOOK_INT( nMeleeCleaves, melee_cleave_attack );

	// We hit, setup the smack.
	if ( DoSwingTraceInternal( trace, nMeleeCleaves > 0, &results ) )
	{
		if ( nMeleeCleaves )
		{
			for ( int i=0; i<results.Count(); ++i )
				OnSwingHit( results[i] );
		}
		else
		{
			OnSwingHit( trace );
		}
	}
	else
	{
		int nSelfHarm = 0;
		CALL_ATTRIB_HOOK_INT( nSelfHarm, hit_self_on_miss );
		if ( nSelfHarm != 0 ) // Hit ourselves, dummy!
		{
			// Do Damage.
			DoMeleeDamage( pPlayer, trace );
		}
	}

#ifdef CLIENT_DLL
	if ( m_pSwingEffect )
	{
		ParticleProp()->StopEmission( m_pSwingEffect );
		m_pSwingEffect = NULL;
	}
#else
	lagcompensation->FinishLagCompensation( pPlayer );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
void CTFWeaponBaseMelee::DoMeleeDamage( CBaseEntity *pTarget, trace_t &trace )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	Vector vecForward; 
	AngleVectors( pPlayer->EyeAngles(), &vecForward );
	Vector vecSwingStart = pPlayer->Weapon_ShootPosition();
	Vector vecSwingEnd = vecSwingStart + vecForward * GetSwingRange();

#if !defined( CLIENT_DLL )
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

	float flDamage = GetMeleeDamage( pTarget, iDmgType, iCustomDamage );

	CTakeDamageInfo info( pPlayer, pPlayer, this, flDamage, iDmgType, iCustomDamage );

	if ( pTarget == pPlayer )
		info.SetDamageForce( vec3_origin );
	else
		CalculateMeleeDamageForce( &info, vecForward, vecSwingEnd, 1.0f / flDamage * GetForceScale() );

	pTarget->DispatchTraceAttack( info, vecForward, &trace ); 
	ApplyMultiDamage();

	OnEntityHit( pTarget, &info );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CTFWeaponBaseMelee::GetMeleeDamage( CBaseEntity *pTarget, int &iDamageTyoe, int &iCustomDamage )
{
	float flDamage = (float)m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_nDamage;

	CALL_ATTRIB_HOOK_FLOAT( flDamage, mult_dmg );

	if ( pTarget->IsBaseObject() || ( pTarget->IsNPC() && pTarget->MyNPCPointer()->IsMech() ) )
		CALL_ATTRIB_HOOK_FLOAT( flDamage, mult_dmg_vs_buildings );

	if ( pTarget->IsPlayer() || pTarget->IsNPC() )
		CALL_ATTRIB_HOOK_FLOAT( flDamage, mult_dmg_vs_players );

	CTFPlayer *pOwner = ToTFPlayer( GetPlayerOwner() );
	if ( pOwner )
	{
		if ( pOwner->m_Shared.InCond( TF_COND_RUNE_STRENGTH ) )
			flDamage *= 2;

		float flHealthMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pOwner, flHealthMult, mult_dmg_with_reduced_health );
		if ( flHealthMult != 1.0f )
		{
			float flFraction = pOwner->HealthFraction();
			flDamage *= RemapValClamped( flFraction, 0.0f, 1.0f, flHealthMult, 1.0f );
		}
	}

	return flDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWeaponBaseMelee::OnEntityHit( CBaseEntity *pEntity, CTakeDamageInfo *pInfo )
{
	CTFPlayer *pOwner = ToTFPlayer( GetPlayerOwner() );
	if ( !pOwner )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( pEntity );
	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pEntity );

	int nSpeedBuff = 0;
	CALL_ATTRIB_HOOK_INT( nSpeedBuff, speed_buff_ally );
	if ( nSpeedBuff )
	{
		if ( pEntity->IsPlayer() )
		{
			if ( ( !pPlayer->m_Shared.InCond( TF_COND_STEALTHED ) ) && ( pPlayer->InSameTeam( pOwner ) || ( ( pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) ) && pPlayer->m_Shared.GetDisguiseTeam() == pOwner->GetTeamNumber() ) ) )
			{
				pPlayer->m_Shared.AddCond( TF_COND_SPEED_BOOST, 2 );
				pOwner->m_Shared.AddCond( TF_COND_SPEED_BOOST, 3.5 );
			}
		}
		else if ( pEntity->IsNPC() && pEntity->InSameTeam( pOwner ) )
		{
			pNPC->AddCond( TF_COND_SPEED_BOOST, 2 );
			pOwner->m_Shared.AddCond( TF_COND_SPEED_BOOST, 3.5 );
		}
	}

	int nMiniCritAndConsumesBurning = 0;
	CALL_ATTRIB_HOOK_INT( nMiniCritAndConsumesBurning, attack_minicrits_and_consumes_burning );
	if ( nMiniCritAndConsumesBurning )
	{
		if ( !pEntity->InSameTeam( pOwner ) )
		{
			if ( pEntity->IsPlayer() && pPlayer->m_Shared.InCond( TF_COND_BURNING ) )
			{
				#ifdef GAME_DLL
				pInfo->AddDamageType( DMG_MINICRITICAL );
				#endif
				pPlayer->m_Shared.RemoveCond( TF_COND_BURNING );
				pOwner->m_Shared.AddCond( TF_COND_SPEED_BOOST, 4 );
			}
			else if ( pEntity->IsNPC() && pNPC->InCond( TF_COND_BURNING ) )
			{
				#ifdef GAME_DLL
				pInfo->AddDamageType( DMG_MINICRITICAL );
				#endif
				pNPC->RemoveCond( TF_COND_BURNING );
				pOwner->m_Shared.AddCond( TF_COND_SPEED_BOOST, 4 );
			}
		}
	}

	if ( pOwner->m_Shared.InCond( TF_COND_RUNE_KNOCKOUT ) && !pEntity->InSameTeam( pOwner ) )
	{
		Vector vecDir;
		QAngle angDir = pOwner->EyeAngles();
		AngleVectors( angDir, &vecDir );
		QAngle angPushDir = angDir;

		// Push them at least airblast degrees up.
		angPushDir[PITCH] = min( -45, angPushDir[PITCH] );

		AngleVectors( angPushDir, &vecDir );

		Vector vecVictimDir = pEntity->WorldSpaceCenter() - pOwner->WorldSpaceCenter();

		Vector vecVictimDir2D( vecVictimDir.x, vecVictimDir.y, 0.0f );
		VectorNormalize( vecVictimDir2D );

		Vector vecDir2D( vecDir.x, vecDir.y, 0.0f );
		VectorNormalize( vecDir2D );

		float flKnockbackMult = 500;

		float flDot = DotProduct( vecDir2D, vecVictimDir2D );
		if ( flDot >= 0.8 )
		{
			if ( pEntity->IsPlayer() )
			{
				pPlayer->SetGroundEntity( NULL );
				pPlayer->ApplyAbsVelocityImpulse( vecDir * flKnockbackMult );
				EmitSound( "Powerup.Knockout_Melee_Hit" );
			}
			else if ( pEntity->IsNPC() )
			{
				pNPC->SetGroundEntity( NULL );
				pNPC->ApplyAbsVelocityImpulse( vecDir * flKnockbackMult );
				EmitSound( "Powerup.Knockout_Melee_Hit" );
			}
		}
	}

#ifdef GAME_DLL
	if ( pOwner && TFGameRules()->GetIT() && pEntity && !pEntity->InSameTeam( pOwner ) && !pEntity->ClassMatches( "headless_hatman" ) )
	{
		if ( TFGameRules()->GetIT() == pOwner )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "tagged_player_as_it" );
			if (event)
			{
				event->SetInt( "player", engine->GetPlayerUserId( pOwner->edict() ) );

				gameeventmanager->FireEvent( event );
			}

			if ( pPlayer )
				UTIL_ClientPrintAll( HUD_PRINTTALK, "#TF_HALLOWEEN_BOSS_ANNOUNCE_TAG", pOwner->GetPlayerName(), pPlayer->GetPlayerName() );
			else if ( pNPC )
				UTIL_ClientPrintAll( HUD_PRINTTALK, "#TF_HALLOWEEN_BOSS_ANNOUNCE_TAG", pOwner->GetPlayerName(), pNPC->GetLocalizeName() );

			CSingleUserRecipientFilter filter( pOwner );
			CBaseEntity::EmitSound( filter, pOwner->entindex(), "Player.TaggedOtherIT" );

			TFGameRules()->SetIT( pEntity );
		}
	}
#endif
}

void CTFWeaponBaseMelee::OnSwingHit( trace_t &trace )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer == nullptr )
		return;

	// Do Damage.
	DoMeleeDamage( trace.m_pEnt, trace );

	// Hit sound - immediate.
	if ( trace.m_pEnt )
	{
		if( trace.m_pEnt->IsPlayer() )
		{
			WeaponSound( MELEE_HIT );
		}
		else if ( trace.m_pEnt->IsNPC() )
		{
			if ( trace.m_pEnt->MyNPCPointer()->IsMech() )
				WeaponSound( MELEE_HIT_WORLD );
			else
				WeaponSound( MELEE_HIT );
		}
		else
		{
			WeaponSound( MELEE_HIT_WORLD );
		}

		// Don't impact trace friendly players or objects
		if ( !pPlayer->InSameTeam( trace.m_pEnt ) )
		{
#if defined( CLIENT_DLL )
			UTIL_ImpactTrace( &trace, DMG_CLUB );
#endif
			m_bConnected = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFWeaponBaseMelee::CalcIsAttackCriticalHelper( void )
{
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return false;

	int nCvarValue = tf_weapon_criticals_melee.GetInt();

	if ( nCvarValue == 0 )
		return false;

	if ( nCvarValue == 1 && !tf_weapon_criticals.GetBool() )
		return false;

	m_bCurrentAttackIsMiniCrit = pPlayer->m_Shared.GetNextMeleeCrit() != kCritType_None;
	if ( pPlayer->m_Shared.GetNextMeleeCrit() == kCritType_Crit )
		return true;

	float flPlayerCritMult = pPlayer->GetCritMult();

	float flCritChance = TF_DAMAGE_CRIT_CHANCE_MELEE * flPlayerCritMult;
	CALL_ATTRIB_HOOK_FLOAT( flCritChance, mult_crit_chance );

	// If the chance is 0, just bail.
	if ( flCritChance == 0.0f )
		return false;

#ifdef GAME_DLL
	if ( tf_debug_criticals.GetBool() )
	{
		Msg( "Rolling crit: %.02f%% chance... ", flCritChance * 100.0f );
	}
#endif

	bool bSuccess = ( RandomInt( 0, WEAPON_RANDOM_RANGE - 1 ) <= flCritChance * WEAPON_RANDOM_RANGE );

#ifdef GAME_DLL
	if ( tf_debug_criticals.GetBool() )
	{
		Msg( "%s\n", bSuccess ? "SUCCESS" : "FAILURE" );
	}
#endif

	return bSuccess;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFWeaponBaseMelee::GetForceScale( void )
{
	return tf_meleeattackforcescale.GetFloat();
}

extern ConVar sk_crowbar_lead_time;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFWeaponBaseMelee::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if ( !pEnemy )
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_crowbar_lead_time.GetFloat();
	dt += random->RandomFloat( -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFWeaponBaseMelee::IsMeleeWeapon() const
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
char const *CTFWeaponBaseMelee::GetShootSound( int iIndex ) const
{
	return BaseClass::GetShootSound( iIndex );
}
