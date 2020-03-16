//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: The system for handling npc population in horde.
//
//=============================================================================//

#include "cbase.h"
#include "lfe_population_manager.h"
#include "lfe_populator.h"
#include "igamesystem.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "soundenvelope.h"
#include "utldict.h"
#include "ai_basenpc.h"
#include "tf_gamerules.h"
#include "nav_mesh/tf_nav_mesh.h"
#include "nav_mesh/tf_nav_area.h"
#include "tf_team.h"
#include "ai_navigator.h"
#include "ai_network.h"
#include "ai_node.h"
#include "eventqueue.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar lfe_horde_debug( "lfe_horde_debug", "0", FCVAR_REPLICATED, "Display debug in horde mode." );

void CC_Horde_ForceStart( void )
{
	if ( TFGameRules() )
		TFGameRules()->State_Transition( GR_STATE_RND_RUNNING );

	if ( LFPopulationManager() )
		LFPopulationManager()->StartCurrentWave();
}
static ConCommand lfe_horde_force_start("lfe_horde_force_start", CC_Horde_ForceStart, "Force.", FCVAR_GAMEDLL | FCVAR_CHEAT);

CLFPopulationManager *g_LFEPopManager = nullptr;

//-----------------------------------------------------------------------------
// Horde Mode
//-----------------------------------------------------------------------------
class CTFLogicHorde : public CPointEntity
{
public:
	DECLARE_CLASS( CTFLogicHorde, CPointEntity );
	DECLARE_DATADESC();
	CTFLogicHorde();
	~CTFLogicHorde();

	void			Spawn( void );
};

BEGIN_DATADESC( CTFLogicHorde )
END_DATADESC()

LINK_ENTITY_TO_CLASS( lfe_logic_horde, CTFLogicHorde );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFLogicHorde::CTFLogicHorde()
{
}

CTFLogicHorde::~CTFLogicHorde()
{
	if ( LFPopulationManager() )
		delete LFPopulationManager();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFLogicHorde::Spawn( void )
{
	CLFPopulationManager *popmanager = new CLFPopulationManager;
	if ( popmanager )
		g_LFEPopManager = popmanager;

	BaseClass::Spawn();
}



BEGIN_DATADESC( CLFPopulationManager )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CLFPopulationManager::CLFPopulationManager()
{
	m_bFinale = false;
	m_pPopFile = NULL;
	m_bIsPaused = false;
	m_bIsPaused = false;

	ListenForGameEvent( "npc_death" );
}

CLFPopulationManager::~CLFPopulationManager()
{
	if ( m_pPopFile )
		m_pPopFile->deleteThis();

	m_Waves.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLFPopulationManager::FireGameEvent( IGameEvent *event )
{
	if ( FStrEq( event->GetName(), "npc_death" ) )
	{
		CAI_BaseNPC *pVictim = dynamic_cast<CAI_BaseNPC *>( UTIL_EntityByIndex( event->GetInt( "victim_index" ) ) );
		if ( pVictim )
		{
			FOR_EACH_VEC( m_Waves, i )
			{
				CWave *wave = m_Waves[i];
				if ( wave != nullptr )
					wave->OnMemberKilled( pVictim );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLFPopulationManager::GameRulesThink()
{
	FOR_EACH_VEC( m_Waves, i )
	{
		CWave *wave = m_Waves[i];
		if ( wave != nullptr )
			wave->Update();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLFPopulationManager::StartCurrentWave( void )
{
	FOR_EACH_VEC( m_Waves, i )
	{
		CWave *wave = m_Waves[i];
		if ( wave != nullptr )
			wave->ForceReset();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLFPopulationManager::Initialize( void )
{
	m_pPopFile = new KeyValues( "WaveSchedule" );
	if ( !m_pPopFile->LoadFromFile( filesystem, GetPopulationFilename(), "MOD" ) )
	{
		ConDColorMsg( Color( 77, 116, 85, 255 ), "[CLFPopulationManager] Could not load popfile '%s'. \n", GetPopulationFilename() );
		m_pPopFile->deleteThis();
		m_pPopFile = NULL;
		return;
	}

	ConColorMsg( Color( 77, 116, 85, 255 ), "[CLFPopulationManager] Loading data from %s. \n", GetPopulationFilename() );

	if ( !Q_strcmp( m_pPopFile->GetName(), "RespawnWaveTime" ) )
	{
		engine->ServerCommand( CFmtStr( "mp_respawnwavetime %f\n", m_pPopFile->GetFloat() ) );
	}

	FOR_EACH_SUBKEY( m_pPopFile, subkey )
	{
		if ( V_stricmp( subkey->GetName(), "Wave" ) == 0 )  
		{
			CWave *wave = new CWave( this );
			if ( !wave->Parse( subkey ) )
			{
				Warning( "Error reading Wave definition\n" );
				return;
			}
			m_Waves.AddToTail( wave );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when a new round is being initialized
//-----------------------------------------------------------------------------
void CLFPopulationManager::SetupOnRoundStart( void )
{
	for ( int i = FIRST_GAME_TEAM; i < MAX_TEAMS; i++ )
	{
		if ( TFGameRules()->IsHordeMode() )
		{
			TFGameRules()->BroadcastSound( i, "music.mvm_class_select" );
		}
	}

	Initialize();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLFPopulationManager::SetupTimerExpired( void )
{
	StartCurrentWave();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CLFPopulationManager::GetPopulationFilename( void )
{
	const char szFullName = NULL;
	Q_snprintf( szFullName,sizeof(szFullName), "scripts/population/%s.pop", STRING( gpGlobals->mapname ) );
	return szFullName;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CSpawnLocation::CSpawnLocation()
{
	m_iWhere = Where::TEAMSPAWN;
}


bool CSpawnLocation::Parse( KeyValues *kv )
{
	return true;
}

SpawnResult CSpawnLocation::FindSpawnLocation( Vector& vec ) const
{
	return SPAWN_NORMAL;
}

CTFNavArea *CSpawnLocation::SelectSpawnArea() const
{
	return NULL;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
IPopulator::IPopulator( CLFPopulationManager *popmgr )
	: m_PopMgr( popmgr )
{
}

IPopulator::~IPopulator()
{
	if ( m_Spawner != nullptr ) 
		delete m_Spawner;
}

void IPopulator::PostInitialize()
{
}

void IPopulator::Update()
{
}

void IPopulator::UnpauseSpawning()
{
}

void IPopulator::OnMemberKilled( CBaseEntity *pMember )
{
}

/*bool IPopulator::HasEventChangeAttributes(const char *name) const
{
	if (this->m_Spawner == nullptr) {
		return false;
	}
	
	return this->m_Spawner->HasEventChangeAttributes(name);
}*/


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CWave::CWave( CLFPopulationManager *popmgr )
: IPopulator( popmgr )
{
	m_iTotalCurrency = 0;
}

CWave::~CWave()
{
}

bool CWave::Parse( KeyValues *kv )
{
	FOR_EACH_SUBKEY( kv, subkey )
	{
		if ( V_stricmp(subkey->GetName(), "WaveSpawn") == 0)  
		{
			CWaveSpawnPopulator *wavespawn = new CWaveSpawnPopulator( m_PopMgr );
			if ( wavespawn )
			{
				if ( !wavespawn->Parse( subkey ) )
				{
					Warning("Error reading WaveSpawn definition\n");
					return false;
				}
				m_WaveSpawns.AddToTail( wavespawn );

				m_iTotalCurrency += wavespawn->m_iTotalCurrency;

				wavespawn->m_Wave = this;
			}
		}
		else if ( V_stricmp(subkey->GetName(), "Sound" ) == 0)
		{
			m_strSound.sprintf("%s", subkey->GetString());
		}
		else if ( V_stricmp( subkey->GetName(), "Description" ) == 0 )
		{
			m_strDescription.sprintf( "%s", subkey->GetString() );
		}
		else if ( V_stricmp( subkey->GetName(), "WaitWhenDone" ) == 0 )
		{
			m_flWaitWhenDone = subkey->GetFloat();
		}
		else if ( V_stricmp( subkey->GetName(), "Checkpoint" ) == 0 )
		{
			/* doesn't do anything! */
		}
		else if ( V_stricmp( subkey->GetName(), "StartWaveOutput" ) == 0 )
		{
			m_StartWaveOutput = ParseEvent( subkey );
		}
		else if ( V_stricmp(subkey->GetName(), "DoneOutput" ) == 0 )
		{
			m_DoneOutput = ParseEvent( subkey );
		}
		else if ( V_stricmp( subkey->GetName(), "InitWaveOutput" ) == 0 )
		{
			m_InitWaveOutput = ParseEvent( subkey );
		}
		else 
		{
			Warning( "[CWave] Unknown attribute '%s' in Wave definition.\n", subkey->GetName() );
		}
	}

	return true;
}

void CWave::Update()
{
	if ( TFGameRules() == nullptr )
		return;

	gamerules_roundstate_t roundstate = TFGameRules()->State_Get();

	if ( roundstate == GR_STATE_RND_RUNNING )
	{
		ActiveWaveUpdate();
	}
	else if ( roundstate == GR_STATE_TEAM_WIN || roundstate == GR_STATE_BETWEEN_RNDS ) 
	{
		WaveIntermissionUpdate();
	}
}

void CWave::OnMemberKilled( CBaseEntity *pMember )
{
	FOR_EACH_VEC( m_WaveSpawns, i )
	{
		m_WaveSpawns[i]->OnMemberKilled( pMember );
	}
}

/*bool CWave::HasEventChangeAttributes(const char *name) const
{
	FOR_EACH_VEC( m_WaveSpawns, i )
	{
		if ( m_WaveSpawns[i]->HasEventChangeAttributes(name) )
		{
			return true;
		}
	}
	
	return false;
}*/

bool CWave::IsDoneWithNonSupportWaves()
{
	FOR_EACH_VEC( m_WaveSpawns, i )
	{
		CWaveSpawnPopulator *wavespawn = m_WaveSpawns[i];
		if ( wavespawn != nullptr && !wavespawn->m_bSupport && wavespawn->m_iState != CWaveSpawnPopulator::InternalStateType::DONE )
			return false;
	}

	return true;
}

void CWave::ForceFinish()
{
	FOR_EACH_VEC( m_WaveSpawns, i )
	{
		m_WaveSpawns[i]->ForceFinish();
	}
}


void CWave::ForceReset()
{
	FOR_EACH_VEC( m_WaveSpawns, i )
	{
		CWaveSpawnPopulator *wavespawn = m_WaveSpawns[i];
		if ( wavespawn != nullptr )
		{
			wavespawn->m_iState = CWaveSpawnPopulator::InternalStateType::INITIAL;
			wavespawn->m_iCurrencyLeft = wavespawn->m_iTotalCurrency;
			wavespawn->m_iCountNotYetSpawned = wavespawn->m_iTotalCount;
		}
	}
}

CWaveSpawnPopulator *CWave::FindWaveSpawnPopulator( const char *name )
{
	FOR_EACH_VEC( m_WaveSpawns, i )
	{
		CWaveSpawnPopulator *wavespawn = m_WaveSpawns[i];
		if ( wavespawn != nullptr )
		{
			if ( V_stricmp( wavespawn->m_strName.Get(), name ) == 0 )
			{
				return wavespawn;
			}
		}
	}
	
	return nullptr;
}

void CWave::ActiveWaveUpdate()
{
	FOR_EACH_VEC( m_WaveSpawns, i )
	{
		CWaveSpawnPopulator *wavespawn = m_WaveSpawns[i];
		if ( wavespawn != nullptr )
			wavespawn->Update();
	}
}

void CWave::WaveCompleteUpdate()
{
}

void CWave::WaveIntermissionUpdate()
{
}

CWaveSpawnPopulator::CWaveSpawnPopulator( CLFPopulationManager *popmgr )
	: IPopulator( popmgr )
{
	m_iTotalCurrency = 0;
}

CWaveSpawnPopulator::~CWaveSpawnPopulator()
{
}

bool CWaveSpawnPopulator::Parse( KeyValues *kv )
{
	/*KeyValues *kv_tref = kv->FindKey( "Template" );
	if (k v_tref != nullptr )
	{
		const char *tname = kv_tref->GetString();
		
		KeyValues *kv_timpl = m_PopMgr->m_kvTemplates->FindKey( tname );
		if ( kv_timpl != nullptr ) 
		{
			if ( !Parse( kv_timpl ) )
				return false;
		}
		else 
		{
			Warning( "Unknown Template '%s' in WaveSpawn definition\n", tname );
		}
	}*/
	
	FOR_EACH_SUBKEY( kv, subkey )
	{
		const char *name = subkey->GetName();
		if ( strlen(name) <= 0 )
			continue;

		if ( m_Where.Parse( subkey ) )
			continue;

		/*if ( V_stricmp(name, "Template" ) == 0 )
			continue;*/

		if ( V_stricmp( name, "TotalCount" ) == 0 )
		{
			m_iTotalCount = subkey->GetInt();
		}
		else if ( V_stricmp( name, "MaxActive" ) == 0 )
		{
			m_iMaxActive = subkey->GetInt();
		}
		else if ( V_stricmp( name, "SpawnCount" ) == 0 )
		{
			m_iSpawnCount = subkey->GetInt();
		}
		else if ( V_stricmp( name, "WaitBeforeStarting" ) == 0 )
		{
			m_flWaitBeforeStarting = subkey->GetFloat();
		}
		else if ( V_stricmp( name, "WaitBetweenSpawns" ) == 0 )
		{
			if ( m_flWaitBetweenSpawns == 0.0f || !m_bWaitBetweenSpawnsAfterDeath )
			{
				m_flWaitBetweenSpawns = subkey->GetFloat();
			}
			else
			{
				Warning("Already specified WaitBetweenSpawnsAfterDeath time, ""WaitBetweenSpawns won't be used\n");
				continue;
			}
		}
		else if ( V_stricmp( name, "WaitBetweenSpawnsAfterDeath" ) == 0 )
		{
			if ( m_flWaitBetweenSpawns == 0.0f )
			{
				m_bWaitBetweenSpawnsAfterDeath = true;
				m_flWaitBetweenSpawns = subkey->GetFloat();
			}
			else
			{
				Warning( "Already specified WaitBetweenSpawns time, ""WaitBetweenSpawnsAfterDeath won't be used\n" );
				continue;
			}
		}
		else if ( V_stricmp( name, "StartWaveWarningSound" ) == 0 )
		{
			m_strStartWaveWarningSound.sprintf( "%s",subkey->GetString() );
		}
		else if ( V_stricmp( name, "StartWaveOutput" ) == 0 )
		{
			m_StartWaveOutput = ParseEvent( subkey );
		} 
		else if (V_stricmp( name, "FirstSpawnWarningSound" ) == 0 )
		{
			m_strFirstSpawnWarningSound.sprintf( "%s", subkey->GetString() );
		}
		else if ( V_stricmp( name, "FirstSpawnOutput" ) == 0 )
		{
			m_FirstSpawnOutput = ParseEvent(subkey);
		}
		else if ( V_stricmp( name, "LastSpawnWarningSound" ) == 0 )
		{
			m_strLastSpawnWarningSound.sprintf( "%s", subkey->GetString() );
		}
		else if ( V_stricmp( name, "LastSpawnOutput")  == 0 )
		{
			m_LastSpawnOutput = ParseEvent( subkey );
		}
		else if ( V_stricmp( name, "DoneWarningSound" ) == 0 )
		{
			m_strDoneWarningSound.sprintf( "%s",subkey->GetString() );
		}
		else if ( V_stricmp( name, "DoneOutput" ) == 0 )
		{
			m_DoneOutput = ParseEvent( subkey );
		}
		else if ( V_stricmp( name, "TotalCurrency" ) == 0 )
		{
			m_iTotalCurrency = subkey->GetInt();
		}
		else if ( V_stricmp( name, "Name" ) == 0 )
		{
			m_strName = subkey->GetString();
		}
		else if ( V_stricmp( name, "WaitForAllSpawned" ) == 0 )
		{
			m_strWaitForAllSpawned = subkey->GetString();
		}
		else if ( V_stricmp( name, "WaitForAllDead" ) == 0 ) 
		{
			m_strWaitForAllDead = subkey->GetString();
		}
		else if ( V_stricmp( name, "Support" ) == 0 ) 
		{
			m_bSupport = true;
			m_bSupportLimited = ( V_stricmp(subkey->GetString(), "Limited" ) == 0 );
		}
		else if ( V_stricmp( name, "RandomSpawn" ) == 0 )
		{
			m_bRandomSpawn = subkey->GetBool();
		}
		else
		{
			m_Spawner = IPopulationSpawner::ParseSpawner( this, subkey );
			if ( m_Spawner == nullptr)  
			{
				Warning( "Unknown attribute '%s' in WaveSpawn definition.\n", name );
			}
		}

		m_iCountNotYetSpawned = m_iTotalCount;
		m_iCurrencyLeft = m_iTotalCurrency;
	}

	return true;
}

void CWaveSpawnPopulator::Update()
{
	switch ( m_iState )
	{
	case InternalStateType::INITIAL:
		m_ctSpawnDelay.Start( m_flWaitBeforeStarting );
		SetState( InternalStateType::PRE_SPAWN_DELAY );
		break;

	case InternalStateType::PRE_SPAWN_DELAY:
		if ( m_ctSpawnDelay.IsElapsed() ) 
		{
			m_iCountSpawned = 0;
			m_iCountToSpawn = 0;
			SetState( InternalStateType::SPAWNING );
		}
		break;

	case InternalStateType::SPAWNING:
	{
		if ( !m_ctSpawnDelay.IsElapsed() || g_LFEPopManager->m_bIsPaused )
			break;

		if ( m_Spawner == nullptr )
		{
			Warning( "Invalid spawner\n" );
			SetState( InternalStateType::DONE );
			break;
		}

		int num_active = 0;
		FOR_EACH_VEC( m_ActiveBots, i ) 
		{
			CBaseEntity *ent = m_ActiveBots[i];
			if ( ent != nullptr && ent->IsAlive() )
			{
				++num_active;
			}
		}

		if ( m_bWaitBetweenSpawnsAfterDeath )
		{
			if ( num_active != 0)
				break;

			if ( m_iSpawnResult != SPAWN_FAIL )
			{
				m_iSpawnResult = SPAWN_FAIL;

				float wait_between_spawns = m_flWaitBetweenSpawns;
				if ( wait_between_spawns != 0.0f )
					m_ctSpawnDelay.Start( wait_between_spawns );

				break;
			}
		}

		int max_active = m_iMaxActive;
		if ( num_active >= max_active )
			break;

		if ( m_iCountToSpawn <= 0 )
		{
			if ( num_active + m_iSpawnCount > max_active )
				break;

			m_iCountToSpawn = m_iSpawnCount;
		}

		Vector vec_spawn = vec3_origin;
		CBaseEntity *pTeamSpawn = gEntList.FindEntityByClassname( NULL, "info_player_teamspawn" );
		if ( pTeamSpawn )
			vec_spawn = pTeamSpawn->GetAbsOrigin();

		/*if ( m_Spawner->IsWhereRequired() )
		{
			if ( m_iSpawnResult != SPAWN_NORMAL )
			{
				m_iSpawnResult = m_Where.FindSpawnLocation( m_vecSpawn );
				if (m_iSpawnResult == SPAWN_FAIL) 
				{
					break;
				}
			}

			vec_spawn = m_vecSpawn;
			if ( m_bRandomSpawn )
			{
				m_iSpawnResult = SPAWN_FAIL;
			}
		}*/

		CUtlVector<CHandle<CBaseEntity>> spawned;
		if ( m_Spawner->Spawn( vec_spawn, &spawned ) == 0 )
		{
			m_ctSpawnDelay.Start( 1.0f );
			break;
		}

		FOR_EACH_VEC( spawned, i )
		{
			CBaseEntity *ent = spawned[i];

			CAI_BaseNPC *bot = dynamic_cast<CAI_BaseNPC*>(ent);
			if ( bot == nullptr )
				continue;

			//bot->m_nCurrency = 0;
		}

		int num_spawned = spawned.Count();
		m_iCountSpawned += num_spawned;

		int count_to_spawn = m_iCountToSpawn;
		if ( num_spawned > count_to_spawn )
			num_spawned = count_to_spawn;

		m_iCountToSpawn -= num_spawned;

		FOR_EACH_VEC( spawned, i ) 
		{
			CBaseEntity *ent1 = spawned[i];

			FOR_EACH_VEC( m_ActiveBots, j )
			{
				CBaseEntity *ent2 = m_ActiveBots[j];
				if ( ent2 == nullptr )
					continue;

				if ( ent1->entindex() == ent2->entindex() ) 
				{
					Warning( "WaveSpawn duplicate entry in active vector\n" );
				}
			}

			m_ActiveBots.AddToTail( ent1 );
		}

		if ( IsFinishedSpawning() )
		{
			SetState( InternalStateType::WAIT_FOR_ALL_DEAD );
		}
		else if ( m_iCountToSpawn <= 0 && !m_bWaitBetweenSpawnsAfterDeath )
		{
			m_iSpawnResult = SPAWN_FAIL;

			float wait_between_spawns = m_flWaitBetweenSpawns;
			if ( wait_between_spawns != 0.0f )
				m_ctSpawnDelay.Start( wait_between_spawns );
		}

		break;
	}

	case InternalStateType::WAIT_FOR_ALL_DEAD:
		FOR_EACH_VEC( m_ActiveBots, i )
		{
			CBaseEntity *ent = m_ActiveBots[i];
			if ( ent != nullptr && ent->IsAlive() )
			{
				break;
			}
		}

		SetState( InternalStateType::DONE );
		break;
	}
}

void CWaveSpawnPopulator::OnMemberKilled( CBaseEntity *pMember )
{
	m_ActiveBots.FindAndFastRemove( pMember );
}

bool CWaveSpawnPopulator::IsFinishedSpawning()
{
	if ( m_bSupport && !m_bSupportLimited )
		return false;

	return ( m_iCountSpawned >= m_iTotalCount );
}

void CWaveSpawnPopulator::OnNonSupportWavesDone()
{
	if ( !m_bSupport )
		return;

	int state = m_iState;
	if ( state == InternalStateType::INITIAL || state == InternalStateType::PRE_SPAWN_DELAY ) 
	{
		SetState( InternalStateType::DONE );
	} 
	else if ( state == InternalStateType::SPAWNING || state == InternalStateType::WAIT_FOR_ALL_DEAD )
	{
		if ( TFGameRules() != nullptr && m_iCurrencyLeft > 0)  
		{
			/*TFGameRules()->DistributeCurrencyAmount(m_iCurrencyLeft, nullptr, true, true, false);*/
			m_iCurrencyLeft = 0;
		}

		SetState( InternalStateType::WAIT_FOR_ALL_DEAD );
	}
}

void CWaveSpawnPopulator::ForceFinish()
{
	int state = m_iState;
	if ( state == InternalStateType::INITIAL || state == InternalStateType::PRE_SPAWN_DELAY || state == InternalStateType::SPAWNING ) 
	{
		SetState( InternalStateType::WAIT_FOR_ALL_DEAD );
	}
	else if ( state != InternalStateType::WAIT_FOR_ALL_DEAD )
	{
		SetState( InternalStateType::DONE );
	}

	FOR_EACH_VEC( m_ActiveBots, i )
	{
		CBaseEntity *ent = m_ActiveBots[i];

		CAI_BaseNPC *bot = dynamic_cast<CAI_BaseNPC*>(ent);
		if ( bot != nullptr ) 
		{
			bot->SetHealth( 0 );
		}
		else
		{
			UTIL_Remove( ent );
		}
	}

	m_ActiveBots.RemoveAll();
}

int CWaveSpawnPopulator::GetCurrencyAmountPerDeath()
{
	if ( m_bSupport && m_iState == InternalStateType::WAIT_FOR_ALL_DEAD )
		m_iCountNotYetSpawned = m_ActiveBots.Count();

	int currency_left = m_iCurrencyLeft;
	if ( currency_left <= 0 )
		return 0;
	
	int bots_left = m_iCountNotYetSpawned;
	if ( bots_left <= 0 )
		bots_left = 1;
	
	int amount = ( currency_left / bots_left );
	--m_iCountNotYetSpawned;
	m_iCurrencyLeft -= amount;

	return amount;
}

void CWaveSpawnPopulator::SetState( CWaveSpawnPopulator::InternalStateType newstate )
{
	m_iState = newstate;

	if ( newstate == InternalStateType::PRE_SPAWN_DELAY ) 
	{
		if ( m_strStartWaveWarningSound.Length() > 0 )
			TFGameRules()->BroadcastSound( 255,m_strStartWaveWarningSound.String() );

		FireEvent( m_StartWaveOutput, "StartWaveOutput" );

		if ( lfe_horde_debug.GetBool() )
			DevMsg("%3.2f: WaveSpawn(%s) started PRE_SPAWN_DELAY\n", gpGlobals->curtime, m_strName.Get() );
	}
	else if ( newstate == InternalStateType::SPAWNING )
	{
		if ( m_strFirstSpawnWarningSound.Length() > 0 )
			TFGameRules()->BroadcastSound( 255,m_strFirstSpawnWarningSound.String() );

		FireEvent( m_FirstSpawnOutput, "FirstSpawnOutput" );

		if ( lfe_horde_debug.GetBool() )
			DevMsg( "%3.2f: WaveSpawn(%s) started SPAWNING\n", gpGlobals->curtime, m_strName.Get() );
	}
	else if ( newstate == InternalStateType::WAIT_FOR_ALL_DEAD )
	{
		if ( m_strLastSpawnWarningSound.Length() > 0 )
			TFGameRules()->BroadcastSound( 255,m_strLastSpawnWarningSound.String() );

		FireEvent( m_LastSpawnOutput, "LastSpawnOutput" );

		if ( lfe_horde_debug.GetBool() )
			DevMsg( "%3.2f: WaveSpawn(%s) started WAIT_FOR_ALL_DEAD\n", gpGlobals->curtime, m_strName.Get() );
	}
	else if ( newstate == InternalStateType::DONE )
	{
		if ( m_strDoneWarningSound.Length() > 0 )
			TFGameRules()->BroadcastSound( 255,m_strDoneWarningSound.String() );

		FireEvent( m_DoneOutput, "DoneOutput" );

		if ( lfe_horde_debug.GetBool() )
			DevMsg( "%3.2f: WaveSpawn(%s) DONE\n", gpGlobals->curtime, m_strName.Get() );
	}
}

EventInfo *ParseEvent( KeyValues *kv )
{
	EventInfo *info = new EventInfo();

	FOR_EACH_SUBKEY( kv, subkey )
	{
		const char *name = subkey->GetName();
		if ( strlen( name ) <= 0 )
			continue;

		if ( V_stricmp( name, "Target" ) == 0 ) 
		{
			info->target.sprintf( subkey->GetString() );
		}
		else if ( V_stricmp( name, "Action" ) == 0 ) 
		{
			info->action.sprintf( subkey->GetString() );
		}
		else
		{
			Warning( "Unknown field '%s' in WaveSpawn event definition.\n", subkey->GetString() );
			delete info;
			return nullptr;
		}
	}
	
	return info;
}

void FireEvent( EventInfo *info, const char *name )
{
	if ( info == nullptr )
		return;
	
	const char *target = info->target.Get();

	const char *action = info->action.Get();
	
	CBaseEntity *ent = gEntList.FindEntityByName( nullptr, target );
	if ( ent != nullptr )
	{
		g_EventQueue.AddEvent( ent, action, 0.0f, nullptr, nullptr );
	}
	else 
	{
		Warning( "WaveSpawnPopulator: Can't find target entity '%s' for %s\n", target, name );
	}
}
