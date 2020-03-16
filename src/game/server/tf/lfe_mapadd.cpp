//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: The system for handling extra scripted entity in-game.
//
//=============================================================================//

#include "cbase.h"

#include "igamesystem.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "soundenvelope.h"
#include "utldict.h"
#include "isaverestore.h"
#include "eventqueue.h"
#include "saverestore_utlvector.h"
#include "ai_basenpc.h"
#include "Sprite.h"
#include "datacache/imdlcache.h"
#include "lfe_mapadd.h"
#include "tf_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAPADD_SPAWNED_SEMAPHORE		"mapp_semaphore"

CTFMapAddSystem g_LFEMapAdd("CTFMapAddSystem");

bool g_bEnableMapAdd = false;

extern ConVar lfe_mapadd_file;

//===========================================================================================================
// LFE MAPPADD GAME SYSTEM
//===========================================================================================================
void CV_GlobalChange_LFEMapAdd( IConVar *var, const char *pOldString, float flOldValue );

BEGIN_DATADESC_NO_BASE( CTFMapAddSystem )
	//int m_afPlayersLastButtons;			DON'T SAVE
	//bool m_bMapAddConvarsChanging;	DON'T SAVE
	//int m_iClearPressedButtons;			DON'T SAVE

	DEFINE_UTLVECTOR( m_ModifiedConvars, FIELD_EMBEDDED ),
	DEFINE_UTLVECTOR( m_hSpawnedEntities, FIELD_EHANDLE ),
END_DATADESC()

BEGIN_SIMPLE_DATADESC( mapadd_modifiedconvars_t )
	DEFINE_ARRAY( pszConvar, FIELD_CHARACTER, MAPADD_MAX_MODIFIED_CONVAR_STRING ),
	DEFINE_ARRAY( pszCurrentValue, FIELD_CHARACTER, MAPADD_MAX_MODIFIED_CONVAR_STRING ),
	DEFINE_ARRAY( pszOrgValue, FIELD_CHARACTER, MAPADD_MAX_MODIFIED_CONVAR_STRING ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::LevelInitPreEntity( void )
{
	m_bInitMapAdd = false;
	m_bMapAddConvarsChanging = false;
	m_iClearPressedButtons = 0;

	g_bEnableMapAdd = TFGameRules()->IsMapAddAllowed();

	CalculateMapAddState();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::CalculateMapAddState( void )
{
	// Set the available cvar if we can find mapadd data for this level
	char szFullName[512];
	Q_snprintf(szFullName,sizeof(szFullName), "maps/mapadd/%s.txt", STRING( gpGlobals->mapname) );
	if ( !filesystem->FileExists( szFullName ) )
	{
		//ConColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Cannot find mapadd data: %s. \n", szFullName );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::LevelShutdownPreEntity( void )
{
	ShutDownMapAdd();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::ParseEntities( KeyValues *pSubKey )
{
	FOR_EACH_SUBKEY( pSubKey, pClassname )
	{
		if ( V_stricmp( pClassname->GetName(), "edit" ) == 0)
		{
			FOR_EACH_VALUE( pClassname, value )
			{
				CBaseEntity *pEntity = NULL;
				if ( V_stricmp(value->GetName(), "classname" ) == 0 )
				{
					pEntity = gEntList.FindEntityByClassname( NULL, value->GetString() );
				}
				else if ( V_stricmp(value->GetName(), "targetname" ) == 0 )
				{
					pEntity = gEntList.FindEntityByName( NULL, value->GetString() );
				}
				else if ( V_stricmp(value->GetName(), "model" ) == 0 || V_stricmp(value->GetName(), "Model" ) == 0 )
				{
					pEntity = gEntList.FindEntityByModel( NULL, value->GetString() );
				}
				if ( pEntity )
				{
					ConDColorMsg( Color(77, 116, 85, 255), "[MapAdd] Editing entity.\n", pEntity->GetClassname() );
					KeyValues *pValues = pClassname->FindKey( "values" );
					if ( pValues )
					{
						FOR_EACH_VALUE(pValues, pKeyvalues)
						{
							if ( V_stricmp( pKeyvalues->GetName(), "edt_addspawnflags" ) == 0 )
							{
								pEntity->AddSpawnFlags( pKeyvalues->GetInt() );
							}

							pEntity->KeyValue( pKeyvalues->GetName(), pKeyvalues->GetString() );
						}
					}
				}

			}
		}
		//else if ( !Q_strcmp( pClassname->GetName(), "delete" ) )
		else if ( V_stricmp( pClassname->GetName(), "delete") == 0 )
		{
			FOR_EACH_VALUE( pClassname, value )
			{
				CBaseEntity *pEntity = NULL;
				if( !Q_strcmp( value->GetName(), "classname" ) )
				{
					pEntity = gEntList.FindEntityByClassname( NULL, value->GetString() );
				}
				else if( !Q_strcmp(value->GetName(), "targetname") )
				{
					pEntity = gEntList.FindEntityByName( NULL, value->GetString() );
				}
				//else if ( !Q_strcmp(value->GetName(), "model") || !Q_strcmp(value->GetName(), "Model") )
				else if ( V_stricmp(value->GetName(), "model") == 0 || V_stricmp(value->GetName(), "Model") == 0 )
				{
					pEntity = gEntList.FindEntityByModel( NULL, value->GetString() );
				}

				if ( pEntity )
				{
					ConDColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Deleting %s.\n", pEntity->GetClassname() );
					UTIL_Remove( pEntity );
				}
			}
		}
		else if ( !Q_strcmp( pClassname->GetName(), "create" ) )
		{
			ParseCreate( pClassname );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::ParseCreate( KeyValues *pSubKey )
{
	FOR_EACH_VALUE( pSubKey, value )
	{
		KeyValues *pEntClassname = pSubKey->FindKey( "classname" );
		if ( pEntClassname )
		{
			const char *pszClassname = pEntClassname->GetString();

			CBaseEntity *pEntity = CreateEntityByName( pszClassname );
			if( pEntity )
			{
				bool bBSPModel = false;
				if( !Q_strcmp( value->GetName(), "origin" ) )
				{
					Vector VecOrigin;
					UTIL_StringToVector( VecOrigin.Base(), value->GetString() );
					pEntity->SetAbsOrigin( VecOrigin );
				}

				KeyValues *pKeyValues = pSubKey->FindKey( "values" );
				if( pKeyValues )
				{
					FOR_EACH_VALUE( pKeyValues, pValues )
					{
						bool bSetBBox = false;
						Vector vecSetBBoxMin = Vector( -10, -10, -10 );
						Vector vecSetBBoxMax = Vector( 10, 10, 10 );

						if (!Q_strcmp(pValues->GetName(), "targetname"))
						{
							CBaseEntity *pExistingEnt = gEntList.FindEntityByName(NULL, pValues->GetString());
							if (pExistingEnt)
							{
								UTIL_Remove(pEntity);
								break;
							}
						}
						else if ( !Q_strcmp( pValues->GetName(), "model" ) || !Q_strcmp( pValues->GetName(), "Model" ) )
						{
							CBaseEntity::PrecacheModel( pValues->GetString() );
							pEntity->SetModel( pValues->GetString() );
						}
						else if ( !Q_strcmp( pValues->GetName(), "edt_getbspmodelfor_targetname" ) )
						{
							bBSPModel = true;
							CBaseEntity *pEntTarget = gEntList.FindEntityByName( NULL, pValues->GetString() );
							if ( pEntTarget )
								pEntity->SetModelIndex( pEntTarget->GetModelIndex() );
						}
						else if ( !Q_strcmp( pValues->GetName(), "edt_getbspmodelfor_classname" ) )
						{
							bBSPModel = true;
							KeyValues *pGetBSPModelOrigin = pKeyValues->FindKey( "edt_getbspmodelfor_origin" );
							if( pGetBSPModelOrigin )
							{
								Vector VecBSPModelOrigin;
								UTIL_StringToVector( VecBSPModelOrigin.Base(), pGetBSPModelOrigin->GetString() );
								CBaseEntity *pEntClass = gEntList.FindEntityByClassnameNearest( pValues->GetString(), VecBSPModelOrigin, 1 );
								if ( pEntClass )
									pEntity->SetModelIndex( pEntClass->GetModelIndex() );
							}
							else
							{
								Vector VecBSPModelOrigin( 0.0f, 0.0f, 0.0f );
								CBaseEntity *pEntClass = gEntList.FindEntityByClassnameNearest( pValues->GetString(), VecBSPModelOrigin, 1 );
								if ( pEntClass )
									pEntity->SetModelIndex( pEntClass->GetModelIndex() );
							}
						}
						else if ( !Q_strcmp( pValues->GetName(), "edt_modelbbox_min" ) )
						{
							bSetBBox = true;
							UTIL_StringToVector( vecSetBBoxMin.Base(), pValues->GetString() );
						}
						else if ( !Q_strcmp( pValues->GetName(), "edt_modelbbox_max" ) )
						{
							bSetBBox = true;
							UTIL_StringToVector( vecSetBBoxMax.Base(), pValues->GetString() );
						}

						pEntity->KeyValue( pValues->GetName(), pValues->GetString() );

						if ( bSetBBox )
						{
							pEntity->SetSolid( SOLID_BBOX );
							UTIL_SetSize( pEntity, vecSetBBoxMin, vecSetBBoxMax );
						}
					}
				}
				if (pEntity)
				{
					ConDColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Creating %s.\n", pszClassname );
					pEntity->Precache();
					DispatchSpawn(pEntity);
					pEntity->Activate();
				}
			}
			else
			{
				ConColorMsg( Color( 100, 116, 85, 255 ), "[MapAdd] Failed to create: '%s'\n", pszClassname );
			}
		}
		else
		{
			ConColorMsg( Color( 100, 116, 85, 255 ), "[MapAdd] Unable to find classname\n" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::ParseConsole( KeyValues *pSubKey )
{
	FOR_EACH_SUBKEY( pSubKey, pCmdSub )
	{
		if( !Q_strcmp( pCmdSub->GetName(), pCmdSub->GetName() ) )
			engine->ServerCommand( UTIL_VarArgs( "%s %s\n", pCmdSub->GetName(), pCmdSub->GetString() ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::ParsePlayer( KeyValues *pSubKey )
{
	FOR_EACH_SUBKEY( pSubKey, pClassname )
	{
		CTFMapAddPlayer *pMapAddPlayer = dynamic_cast<CTFMapAddPlayer *>( CreateEntityByName( "lfe_mapadd_player" ) );
		if ( !pMapAddPlayer )
			return;

		if ( V_stricmp( pClassname->GetName(), "spawn_sound" ) == 0 )
		{
			Q_strncpy( pMapAddPlayer->m_pSpawnSound, pClassname->GetString(), sizeof(pMapAddPlayer->m_pSpawnSound) );
		}
		else if ( V_stricmp( pClassname->GetName(), "addcond" ) == 0 )
		{
			pMapAddPlayer->m_iAddCond = pClassname->GetInt();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::SetMapAddConvarsChanging( bool bChanging )
{
	m_bMapAddConvarsChanging = bChanging;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::ConvarChanged( IConVar *pConVar, const char *pOldString, float flOldValue )
{
	ConVarRef var( pConVar );

	// A convar has been changed by a commentary node. We need to store
	// the old state. If the engine shuts down, we need to restore any
	// convars that the commentary changed to their previous values.
	for ( int i = 0; i < m_ModifiedConvars.Count(); i++ )
	{
		// If we find it, just update the current value
		if ( !Q_strncmp( var.GetName(), m_ModifiedConvars[i].pszConvar, MAPADD_MAX_MODIFIED_CONVAR_STRING ) )
		{
			Q_strncpy( m_ModifiedConvars[i].pszCurrentValue, var.GetString(), MAPADD_MAX_MODIFIED_CONVAR_STRING );
			//Msg("    Updating Convar %s: value %s (org %s)\n", m_ModifiedConvars[i].pszConvar, m_ModifiedConvars[i].pszCurrentValue, m_ModifiedConvars[i].pszOrgValue );
			return;
		}
	}

	// We didn't find it in our list, so add it
	mapadd_modifiedconvars_t newConvar;
	Q_strncpy( newConvar.pszConvar, var.GetName(), MAPADD_MAX_MODIFIED_CONVAR_STRING );
	Q_strncpy( newConvar.pszCurrentValue, var.GetString(), MAPADD_MAX_MODIFIED_CONVAR_STRING );
	Q_strncpy( newConvar.pszOrgValue, pOldString, MAPADD_MAX_MODIFIED_CONVAR_STRING );
	m_ModifiedConvars.AddToTail( newConvar );

	/*
	Msg(" Commentary changed '%s' to '%s' (was '%s')\n", var->GetName(), var->GetString(), pOldString );
	Msg(" Convars stored: %d\n", m_ModifiedConvars.Count() );
	for ( int i = 0; i < m_ModifiedConvars.Count(); i++ )
	{
		Msg("    Convar %d: %s, value %s (org %s)\n", i, m_ModifiedConvars[i].pszConvar, m_ModifiedConvars[i].pszCurrentValue, m_ModifiedConvars[i].pszOrgValue );
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::InitMapAdd( const char *pszFileName )
{
	// Install the global cvar callback
	cvar->InstallGlobalChangeCallback( CV_GlobalChange_LFEMapAdd );

	// If we find the mapadd semaphore, the mapadd entities already exist.
	// This occurs when you transition back to a map that has saved mapadd nodes in it.
	if ( gEntList.FindEntityByName( NULL, MAPADD_SPAWNED_SEMAPHORE ) )
		return;

	// Spawn the mapadd semaphore entity
	CBaseEntity *pSemaphore = CreateEntityByName( "info_target" );
	pSemaphore->SetName( MAKE_STRING(MAPADD_SPAWNED_SEMAPHORE) );

	// Find the mapadd file
	char szFullName[128];
	Q_snprintf( szFullName, sizeof(szFullName), "maps/mapadd/%s.txt", STRING( gpGlobals->mapname ) );

	char szFullNameP2[128];
	Q_snprintf( szFullNameP2, sizeof(szFullNameP2), "maps/mapadd/%s_p2.txt", STRING( gpGlobals->mapname ) );
	char szFullNameP3[128];
	Q_snprintf( szFullNameP3, sizeof(szFullNameP3), "maps/mapadd/%s_p3.txt", STRING( gpGlobals->mapname ) );

	KeyValues *pkvFile = new KeyValues( "MapAdd" );

	if ( V_stricmp( STRING(gpGlobals->mapname), mapedit_desiredmap.GetString() ) != 0 )
	{
		mapedit_desiredmap.SetValue(STRING(gpGlobals->mapname));
		mapedit_desiredpart.SetValue(0);
	}
	if ( mapedit_desiredpart.GetInt() > 2 )
	{
		mapedit_desiredpart.SetValue(0);
	}
	if ( filesystem->FileExists(szFullNameP2, "MOD") && mapedit_desiredpart.GetInt() == 1 )
	{
		Q_snprintf(szFullName, sizeof(szFullName), "%s", szFullNameP2);
	}
	if ( filesystem->FileExists(szFullNameP3, "MOD") && mapedit_desiredpart.GetInt() == 2 )
	{
		Q_snprintf(szFullName, sizeof(szFullName), "%s", szFullNameP3);
	}
	mapedit_desiredpart.SetValue( mapedit_desiredpart.GetInt() + 1 );

	if ( pszFileName && pszFileName[0] )
		Q_snprintf( szFullName, sizeof(szFullName), pszFileName );

	if ( pkvFile->LoadFromFile( filesystem, szFullName, "MOD" ) )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Loading mapadd data from %s. \n", szFullName );

		lfe_mapadd_file.SetValue( szFullName );

		KeyValues *pConsole = pkvFile->FindKey( "console" );
		if( pConsole )
		{
			ConColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Executing commands... \n" );
			ParseConsole( pConsole );
		}

		KeyValues *pEntity = pkvFile->FindKey( "entity" );
		if( pEntity )
		{
			ConColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Parsing entities... \n" );
			ParseEntities( pEntity );
		}

		KeyValues *pPlayer = pkvFile->FindKey( "player" );
		if( pPlayer )
		{
			ConColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Parsing player... \n" );
			ParsePlayer( pPlayer );
		}
	}
	else
	{
		ConColorMsg( Color( 77, 116, 85, 255 ), "[MapAdd] Cannot find mapadd data '%s'. \n", szFullName );
	}

	pkvFile->deleteThis();

	m_bInitMapAdd = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::ShutDownMapAdd( void )
{
	// Destroy all the entities created by mapadd
	for ( int i = m_hSpawnedEntities.Count()-1; i >= 0; i-- )
	{
		if ( m_hSpawnedEntities[i] )
		{
			UTIL_Remove( m_hSpawnedEntities[i] );
		}
	}
	m_hSpawnedEntities.Purge();

	// Remove the mapadd semaphore
	CBaseEntity *pSemaphore = gEntList.FindEntityByName( NULL, MAPADD_SPAWNED_SEMAPHORE );
	if ( pSemaphore )
	{
		UTIL_Remove( pSemaphore );
	}

	// Remove our global convar callback
	cvar->RemoveGlobalChangeCallback( CV_GlobalChange_LFEMapAdd );

	// Reset any convars that have been changed by the mapadd
	for ( int i = 0; i < m_ModifiedConvars.Count(); i++ )
	{
		ConVar *pConVar = (ConVar *)cvar->FindVar( m_ModifiedConvars[i].pszConvar );
		if ( pConVar )
		{
			pConVar->SetValue( m_ModifiedConvars[i].pszOrgValue );
		}
	}
	m_ModifiedConvars.Purge();

	lfe_mapadd_file.SetValue( "" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::SetMapAddMode( bool bEnableMapAdd )
{
	g_bEnableMapAdd = bEnableMapAdd;
	CalculateMapAddState();

	// If we're turning on mapadd, create all the entities.
	if ( bEnableMapAdd )
	{
		const char *pszFileName = lfe_mapadd_file.GetString();
		if ( pszFileName && pszFileName[0] )
			InitMapAdd( pszFileName );
		else
			InitMapAdd();
	}
	else
	{
		ShutDownMapAdd();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMapAddSystem::OnRestore( void )
{
	cvar->RemoveGlobalChangeCallback( CV_GlobalChange_LFEMapAdd );

	if ( !TFGameRules()->IsMapAddAllowed() )
		return;

	// Set any convars that have already been changed by the mapadd before the save
	for ( int i = 0; i < m_ModifiedConvars.Count(); i++ )
	{
		ConVar *pConVar = (ConVar *)cvar->FindVar( m_ModifiedConvars[i].pszConvar );
		if ( pConVar )
		{
			//Msg("    Restoring Convar %s: value %s (org %s)\n", m_ModifiedConvars[i].pszConvar, m_ModifiedConvars[i].pszCurrentValue, m_ModifiedConvars[i].pszOrgValue );
			pConVar->SetValue( m_ModifiedConvars[i].pszCurrentValue );
		}
	}

	// Install the global cvar callback
	cvar->InstallGlobalChangeCallback( CV_GlobalChange_LFEMapAdd );
}

//-----------------------------------------------------------------------------
// Purpose: We need to revert back any convar changes that are made by the
//			mapadd system during mapadd. This code stores convar changes
//			made by the mapadd system, and reverts them when finished.
//-----------------------------------------------------------------------------
void CV_GlobalChange_LFEMapAdd( IConVar *var, const char *pOldString, float flOldValue )
{
	if ( !g_LFEMapAdd.MapAddConvarsChanging() )
	{
		// A convar has changed, but not due to mapadd nodes. Ignore it.
		return;
	}

	g_LFEMapAdd.ConvarChanged( var, pOldString, flOldValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CC_MapAddNotChanging( void )
{
	g_LFEMapAdd.SetMapAddConvarsChanging( false );
}

static ConCommand lfe_mapadd_cvarsnotchanging("lfe_mapadd_cvarsnotchanging", CC_MapAddNotChanging, 0 );


LINK_ENTITY_TO_CLASS( lfe_mapadd_player, CTFMapAddPlayer );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CTFMapAddPlayer )
END_DATADESC()

CTFMapAddPlayer::CTFMapAddPlayer()
{
	m_iAddCond = -1;
}

void CTFMapAddPlayer::Spawn( void )
{
	BaseClass::Spawn();
}

void CTFMapAddPlayer::Touch( CBaseEntity *pOther )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pOther );
	if ( !pTFPlayer )
		return;

	if ( m_iAddCond != -1 )
		pTFPlayer->m_Shared.AddCond( m_iAddCond );

	if ( m_pSpawnSound && m_pSpawnSound[0] != '\0' )
	{
		PrecacheScriptSound( m_pSpawnSound );
		pTFPlayer->EmitSound( m_pSpawnSound );
	}
}
