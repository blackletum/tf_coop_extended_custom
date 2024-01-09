//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: Vote issues
//
//
//=============================================================================//
#include "cbase.h"
#include "tf_shareddefs.h"
#include "tf_voteissues.h"
#include "tf_gamerules.h"

extern ConVar sv_vote_timer_duration;

ConVar sv_vote_issue_kick_allowed( "sv_vote_issue_kick_allowed", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Can players call votes to kick players from the server?" );
ConVar sv_vote_kick_ban_duration( "sv_vote_kick_ban_duration", "20", FCVAR_REPLICATED, "The number of minutes a vote ban should last. (0 = Disabled)" );
ConVar sv_vote_issue_restart_game_allowed( "sv_vote_issue_restart_game_allowed", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Can players call votes to restart the game?" );
ConVar sv_vote_issue_restart_game_cooldown( "sv_vote_issue_restart_game_cooldown", "300", FCVAR_REPLICATED, "Minimum time before another restart vote can occur (in seconds)." );
ConVar sv_vote_issue_changelevel_allowed( "sv_vote_issue_changelevel_allowed", "1", FCVAR_NOTIFY | FCVAR_REPLICATED,  "Can players call votes to change levels?" );
ConVar sv_vote_issue_nextlevel_allowed( "sv_vote_issue_nextlevel_allowed", "0", FCVAR_NOTIFY | FCVAR_REPLICATED,  "Can players call votes to set the next level?" );
ConVar sv_vote_issue_scramble_teams_allowed( "sv_vote_issue_scramble_teams_allowed", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Can players call votes to scramble the teams?" );

ConVar sv_vote_issue_changedifficulty_allowed( "sv_vote_issue_changedifficulty_allowed", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Can players call votes to change difficulty?" );
ConVar sv_vote_issue_changedifficulty_cooldown( "sv_vote_issue_changedifficulty_cooldown", "150", FCVAR_REPLICATED, "Minimum time before another change difficulty vote can occur (in seconds)." );
ConVar sv_vote_issue_changemapadd_allowed( "sv_vote_issue_changemapadd_allowed", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Can players call votes to change mapadd?" );
ConVar sv_vote_issue_changemapadd_cooldown( "sv_vote_issue_changemapadd_cooldown", "150", FCVAR_REPLICATED, "Minimum time before another change mapadd vote can occur (in seconds)." );

//-----------------------------------------------------------------------------
// Purpose: Base TF2 Issue
//-----------------------------------------------------------------------------
void CBaseTFIssue::ExecuteCommand( void )
{
	const char* pszDetails = m_szDetailsString;
	if ( FStrEq(m_szTypeString, "kick") )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByUserId( atoi(pszDetails) );
		if (!pPlayer)
			return;

		CSteamID id;
		pPlayer->GetSteamID(&id);

		if (pPlayer->IsBot())
			pszDetails = UTIL_VarArgs("%s (BOT)", pPlayer->GetPlayerName());
		else
			pszDetails = UTIL_VarArgs("%s (Account id: %d)", pPlayer->GetPlayerName(), (int)id.GetAccountID());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Kick Issue
//-----------------------------------------------------------------------------
const char *CKickIssue::GetDisplayString()
{
	/*char result[64];
	Q_snprintf(result, sizeof(result), "#TF_vote_kick_player_%s", m_pzReason);
	char *szResult = (char*)malloc(sizeof(result));
	Q_strncpy(szResult, result, sizeof(result));
	return szResult;*/
	return "#TF_vote_kick_player";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CKickIssue::GetVotePassedString()
{
	return "#TF_vote_passed_kick_player";
	//TODO: add something for "#TF_vote_passed_ban_player";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CKickIssue::IsEnabled()
{
	return sv_vote_issue_kick_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CKickIssue::GetVoteOptions(CUtlVector <const char*> &vecNames)
{
	if (!CBaseTFIssue::GetVoteOptions(vecNames))
		return false;

	vecNames.AddToTail("Abstain");
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
/*void CKickIssue::OnVoteStarted()
{
	int iUserID = atoi(GetDetailsString());
	CBasePlayer* pVoteCaller = UTIL_PlayerByUserId(iUserID);
	if ( !pVoteCaller )
		return;

	CSteamID pTargetSteamID;	pVoteCaller->GetSteamID(&pTargetSteamID);
	g_voteController->AddPlayerToKickWatchList( pTargetSteamID, sv_vote_timer_duration.GetFloat() );

	CSteamID pCallerSteamID;	pVoteCaller->GetSteamID(&pCallerSteamID);
	g_voteController->AddPlayerToNameLockedList( pCallerSteamID, sv_vote_timer_duration.GetFloat(), m_iPlayerID );
}*/

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CKickIssue::CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	int iUserID = atoi(pszDetails);
	CBasePlayer* pPlayer = UTIL_PlayerByUserId(iUserID);
	if (!pPlayer)
		return false;

	return CBaseTFIssue::CanCallVote(nEntIndex, pszDetails, nFailCode, nTime);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CKickIssue::ExecuteCommand( void )
{
	CBaseTFIssue::ExecuteCommand();

	int iUserID = atoi(GetDetailsString());
	CBasePlayer* pPlayer = UTIL_PlayerByUserId(iUserID);
	if ( pPlayer && pPlayer->IsBot() )
	{
		ConVarRef bot_quota("bot_quota");
		bot_quota.SetValue(bot_quota.GetInt()-1);
		return;
	}

	engine->ServerCommand(UTIL_VarArgs( "banid %i %s kick\n", sv_vote_kick_ban_duration.GetInt(), networkIDString.String() ));
	engine->ServerCommand("writeip\n");
	engine->ServerCommand("writeid\n");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CKickIssue::SetIssueDetails( const char *pszDetails )
{
	CBaseTFIssue::SetIssueDetails( pszDetails );

	int iUserID = atoi(pszDetails);
	CBasePlayer* pPlayer = UTIL_PlayerByUserId(iUserID);

	// We already check this in CanCallVote, but let's check it again, just to be safe...
	if ( !pPlayer )
		return;

	networkIDString = pPlayer->GetNetworkIDString();
}

//-----------------------------------------------------------------------------
// Purpose: Restart Game Issue
//-----------------------------------------------------------------------------
const char *CRestartGameIssue::GetDisplayString()
{
	return "#TF_vote_restart_game";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CRestartGameIssue::GetVotePassedString()
{
	return "#TF_vote_passed_restart_game";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRestartGameIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	AssertMsg(false, "Unimplemented");
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CRestartGameIssue::IsEnabled()
{
	return sv_vote_issue_restart_game_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRestartGameIssue::ExecuteCommand()
{
	CBaseTFIssue::ExecuteCommand();

	engine->ServerCommand( "mp_restartgame 1\n" );
}

//-----------------------------------------------------------------------------
// Purpose: Scramble Teams Issue
//-----------------------------------------------------------------------------
const char *CScrambleTeams::GetDisplayString()
{
	return "#TF_vote_scramble_teams";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CScrambleTeams::GetVotePassedString()
{
	return "#TF_vote_passed_scramble_teams";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CScrambleTeams::ListIssueDetails( CBasePlayer *pForWhom )
{
	AssertMsg(false, "Unimplemented");
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CScrambleTeams::IsEnabled()
{
	if ( TFGameRules()->IsAnyCoOp() )
		return false;

	return sv_vote_issue_scramble_teams_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CScrambleTeams::ExecuteCommand()
{
	CBaseTFIssue::ExecuteCommand();

	engine->ServerCommand( "mp_scrambleteams 1\n" );
}

//-----------------------------------------------------------------------------
// Purpose: Change Level Issue
//-----------------------------------------------------------------------------
const char *CChangeLevelIssue::GetDisplayString()
{
	return "#TF_vote_changelevel";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CChangeLevelIssue::GetVotePassedString()
{
	return "#TF_vote_passed_changelevel";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CChangeLevelIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	AssertMsg(false, "Unimplemented");
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CChangeLevelIssue::IsEnabled()
{
	return sv_vote_issue_changelevel_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CChangeLevelIssue::ExecuteCommand()
{
	CBaseTFIssue::ExecuteCommand();
	engine->ServerCommand( UTIL_VarArgs( "changelevel %s\n", m_szDetailsString ) );
}

//-----------------------------------------------------------------------------
// Purpose: Next Level Issue
//-----------------------------------------------------------------------------
const char *CNextLevelIssue::GetDisplayString()
{
	return "#TF_vote_nextlevel";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CNextLevelIssue::GetVotePassedString()
{
	return "#TF_vote_passed_changelevel";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNextLevelIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	AssertMsg(false, "Unimplemented");
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNextLevelIssue::IsEnabled()
{
	return sv_vote_issue_nextlevel_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNextLevelIssue::GetVoteOptions( CUtlVector <const char*> &vecNames )
{
	CUtlVector<char*> apszMapList;
	apszMapList.AddVectorToTail( TFGameRules()->GetMapList() );

	static char szNextMap[MAX_MAP_NAME];
	TFGameRules()->GetNextLevelName( szNextMap, sizeof( szNextMap ) );
	vecNames.AddToTail( szNextMap );

	while ( vecNames.Count() < 5 )
	{
		if (!apszMapList.Count())
			break;

		int iRandom = RandomInt(0, apszMapList.Count()-1);

		if (FStrEq(apszMapList[iRandom], szNextMap))
		{
			apszMapList.Remove(iRandom);
			continue;
		}

		vecNames.AddToTail( apszMapList[iRandom] );
		apszMapList.Remove(iRandom);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNextLevelIssue::ExecuteCommand()
{
	CBaseTFIssue::ExecuteCommand();

	ConVarRef nextlevel( "nextlevel" );
	nextlevel.SetValue( m_szDetailsString );
}

//-----------------------------------------------------------------------------
// Purpose: Change Difficulty Issue
//-----------------------------------------------------------------------------
const char *CChangeDifficultyIssue::GetDisplayString()
{
	return "#LFE_vote_changedifficulty";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CChangeDifficultyIssue::GetVotePassedString()
{
	return "#LFE_vote_passed_changedifficulty";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CChangeDifficultyIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	AssertMsg(false, "Unimplemented");
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CChangeDifficultyIssue::IsEnabled()
{
	return sv_vote_issue_changedifficulty_allowed.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CChangeDifficultyIssue::ExecuteCommand()
{
	CBaseTFIssue::ExecuteCommand();
	if (strchr(m_szDetailsString, ';') == NULL){
		engine->ServerCommand(UTIL_VarArgs("sv_difficulty %s\n", m_szDetailsString));	// Else, do nothing to prevent people from using ; to run commands on the server!
	}
}

//-----------------------------------------------------------------------------
// Purpose: Change MapAdd Issue
//-----------------------------------------------------------------------------
const char *CChangeMapAddIssue::GetDisplayString()
{
	return "#LFE_vote_changemapadd";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CChangeMapAddIssue::GetVotePassedString()
{
	return "#LFE_vote_passed_changemapadd";
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CChangeMapAddIssue::ListIssueDetails( CBasePlayer *pForWhom )
{
	AssertMsg(false, "Unimplemented");
	ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CChangeMapAddIssue::IsEnabled()
{
	return ( TFGameRules()->IsMapAddAllowed() && sv_vote_issue_changemapadd_allowed.GetBool() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CChangeMapAddIssue::ExecuteCommand()
{
	CBaseTFIssue::ExecuteCommand();

	if ( FStrEq( m_szDetailsString, "#Lfe_vote_no_mapadd" ) )
		return;
	if (strchr(m_szDetailsString, ';') == NULL){
		engine->ServerCommand(UTIL_VarArgs("lfe_mapadd_file %s\n", m_szDetailsString));
	}
	mp_restartgame_immediate.SetValue( 1 );
}

// live tf2 stuff
//CExtendLevelIssue, CMannVsMachineChangeChallengeIssue, CEnableTemporaryHalloweenIssue, CTeamAutoBalanceIssue, CClassLimitsIssue, CPauseGameIssue