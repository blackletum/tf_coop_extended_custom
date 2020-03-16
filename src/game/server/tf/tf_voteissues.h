//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: Vote issues
//
//
//=============================================================================//
#ifndef TF_VOTEISSUES_H
#define TF_VOTEISSUES_H
#ifdef _WIN32
#pragma once
#endif

#include "vote_controller.h"

//-----------------------------------------------------------------------------
// Purpose: Base TF2 Issue
//-----------------------------------------------------------------------------
class CBaseTFIssue : public CBaseIssue
{
public:
	CBaseTFIssue( const char* pszName ) : CBaseIssue(pszName) {}

public:

	virtual void		ExecuteCommand( void );
};

//-----------------------------------------------------------------------------
// Purpose: Kick Issue
//-----------------------------------------------------------------------------
class CKickIssue : public CBaseTFIssue
{
private:
	CUtlString networkIDString;

public:
	CKickIssue() : CBaseTFIssue( "kick" ) {}

public:
	virtual bool		IsEnabled( void );

	virtual const char *GetDisplayString( void );
	virtual const char *GetVotePassedString( void );

	virtual bool		GetVoteOptions( CUtlVector <const char*> &vecNames );
	virtual void		ExecuteCommand( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom )
	{
		AssertMsg(false, "Unimplemented");
		ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
	}

	virtual bool CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual void SetIssueDetails( const char *pszDetails );

	virtual bool	IsTeamRestrictedVote() { return true; }
};

//-----------------------------------------------------------------------------
// Purpose: Restart Game Issue
//-----------------------------------------------------------------------------
class CRestartGameIssue : public CBaseTFIssue
{
public:
	CRestartGameIssue() : CBaseTFIssue( "restartgame" ) {}

public:
	virtual bool		IsEnabled();

	virtual const char *GetDisplayString( void );
	virtual const char *GetVotePassedString( void );

	virtual void		ExecuteCommand( void );

	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
};

//-----------------------------------------------------------------------------
// Purpose: Scramble Game Issue
//-----------------------------------------------------------------------------
class CScrambleTeams : public CBaseTFIssue
{
public:
	CScrambleTeams() : CBaseTFIssue( "scrambleteams" ) {}

public:
	virtual bool		IsEnabled();

	virtual const char *GetDisplayString( void );
	virtual const char *GetVotePassedString( void );

	virtual void		ExecuteCommand( void );

	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
};

//-----------------------------------------------------------------------------
// Purpose: Change Level Issue
//-----------------------------------------------------------------------------
class CChangeLevelIssue : public CBaseTFIssue
{
public:
	CChangeLevelIssue() : CBaseTFIssue( "changelevel" ) {}

public:
	virtual bool		IsEnabled( void );

	virtual const char *GetDisplayString( void );
	virtual const char *GetVotePassedString( void );

	virtual void		ExecuteCommand( void );

	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
};

//-----------------------------------------------------------------------------
// Purpose: Next Level Issue
//-----------------------------------------------------------------------------
class CNextLevelIssue : public CBaseTFIssue
{
public:
	CNextLevelIssue() : CBaseTFIssue( "nextlevel" ) {}

public:
	virtual bool		IsEnabled( void );
	virtual bool		IsYesNoVote( void ) { return false; }
	virtual const char *GetDisplayString( void );
	virtual const char *GetVotePassedString( void );

	virtual bool        GetVoteOptions( CUtlVector <const char*> &vecNames );

	virtual void		ExecuteCommand( void );

	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
};

//-----------------------------------------------------------------------------
// Purpose: Change Difficulty Issue
//-----------------------------------------------------------------------------
class CChangeDifficultyIssue : public CBaseTFIssue
{
public:
	CChangeDifficultyIssue() : CBaseTFIssue( "changedifficulty" ) {}

public:
	virtual bool		IsEnabled( void );
	virtual const char *GetDisplayString( void );
	virtual const char *GetVotePassedString( void );

	virtual void		ExecuteCommand( void );

	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
};

//-----------------------------------------------------------------------------
// Purpose: Change MapAdd Issue
//-----------------------------------------------------------------------------
class CChangeMapAddIssue : public CBaseTFIssue
{
public:
	CChangeMapAddIssue() : CBaseTFIssue( "changemapadd" ) {}

public:
	virtual bool		IsEnabled( void );
	virtual const char *GetDisplayString( void );
	virtual const char *GetVotePassedString( void );

	virtual void		ExecuteCommand( void );

	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
};
#endif // TF_VOTEISSUES_H
