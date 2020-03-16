//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: The TF Game rules object
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================

#ifndef TF_GAMERULES_H
#define TF_GAMERULES_H

#ifdef _WIN32
#pragma once
#endif


#include "teamplayroundbased_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"
#include "GameEventListener.h"
#include "tf_gamestats_shared.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#else
#include "tf_player.h"
#endif

#ifdef CLIENT_DLL

#define CTFGameRules C_TFGameRules
#define CTFGameRulesProxy C_TFGameRulesProxy

#else

extern BOOL no_cease_fire_text;
extern BOOL cease_fire;

class CHealthKit;

#endif

extern ConVar	sv_hl1_ff;

extern ConVar	tf_avoidteammates;
extern ConVar	tf_avoidteammates_pushaway;
extern ConVar	sv_dynamicnpcs;
extern ConVar	sv_hl2_beta;
extern ConVar	fraglimit;
extern ConVar	mapedit_desiredpart;
extern ConVar	mapedit_desiredmap;

extern ConVar	lfe_allow_antirush;

extern Vector g_TFClassViewVectors[];

#ifdef GAME_DLL
class CTFRadiusDamageInfo
{
public:
	CTFRadiusDamageInfo();

	bool	ApplyToEntity( CBaseEntity *pEntity );

public:
	const CTakeDamageInfo *info;
	Vector m_vecSrc;
	float m_flRadius;
	float m_flSelfDamageRadius;
	int m_iClassIgnore;
	CBaseEntity *m_pEntityIgnore;
	bool m_bStockSelfDamage;
};
#endif

class CTFGameRulesProxy : public CTeamplayRoundBasedRulesProxy
{
public:
	DECLARE_CLASS( CTFGameRulesProxy, CTeamplayRoundBasedRulesProxy );
	DECLARE_NETWORKCLASS();

#ifdef GAME_DLL
	DECLARE_DATADESC();
	virtual void FireGameEvent( IGameEvent *event );
	void	InputSetRedTeamRespawnWaveTime( inputdata_t &inputdata );
	void	InputSetBlueTeamRespawnWaveTime( inputdata_t &inputdata );
	void	InputAddRedTeamRespawnWaveTime( inputdata_t &inputdata );
	void	InputAddBlueTeamRespawnWaveTime( inputdata_t &inputdata );
	void	InputSetRedTeamGoalString( inputdata_t &inputdata );
	void	InputSetBlueTeamGoalString( inputdata_t &inputdata );
	void	InputSetRedTeamRole( inputdata_t &inputdata );
	void	InputSetBlueTeamRole( inputdata_t &inputdata );
	void	InputAddRedTeamScore( inputdata_t &inputdata );
	void	InputAddBlueTeamScore( inputdata_t &inputdata );

	void	InputSetRedKothClockActive( inputdata_t &inputdata );
	void	InputSetBlueKothClockActive( inputdata_t &inputdata );

	void	InputSetCTFCaptureBonusTime( inputdata_t &inputdata );

	void	InputPlayVO( inputdata_t &inputdata );
	void	InputPlayVORed( inputdata_t &inputdata );
	void	InputPlayVOBlue( inputdata_t &inputdata );

	void	InputSetTimeScale( inputdata_t &inputdata );
	void	InputSpawnBoss( inputdata_t &inputdata );

	virtual void Activate();

	int		m_iHud_Type;
	bool	m_bCTF_Overtime;
	bool	m_bForceDynamicNPC;

	COutputEvent			m_IsHL2BetaEnabled;
	COutputEvent			m_IsDynamicNPCEnabled;
#endif
};

struct PlayerRoundScore_t
{
	int iPlayerIndex;	// player index
	int iRoundScore;	// how many points scored this round
	int	iTotalScore;	// total points scored across all rounds
	int	iKills;
	int iDeaths;
};

#define MAX_TEAMGOAL_STRING		256

class CTFGameRules : public CTeamplayRoundBasedRules
{
public:
	DECLARE_CLASS( CTFGameRules, CTeamplayRoundBasedRules );

	CTFGameRules();

	// Halloween Scenarios
	enum
	{
		HALLOWEEN_SCENARIO_MANOR = 1,
		HALLOWEEN_SCENARIO_VIADUCT,
		HALLOWEEN_SCENARIO_LAKESIDE,
		HALLOWEEN_SCENARIO_HIGHTOWER,
		HALLOWEEN_SCENARIO_DOOMSDAY
	};

	// Damage Queries.
	virtual bool	Damage_IsTimeBased( int iDmgType );			// Damage types that are time-based.
	virtual bool	Damage_ShowOnHUD( int iDmgType );				// Damage types that have client HUD art.
	virtual bool	Damage_ShouldNotBleed( int iDmgType );			// Damage types that don't make the player bleed.
	// TEMP:
	virtual int		Damage_GetTimeBased( void );
	virtual int		Damage_GetShowOnHud( void );
	virtual int		Damage_GetShouldNotBleed( void );

	int				GetFarthestOwnedControlPoint( int iTeam, bool bWithSpawnpoints );
#ifdef GAME_DLL
	int				GetSpawningVehicleType( void );
#endif
	virtual bool	TeamMayCapturePoint( int iTeam, int iPointIndex );
	virtual bool	PlayerMayCapturePoint( CBasePlayer *pPlayer, int iPointIndex, char *pszReason = NULL, int iMaxReasonLength = 0 );
	virtual bool	PlayerMayBlockPoint( CBasePlayer *pPlayer, int iPointIndex, char *pszReason = NULL, int iMaxReasonLength = 0 );

	static int		CalcPlayerScore( RoundStats_t *pRoundStats );

	virtual bool	IsHolidayActive( int nHoliday ) const;
	virtual bool	IsBirthday( void );
	virtual bool	IsLFBirthday( void );
#ifdef CLIENT_DLL
	virtual bool	IsBirthdayOrPyroVision( void );
#else
	virtual bool	IsVehicleSpawningAllowed( void );
#endif
	virtual bool	CanPlayerSearchSpawn( void );
	virtual bool	CanPlayerStopSearchSpawn( void );

	virtual bool	IsAlyxInDarknessMode( void ) { return m_bDarknessMode; }

	virtual const unsigned char *GetEncryptionKey( void ) { return ( unsigned char * )"E2NcUkG2"; }

	virtual bool	AllowThirdPersonCamera( void );

	virtual float	GetRespawnWaveMaxLength( int iTeam, bool bScaleWithNumPlayers = true );

	virtual bool	ShouldBalanceTeams( void );

	CTeamRoundTimer* GetBlueKothRoundTimer( void ) { return m_hBlueKothTimer.Get(); }
	CTeamRoundTimer* GetRedKothRoundTimer( void ) { return m_hRedKothTimer.Get(); }

	bool	MegaPhyscannonActive( void ) { return m_bMegaPhysgun; }

	CNetworkVar( bool, m_bMegaPhysgun );

	bool	LongJumpActive( void ) { return m_bLongJump; }
	CNetworkVar( bool, m_bLongJump );

	CNetworkVar( bool, m_bDarknessMode );

	CBaseEntity*	GetIT( void ) const { return m_itHandle.Get(); }
	void			SetIT( CBaseEntity *pPlayer );

#ifdef GAME_DLL
public:
	// Override this to prevent removal of game specific entities that need to persist
	virtual bool	RoundCleanupShouldIgnore( CBaseEntity *pEnt );
	virtual bool	ShouldCreateEntity( const char *pszClassName );
	virtual void	CleanUpMap( void );

	virtual void	FrameUpdatePostEntityThink();

	int	m_iDifficultyLevel;

	virtual bool IsSkillLevel( int iLevel ) { return GetSkillLevel() == iLevel; }
	virtual int	 GetSkillLevel() { return m_iDifficultyLevel; }
	virtual void OnSkillLevelChanged( int iNewLevel );
	virtual void SetSkillLevel( int iLevel );

	bool	NPC_ShouldDropHealth( CBasePlayer *pRecipient );
	bool	NPC_ShouldDropGrenade( CBasePlayer *pRecipient );
	void	NPC_DroppedHealth( void );
	void	NPC_DroppedGrenade( void );

	void	ChangeLevel( const char *pszMap, bool bWait = true, bool bSave = true, bool bForceWin = true );
private:
	float	m_flLastHealthDropTime;
	float	m_flLastGrenadeDropTime;
public:
	// Called when a new round is being initialized
	virtual void	SetupOnRoundStart( void );

	// Called when a new round is off and running
	virtual void	SetupOnRoundRunning( void );

	// Called before a new round is started (so the previous round can end)
	virtual void	PreviousRoundEnd( void );

	// Send the team scores down to the client
	virtual void	SendTeamScoresEvent( void ) { return; }

	// Send the end of round info displayed in the win panel
	virtual void	SendWinPanelInfo( void );

	// Setup spawn points for the current round before it starts
	virtual void	SetupSpawnPointsForRound( void );

	// Called when a round has entered stalemate mode (timer has run out)
	virtual void	SetupOnStalemateStart( void );
	virtual void	SetupOnStalemateEnd( void );

	virtual bool	ShouldBurningPropsEmitLight();

	void			RecalculateControlPointState( void );

	virtual void	HandleSwitchTeams( void );
	virtual void	HandleScrambleTeams( void );
	bool			CanChangeClassInStalemate( void );

	virtual void	SetRoundOverlayDetails( void );
	virtual void	ShowRoundInfoPanel( CTFPlayer *pPlayer = NULL ); // NULL pPlayer means show the panel to everyone

	virtual bool	TimerMayExpire( void );

	void			HandleCTFCaptureBonus( int iTeam );

	virtual void	Arena_NotifyTeamSizeChange( void );
	virtual int		Arena_PlayersNeededForMatch( void );
	virtual void	Arena_ResetLosersScore( bool bUnknown );
	virtual void	Arena_RunTeamLogic( void );

	virtual void	Activate();

	virtual void	SetHudType( int iHudType ) { m_nHudType = iHudType; };

	virtual bool	AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info );

	virtual int		GetClassLimit( int iDesiredClassIndex, int iTeam );
	virtual bool	CanPlayerChooseClass( CBasePlayer *pPlayer, int iDesiredClassIndex );

	void			SetTeamGoalString( int iTeam, const char *pszGoal );

	// Speaking, vcds, voice commands.
	virtual void	InitCustomResponseRulesDicts();
	virtual void	ShutdownCustomResponseRulesDicts();

	virtual bool	HasPassedMinRespawnTime( CBasePlayer *pPlayer );

	bool			ShouldScorePerRound( void );

	virtual int		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );

	virtual void	PlayTrainCaptureAlert( CTeamControlPoint *pPoint, bool bFinalPointInMap );

	void			SetBlueKothRoundTimer( CTeamRoundTimer *pTimer ) { m_hBlueKothTimer.Set( pTimer ); }
	void			SetRedKothRoundTimer( CTeamRoundTimer *pTimer ) { m_hRedKothTimer.Set( pTimer ); }

	virtual bool	ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );

	bool			PlayerReadyStatus_ArePlayersOnTeamReady( int iTeam );
	//bool			PlayerReadyStatus_HaveMinPlayersToEnable( void );
	void			PlayerReadyStatus_ResetState( void );
	bool			PlayerReadyStatus_ShouldStartCountdown( void );
	void			PlayerReadyStatus_UpdatePlayerState( CTFPlayer *pPlayer, bool bReady );

	virtual void	SetupTimerExpired( void );

	void			RegisterBoss( CBaseCombatCharacter *pNPC )  { if( m_hBosses.Find( pNPC ) == m_hBosses.InvalidIndex() ) m_hBosses.AddToHead( pNPC ); }
	void			RemoveBoss( CBaseCombatCharacter *pNPC )    { EHANDLE hNPC( pNPC ); m_hBosses.FindAndRemove( hNPC ); }

protected:
	virtual void	InitTeams( void );

	virtual void	RoundRespawn( void );

	virtual void	InternalHandleTeamWin( int iWinningTeam );

	static int		PlayerRoundScoreSortFunc( const PlayerRoundScore_t *pRoundScore1, const PlayerRoundScore_t *pRoundScore2 );

	virtual void	FillOutTeamplayRoundWinEvent( IGameEvent *event );

	virtual bool	CanChangelevelBecauseOfTimeLimit( void );
	virtual bool	CanGoToStalemate( void );

	void PlayWinSong( int team );

	virtual const char* GetStalemateSong( int nTeam );
	virtual const char* WinSongName( int nTeam );
	virtual const char* LoseSongName( int nTeam );
#endif // GAME_DLL

public:
	// Return the value of this player towards capturing a point
	virtual int		GetCaptureValueForPlayer( CBasePlayer *pPlayer );

	// Collision and Damage rules.
	virtual bool	ShouldCollide( int collisionGroup0, int collisionGroup1 );

	int GetTimeLeft( void );

	// Get the view vectors for this mod.
	virtual const CViewVectors *GetViewVectors() const;

	virtual void	FireGameEvent( IGameEvent *event );

	virtual const char *GetGameTypeName( void ) { return g_aGameTypeNames[m_nGameType]; }
	virtual int		GetGameType( void ) { return m_nGameType; }
	virtual int		GetServerGameType( void ) { return m_nServerGameType; }
	virtual int		GetHalloweenScenario( void ) { return m_halloweenScenario; }

	virtual bool	FlagsMayBeCapped( void );

	void			RunPlayerConditionThink( void );

	const char		*GetTeamGoalString( int iTeam );

	virtual int		GetHudType( void ) { return m_nHudType; };

	virtual bool	IsMultiplayer( void ) { return true; };

	virtual bool	IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer );

	float			GetGravityMultiplier( void ) const { return m_flGravityScale; }
	void			SetGravityMultiplier( float value ) { m_flGravityScale = value; }

	virtual bool    IsMannVsMachineMode( void ) { return GetGameType() == TF_GAMETYPE_MVM; }
	virtual bool	IsInArenaMode( void ) { return GetGameType() == TF_GAMETYPE_ARENA; }
	virtual bool	IsInKothMode( void ) const { return m_bPlayingKoth; }
	virtual bool    IsHalloweenScenario( int iEventType ) { return m_halloweenScenario == iEventType; }
	virtual bool	IsPVEModeActive( void ) { return ( IsMannVsMachineMode() || IsHordeMode() || IsAnyCoOp() ); }
	virtual bool	IsCompetitiveMode( void ) { return m_bCompetitiveMode; };
	virtual bool	IsInHybridCTF_CPMode( void ) { return m_bPlayingHybrid_CTF_CP; };
	virtual bool	IsPlayingSpecialDeliveryMode( void ) { return m_bPlayingSpecialDeliveryMode; };
	virtual bool	IsInMedievalMode( void ) { return m_bPlayingMedieval; };
	virtual bool	IsAttackDefenseMode( void ) { return false; };
	virtual bool	IsDefaultGameMode( void ) { return GetGameType() == TF_GAMETYPE_UNDEFINED; };
	virtual bool	IsInItemTestingMode( void ) { return m_bIsInItemTestingMode; }
	virtual bool	IsInTraining( void ) { return m_bIsInTraining; }
	virtual bool	IsPowerupMode( void ) { return m_bPowerupMode; }

	virtual bool	IsCoOp( void ) { return ( GetGameType() == TF_GAMETYPE_COOP || GetServerGameType() == TF_GAMETYPE_COOP ); }
	virtual bool	IsBluCoOp( void ) { return ( GetGameType() == TF_GAMETYPE_BLUCOOP || GetServerGameType() == TF_GAMETYPE_BLUCOOP ); }
	virtual bool	IsAnyCoOp( void ) { return ( GetGameType() == TF_GAMETYPE_COOP || GetServerGameType() == TF_GAMETYPE_COOP || GetGameType() == TF_GAMETYPE_BLUCOOP || GetServerGameType() == TF_GAMETYPE_BLUCOOP ); }
	virtual bool	IsCoOpGameRunning( void ) { return ( IsCoOp() && State_Get() == GR_STATE_RND_RUNNING && !IsInWaitingForPlayers() ); }
	virtual bool	IsBluCoOpGameRunning( void ) { return ( IsBluCoOp() && State_Get() == GR_STATE_RND_RUNNING && !IsInWaitingForPlayers()); }
	virtual bool	IsAnyCoOpGameRunning( void ) { return ( IsBluCoOp() || IsCoOp() && State_Get() == GR_STATE_RND_RUNNING && !IsInWaitingForPlayers()); }
	virtual bool	IsVersus( void ) { return ( GetGameType() == TF_GAMETYPE_VS || GetServerGameType() == TF_GAMETYPE_VS ); }
	virtual bool	IsHordeMode( void ) { return ( GetGameType() == TF_GAMETYPE_HORDE || GetServerGameType() == TF_GAMETYPE_HORDE ); }
	virtual bool	IsFreeMode( void ) { return GetGameType() == TF_GAMETYPE_FREE || GetServerGameType() == TF_GAMETYPE_FREE; }
	virtual bool	IsInfectionMode( void ) { return GetGameType() == TF_GAMETYPE_INFECTION; }

	virtual bool	IsFirstBloodAllowed( void ) { return false; }
	virtual bool	IsTruceActive( void ) { return m_bTruceActive; }
	virtual bool	IsUsingGrapplingHook( void ) const;
	virtual bool	IsUsingSpells( void ) const;

	virtual bool	IsFriendlyFire( void ) { return ( friendlyfire.GetBool() && sv_hl1_ff.GetBool() ); }
	virtual bool	IsHL1FriendlyFire( void ) { return ( sv_hl1_ff.GetBool() ); }
	virtual bool	IsMPFriendlyFire( void ) { return ( friendlyfire.GetBool() ); }

	virtual bool	IsMapAddAllowed( void );
	virtual bool	IsDynamicNPCAllowed( void ) { return ( sv_dynamicnpcs.GetBool() ); }

	virtual bool	IsHL2Beta( void ) { return ( sv_hl2_beta.GetBool() ); }
	virtual bool	IsTFCAllowed( void );
	virtual bool	IsGrenadesAllowed( void );
	virtual bool	IsMvMModelsAllowed( void );

	int		m_iDirectorAnger;
	int		m_iMaxDirectorAnger;

	int		DirectorAnger( void ) { return m_iDirectorAnger; }
	int		DirectorMaxAnger( void ) { return m_iMaxDirectorAnger; }

	virtual void	SetDirectorAnger( int iAnger ) {  m_iDirectorAnger = iAnger; }
	virtual void	SetDynamicNPC( bool bEnable );

	virtual void	UpdateDirectorAnger();

#ifdef CLIENT_DLL
	virtual bool	IsBonusChallengeTimeBased( void );

	virtual bool	AllowMapParticleEffect( const char *pszParticleEffect );
	virtual bool	AllowWeatherParticles( void );

	virtual void	SetUpVisionFilterKeyValues( void );
	virtual bool	AllowMapVisionFilterShaders( void );
	virtual const char* TranslateEffectForVisionFilter( const char *pchEffectType, const char *pchEffectName );

	virtual void	ModifySentChat( char *pBuf, int iBufSize );

	virtual bool	ShouldWarnOfAbandonOnQuit( void );

	virtual bool	GameModeUsesUpgrades( void ) { return IsHordeMode(); }
#endif

	bool	UsePlayerReadyStatusMode( void );

public: 

	virtual bool IsInHL1Map();
	virtual bool IsInHL2Map();
	virtual bool IsInHL2EP1Map();
	virtual bool IsInHL2EP2Map();
	virtual bool IsInPortalMap();

	virtual bool IsValveMap( void );

	float GetMapRemainingTime();
#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes data tables able to access our private vars.

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	HandleOvertimeBegin();
	virtual void	GetTeamGlowColor( int nTeam, float &r, float &g, float &b );

	bool			ShouldShowTeamGoal( void );

	const char *GetVideoFileForMap( bool bWithExtension = true );

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes data tables able to access our private vars.

	virtual ~CTFGameRules();

	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual void Think();

	virtual void InitDefaultAIRelationships( void );

	bool CheckWinLimit();
	bool CheckFragLimit();
	bool CheckCapsPerRound();

	virtual bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker, const CTakeDamageInfo &info );

	// Spawing rules.
	CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer );
	bool IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer, bool bIgnorePlayers );

	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );
	virtual QAngle VecItemRespawnAngles( CItem *pItem );

	virtual const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );
	void ClientSettingsChanged( CBasePlayer *pPlayer );
	virtual void GetTaggedConVarList( KeyValues *pCvarTagList );
	void ChangePlayerName( CTFPlayer *pPlayer, const char *pszNewName );

	virtual VoiceCommandMenuItem_t *VoiceCommand( CBaseMultiplayerPlayer *pPlayer, int iMenu, int iItem );

	bool IsInPreMatch() const;
	float GetPreMatchEndTime() const;	// Returns the time at which the prematch will be over.
	void GoToIntermission( void );

	virtual int GetAutoAimMode() { return AUTOAIM_NONE; }

	bool CanHaveAmmo( CBaseCombatCharacter *pPlayer, int iAmmoIndex );

	virtual const char *GetGameDescription( void );

	// Sets up g_pPlayerResource.
	virtual void CreateStandardEntities();

	virtual void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual void NPCKilled( CAI_BaseNPC *pVictim, const CTakeDamageInfo &info );
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual CBasePlayer *GetDeathScorer( CBaseEntity *pKiller, CBaseEntity *pInflictor, CBaseEntity *pVictim );

	void CalcDominationAndRevenge( CTFPlayer *pAttacker, CTFPlayer *pVictim, bool bIsAssist, int *piDeathFlags );

	const char *GetKillingWeaponName( const CTakeDamageInfo &info, CTFPlayer *pVictim, int &iWeaponID );
	CBasePlayer *GetAssister( CBasePlayer *pVictim, CBasePlayer *pScorer, CBaseEntity *pInflictor );
	CBaseEntity *GetAssister( CBaseEntity *pVictim, CBaseEntity *pKiller, CBaseEntity *pInflictor );
	CBaseEntity *GetRecentDamager( CBaseEntity *pVictim, int iDamager, float flMaxElapsed );

	virtual void ClientDisconnected( edict_t *pClient );

	void	RadiusDamage( CTFRadiusDamageInfo &radiusInfo );
	virtual void  RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore );
	bool   RadiusJarEffect( CTFRadiusDamageInfo &radiusInfo, int iCond );

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
	virtual float FlNPCFallDamage( CAI_BaseNPC *pNPC );

	virtual bool  FlPlayerFallDeathDoesScreenFade( CBasePlayer *pl ) { return false; }

	virtual bool UseSuicidePenalty() { return false; }

	int		GetPreviousRoundWinners( void ) { return m_iPreviousRoundWinners; }

	void	SendHudNotification( IRecipientFilter &filter, HudNotification_t iType );
	void	SendHudNotification( IRecipientFilter &filter, const char *pszText, const char *pszIcon, int iTeam = TEAM_UNASSIGNED );

	void	SetPlayingMedieval( bool bPlaying ) { m_bPlayingMedieval = bPlaying; }
	void	SetPowerupMode( bool bPower ) { m_bPowerupMode = bPower; }
	void	SetGameType( int iGameType ) { m_nGameType.Set( iGameType ); }
	void	SetServerGameType( int iGameType ) { m_nServerGameType.Set( iGameType ); }
	void	Reactivate( void ) { Activate(); }
	void	SetTruce( bool bActive ) { m_bTruceActive = bActive; }

	const	CUtlVector<char*>& GetMapList() const { return m_MapList; }
private:

	int DefaultFOV( void ) { return 75; }

	void			SpawnHalloweenBoss( void );
	//void			SpawnZombieMob( void );
	CountdownTimer	m_bossSpawnTimer;
	//CountdownTimer	m_mobSpawnTimer;

#endif

private:

#ifdef GAME_DLL

	Vector2D	m_vecPlayerPositions[MAX_PLAYERS];

	CUtlVector<CHandle<CHealthKit> > m_hDisabledHealthKits;

	char	m_szMostRecentCappers[MAX_PLAYERS + 1];	// list of players who made most recent capture.  Stored as string so it can be passed in events.
	int		m_iNumCaps[TF_TEAM_COUNT];				// # of captures ever by each team during a round

	int SetCurrentRoundStateBitString();
	void SetMiniRoundBitMask( int iMask );
	int m_iPrevRoundState;	// bit string representing the state of the points at the start of the previous miniround
	int m_iCurrentRoundState;
	int m_iCurrentMiniRoundMask;

	bool m_bFirstBlood;
	int	m_iArenaTeamCount;

	CUtlVector< CHandle<CBaseCombatCharacter> > m_hBosses;
#endif

	CNetworkVar( int, m_nGameType ); // Type of game this map is (CTF, CP)
	CNetworkVar( int, m_nServerGameType ); // Type of game this server is (COOP, VS)
	//CNetworkVar( int, m_nStopWatchState );
	CNetworkString( m_pszTeamGoalStringRed, MAX_TEAMGOAL_STRING );
	CNetworkString( m_pszTeamGoalStringBlue, MAX_TEAMGOAL_STRING );
	CNetworkVar( float, m_flCapturePointEnableTime );
	CNetworkVar( int, m_nHudType );
	CNetworkVar( bool, m_bIsInTraining );
	//CNetworkVar( bool, m_bAllowTrainingAchievements );
	//CNetworkVar( bool, m_bIsWaitingForTrainingContinue );
	//CNetworkVar( bool, m_bIsTrainingHUDVisible );
	CNetworkVar( bool, m_bIsInItemTestingMode );
	//CNetworkVar( bool, m_hBonusLogic );
	CNetworkVar( bool, m_bPlayingKoth );
	CNetworkVar( bool, m_bPlayingMedieval );
	CNetworkVar( bool, m_bPlayingHybrid_CTF_CP );
	CNetworkVar( bool, m_bPlayingSpecialDeliveryMode );
	CNetworkVar( bool, m_bPlayingRobotDestructionMode );
	CNetworkVar( CHandle<CTeamRoundTimer>, m_hRedKothTimer );
	CNetworkVar( CHandle<CTeamRoundTimer>, m_hBlueKothTimer );
	CNetworkVar( int, m_nMapHolidayType );
	CNetworkVar( EHANDLE, m_itHandle );
	CNetworkVar( bool, m_bPlayingMannVsMachine );
	//CNetworkVar( CHandle<CTFPlayer?>, m_hBirthdayPlayer );
	//CNetworkVar( int, m_nBossHealth );
	//CNetworkVar( int, m_nMaxBossHealth );
	//CNetworkVar( float, m_fBossNormalizedTravelDistance );
	//CNetworkVar( bool, m_bMannVsMachineAlarmStatus );
	//CNetworkVar( bool, m_bHaveMinPlayersToEnableReady );
	//CNetworkVar( bool, m_bBountyModeEnabled );
	//CNetworkVar( int, m_nHalloweenEffect );
	//CNetworkVar( float, m_fHalloweenEffectStartTime );
	//CNetworkVar( float, m_fHalloweenEffectDuration );
	CNetworkVar( int, m_halloweenScenario );
	//CNetworkVar( bool, m_bHelltowerPlayersInHell );
	//CNetworkVar( bool, m_bIsUsingSpells );
	CNetworkVar( bool, m_bCompetitiveMode );
	CNetworkVar( bool, m_bPowerupMode );
	//CNetworkVar( int, m_nMatchGroupType );
	//CNetworkVar( bool, m_bMatchEnded );
	CNetworkString( m_pszCustomUpgradesFile, MAX_PATH );
	CNetworkVar( bool, m_bTruceActive );
	//CNetworkVar( bool, m_bShowMatchSummary );
	//CNetworkVar( bool, m_bTeamsSwitched );
	CNetworkVar( bool, m_bMapHasMatchSummaryStage );
	//CNetworkVar( bool, m_bPlayersAreOnMatchSummaryStage );
	//CNetworkVar( bool, m_bStopWatchWinner );
	//CNetworkVar( ?, m_ePlayerWantsRematch );
	//CNetworkVar( ?, m_eRematchState );
	//CNetworkVar( int, m_nNextMapVoteOptions );
	CNetworkVar( float, m_flGravityScale );
	CNetworkVar( CHandle<CTeamRoundTimer>, m_hLevelTransitionTimer );

public:
	bool m_bControlSpawnsPerTeam[MAX_TEAMS][MAX_CONTROL_POINTS];
	int	 m_iPreviousRoundWinners;

#ifdef GAME_DLL
	float	m_flCTFBonusTime;
#endif

};

//-----------------------------------------------------------------------------
// Vehicle blocks
//-----------------------------------------------------------------------------
class CTFVehicleBlock : public CBaseEntity
{
public:
	DECLARE_CLASS(CTFVehicleBlock, CBaseEntity);
	DECLARE_DATADESC();

	CTFVehicleBlock();
	~CTFVehicleBlock();

	void	Spawn( void );

	bool	m_bAllowAirboat;
	bool	m_bAllowJeep;
	bool	m_bAllowJalopy;
};

//-----------------------------------------------------------------------------
// Gets us at the team fortress game rules
//-----------------------------------------------------------------------------

inline CTFGameRules* TFGameRules()
{
	return static_cast<CTFGameRules*>( g_pGameRules );
}

#ifdef GAME_DLL
bool EntityPlacementTest( CBaseEntity *pMainEnt, const Vector &vOrigin, Vector &outPos, bool bDropToGround );
#endif

extern ConVar lfe_vs_min_red_players;

#ifdef CLIENT_DLL
void AddSubKeyNamed( KeyValues *pKeys, const char *pszName );
#endif

class CLFWeaponManager : public CBaseEntity
{
public:
	DECLARE_CLASS(CLFWeaponManager, CBaseEntity);
	DECLARE_DATADESC();
	CLFWeaponManager();
	~CLFWeaponManager();
	void	Spawn(void);
	void	AllowedWeaponThink(void);

	void	InputSetWeaponProgress(inputdata_t &inputdata);
	int		m_nWeaponProgress;
	bool	m_bGravitygunProgress;
};

#endif // TF_GAMERULES_H
