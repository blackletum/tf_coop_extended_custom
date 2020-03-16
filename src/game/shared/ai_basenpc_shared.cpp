//=============================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "ai_basenpc_shared.h"
#include "tf_shareddefs.h"
#include "tf_fx_shared.h"
#include "effect_dispatch_data.h"
#include "tf_item.h"
#include "entity_capture_flag.h"
// Client specific.
#ifdef CLIENT_DLL
#include "engine/ivdebugoverlay.h"
#include "c_te_effect_dispatch.h"
#else // Server specific
#include "te_effect_dispatch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined(TF_CLASSIC) || defined(TF_CLASSIC_CLIENT)
TF_NPCData g_aNPCData[] =
{
	// Friendly actors.
	{
		"npc_dog",
		TF_TEAM_RED,
		TFFL_BUILDING | TFFL_NOFORCE,
	},
	{
		"npc_eli",
		TF_TEAM_RED,
		0,
	},
	{
		"npc_fisherman",
		TF_TEAM_RED,
		0,
	},
	{
		"npc_gman",
		TF_TEAM_RED,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_FIREPROOF | TFFL_NODEFLECT | TFFL_NOJAR | TFFL_CANBLEED | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	{
		"npc_kleiner",
		TF_TEAM_RED,
		0,
	},
	{
		"npc_magnusson",
		TF_TEAM_RED,
		0,
	},
	{
		"npc_mossman",
		TF_TEAM_RED,
		0,
	},
	// Vital allies.
	{
		"npc_alyx",
		TF_TEAM_RED,
		TFFL_NOBACKSTAB,
	},
	{
		"npc_barney",
		TF_TEAM_RED,
		TFFL_NOBACKSTAB,
	},
	{
		"npc_monk",
		TF_TEAM_RED,
		TFFL_NOBACKSTAB,
	},
	// Regular allies.
	{
		"npc_citizen",
		TF_TEAM_RED,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_vortigaunt",
		TF_TEAM_RED,
		0,
	},
	{
		"npc_conscript",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_conscriptred",
		TF_TEAM_RED,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_crematorred",
		TF_TEAM_RED,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_combine_synth",
		TF_TEAM_BLUE,
		0,
	},
	{
		"monster_scientist",
		TF_TEAM_RED,
		TFFL_UNUSUALCOND,
	},
	{
		"monster_barney",
		TF_TEAM_RED,
		0,
	},
	{
		"monster_bigmomma",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_UNUSUALCOND | TFFL_NOSTUN,
	},
	{
		"monster_human_grunt",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"monster_robo_grunt",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"monster_alien_grunt",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"monster_alien_slave",
		TF_TEAM_GREEN,
		0,
	},
	{
		"monster_vortigaunt",
		TF_TEAM_GREEN,
		0,
	},
	{
		"monster_headcrab",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	{
		"monster_zombie",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"monster_gonome",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"monster_bullchicken",
		TF_TEAM_YELLOW,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	{
		"monster_apache",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOJAR,
	},
	{
		"monster_nihilanth",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_NODEFLECT | TFFL_NOJAR | TFFL_CANBLEED | TFFL_NOFORCE,
	},
	{
		"monster_hornet",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_NOCAP,
	},
	{
		"monster_snark",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_NOHEALING,
	},
	{
		"monster_alien_controller",
		TF_TEAM_GREEN,
		0,
	},
	{
		"monster_gargantua",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND | TFFL_NOSTUN,
	},
	{
		"monster_ichthyosaur",
		TF_TEAM_YELLOW,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	{
		"monster_osprey",
		TF_TEAM_BLUE,
		TFFL_BUILDING,
	},
	{
		"monster_sentry",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NODEFLECT | TFFL_NOCAP | TFFL_NOFORCE,
	},
	{
		"monster_tentacle",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_NODEFLECT | TFFL_NOJAR | TFFL_CANBLEED | TFFL_NOCAP | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	{
		"monster_gman",
		TF_TEAM_RED,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_FIREPROOF | TFFL_NODEFLECT | TFFL_NOJAR | TFFL_CANBLEED | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	{
		"monster_human_assassin",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"monster_leech",
		TF_TEAM_YELLOW,
		TFFL_NOBACKSTAB,
	},
	{
		"monster_sitting_scientist",
		TF_TEAM_RED,
		0,
	},
	{
		"monster_houndeye",
		TF_TEAM_YELLOW,
		TFFL_NOBACKSTAB,
	},
	{
		"npc_breen",
		TF_TEAM_BLUE,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_FIREPROOF | TFFL_NODEFLECT | TFFL_NOJAR | TFFL_CANBLEED | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	// Regular enemies.
	{
		"npc_combine_s",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_metropolice",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_stalker",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_cremator",
		TF_TEAM_BLUE,
		TFFL_FIREPROOF | TFFL_UNUSUALCOND,
	},
	{
		"npc_assassin",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_combineelite",
		TF_TEAM_BLUE,
		TFFL_UNUSUALCOND,
	},
	// Regular combine mechs.
	{
		"npc_cscanner",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOCAP | TFFL_JARSHOCK | TFFL_CANSAP,
	},
	{
		"npc_clawscanner",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOCAP | TFFL_JARSHOCK | TFFL_CANSAP,
	},
	{
		"npc_manhack",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOCAP | TFFL_JARSHOCK | TFFL_CANSAP,
	},
	// Indestructible combine mechs.
	{
		"npc_combine_camera",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NODEFLECT | TFFL_NOCAP | TFFL_JARSHOCK | TFFL_CANSAP | TFFL_CANUPGRADE | TFFL_NOFORCE,
	},
	{
		"npc_rollermine",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOCAP | TFFL_JARSHOCK | TFFL_CANUPGRADE,
	},
	{
		"npc_turret_ceiling",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NODEFLECT | TFFL_NOCAP | TFFL_CANUPGRADE | TFFL_NOFORCE,
	},
	{
		"npc_turret_floor",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOBACKSTAB | TFFL_NOCAP | TFFL_CANSAP | TFFL_CANUPGRADE,
	},
	{
		"npc_turret_ground",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NODEFLECT | TFFL_NOCAP | TFFL_CANSAP | TFFL_CANUPGRADE | TFFL_NOFORCE,
	},
	// Combine synths.
	{
		"npc_combinegunship",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NODEFLECT | TFFL_NOFORCE,
	},
	{
		"npc_hunter",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_UNUSUALCOND,
	},
	{
		"npc_strider",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NODEFLECT | TFFL_UNUSUALCOND | TFFL_NOFORCE,
	},
	{
		"npc_helicopter",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NODEFLECT | TFFL_NOFORCE,
	},
	{
		"npc_combinedropship",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NODEFLECT | TFFL_NOFORCE,
	},
	{
		"npc_combineguard",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NODEFLECT | TFFL_NOFORCE,
	},
	{
		"npc_combine_synth",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NODEFLECT | TFFL_NOFORCE,
	},
	// Misc Combine NPCs.
	{
		"npc_crabsynth",
		TF_TEAM_BLUE,
		TFFL_BUILDING,
	},
	{
		"npc_mortarsynth",
		TF_TEAM_BLUE,
		TFFL_BUILDING,
	},
	{
		"npc_sniper",
		TF_TEAM_BLUE,
		TFFL_NOHEALING | TFFL_NODEFLECT | TFFL_NOCAP | TFFL_UNUSUALCOND | TFFL_NOFORCE,
	},
	// Headcrabs.
	{
		"npc_headcrab",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	{
		"npc_headcrab_fast",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	{
		"npc_headcrab_black",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	{
		"npc_headcrab_poison",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	// Zombies.
	{
		"npc_zombie",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_zombie_torso",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_fastzombie",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_fastzombie_torso",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_poisonzombie",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_zombie_custom",
		TF_TEAM_GREEN,
		TFFL_NODEATHNOTICE | TFFL_UNUSUALCOND,
	},
	{
		"npc_zombine",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	// Antlions.
	{
		"npc_antlion",
		TF_TEAM_YELLOW,
		TFFL_UNUSUALCOND,
	},
	{
		"npc_antlionguard",
		TF_TEAM_YELLOW,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND | TFFL_NOSTUN,
	},
	{
		"npc_ichthyosaur",
		TF_TEAM_YELLOW,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	// Neutral NPCs.
	{
		"npc_barnacle",
		TEAM_UNASSIGNED,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_NODEFLECT | TFFL_NOCAP | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	{
		"monster_barnacle",
		TEAM_UNASSIGNED,
		TFFL_NOBACKSTAB | TFFL_NOHEALING | TFFL_NODEFLECT | TFFL_NOCAP | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	// Birds
	{
		"npc_crow",
		TEAM_UNASSIGNED,
		TFFL_NOBACKSTAB | TFFL_NOREWARD,
	},
	{
		"npc_pigeon",
		TEAM_UNASSIGNED,
		TFFL_NOBACKSTAB | TFFL_NOREWARD,
	},
	{
		"npc_seagull",
		TEAM_UNASSIGNED,
		TFFL_NOBACKSTAB | TFFL_NOREWARD,
	},
	{
		"generic_actor",
		TEAM_UNASSIGNED,
		TFFL_NODEFLECT | TFFL_NODEATHNOTICE | TFFL_NOREWARD | TFFL_NOCAP | TFFL_NOFORCE,
	},
	{
		"cyler_actor",
		TEAM_UNASSIGNED,
		TFFL_NODEFLECT | TFFL_NODEATHNOTICE | TFFL_NOREWARD | TFFL_NOCAP | TFFL_NOFORCE,
	},
	{
		"monster_furniture",
		TEAM_UNASSIGNED,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NODEFLECT | TFFL_NODEATHNOTICE | TFFL_NOREWARD | TFFL_NOCAP | TFFL_NOFORCE,
	},
	{
		"npc_furniture",
		TEAM_UNASSIGNED,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NODEFLECT | TFFL_NODEATHNOTICE | TFFL_NOREWARD | TFFL_NOCAP | TFFL_NOFORCE,
	},
	{
		"monster_generic",
		TEAM_UNASSIGNED,
		TFFL_NODEFLECT | TFFL_NODEATHNOTICE | TFFL_NOREWARD | TFFL_NOCAP | TFFL_NOFORCE,
	},
	{
		"npc_bullseye",
		TEAM_UNASSIGNED,
		TFFL_BUILDING | TFFL_NODEFLECT | TFFL_NODEATHNOTICE | TFFL_NOREWARD | TFFL_NOCAP | TFFL_NOFORCE,
	},
	{
		"npc_bullsquid",
		TF_TEAM_YELLOW,
		TFFL_NOBACKSTAB | TFFL_UNUSUALCOND,
	},
	// Portals
	{
		"npc_portal_turret_floor",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOBACKSTAB | TFFL_NOCAP | TFFL_CANSAP | TFFL_CANUPGRADE,
	},
	{
		"npc_portal_turret_ground",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOBACKSTAB | TFFL_NOCAP | TFFL_CANSAP | TFFL_CANUPGRADE,
	},
	{
		"npc_security_camera",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NOBACKSTAB | TFFL_NOCAP | TFFL_CANUPGRADE,
	},
	{
		"npc_rocket_turret",
		TF_TEAM_BLUE,
		TFFL_BUILDING | TFFL_NOJAR | TFFL_NOBACKSTAB | TFFL_NOCAP | TFFL_CANUPGRADE,
	},
	// Lambda Fortress: Extended
	{
		"npc_custom",
		TEAM_UNASSIGNED,		//Unassigned because maps set their team
		TFFL_UNUSUALCOND,
	},
	{
		"npc_custom",
		TEAM_UNASSIGNED,		//Unassigned because maps set their team
		TFFL_UNUSUALCOND,
	},
	{
		"npc_custom_combine",
		TEAM_UNASSIGNED,		//Unassigned because maps set their team
		TFFL_UNUSUALCOND,
	},
	{
		"npc_custom_zombie",
		TEAM_UNASSIGNED,		//Unassigned because maps set their team
		TFFL_UNUSUALCOND,
	},
	// Team Fortress 2
	{
		"headless_hatman",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_FIREPROOF | TFFL_NODEFLECT | TFFL_NOJAR | TFFL_CANBLEED | TFFL_NOTOUCH_CP | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	{
		"eyeball_boss",
		TF_TEAM_GREEN,
		TFFL_NOBACKSTAB | TFFL_FIREPROOF | TFFL_NODEFLECT | TFFL_NOJAR | TFFL_CANBLEED | TFFL_NOTOUCH_CP | TFFL_NOSTUN | TFFL_NOFORCE,
	},
	{
		"tf_zombie",
		TF_TEAM_GREEN,
		TFFL_UNUSUALCOND,
	},
	{
		"bot_npc_archer",
		TEAM_UNASSIGNED,
		0,
	},
	// End marker.
	{
		NULL,
		0,
		0,
	},
};

extern ConVar tf_max_health_boost;

//-----------------------------------------------------------------------------
// Purpose: Add a condition and duration
// duration of PERMANENT_CONDITION means infinite duration
//-----------------------------------------------------------------------------
void CAI_BaseNPC::AddCond( int nCond, float flDuration /* = PERMANENT_CONDITION */ )
{
	if ( !IsAlive() )
		return;

	Assert( nCond >= 0 && nCond < TF_COND_LAST );
	int nCondFlag = nCond;
	int *pVar = NULL;
	if ( nCond < 128 )
	{
		if ( nCond < 96 )
		{
			if ( nCond < 64 )
			{
				if ( nCond < 32 )
				{
					pVar = &m_nPlayerCond.GetForModify();
				}
				else
				{
					pVar = &m_nPlayerCondEx.GetForModify();
					nCondFlag -= 32;
				}
			}
			else
			{
				pVar = &m_nPlayerCondEx2.GetForModify();
				nCondFlag -= 64;
			}
		}
		else
		{
			pVar = &m_nPlayerCondEx3.GetForModify();
			nCondFlag -= 96;
		}
	}
	else
	{
		pVar = &m_nPlayerCondEx4.GetForModify();
		nCondFlag -= 128;
	}

	*pVar |= ( 1 << nCondFlag );
	m_flCondExpireTimeLeft.Set( nCond, flDuration );
	OnConditionAdded( nCond );
}

//-----------------------------------------------------------------------------
// Purpose: Forcibly remove a condition
//-----------------------------------------------------------------------------
void CAI_BaseNPC::RemoveCond( int nCond )
{
	Assert( nCond >= 0 && nCond < TF_COND_LAST );
	int nCondFlag = nCond;
	int *pVar = NULL;
	if ( nCond < 128 )
	{
		if ( nCond < 96 )
		{
			if ( nCond < 64 )
			{
				if ( nCond < 32 )
				{
					pVar = &m_nPlayerCond.GetForModify();
				}
				else
				{
					pVar = &m_nPlayerCondEx.GetForModify();
					nCondFlag -= 32;
				}
			}
			else
			{
				pVar = &m_nPlayerCondEx2.GetForModify();
				nCondFlag -= 64;
			}
		}
		else
		{
			pVar = &m_nPlayerCondEx3.GetForModify();
			nCondFlag -= 96;
		}
	}
	else
	{
		pVar = &m_nPlayerCondEx4.GetForModify();
		nCondFlag -= 128;
	}

	*pVar &= ~( 1 << nCondFlag );
	m_flCondExpireTimeLeft.Set( nCond, 0 );
	OnConditionRemoved( nCond );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::InCond( int nCond )
{
	Assert( nCond >= 0 && nCond < TF_COND_LAST );

	int nCondFlag = nCond;
	const int *pVar = NULL;
	if ( nCond < 128 )
	{
		if ( nCond < 96 )
		{
			if ( nCond < 64 )
			{
				if ( nCond < 32 )
				{
					pVar = &m_nPlayerCond.GetForModify();
				}
				else
				{
					pVar = &m_nPlayerCondEx.GetForModify();
					nCondFlag -= 32;
				}
			}
			else
			{
				pVar = &m_nPlayerCondEx2.GetForModify();
				nCondFlag -= 64;
			}
		}
		else
		{
			pVar = &m_nPlayerCondEx3.GetForModify();
			nCondFlag -= 96;
		}
	}
	else
	{
		pVar = &m_nPlayerCondEx4.GetForModify();
		nCondFlag -= 128;
	}

	return ((*pVar & (1 << nCondFlag)) != 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CAI_BaseNPC::GetConditionDuration( int nCond )
{
#ifdef GAME_DLL
	Assert( nCond >= 0 && nCond < TF_COND_LAST );

	if ( InCond( nCond ) )
	{
		return m_flCondExpireTimeLeft[nCond];
	}
#endif
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Remove any conditions affecting players
//-----------------------------------------------------------------------------
void CAI_BaseNPC::RemoveAllCond( void )
{
	int i;
	for ( i = 0; i < TF_COND_LAST; i++ )
	{
		if ( InCond( i ) )
		{
			RemoveCond( i );
		}
	}

	// Now remove all the rest
	m_nPlayerCond = 0;
	m_nPlayerCondEx = 0;
	m_nPlayerCondEx2 = 0;
	m_nPlayerCondEx3 = 0;
	m_nPlayerCondEx4 = 0;
}


//-----------------------------------------------------------------------------
// Purpose: Called on both client and server. Server when we add the bit,
// and client when it recieves the new cond bits and finds one added
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnConditionAdded( int nCond )
{
	switch( nCond )
	{
	case TF_COND_HEALTH_BUFF:
#ifdef GAME_DLL
		m_flHealFraction = 0;
#endif
		break;

	case TF_COND_INVULNERABLE:
	case TF_COND_INVULNERABLE_USER_BUFF:
	case TF_COND_INVULNERABLE_CARD_EFFECT:
		OnAddInvulnerable();
		break;

	case TF_COND_BURNING:
		OnAddBurning();
		break;

	case TF_COND_BLEEDING:
	case TF_COND_GRAPPLINGHOOK_BLEEDING:
		OnAddBleeding();
		break;

	case TF_COND_PHASE:
		OnAddPhase();
		break;

	case TF_COND_HEALTH_OVERHEALED:
#ifdef CLIENT_DLL
		UpdateOverhealEffect();
#endif
		break;

	case TF_COND_SHIELD_CHARGE:
		OnAddShieldCharge();
		break;

#ifdef CLIENT_DLL
	case TF_COND_CRITBOOSTED_DEMO_CHARGE:
		UpdateCritBoostEffect();
		break;

	case TF_COND_MEGAHEAL:
		OnAddMegaHeal();
		break;
#endif

	case TF_COND_CRITBOOSTED:
	case TF_COND_CRITBOOSTED_PUMPKIN:
	case TF_COND_CRITBOOSTED_USER_BUFF:
	case TF_COND_CRITBOOSTED_FIRST_BLOOD:
	case TF_COND_CRITBOOSTED_BONUS_TIME:
	case TF_COND_CRITBOOSTED_CTF_CAPTURE:
	case TF_COND_CRITBOOSTED_ON_KILL:
	case TF_COND_CRITBOOSTED_CARD_EFFECT:
	case TF_COND_CRITBOOSTED_RUNE_TEMP:
	//case TF_COND_NOHEALINGDAMAGEBUFF: // this one doesn't have spark effect.
	case TF_COND_MINICRITBOOSTED_ON_KILL:
		OnAddCritboosted();
		break;
#ifdef GAME_DLL
	case TF_COND_HALLOWEEN_GIANT:
		OnAddHalloweenGiant();
		break;

	case TF_COND_HALLOWEEN_TINY:
		OnAddHalloweenTiny();
		break;
#endif
	case TF_COND_STUNNED:
		OnAddStunned();
		break;

	case TF_COND_URINE:
	case TF_COND_SWIMMING_CURSE:
		OnAddUrine();
		break;

	case TF_COND_MAD_MILK:
		OnAddMadMilk();
		break;

	case TF_COND_GAS:
		OnAddCondGas();
		break;

#ifdef CLIENT_DLL
	case TF_COND_MARKEDFORDEATH:
		if ( !m_pMarkForDeath )
		{
			Vector vecOffset( 0,0,10 );
			int iAttachment = LookupAttachment( "eyes" );
			m_pMarkForDeath = ParticleProp()->Create( "mark_for_death", PATTACH_POINT_FOLLOW, iAttachment, vecOffset );
		}
		break;
#endif
	case TF_COND_SPEED_BOOST:
	case TF_COND_HALLOWEEN_SPEED_BOOST:
		OnAddSpeedBoost( true );
		break;

#ifdef GAME_DLL
	case TF_COND_RUNE_STRENGTH:
	case TF_COND_RUNE_HASTE:
	case TF_COND_RUNE_REGEN:
	case TF_COND_RUNE_RESIST:
	case TF_COND_RUNE_VAMPIRE:
	case TF_COND_RUNE_WARLOCK:
	case TF_COND_RUNE_PRECISION:
	case TF_COND_RUNE_AGILITY:
	case TF_COND_RUNE_KNOCKOUT:
	case TF_COND_RUNE_KING:
	case TF_COND_RUNE_SUPERNOVA:
		OnAddRune();
		break;
#else
	case TF_COND_OFFENSEBUFF:
	case TF_COND_DEFENSEBUFF:
	case TF_COND_REGENONDAMAGEBUFF:
		OnAddBuff();
		if ( nCond == TF_COND_REGENONDAMAGEBUFF && !InCond( TF_COND_SPEED_BOOST ) )
			AddCond( TF_COND_SPEED_BOOST );
		break;

	case TF_COND_RADIUSHEAL:
		OnAddRadiusHeal();
		break;

	case TF_COND_RUNE_PLAGUE:
		OnAddRunePlague();
		break;
#endif

	case TF_COND_SAPPED:
		OnAddSapped();
		break;

	case TF_COND_HALLOWEEN_GHOST_MODE:
		if ( GetTeamNumber() == TF_TEAM_RED )
			SetModelName( MAKE_STRING( "models/props_halloween/ghost_no_hat_red.mdl" ) );
		else
			SetModelName( MAKE_STRING( "models/props_halloween/ghost_no_hat.mdl" ) );

		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called on both client and server. Server when we remove the bit,
// and client when it recieves the new cond bits and finds one removed
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnConditionRemoved( int nCond )
{
	switch( nCond )
	{
	case TF_COND_BURNING:
		OnRemoveBurning();
		break;

	case TF_COND_BLEEDING:
	case TF_COND_GRAPPLINGHOOK_BLEEDING:
		OnRemoveBleeding();
		break;

	case TF_COND_PHASE:
		OnRemovePhase();
		break;

	case TF_COND_HEALTH_BUFF:
#ifdef GAME_DLL
		m_flHealFraction = 0;
#endif
		break;

	case TF_COND_HEALTH_OVERHEALED:
#ifdef CLIENT_DLL
		UpdateOverhealEffect();
#endif
		break;

	case TF_COND_INVULNERABLE:
	case TF_COND_INVULNERABLE_USER_BUFF:
	case TF_COND_INVULNERABLE_CARD_EFFECT:
		OnRemoveInvulnerable();
		break;

	case TF_COND_SHIELD_CHARGE:
		OnRemoveShieldCharge();
		break;

#ifdef CLIENT_DLL
	case TF_COND_CRITBOOSTED_DEMO_CHARGE:
		UpdateCritBoostEffect();

		break;
	case TF_COND_MEGAHEAL:
		OnRemoveMegaHeal();
		break;
#endif

	case TF_COND_CRITBOOSTED:
	case TF_COND_CRITBOOSTED_PUMPKIN:
	case TF_COND_CRITBOOSTED_USER_BUFF:
	case TF_COND_CRITBOOSTED_FIRST_BLOOD:
	case TF_COND_CRITBOOSTED_BONUS_TIME:
	case TF_COND_CRITBOOSTED_CTF_CAPTURE:
	case TF_COND_CRITBOOSTED_ON_KILL:
	case TF_COND_CRITBOOSTED_CARD_EFFECT:
	case TF_COND_CRITBOOSTED_RUNE_TEMP:
	//case TF_COND_NOHEALINGDAMAGEBUFF: // this one doesn't have spark effect.
	case TF_COND_MINICRITBOOSTED_ON_KILL:
		OnRemoveCritboosted();
		break;
#ifdef GAME_DLL
	case TF_COND_HALLOWEEN_GIANT:
		OnRemoveHalloweenGiant();
		break;

	case TF_COND_HALLOWEEN_TINY:
		OnRemoveHalloweenTiny();
		break;
#endif
	case TF_COND_STUNNED:
		OnRemoveStunned();
		break;

	case TF_COND_URINE:
	case TF_COND_SWIMMING_CURSE:
		OnRemoveUrine();
		break;

	case TF_COND_MAD_MILK:
		OnRemoveMadMilk();
		break;

	case TF_COND_GAS:
		OnRemoveCondGas();
		break;

#ifdef CLIENT_DLL
	case TF_COND_MARKEDFORDEATH:
		if ( m_pMarkForDeath )
		{
			ParticleProp()->StopEmission( m_pMarkForDeath );
			m_pMarkForDeath = NULL;
		}
		break;
#endif

	case TF_COND_SPEED_BOOST:
	case TF_COND_HALLOWEEN_SPEED_BOOST:
		OnRemoveSpeedBoost();
		break;
#ifdef GAME_DLL
	case TF_COND_RUNE_STRENGTH:
	case TF_COND_RUNE_HASTE:
	case TF_COND_RUNE_REGEN:
	case TF_COND_RUNE_RESIST:
	case TF_COND_RUNE_VAMPIRE:
	case TF_COND_RUNE_WARLOCK:
	case TF_COND_RUNE_PRECISION:
	case TF_COND_RUNE_AGILITY:
	case TF_COND_RUNE_KNOCKOUT:
	case TF_COND_RUNE_KING:
	case TF_COND_RUNE_PLAGUE:
	case TF_COND_RUNE_SUPERNOVA:
		OnRemoveRune();
		break;
#else
	case TF_COND_OFFENSEBUFF:
	case TF_COND_DEFENSEBUFF:
	case TF_COND_REGENONDAMAGEBUFF:
		OnRemoveBuff();
		if ( nCond == TF_COND_REGENONDAMAGEBUFF && InCond( TF_COND_SPEED_BOOST ) )
			RemoveCond( TF_COND_SPEED_BOOST );
		break;

	case TF_COND_RADIUSHEAL:
		OnRemoveRadiusHeal();
		break;

	case TF_COND_RUNE_PLAGUE:
		OnRemoveRunePlague();
		break;
#endif

	case TF_COND_SAPPED:
		OnRemoveSapped();
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsCritBoosted( void )
{
	// Oh man...
	if ( InCond( TF_COND_CRITBOOSTED ) ||
		InCond( TF_COND_CRITBOOSTED_PUMPKIN ) ||
		InCond( TF_COND_CRITBOOSTED_USER_BUFF ) ||
		InCond( TF_COND_CRITBOOSTED_FIRST_BLOOD ) ||
		InCond( TF_COND_CRITBOOSTED_BONUS_TIME ) ||
		InCond( TF_COND_CRITBOOSTED_CTF_CAPTURE ) ||
		InCond( TF_COND_CRITBOOSTED_ON_KILL ) ||
		InCond( TF_COND_CRITBOOSTED_CARD_EFFECT ) ||
		InCond( TF_COND_CRITBOOSTED_RUNE_TEMP ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsMiniCritBoosted( void )
{
	if ( InCond( TF_COND_NOHEALINGDAMAGEBUFF ) ||
		InCond( TF_COND_MINICRITBOOSTED_ON_KILL ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsInvulnerable( void )
{
	// Oh man again...
	if ( InCond( TF_COND_INVULNERABLE ) ||
		InCond( TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE ) ||
		InCond( TF_COND_INVULNERABLE_USER_BUFF ) ||
		InCond( TF_COND_INVULNERABLE_CARD_EFFECT ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsStealthed( void )
{
	if ( InCond( TF_COND_STEALTHED ) ||
		InCond( TF_COND_STEALTHED_USER_BUFF ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsJared( void )
{
	if ( InCond( TF_COND_URINE ) ||
		InCond( TF_COND_MAD_MILK ) ||
		InCond( TF_COND_GAS ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsSpeedBoosted( void )
{
	if ( InCond( TF_COND_SPEED_BOOST ) ||
		InCond( TF_COND_HALLOWEEN_SPEED_BOOST ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsBuffed( void )
{
	if ( InCond( TF_COND_OFFENSEBUFF ) ||
		InCond( TF_COND_DEFENSEBUFF ) || 
		InCond( TF_COND_REGENONDAMAGEBUFF ) )
		return true;

	return false;
}

int CAI_BaseNPC::GetMaxBuffedHealth( void )
{
	float flBoostMax = GetMaxHealth() * tf_max_health_boost.GetFloat();

	int iRoundDown = floor( flBoostMax / 5 );
	iRoundDown = iRoundDown * 5;

	return iRoundDown;
}

extern ConVar	tf_fireball_burn_duration;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::Burn( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon /*= NULL*/, float flFlameDuration /*= -1.0f*/ )
{
#ifdef GAME_DLL
	// Don't bother igniting players who have just been killed by the fire damage.
	if ( !IsAlive() )
		return;

	if ( !InCond( TF_COND_BURNING ) )
	{
		if ( AllowedToIgnite() == true )
		{
			// Start burning
			AddCond( TF_COND_BURNING );
			m_flFlameBurnTime = gpGlobals->curtime;	//asap
		}
		// let the attacker know he burned me
		if ( pAttacker  )
		{
			pAttacker->OnBurnOther( this );
		}
	}

	float flFlameLife = TF_BURNING_FLAME_LIFE;
	if ( pWeapon )
	{
		flFlameLife = pWeapon->GetAfterburnRateOnHit();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flFlameLife, mult_wpn_burntime );
	}

	if ( flFlameDuration != -1.0f )
		flFlameLife = flFlameDuration;

	if ( pWeapon && !pWeapon->IsWeapon( TF_WEAPON_ROCKETLAUNCHER_FIREBALL ) )
	{
		m_flFlameRemoveTime = gpGlobals->curtime + flFlameLife;
	}
	else
	{
		// dragon's fury afterburn is 2 second
		m_flFlameRemoveTime = gpGlobals->curtime + tf_fireball_burn_duration.GetFloat();
	}

	m_hBurnAttacker = pAttacker;
	m_hBurnWeapon = pWeapon;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: BLOOD LEAKING
//-----------------------------------------------------------------------------
void CAI_BaseNPC::MakeBleed( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon, float flBleedDuration, int iDamage )
{
	if ( !CanBleed() )
		return;

#ifdef GAME_DLL
	if ( IsAlive() && ( pAttacker || pWeapon ) )
	{
		float flEndAt = gpGlobals->curtime + flBleedDuration;
		for (int i=0; i<m_aBleeds.Count(); ++i)
		{
			bleed_struct_t *bleed = &m_aBleeds[i];
			if (bleed->m_hAttacker == pAttacker && bleed->m_hWeapon == pWeapon)
			{
				bleed->m_flEndTime = flEndAt;

				if ( !InCond( TF_COND_BLEEDING ) )
					AddCond( TF_COND_BLEEDING );

				return;
			}
		}

		bleed_struct_t bleed = {
			pAttacker,
			pWeapon,
			flBleedDuration,
			flEndAt,
			iDamage
		};
		m_aBleeds.AddToTail( bleed );

		if (!InCond( TF_COND_BLEEDING ))
			AddCond( TF_COND_BLEEDING );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::StunNPC( float flDuration, float flSpeed, float flResistance, int nStunFlags, CTFPlayer *pStunner )
{
	float flNextStunExpireTime = max( m_flStunExpireTime, gpGlobals->curtime + flDuration );
	#ifdef GAME_DLL
	m_hStunner = pStunner;
	#endif
	m_nStunFlags = nStunFlags;
	m_flStunMovementSpeed = flSpeed;
	m_flStunResistance = flResistance;

	if ( m_flStunExpireTime < flNextStunExpireTime )
	{
		AddCond( TF_COND_STUNNED );
		m_flStunExpireTime = flNextStunExpireTime;

#ifdef GAME_DLL
		if( !( m_nStunFlags & TF_STUNFLAG_THIRDPERSON ) )
			StunSound( pStunner, m_nStunFlags /*, current stun flags*/ );
		NPC_TranslateActivity(ACT_MP_STUN_BEGIN);
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::Concussion( CTFPlayer *pAttacker, float flDistance )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::AttachSapper( CTFPlayer *pAttacker )
{
	if ( InCond( TF_COND_SAPPED ) )
		return;

#ifdef GAME_DLL
	OnAttachSapper();
#endif

	AddCond( TF_COND_SAPPED );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddBurning( void )
{
#ifdef CLIENT_DLL
	// Start the burning effect
	if ( !m_pBurningEffect )
	{
		const char *pEffectName = ( GetTeamNumber() == TF_TEAM_BLUE ) ? "burningplayer_blue" : "burningplayer_red";
		m_pBurningEffect = ParticleProp()->Create( pEffectName, PATTACH_ABSORIGIN_FOLLOW );

		m_flBurnEffectStartTime = gpGlobals->curtime;
		m_flBurnEffectEndTime = gpGlobals->curtime + TF_BURNING_FLAME_LIFE;
	}
#endif

	// play a fire-starting sound
	EmitSound( "Fire.Engulf" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveBurning( void )
{
#ifdef CLIENT_DLL
	StopBurningSound();

	if ( m_pBurningEffect )
	{
		ParticleProp()->StopEmission( m_pBurningEffect );
		m_pBurningEffect = NULL;
	}

	m_flBurnEffectStartTime = 0;
	m_flBurnEffectEndTime = 0;
#else
	m_hBurnAttacker = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddBleeding( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveBleeding( void )
{
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddInvulnerable( void )
{
	// Stock uber removes negative conditions.
	if ( InCond( TF_COND_BURNING ) )
	{
		RemoveCond( TF_COND_BURNING );
	}

	if ( InCond( TF_COND_BLEEDING ) )
	{
		RemoveCond( TF_COND_BLEEDING );
	}

	if ( IsJared() )
	{
		RemoveCond( TF_COND_URINE );
		RemoveCond( TF_COND_MAD_MILK );
		RemoveCond( TF_COND_GAS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveInvulnerable( void )
{
}
#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddSlowed( void )
{
	RunAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: Remove slowdown effect
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveSlowed( void )
{
	RunAnimation();
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddCritboosted( void )
{
#ifdef CLIENT_DLL
	UpdateCritBoostEffect();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveCritboosted( void )
{
#ifdef CLIENT_DLL
	UpdateCritBoostEffect();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddStunned( void )
{
	// Check if effects are disabled
	if ( !( m_nStunFlags & TF_STUNFLAG_NOSOUNDOREFFECT ) )
	{
#ifdef CLIENT_DLL
		if ( !m_pStun )
		{
			Vector vecOffset( 0,0,10 );
			int iAttachment = LookupAttachment( "eyes" );
			if ( m_nStunFlags & TF_STUNFLAG_BONKEFFECT )
			{
				// Half stun
				m_pStun = ParticleProp()->Create( "conc_stars", PATTACH_POINT_FOLLOW, iAttachment, vecOffset );
			}
			else if ( m_nStunFlags & TF_STUNFLAG_GHOSTEFFECT )
			{
				// Ghost stun
				m_pStun = ParticleProp()->Create( "yikes_fx", PATTACH_POINT_FOLLOW, iAttachment, vecOffset );
			}
		}
#else
		if ( m_nStunFlags & TF_STUNFLAG_GHOSTEFFECT )
		{
			// Play the scream sound
			EmitSound( "Halloween.PlayerScream" );
		}

		// (Potentially) Reduce our speed if we were stunned
		if ( m_nStunFlags & TF_STUNFLAG_SLOWDOWN )
		{
			SetPlaybackRate( GetPlaybackRate() * m_flStunMovementSpeed );
			RunAnimation();
		}
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveStunned( void )
{
	m_flStunExpireTime = 0.0f;
	m_flStunMovementSpeed = 0.0f;
	m_flStunResistance = 0.0f;
#ifdef GAME_DLL
	m_hStunner = NULL;
	m_iStunPhase = STUN_PHASE_NONE;
	RunAnimation();
#else
	ParticleProp()->StopEmission( m_pStun );
	m_pStun = NULL;
#endif
}
#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddHalloweenGiant( void )
{
	SetModelScale( 2.0, 0.0 );

	SetMaxHealth( GetMaxHealth() * 10 );
	SetHealth( GetMaxHealth() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveHalloweenGiant( void )
{
	SetModelScale( 1.0, 0.0 );

	SetMaxHealth( GetMaxHealth() );
	SetHealth( GetMaxHealth() );

}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddPhase(void)
{
#ifdef GAME_DLL
	DropFlag();
#endif
	UpdatePhaseEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemovePhase(void)
{
#ifdef GAME_DLL
	for ( int i = 0; i < m_pPhaseTrails.Count(); i++ )
	{
		m_pPhaseTrails[i]->SUB_Remove();
	}
	m_pPhaseTrails.RemoveAll();
#else
	ParticleProp()->StopEmission( m_pWarp );
	m_pWarp = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddShieldCharge( void )
{
#ifdef GAME_DLL
	RunAnimation();
	EmitSound( "DemoCharge.Charging" );
#endif

	UpdatePhaseEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveShieldCharge( void )
{
#ifdef GAME_DLL
	RunAnimation();

	for (int i = 0; i < m_pPhaseTrails.Count(); i++)
	{
		UTIL_Remove( m_pPhaseTrails[i] );
	}
	m_pPhaseTrails.RemoveAll();
#else
	ParticleProp()->StopEmission( m_pWarp );
	m_pWarp = NULL;
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddHalloweenTiny( void )
{
	SetModelScale( 0.5, 0.0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveHalloweenTiny( void )
{

	SetModelScale( 1.0, 0.0 );
}

//-----------------------------------------------------------------------------
// Purpose: Bonk phase effects
//-----------------------------------------------------------------------------
void CAI_BaseNPC::AddPhaseEffects(void)
{
	const char* pszEffect = GetTeamNumber() == TF_TEAM_BLUE ? "effects/beam001_blu.vmt" : "effects/beam001_red.vmt";
	Vector vecOrigin = GetAbsOrigin();
	
	/*CSpriteTrail *pPhaseTrail = CSpriteTrail::SpriteTrailCreate( pszEffect, vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 12.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 1.0 );
	pPhaseTrail->SetAttachment( this, LookupAttachment( "back_upper" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );

	pPhaseTrail = CSpriteTrail::SpriteTrailCreate( pszEffect, vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 16.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 1.0 );
	pPhaseTrail->SetAttachment( this, LookupAttachment( "back_lower" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );

	// White trail for socks
	pPhaseTrail = CSpriteTrail::SpriteTrailCreate( "effects/beam001_white.vmt", vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 8.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 0.5 );
	pPhaseTrail->SetAttachment( this, LookupAttachment( "foot_R" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );

	pPhaseTrail = CSpriteTrail::SpriteTrailCreate( "effects/beam001_white.vmt", vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 8.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 0.5 );
	pPhaseTrail->SetAttachment( this, LookupAttachment( "foot_L" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );*/

	CSpriteTrail *pPhaseTrail = CSpriteTrail::SpriteTrailCreate( pszEffect, vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 16.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 1.0 );
	pPhaseTrail->SetAttachment( this, LookupAttachment( "chest" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Update phase effects
//-----------------------------------------------------------------------------
void CAI_BaseNPC::UpdatePhaseEffects(void)
{
	if ( !InCond( TF_COND_PHASE ) && !InCond( TF_COND_SHIELD_CHARGE ) )
		return;

#ifdef CLIENT_DLL
	if( GetAbsVelocity() != vec3_origin )
	{
		if ( !m_pWarp )
		{
			m_pWarp = ParticleProp()->Create( "warp_version", PATTACH_ABSORIGIN_FOLLOW );
		}
	}
#else
	if ( m_pPhaseTrails.IsEmpty() )
	{
		AddPhaseEffects();
	}
		
	// Turn on the trails if they're not active already
	if ( m_pPhaseTrails[0] && !m_pPhaseTrails[0]->IsOn() )
	{
		for( int i = 0; i < m_pPhaseTrails.Count(); i++ )
		{
			m_pPhaseTrails[i]->TurnOn();
		}
	}
#endif
}
//-----------------------------------------------------------------------------
// Purpose: Update speedboost effects
//-----------------------------------------------------------------------------
void CAI_BaseNPC::UpdateSpeedBoostEffects(void)
{
#ifdef CLIENT_DLL
	if ( IsSpeedBoosted() )
	{
		if(  GetAbsVelocity() != vec3_origin )
		{
			// We're on the move
			if ( !m_pSpeedTrails )
			{
				m_pSpeedTrails = ParticleProp()->Create( "speed_boost_trail", PATTACH_ABSORIGIN_FOLLOW );
			}
		}
		else
		{
			// We're not moving
			if( m_pSpeedTrails )
			{
				ParticleProp()->StopEmission( m_pSpeedTrails );
				m_pSpeedTrails = NULL;
			}
		}
	}
	else
	{
		ParticleProp()->StopEmission( m_pSpeedTrails );
		m_pSpeedTrails = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddUrine( void )
{
#ifdef CLIENT_DLL
	ParticleProp()->Create( "peejar_drips", PATTACH_ABSORIGIN_FOLLOW );
#else
	if ( m_nTFFlags & TFFL_JARSHOCK )
		AddCond( TF_COND_SAPPED );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddMadMilk( void )
{
#ifdef CLIENT_DLL
	ParticleProp()->Create( "peejar_drips_milk", PATTACH_ABSORIGIN_FOLLOW );
#else
	if ( m_nTFFlags & TFFL_JARSHOCK )
		AddCond( TF_COND_SAPPED );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddCondGas( void )
{
#ifdef CLIENT_DLL
	const char *pszEffectName = ConstructTeamParticle( "gas_can_drips_%s", GetTeamNumber() );
	ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
#else
	if ( m_nTFFlags & TFFL_JARSHOCK )
		AddCond( TF_COND_SAPPED );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveUrine( void )
{
#ifdef GAME_DLL
	if( IsAlive() )
	{
		m_hUrineAttacker = NULL;
	}
#else
	ParticleProp()->StopParticlesNamed( "peejar_drips" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveMadMilk( void )
{
#ifdef GAME_DLL
	if( IsAlive() )
	{
		m_hUrineAttacker = NULL;
	}
#else
	ParticleProp()->StopParticlesNamed( "peejar_drips_milk" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveCondGas( void )
{
#ifdef GAME_DLL
	if( IsAlive() )
	{
		m_hUrineAttacker = NULL;
	}
#else
	ParticleProp()->StopParticlesNamed( "gas_can_drips_red" );
	ParticleProp()->StopParticlesNamed( "gas_can_drips_blue" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddRune( void )
{
#ifdef CLIENT_DLL
	UpdateRuneIcon( true );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveRune( void )
{
#ifdef CLIENT_DLL
	UpdateRuneIcon( false );
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddBuff( void )
{
	// Start the buff effect
	if ( !m_pBuffAura )
	{
		const char *pszEffectName = ConstructTeamParticle( "soldierbuff_%s_buffed", GetTeamNumber() );
		if ( GetTeamNumber() == TF_TEAM_GREEN || GetTeamNumber() == TF_TEAM_YELLOW )
			pszEffectName = "soldierbuff_blue_buffed";

		m_pBuffAura = ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveBuff( void )
{
	if ( m_pBuffAura )
	{
		ParticleProp()->StopEmission( m_pBuffAura );
		m_pBuffAura = NULL;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddSapped( void )
{
#ifdef CLIENT_DLL
	if ( !m_pSapped )
	{
		Vector vecOffset( 0,0,10 );
		int iAttachment = LookupAttachment( "eyes" );
		m_pSapped = ParticleProp()->Create( "sapper_sentry1_fx", PATTACH_POINT_FOLLOW, iAttachment, vecOffset );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveSapped( void )
{
#ifdef CLIENT_DLL
	if ( m_pSapped )
	{
		ParticleProp()->StopEmission( m_pSapped );
		m_pSapped = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddSpeedBoost( bool bParticle )
{
#ifdef GAME_DLL
	RunAnimation();
#else
	if ( bParticle )
		UpdateSpeedBoostEffects();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveSpeedBoost( void )
{
#ifdef GAME_DLL
	RunAnimation();
#else
	UpdateSpeedBoostEffects();
#endif
}
#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddRadiusHeal( void )
{
	// Start the heal effect
	if ( !m_pRadiusHeal )
	{
		const char *pszEffectName = ConstructTeamParticle( "medic_healradius_%s_buffed", GetTeamNumber() );

		m_pRadiusHeal = ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveRadiusHeal( void )
{
	if ( m_pRadiusHeal )
	{
		ParticleProp()->StopEmission( m_pRadiusHeal );
		m_pRadiusHeal = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnAddRunePlague( void )
{
	// Start the heal effect
	if ( !m_pRunePlague )
	{
		m_pRunePlague = ParticleProp()->Create( "plague", PATTACH_ABSORIGIN_FOLLOW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::OnRemoveRunePlague( void )
{
	if ( m_pRunePlague )
	{
		ParticleProp()->StopEmission( m_pRunePlague );
		m_pRunePlague = NULL;
	}
}
#endif
extern ConVar tf_debug_bullets;
extern ConVar tf_useparticletracers;
extern ConVar sv_showimpacts;

//-----------------------------------------------------------------------------
// Purpose:
//   Input: info
//          bDoEffects - effects (blood, etc.) should only happen client-side.
//-----------------------------------------------------------------------------
void CAI_BaseNPC::FireBullet( const FireBulletsInfo_t &info, bool bDoEffects, int nDamageType, int nCustomDamageType /*= TF_DMG_CUSTOM_NONE*/ )
{
	Vector vecStart = info.m_vecSrc;
	Vector vecEnd = vecStart + info.m_vecDirShooting * info.m_flDistance;
	trace_t trace;

	// Skip multiple entities when tracing
	CTraceFilterSimpleList traceFilter( COLLISION_GROUP_NONE );
	traceFilter.SetPassEntity( this ); // Standard pass entity for THIS so that it can be easily removed from the list after passing through a portal
	// Also ignore a vehicle we're a passenger in
	//if ( MyCombatCharacterPointer() && MyCombatCharacterPointer()->IsInAVehicle() )
	//	traceFilter.AddEntityToIgnore( MyCombatCharacterPointer()->GetVehicleEntity() );

	CProp_Portal *pShootThroughPortal = NULL;
	float flPortalFraction = 2.0f;
	Ray_t rayBullet;
	rayBullet.Init( info.m_vecSrc, vecEnd );
	pShootThroughPortal = UTIL_Portal_FirstAlongRay( rayBullet, flPortalFraction );

	CTraceFilterIgnoreFriendlyCombatItems traceFilterCombatItem( this, COLLISION_GROUP_NONE, GetTeamNumber() );
	CTraceFilterChain traceFilterChain( &traceFilter, &traceFilterCombatItem );
	if ( !UTIL_Portal_TraceRay_Bullets( pShootThroughPortal, rayBullet, MASK_SHOT, &traceFilterChain, &trace ) )
		pShootThroughPortal = NULL;

#ifdef CLIENT_DLL
	if ( sv_showimpacts.GetInt() == 1 || sv_showimpacts.GetInt() == 2 )
	{
		// draw red client impact markers
		debugoverlay->AddBoxOverlay( trace.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

		if ( trace.m_pEnt && trace.m_pEnt->IsCombatCharacter() )
		{
			C_BaseCombatCharacter *pCombat = ToBaseCombatCharacter( trace.m_pEnt );
			pCombat->DrawClientHitboxes( 4, true );
		}
	}
#else
	if ( sv_showimpacts.GetInt() == 1 || sv_showimpacts.GetInt() == 3 )
	{
		// draw blue server impact markers
		NDebugOverlay::Box( trace.endpos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 0, 255, 127, 4 );

		if ( trace.m_pEnt && trace.m_pEnt->IsCombatCharacter() )
		{
			CBaseCombatCharacter *pCombat = ToBaseCombatCharacter( trace.m_pEnt );
			pCombat->DrawServerHitboxes( 4, true );
		}
	}

	if ( tf_debug_bullets.GetBool() )
		NDebugOverlay::Line( vecStart, trace.endpos, 0, 255, 0, true, 30 );
#endif

	if ( !trace.startsolid )
	{
		vecStart = trace.endpos - trace.startpos;
		VectorNormalize( vecStart );
	}

	if ( trace.fraction < 1.0 )
	{
		// Verify we have an entity at the point of impact.
		Assert( trace.m_pEnt );

		if ( bDoEffects )
		{
			// If shot starts out of water and ends in water
			if ( !( enginetrace->GetPointContents( trace.startpos ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) &&
				( enginetrace->GetPointContents( trace.endpos ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) )
			{
				// Water impact effects.
				ImpactWaterTrace( trace, vecStart );
			}
			else
			{
				// Regular impact effects.

				// don't decal your teammates or objects on your team
				if ( trace.m_pEnt->GetTeamNumber() != GetTeamNumber() )
				{
					UTIL_ImpactTrace( &trace, nDamageType );
				}
			}

#ifdef CLIENT_DLL
			static int	tracerCount;
			if ( ( info.m_iTracerFreq != 0 ) && ( tracerCount++ % info.m_iTracerFreq ) == 0 )
			{
				// if this is a local player, start at attachment on view model
				// else start on attachment on weapon model

				int iEntIndex = entindex();
				int iUseAttachment = TRACER_DONT_USE_ATTACHMENT;
				int iAttachment = 1;

				C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
				if( pWeapon )
					iAttachment = pWeapon->LookupAttachment( "muzzle" );

				bool bInToolRecordingMode = clienttools->IsInRecordingMode();

				if ( !IsDormant() )
				{
					if( pWeapon )
					{
						iEntIndex = pWeapon->entindex();

						int nModelIndex = pWeapon->GetModelIndex();
						int nWorldModelIndex = pWeapon->GetWorldModelIndex();
						if ( bInToolRecordingMode && nModelIndex != nWorldModelIndex )
						{
							pWeapon->SetModelIndex( nWorldModelIndex );
						}

						pWeapon->GetAttachment( iAttachment, vecStart );

						if ( bInToolRecordingMode && nModelIndex != nWorldModelIndex )
						{
							pWeapon->SetModelIndex( nModelIndex );
						}
					}
				}

				if ( tf_useparticletracers.GetBool() )
				{
					const char *pszTracerEffect = GetTracerType();
					if ( pszTracerEffect && pszTracerEffect[0] )
					{
						char szTracerEffect[128];
						if ( nDamageType & DMG_CRITICAL )
						{
							Q_snprintf( szTracerEffect, sizeof(szTracerEffect), "%s_crit", pszTracerEffect );
							pszTracerEffect = szTracerEffect;
						}

						if ( pShootThroughPortal )
						{
							trace.endpos = info.m_vecSrc + ( vecEnd - info.m_vecSrc ) * flPortalFraction;
						}

						FX_TFTracer( pszTracerEffect, vecStart, trace.endpos, entindex(), true );

						if ( pShootThroughPortal )
						{
							Vector vTransformedIntersection;
							UTIL_Portal_PointTransform( pShootThroughPortal->MatrixThisToLinked(), trace.endpos, vTransformedIntersection );
							ComputeTracerStartPosition( vTransformedIntersection, &vecStart );

							FX_TFTracer( pszTracerEffect, vecStart, trace.endpos, entindex(), true );

							// Shooting through a portal, the damage direction is translated through the passed-through portal
							// so the damage indicator hud animation is correct
							Vector vDmgOriginThroughPortal;
							UTIL_Portal_PointTransform( pShootThroughPortal->MatrixThisToLinked(), vecStart, vDmgOriginThroughPortal );
							trace.endpos = vDmgOriginThroughPortal;
						}
					}
				}
				else
				{
					UTIL_Tracer( vecStart, trace.endpos, entindex(), iUseAttachment, 5000, true, GetTracerType() );
				}
			}
#endif

		}

		// Server specific.
#ifndef CLIENT_DLL
		// See what material we hit.
		CTakeDamageInfo dmgInfo( this, info.m_pAttacker, GetActiveWeapon(), info.m_flDamage, nDamageType, nCustomDamageType );
		CalculateBulletDamageForce( &dmgInfo, info.m_iAmmoType, info.m_vecDirShooting, trace.endpos, 1.0 );	//MATTTODO bullet forces
		trace.m_pEnt->DispatchTraceAttack( dmgInfo, info.m_vecDirShooting, &trace );
#endif
	}
}

#ifdef CLIENT_DLL
extern ConVar tf_impactwatertimeenable;
extern ConVar tf_impactwatertime;
#endif

//-----------------------------------------------------------------------------
// Purpose: Trace from the shooter to the point of impact (another player,
//          world, etc.), but this time take into account water/slime surfaces.
//   Input: trace - initial trace from player to point of impact
//          vecStart - starting point of the trace 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::ImpactWaterTrace( trace_t &trace, const Vector &vecStart )
{
#ifdef CLIENT_DLL
	if ( tf_impactwatertimeenable.GetBool() )
	{
		if ( m_flWaterImpactTime > gpGlobals->curtime )
			return;
	}
#endif 

	trace_t traceWater;
	Ray_t ray; ray.Init( vecStart, trace.endpos );
	UTIL_Portal_TraceRay( ray, ( MASK_SHOT | CONTENTS_WATER | CONTENTS_SLIME ), this, COLLISION_GROUP_NONE, &traceWater );
	if ( traceWater.fraction < 1.0f )
	{
		CEffectData	data;
		data.m_vOrigin = traceWater.endpos;
		data.m_vNormal = traceWater.plane.normal;
		data.m_flScale = random->RandomFloat( 8, 12 );
		if ( traceWater.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		const char *pszEffectName = "tf_gunshotsplash";

		DispatchEffect( pszEffectName, data );

#ifdef CLIENT_DLL
		if ( tf_impactwatertimeenable.GetBool() )
		{
			m_flWaterImpactTime = gpGlobals->curtime + tf_impactwatertime.GetFloat();
		}
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add Mannpower Revenge Crit
//-----------------------------------------------------------------------------
void CAI_BaseNPC::AddTempCritBonus( float flDuration )
{
	AddCond( TF_COND_RUNE_IMBALANCE, flDuration );
	AddCond( TF_COND_CRITBOOSTED_RUNE_TEMP, flDuration );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::HasItem( void )
{
	return ( m_hItem != NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_BaseNPC::SetItem( CTFItem *pItem )
{
	m_hItem = pItem;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFItem	*CAI_BaseNPC::GetItem( void )
{
	return m_hItem;
}

//-----------------------------------------------------------------------------
// Purpose: Is the player allowed to use a teleporter ?
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::HasTheFlag( void )
{
	if ( HasItem() && GetItem()->GetItemID() == TF_ITEM_CAPTURE_FLAG )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Are we allowed to pick the flag up?
//-----------------------------------------------------------------------------
bool CAI_BaseNPC::IsAllowedToPickUpFlag( void )
{
	int bNotAllowedToPickUpFlag = 0;
	CALL_ATTRIB_HOOK_INT( bNotAllowedToPickUpFlag, cannot_pick_up_intelligence );

	if ( bNotAllowedToPickUpFlag > 0 )
		return false;

	return ( m_nTFFlags & TFFL_NOTOUCH_FLAG ) == 0;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseNPC::SetCarryingRuneType( RuneTypes_t RuneType )
{
	switch ( RuneType )
	{
	case TF_RUNE_NONE:
		break;
	case TF_RUNE_STRENGTH:
		AddCond( TF_COND_RUNE_STRENGTH );
		EmitSound( "Powerup.PickUpStrength" );
		break;
	case TF_RUNE_HASTE:
		AddCond( TF_COND_RUNE_HASTE );
		EmitSound( "Powerup.PickUpHaste" );
		break;
	case TF_RUNE_REGEN:
		AddCond( TF_COND_RUNE_REGEN );
		EmitSound( "Powerup.PickUpRegeneration" );
		break;
	case TF_RUNE_RESIST:
		AddCond( TF_COND_RUNE_RESIST );
		EmitSound( "Powerup.PickUpResistance" );
		break;
	case TF_RUNE_VAMPIRE:
		AddCond( TF_COND_RUNE_VAMPIRE );
		EmitSound( "Powerup.PickUpVampire" );
		break;
	case TF_RUNE_WARLOCK:
		AddCond( TF_COND_RUNE_WARLOCK );
		EmitSound( "Powerup.PickUpReflect" );
		break;
	case TF_RUNE_PRECISION:
		AddCond( TF_COND_RUNE_PRECISION );
		EmitSound( "Powerup.PickUpPrecision" );
		break;
	case TF_RUNE_AGILITY:
		AddCond( TF_COND_RUNE_AGILITY );
		EmitSound( "Powerup.PickUpAgility" );
		break;
	case TF_RUNE_KNOCKOUT:
		AddCond( TF_COND_RUNE_KNOCKOUT );
		EmitSound( "Powerup.PickUpKnockout" );
		break;
	case TF_RUNE_KING:
		AddCond( TF_COND_RUNE_KING );
		EmitSound( "Powerup.PickUpKing" );
		break;
	case TF_RUNE_PLAGUE:
		AddCond( TF_COND_RUNE_PLAGUE );
		EmitSound( "Powerup.PickUpPlague" );
		break;
	case TF_RUNE_SUPERNOVA:
		AddCond( TF_COND_RUNE_SUPERNOVA );
		EmitSound( "Powerup.PickUpSupernova" );
		break;

	default:
		break;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
RuneTypes_t CAI_BaseNPC::GetCarryingRuneType( void )
{
	if ( InCond( TF_COND_RUNE_STRENGTH ) )
		return TF_RUNE_STRENGTH;
	else if ( InCond( TF_COND_RUNE_HASTE ) )
		return TF_RUNE_HASTE;
	else if ( InCond( TF_COND_RUNE_REGEN ) )
		return TF_RUNE_REGEN;
	else if ( InCond( TF_COND_RUNE_RESIST ) )
		return TF_RUNE_RESIST;
	else if ( InCond( TF_COND_RUNE_VAMPIRE ) )
		return TF_RUNE_VAMPIRE;
	else if ( InCond( TF_COND_RUNE_WARLOCK ) )
		return TF_RUNE_WARLOCK;
	else if ( InCond( TF_COND_RUNE_PRECISION ) )
		return TF_RUNE_PRECISION;
	else if ( InCond( TF_COND_RUNE_AGILITY ) )
		return TF_RUNE_AGILITY;
	else if ( InCond( TF_COND_RUNE_KNOCKOUT ) )
		return TF_RUNE_KNOCKOUT;
	else if ( InCond( TF_COND_RUNE_KING ) )
		return TF_RUNE_KING;
	else if ( InCond( TF_COND_RUNE_PLAGUE ) )
		return TF_RUNE_PLAGUE;
	else if ( InCond( TF_COND_RUNE_SUPERNOVA ) )
		return TF_RUNE_SUPERNOVA;

	return TF_RUNE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Do CLIENT/SERVER SHARED condition thinks.
//-----------------------------------------------------------------------------
void CAI_BaseNPC::ConditionThink( void )
{
	/*if ( InCond( TF_COND_PHASE ) )
	{
		if ( gpGlobals->curtime > m_flPhaseTime )
		{
			UpdatePhaseEffects();

			// limit how often we can update in case of spam
			m_flPhaseTime = gpGlobals->curtime + 0.25f;
		}
	}*/

	if ( InCond( TF_COND_SPEED_BOOST ) )
		UpdateSpeedBoostEffects();

	//UpdateRageBuffsAndRage();

	if ( InCond( TF_COND_RUNE_KING ) )
	{
		m_bKingRuneBuffActive = true;
		#ifdef GAME_DLL
		PulseKingRuneBuff();
		#endif
	}
	else
	{
		m_bKingRuneBuffActive = false;
	}

	// Supernova
	/*if ( CanRuneCharge() )
	{
		#ifdef GAME_DLL
		float flChargeTime = tf_powerup_max_charge_time.GetFloat();
		m_flRuneCharge += ( 100.0f / flChargeTime ) * gpGlobals->frametime;
		#endif

		if ( m_flRuneCharge >= 100.0f )
		{
			m_flRuneCharge = 100.0f;
		}
	}*/
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_BaseNPC::PulseKingRuneBuff( void )
{
	CBaseEntity *pEntity = NULL;
	Vector vecOrigin = GetAbsOrigin();
	for ( CEntitySphereQuery sphere( vecOrigin, 450.0f ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pEntity )
			continue;

		Vector vecHitPoint;
		pEntity->CollisionProp()->CalcNearestPoint( vecOrigin, &vecHitPoint );
		Vector vecDir = vecHitPoint - vecOrigin;
		CTFPlayer *pPlayer = ToTFPlayer( pEntity );
		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
		if ( vecDir.LengthSqr() < ( 450.0f * 450.0f ) )
		{
			if ( pPlayer && InSameTeam( pPlayer ) )
			{
				pPlayer->m_Shared.AddCond( TF_COND_RADIUSHEAL, 1.2f );
				pPlayer->TakeHealth( 1, DMG_GENERIC );
			}
			else if ( pNPC && InSameTeam( pNPC ) )
			{
				pNPC->AddCond( TF_COND_RADIUSHEAL, 1.2f );
				pNPC->TakeHealth( 1, DMG_GENERIC );
			}
		}
	}

}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CAI_BaseNPC::GetLocalizeName( void )
{
	if ( Q_stricmp( m_pszLocalizeName, "" ) )
		return m_pszLocalizeName;

	return m_szClassname;
}
