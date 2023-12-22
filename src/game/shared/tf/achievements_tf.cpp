//===========================================================================//
//
// Purpose: Achievements
//
//===========================================================================//


#include "cbase.h"

#ifdef CLIENT_DLL

#include "achievements_tf.h"
#include "tf_hud_statpanel.h"
#include "c_tf_team.h"
#include "c_tf_player.h"

CAchievementMgr g_AchievementMgrTF;	// global achievement mgr for TF

bool CheckWinNoEnemyCaps( IGameEvent *event, int iRole );

// Grace period that we allow a player to start after level init and still consider them to be participating for the full round.  This is fairly generous
// because it can in some cases take a client several minutes to connect with respect to when the server considers the game underway
#define TF_FULL_ROUND_GRACE_PERIOD	( 4 * 60.0f )

bool IsLocalTFPlayerClass( int iClass );

//-----------------------------------------------------------------------------
// Purpose: see if a round win was a win for the local player with no enemy caps
//-----------------------------------------------------------------------------
bool CheckWinNoEnemyCaps( IGameEvent *event, int iRole )
{
	if ( 0 == Q_strcmp( event->GetName(), "teamplay_round_win" ) )
	{
		if ( event->GetInt( "team" ) == GetLocalPlayerTeam() )
		{
			int iLosingTeamCaps = event->GetInt( "losing_team_num_caps" );
			if ( 0 == iLosingTeamCaps )
			{
				C_TFTeam *pLocalTeam = GetGlobalTFTeam( GetLocalPlayerTeam() );
				if ( pLocalTeam )
				{
					int iRolePlayer = pLocalTeam->GetRole();
					if ( iRole > TEAM_ROLE_NONE && ( iRolePlayer != iRole ) )
						return false;
					return true;
				}
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Helper function to determine if local player is specified class
//-----------------------------------------------------------------------------
bool IsLocalTFPlayerClass( int iClass )
{
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	return( pLocalPlayer && pLocalPlayer->IsPlayerClass( iClass ) );
}

class CAchievementTFGetHealPoints : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 25000 );		
	}

	void OnPlayerStatsUpdate()
	{
		ClassStats_t &classStats = CTFStatPanel::GetClassStats( TF_CLASS_MEDIC );
		int iOldCount = m_iCount;
		m_iCount = classStats.accumulated.m_iStat[TFSTAT_HEALING];
		if ( m_iCount != iOldCount )
		{
			m_pAchievementMgr->SetDirty( true );
		}

		if ( IsLocalTFPlayerClass( TF_CLASS_MEDIC ) )
		{
			EvaluateNewAchievement();
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFGetHealPoints, ACHIEVEMENT_TF_GET_HEALPOINTS, "TF_GET_HEALPOINTS", 5 );

class CAchievementTFGetTurretKills : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );		
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT( CAchievementTFGetTurretKills, ACHIEVEMENT_TF_GET_TURRETKILLS, "TF_GET_TURRETKILLS", 5 );

class CAchievementTFGetHeadshots: public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 25 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( Q_strcmp( event->GetName(), "player_death" ) == 0 || Q_strcmp( event->GetName(), "npc_death" ) == 0 )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_HEADSHOT ) && ( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFGetHeadshots, ACHIEVEMENT_TF_GET_HEADSHOTS, "TF_GET_HEADSHOTS", 5 );

class CAchievementTFGetMultipleKills : public CBaseAchievement
{
	void Init() 
	{
		// listen for player kill enemy events, base class will increment count each time that happens
		SetFlags( ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL );
		SetGoal( 1000 );
	}

	void OnPlayerStatsUpdate()
	{
		// when stats are updated by server, use most recent stat values

		int iKills = 0;
		// get sum of kills per class across all classes to get total kills
		for ( int iClass = TF_FIRST_NORMAL_CLASS; iClass <= TF_CLASS_COUNT; iClass++ )
		{
			ClassStats_t &classStats = CTFStatPanel::GetClassStats( iClass );
			iKills += classStats.accumulated.m_iStat[TFSTAT_KILLS];
		}

		int iOldCount = m_iCount;
		m_iCount = iKills;
		if ( m_iCount != iOldCount )
		{
			m_pAchievementMgr->SetDirty( true );
		}

		EvaluateNewAchievement();
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFGetMultipleKills, ACHIEVEMENT_TF_GET_MULTIPLEKILLS, "TF_GET_MULTIPLEKILLS", 15 );

//-----------------------------------------------------------------------------
//
//
//
//
//
//-----------------------------------------------------------------------------

// The Wrong Mann in the Wrong Place
class CAchievementLFEBeatHL2 : public CBaseAchievement
{
	DECLARE_CLASS( CAchievementLFEBeatHL2, CBaseAchievement );
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS );

		static const char *szComponents[] =
		{
			"d1_trainstation_01",
			"d1_trainstation_02",
			"d1_trainstation_03",
			"d1_trainstation_04",
			"d1_trainstation_05",
			"d1_trainstation_06",
			"d1_canals_01",
			"d1_canals_01a",
			"d1_canals_02",
			"d1_canals_03",
			"d1_canals_05",
			"d1_canals_06",
			"d1_canals_07",
			"d1_canals_08",
			"d1_canals_09",
			"d1_canals_10",
			"d1_canals_11",
			"d1_canals_12",
			"d1_canals_13",
			"d1_eli_01",
			"d1_eli_02",
			"d1_town_01",
			"d1_town_01a",
			"d1_town_02",
			"d1_town_03",
			"d1_town_02a",
			"d1_town_04",
			"d1_town_05",
			"d2_coast_01",
			"d2_coast_03",
			"d2_coast_04",
			"d2_coast_05",
			"d2_coast_07",
			"d2_coast_08",
			"d2_coast_09",
			"d2_coast_10",
			"d2_coast_11",
			"d2_coast_12",
			"d2_prison_01",
			"d2_prison_02",
			"d2_prison_03",
			"d2_prison_04",
			"d2_prison_05",
			"d2_prison_06",
			"d2_prison_07",
			"d2_prison_08",
			"d3_c17_02",
			"d3_c17_04",
			"d3_c17_05",
			"d3_c17_06a",
			"d3_c17_06b",
			"d3_c17_07",
			"d3_c17_08",
			"d3_c17_09",
			"d3_c17_10a",
			"d3_c17_10b",
			"d3_c17_11",
			"d3_c17_12",
			"d3_c17_12b",
			"d3_c17_13",
			"d3_citadel_01",
			"d3_citadel_02",
			"d3_citadel_03",
			"d3_citadel_04",
			"d3_citadel_05",
			"d3_breen_01",
		};
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE( szComponents );
		SetGoal( m_iNumComponents );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "tf_changelevel" );
		ListenForGameEvent( "credits_outro_roll" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( Q_strcmp( event->GetName(), "tf_changelevel" ) == 0 || Q_strcmp( event->GetName(), "credits_outro_roll" ) == 0 )
		{
			OnComponentEvent( m_pAchievementMgr->GetMapName() );
		}
	}
			
};
DECLARE_ACHIEVEMENT( CAchievementLFEBeatHL2, ACHIEVEMENT_LFE_BEAT_HL2, "LFE_BEAT_HL2", 15 );

// City Destruction
class CAchievementLFEBeatHL2EP1 : public CBaseAchievement
{
	DECLARE_CLASS( CAchievementLFEBeatHL2EP1, CBaseAchievement );
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS );

		static const char *szComponents[] =
		{
			"ep1_citadel_00",
			"ep1_citadel_01",
			"ep1_citadel_02",
			"ep1_citadel_02b",
			"ep1_citadel_03",
			"ep1_citadel_04",
			"ep1_c17_00",
			"ep1_c17_00a",
			"ep1_c17_01"
			"ep1_c17_01a",
			"ep1_c17_02",
			"ep1_c17_02b",
			"ep1_c17_02a",
			"ep1_c17_05",
			"ep1_c17_06",
		};
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE( szComponents );
		SetGoal( m_iNumComponents );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "tf_changelevel" );
		ListenForGameEvent( "credits_outro_roll" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( Q_strcmp( event->GetName(), "tf_changelevel" ) == 0 || Q_strcmp( event->GetName(), "credits_outro_roll" ) == 0 )
		{
			OnComponentEvent( m_pAchievementMgr->GetMapName() );
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEBeatHL2EP1, ACHIEVEMENT_LFE_BEAT_HL2EP1, "LFE_BEAT_HL2EP1", 15 );


// Foreseen Consequences
class CAchievementLFEBeatHL2EP2 : public CBaseAchievement
{
	DECLARE_CLASS( CAchievementLFEBeatHL2EP2, CBaseAchievement );
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS );

		static const char *szComponents[] =
		{
			"ep2_outland_01",
			"ep2_outland_01a",
			"ep2_outland_02",
			"ep2_outland_03",
			"ep2_outland_04",
			"ep2_outland_05",
			"ep2_outland_06",
			"ep2_outland_06a",
			"ep2_outland_07",
			"ep2_outland_08",
			"ep2_outland_09",
			"ep2_outland_10a",
			"ep2_outland_11",
			"ep2_outland_11a",
			"ep2_outland_11b",
			"ep2_outland_12",
			"ep2_outland_12a",
		};
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE( szComponents );
		SetGoal( m_iNumComponents );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "tf_changelevel" );
		ListenForGameEvent( "credits_outro_roll" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( Q_strcmp( event->GetName(), "tf_changelevel" ) == 0 || Q_strcmp( event->GetName(), "credits_outro_roll" ) == 0 )
		{
			OnComponentEvent( m_pAchievementMgr->GetMapName() );
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEBeatHL2EP2, ACHIEVEMENT_LFE_BEAT_HL2EP2, "LFE_BEAT_HL2EP2", 15 );


// Technical Teleport
class CAchievementLFEBeatP1 : public CBaseAchievement
{
	DECLARE_CLASS( CAchievementLFEBeatP1, CBaseAchievement );
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS );

		static const char *szComponents[] =
		{
			"testchmb_a_00",
			"testchmb_a_01",
			"testchmb_a_02",
			"testchmb_a_03",
			"testchmb_a_04",
			"testchmb_a_05",
			"testchmb_a_06",
			"testchmb_a_07",
			"testchmb_a_08",
			"testchmb_a_08_advanced",
			"testchmb_a_09",
			"testchmb_a_09_advanced",
			"testchmb_a_10",
			"testchmb_a_10_advanced",
			"testchmb_a_11",
			"testchmb_a_11_advanced",
			"testchmb_a_13",
			"testchmb_a_13_advanced",
			"testchmb_a_14",
			"testchmb_a_14_advanced",
			"testchmb_a_15",
			"escape_00",
			"escape_01",
			"escape_02",
		};
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE( szComponents );
		SetGoal( m_iNumComponents );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "tf_changelevel" );
		ListenForGameEvent( "credits_outro_roll" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( Q_strcmp( event->GetName(), "tf_changelevel" ) == 0 || Q_strcmp( event->GetName(), "credits_outro_roll" ) == 0 )
		{
			OnComponentEvent( m_pAchievementMgr->GetMapName() );
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEBeatP1, ACHIEVEMENT_LFE_BEAT_P1, "LFE_BEAT_P1", 15 );


static const char *pszNPCCombine[] =
{
	"npc_advisor",
	"npc_combine_s",
	"npc_combinedropship",
	"npc_combinegunship",
	"npc_strider",
	"npc_metropolice",
	"npc_cscanner",
	"npc_sniper",
	"npc_clawscanner",
	"npc_stalker",
	"npc_hunter",
};

// True Civil Protection
class CAchievementLFEKillCombine : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 100 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				for ( int i = 0; i < ARRAYSIZE( pszNPCCombine ); i++ )
				{
					if ( ( !Q_strcmp( event->GetString( "victim_name" ), pszNPCCombine[i] ) ) && ( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() || event->GetInt( "assister_index" ) == pLocalPlayer->entindex() ) )
					{
						IncrementCount();
					}
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEKillCombine, ACHIEVEMENT_LFE_KILL_COMBINE, "LFE_KILL_COMBINE", 10 );

static const char *pszNPCZombies[] =
{
	"monster_headcrab",
	"monster_babycrab",
	"monster_bigmomma",
	"monster_zombie",
	"monster_gonome",
	"npc_headcrab",
	"npc_headcrab_fast",
	"npc_headcrab_poison",
	"npc_headcrab_black",
	"npc_zombie",
	"npc_zombie_torso",
	"npc_fastzombie",
	"npc_fastzombie_torso",
	"npc_poisonzombie",
	"npc_zombine",
};

// Crab Hunt
class CAchievementLFEKillZombies : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 100 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				for ( int i = 0; i < ARRAYSIZE( pszNPCZombies ); i++ )
				{
					if ( ( !Q_strcmp( event->GetString( "victim_name" ), pszNPCZombies[i] ) ) && ( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() || event->GetInt( "assister_index" ) == pLocalPlayer->entindex() ) )
					{
						IncrementCount();
					}
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEKillZombies, ACHIEVEMENT_LFE_KILL_ZOMBIE, "LFE_KILL_ZOMBIE", 10 );

static const char *pszNPCAntlions[] =
{
	"npc_antlion",
	"npc_antlionguard",
	"npc_antlion_grub",
};

// Mann Season
class CAchievementLFEKillAntlions : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 100 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				for ( int i = 0; i < ARRAYSIZE( pszNPCAntlions ); i++ )
				{
					if ( ( !Q_strcmp( event->GetString( "victim_name" ), pszNPCAntlions[i] ) ) && ( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() || event->GetInt( "assister_index" ) == pLocalPlayer->entindex() ) )
					{
						IncrementCount();
					}
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEKillAntlions, ACHIEVEMENT_LFE_KILL_ANTLION, "LFE_KILL_ANTLION", 10 );

// Brutal Defiant
class CAchievementLFEKillCanCop : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
		SetMapNameFilter( "d1_trainstation_02" );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( !Q_strcmp( event->GetString( "victim_name" ), "npc_metropolice" ) && !Q_strcmp( event->GetString( "victim_entname" ), "cupcop" ) && ( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() || event->GetInt( "assister_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEKillCanCop, ACHIEVEMENT_LFE_KILL_CANCOP, "LFE_KILL_CANCOP", 5 );

// WTF?
class CAchievementLFEDevAbuse : public CBaseAchievement
{
	// server fires an event for this achievement, no other code within achievement necessary
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );		
	}
};
DECLARE_ACHIEVEMENT_HIDDEN( CAchievementLFEDevAbuse, ACHIEVEMENT_LFE_DEV_ABUSE, "LFE_DEV_ABUSE", 10 );

// Fight Fire with Fire
class CAchievementLFEKillGargantuaWithFlame : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( !Q_strcmp( event->GetString( "victim_name" ), "monster_gargantua" ) &&
				( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_BURNING || event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_HADOUKEN || event->GetInt( "customkill" ) == TF_DMG_CUSTOM_BURNING_FLARE || event->GetInt( "customkill" ) == TF_DMG_CUSTOM_BURNING_ARROW || event->GetInt( "customkill" ) == TF_DMG_CUSTOM_FLYINGBURN || event->GetInt( "customkill" ) == TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE || event->GetInt( "customkill" ) == TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() || event->GetInt( "assister_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEKillGargantuaWithFlame, ACHIEVEMENT_LFE_KILL_GARGANTUA_WITH_FLAME, "LFE_KILL_GARGANTUA_WITH_FLAME", 5 );

// Professionals Have Standards
/*class CAchievementLFEKillSniperWithSniper : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( !Q_strcmp( event->GetString( "victim_name" ), "npc_sniper" ) || !Q_strcmp( event->GetString( "victim_name" ), "proto_sniper" ) ) &&
				( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_HEADSHOT ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEKillSniperWithSniper, ACHIEVEMENT_LFE_KILL_SNIPER_WITH_SNIPER, "ACHIEVEMENT_LFE_KILL_SNIPER_WITH_SNIPER", 5 );*/


// Show Trial
class CAchievementTFHeavy_KillTaunt : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFHeavy_KillTaunt, ACHIEVEMENT_TF_HEAVY_KILL_TAUNT, "TF_HEAVY_KILL_TAUNT", 5 );

// OMGWTFBBQ
class CAchievementTFPyro_KillWithTaunt : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_HADOUKEN ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFPyro_KillWithTaunt, ACHIEVEMENT_TF_PYRO_KILL_TAUNT, "TF_PYRO_KILL_TAUNT", 5 );

// Out of the Park
class CAchievementTFScout_TauntKill : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFScout_TauntKill, ACHIEVEMENT_TF_SCOUT_TAUNT_KILL, "TF_SCOUT_TAUNT_KILL", 5 );

// The Man from P.U.N.C.T.U.R.E.
class CAchievementTFSpy_SpyTauntKill : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_FENCING ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFSpy_SpyTauntKill, ACHIEVEMENT_TF_SPY_TAUNT_KILL, "TF_SPY_TAUNT_KILL", 5 );

// Shafted
class CAchievementTFSniper_SniperTauntKill : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFSniper_SniperTauntKill, ACHIEVEMENT_TF_SNIPER_TAUNT_KILL, "TF_SNIPER_TAUNT_KILL", 5 );

// Spray of Defeat
class CAchievementTFSoldier_KillTaunt : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_GRENADE ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFSoldier_KillTaunt, ACHIEVEMENT_TF_SOLDIER_KILL_TAUNT, "TF_SOLDIER_KILL_TAUNT", 5 );

// Scotch Tap
class CAchievementTFDemoman_TauntKill : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFDemoman_TauntKill, ACHIEVEMENT_TF_DEMOMAN_TAUNT_KILL, "TF_DEMOMAN_TAUNT_KILL", 5 );

// Honky Tonk Man
class CAchievementTFEngineer_TauntKill : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "npc_death" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( 0 == Q_strcmp( event->GetName(), "npc_death" ) || 0 == Q_strcmp( event->GetName(), "player_death" ) )
		{
			C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pLocalPlayer )
			{
				if ( ( event->GetInt( "customkill" ) == TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH ) &&
				( event->GetInt( "attacker_index" ) == pLocalPlayer->entindex() ) )
				{
					IncrementCount();
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFEngineer_TauntKill, ACHIEVEMENT_TF_ENGINEER_TAUNT_KILL, "TF_ENGINEER_TAUNT_KILL", 5 );

class CAchievementLFEManOfFewWords : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
		SetMapNameFilter( "d1_trainstation_04" );
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFEManOfFewWords, ACHIEVEMENT_LFE_MANOFFEWWORDS, "LFE_MANOFFEWWORDS", 5 );

// AR3?
/*class CAchievementLFETauntAR3 : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
		SetMapNameFilter( "ep2_outland_11" );
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFETauntAR3, ACHIEVEMENT_LFE_TAUNT_AR3, "LFE_TAUNT_AR3", 5 );

// F***! You
class CAchievementLFETauntBreenFYou : public CBaseAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
		SetMapNameFilter( "d3_breen_01" );
	}
};
DECLARE_ACHIEVEMENT( CAchievementLFETauntBreenFYou, ACHIEVEMENT_LFE_TAUNT_BREEN, "LFE_TAUNT_BREEN", 5 );*/

// names from live tf2
// CAchievementTFPyro_IgniteWithRainbow

// news stuff
// CAchievementLFESavingSandyLaszo
#endif // CLIENT_DLL