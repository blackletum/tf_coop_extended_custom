//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: The system for handling npc population in horde.
//
//=============================================================================//
#ifndef LFE_POPULATION_MANAGER_H
#define LFE_POPULATION_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/icommandline.h"
#include "filesystem.h"
#include <KeyValues.h>
#include "gamestats.h"
#include "fmtstr.h"

class CTFNavArea;
class CWave;
class IPopulationSpawner;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CLFPopulationManager : public CPointEntity, public CGameEventListener
{
public:
	DECLARE_CLASS( CLFPopulationManager, CPointEntity );
	DECLARE_DATADESC();

	CLFPopulationManager();
	~CLFPopulationManager();

	virtual void	FireGameEvent( IGameEvent *event );

	virtual void	GameRulesThink();

	void			Initialize( void );
	void			StartCurrentWave( void );

	void			SetupOnRoundStart( void );
	void			SetupTimerExpired( void );

	const char		*GetPopulationFilename( void );

	bool			m_bFinale;

	//KeyValues	*m_kvTemplates;

	bool m_bIsPaused;

	CUtlVector<CWave *> m_Waves;

	bool IsInEndlessWaves() { return m_bIsEndless; }
protected:
	KeyValues	*m_pPopFile;
private:

	int		m_nEventPopfileType;
	bool	m_bIsEndless;
};

extern CLFPopulationManager *g_LFEPopManager;

inline CLFPopulationManager* LFPopulationManager( void )
{
	return g_LFEPopManager;
}



struct EventInfo
{
	CFmtStrN<256> target;
	CFmtStrN<256> action;
};


enum SpawnResult
{
	SPAWN_FAIL     = 0,
	SPAWN_NORMAL   = 1,
	SPAWN_TELEPORT = 2,
};

class CSpawnLocation
{
public:
	enum class Where : int
	{
		TEAMSPAWN = 0,
		AHEAD     = 1,
		BEHIND    = 2,
		ANYWHERE  = 3,
	};
	
	CSpawnLocation();
	
	bool Parse( KeyValues *kv );
	SpawnResult FindSpawnLocation(Vector& vec) const;
	
private:
	CTFNavArea *SelectSpawnArea() const;
	
	Where m_iWhere;
	CUtlVector<CHandle<CBaseEntity>> m_Spawns;
};

class IPopulator
{
public:
	IPopulator( CLFPopulationManager *popmgr );
	virtual ~IPopulator();
	
	virtual bool Parse( KeyValues *kv ) = 0;
	virtual void PostInitialize();
	virtual void Update();
	virtual void UnpauseSpawning();
	virtual void OnMemberKilled( CBaseEntity *pMember );
	//virtual bool HasEventChangeAttributes(const char *name) const;

	CLFPopulationManager *m_PopMgr;
protected:
	IPopulationSpawner *m_Spawner;
};

class CWaveSpawnPopulator : public IPopulator
{
public:
	CWaveSpawnPopulator( CLFPopulationManager *popmgr );
	virtual ~CWaveSpawnPopulator();
	
	virtual bool Parse( KeyValues *kv ) override;
	virtual void Update() override;
	virtual void OnMemberKilled( CBaseEntity *pMember ) override;
	
	bool IsFinishedSpawning();
	void OnNonSupportWavesDone();
	void ForceFinish();
	
	int GetCurrencyAmountPerDeath();

public:
	enum InternalStateType : int
	{
		INITIAL           = 0,
		PRE_SPAWN_DELAY   = 1,
		SPAWNING          = 2,
		WAIT_FOR_ALL_DEAD = 3,
		DONE              = 4,
	};

	void SetState( InternalStateType newstate );

	CSpawnLocation m_Where;
	int m_iTotalCount;
	int m_iCountNotYetSpawned;
	int m_iMaxActive;
	int m_iSpawnCount;
	float m_flWaitBeforeStarting;
	float m_flWaitBetweenSpawns;
	bool m_bWaitBetweenSpawnsAfterDeath;
	CFmtStrN<256> m_strStartWaveWarningSound;
	EventInfo *m_StartWaveOutput;
	CFmtStrN<256> m_strFirstSpawnWarningSound;
	EventInfo *m_FirstSpawnOutput;
	CFmtStrN<256> m_strLastSpawnWarningSound;
	EventInfo *m_LastSpawnOutput;
	CFmtStrN<256> m_strDoneWarningSound;
	EventInfo *m_DoneOutput;
	int m_iTotalCurrency;
	int m_iCurrencyLeft;
	CUtlString m_strName;
	CUtlString m_strWaitForAllSpawned;
	CUtlString m_strWaitForAllDead;
	CountdownTimer m_ctSpawnDelay;
	CUtlVector<CHandle<CBaseEntity>> m_ActiveBots;
	int m_iCountSpawned;
	int m_iCountToSpawn;
	bool m_bSupport;
	bool m_bSupportLimited;
	CWave *m_Wave;
	InternalStateType m_iState;
	bool m_bRandomSpawn;
	SpawnResult m_iSpawnResult;
	Vector m_vecSpawn;
};

class CWave : public IPopulator
{
public:
	CWave( CLFPopulationManager *popmgr );
	virtual ~CWave();
	
	virtual bool Parse( KeyValues *kv ) override;
	virtual void Update() override;
	virtual void OnMemberKilled( CBaseEntity *pMember ) override;
	//virtual bool HasEventChangeAttributes( const char *name ) const override;

	void ForceFinish();
	void ForceReset();

	CWaveSpawnPopulator *FindWaveSpawnPopulator( char const *name );

private:

	void ActiveWaveUpdate();
	void WaveCompleteUpdate();
	void WaveIntermissionUpdate();

	bool IsDoneWithNonSupportWaves();

	CUtlVectorAutoPurge<CWaveSpawnPopulator *> m_WaveSpawns;

	int m_iTotalCurrency;
	EventInfo *m_StartWaveOutput;
	EventInfo *m_DoneOutput;
	EventInfo *m_InitWaveOutput;
	CFmtStrN<256> m_strDescription;
	CFmtStrN<256> m_strSound;
	float m_flWaitWhenDone;
};

EventInfo *ParseEvent( KeyValues *kv );
void FireEvent( EventInfo *info, const char *name );

#endif // LFE_POPULATION_MANAGER_H
