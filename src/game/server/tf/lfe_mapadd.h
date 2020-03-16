//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: The system for handling extra scripted entity in-game.
//
//=============================================================================//

#ifndef LFE_MAPADD_H
#define LFE_MAPADD_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/icommandline.h"
#include "filesystem.h"
#include <KeyValues.h>
#include "gamestats.h"

// Convar restoration save/restore
#define MAPADD_MAX_MODIFIED_CONVAR_STRING		128
struct mapadd_modifiedconvars_t 
{
	DECLARE_SIMPLE_DATADESC();

	char pszConvar[MAPADD_MAX_MODIFIED_CONVAR_STRING];
	char pszCurrentValue[MAPADD_MAX_MODIFIED_CONVAR_STRING];
	char pszOrgValue[MAPADD_MAX_MODIFIED_CONVAR_STRING];
};

//-----------------------------------------------------------------------------
// Purpose: Game system to kickstart the lfe's mapadd system
//-----------------------------------------------------------------------------
class CTFMapAddSystem : public CAutoGameSystemPerFrame
{
public:
	DECLARE_DATADESC();

	CTFMapAddSystem( char const *name ) : CAutoGameSystemPerFrame( name ) {}

	virtual void LevelInitPreEntity();

	void CalculateMapAddState( void );

	virtual void LevelShutdownPreEntity();

	void	ParseConsole( KeyValues *pSubKey );
	void	ParseEntities( KeyValues *pSubKey );
	void	ParseCreate( KeyValues *pSubKey );
	void	ParsePlayer( KeyValues *pSubKey );

	bool	MapAddConvarsChanging( void ) { return m_bMapAddConvarsChanging; }
	void	SetMapAddConvarsChanging( bool bChanging );
	void	ConvarChanged( IConVar *pConVar, const char *pOldString, float flOldValue );

	void	InitMapAdd( const char *pszFileName = NULL );
	void	ShutDownMapAdd( void );

	void	SetMapAddMode( bool bEnableMapAdd );

	void	OnRestore( void );

	bool	MapAddWasEnabledMidGame( void ) { return m_bMapAddEnabledMidGame; }

private:
	bool	m_bInitMapAdd;

	int		m_afPlayersLastButtons;
	bool	m_bMapAddConvarsChanging;
	int		m_iClearPressedButtons;
	bool	m_bMapAddEnabledMidGame;

	CUtlVector< mapadd_modifiedconvars_t > m_ModifiedConvars;
	CUtlVector<EHANDLE>				m_hSpawnedEntities;
};

extern CTFMapAddSystem g_LFEMapAdd;

inline CTFMapAddSystem* TFMapAddSystem( void )
{
	return &g_LFEMapAdd;
}

class CTFMapAddPlayer : public CBaseEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CTFMapAddPlayer, CBaseEntity );
	CTFMapAddPlayer();

	void		Spawn( void );
	void		Touch( CBaseEntity *pOther );

	int			m_iAddCond;
	char		m_pSpawnSound[128];
};

#endif // LFE_MAPADD_H
