#include "cbase.h"
#include "gamemounter.h"
#include "filesystem.h"
#include "steam/steam_api.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "engine/IEngineSound.h"
#include "tier1/KeyValues.h"
#include "tier0/icommandline.h"
#include "datacache/imdlcache.h"

#ifdef CLIENT_DLL
#include "tf/vgui/panels/lfe_createmultiplayergameserverpage.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_gamemounted("cl_gamemounted", "", FCVAR_REPLICATED | FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY);
bool bDisconnected = false;
bool bMapMountChange = false;
bool bCustomMounted = true; // Init as true
bool bInitShadow = false;

const char *szCurrentGameName;

CUtlStringList g_pFullMapList;

bool EvaluateExtraConditionals( const char* str )
{
	bool bNot = false; // should we negate this command?
	if ( *str == '!' )
		bNot = true;

	if ( Q_stristr( str, "$DEDICATED" ) )
	{
#ifdef CLIENT_DLL
		return false ^ bNot;
#else
		return engine->IsDedicatedServer() ^ bNot;
#endif
	}

	return false;
}

#ifdef GAME_DLL
bool AttemptFindSteamLibraries(char *szRootDir, char szRetPath[MAX_PATH * 2], char szSubDir[MAX_PATH])
{
	if (!szRootDir) return false;

	char szSteamPath[MAX_PATH * 2];
	Q_snprintf(szSteamPath, sizeof(szSteamPath), "C:/Program Files (x86)/Steam/steamapps/common/%s", szRootDir);
	Q_FixSlashes(szSteamPath);
	
	if (filesystem->IsDirectory(szSteamPath, NULL))
	{
		if (Q_strlen(szSubDir))
		{
			Q_snprintf(szSteamPath, sizeof(szSteamPath), "%s/%s", szSteamPath, szSubDir);
			if (filesystem->IsDirectory(szSteamPath, NULL))
			{
				bool bFoundVPK = false;
				FileFindHandle_t vpkfh;
				char const *vpkfn = g_pFullFileSystem->FindFirst(UTIL_VarArgs("%s/*.vpk", szSteamPath), &vpkfh);
				g_pFullFileSystem->FindClose(vpkfh);
				if (vpkfn) bFoundVPK = true;
				if ((filesystem->IsDirectory(UTIL_VarArgs("%s/maps", szSteamPath))) || (bFoundVPK))
				{
					Q_snprintf(szSteamPath, sizeof(szSteamPath), "C:/Program Files (x86)/Steam/steamapps/common/%s", szRootDir);
					Q_FixSlashes(szSteamPath);
					Q_snprintf(szRetPath, MAX_PATH * 2, "%s", szSteamPath);
					return true;
				}
			}
		}
		else
		{
			Q_FixSlashes(szSteamPath);
			Q_snprintf(szRetPath, MAX_PATH * 2, "%s", szSteamPath);
			return true;
		}
	}
	char szFullPath[512];
	if (Q_strlen(SteamAPI_GetSteamInstallPath()) > 0)
	{
		Q_snprintf(szFullPath, sizeof(szFullPath), "%s/steamapps/libraryfolders.vdf", SteamAPI_GetSteamInstallPath());
		if (!filesystem->FileExists(szFullPath, NULL))
		{
			V_GetCurrentDirectory(szFullPath, sizeof(szFullPath));
			if (Q_strlen(szFullPath) > 0) V_strcat(szFullPath, "/../../libraryfolders.vdf", COPY_ALL_CHARACTERS);
			else Q_snprintf(szFullPath, sizeof(szFullPath), "../../libraryfolders.vdf");
		}
	}
	else
	{
		V_GetCurrentDirectory(szFullPath, sizeof(szFullPath));
		Q_snprintf(szFullPath, sizeof(szFullPath), "%s/../../libraryfolders.vdf", szFullPath);
		if (!filesystem->FileExists(szFullPath, NULL))
		{
			if (filesystem->FileExists("C:/Program Files (x86)/Steam/steamapps/libraryfolders.vdf", NULL))
			{
				Q_snprintf(szFullPath, sizeof(szFullPath), "C:/Program Files (x86)/Steam/steamapps/libraryfolders.vdf");
			}
			else if (filesystem->FileExists("C:/Program Files/Steam/steamapps/libraryfolders.vdf", NULL))
			{
				Q_snprintf(szFullPath, sizeof(szFullPath), "C:/Program Files/Steam/steamapps/libraryfolders.vdf");
			}
			else
			{
				Q_snprintf(szFullPath, sizeof(szFullPath), "../../libraryfolders.vdf");
			}
		}
	}

	if (filesystem->FileExists(szFullPath, NULL))
	{
		KeyValues *pkvFile = new KeyValues("");
		if (pkvFile->LoadFromFile(filesystem, szFullPath, NULL))
		{
			KeyValues *pkvNode = pkvFile->GetFirstSubKey();
			while (pkvNode)
			{
				const char *pszValue = pkvNode->GetString();
				if ((Q_strlen(pszValue) < 1) && (Q_strlen(pkvNode->GetName()) > 0))
				{
					// Get next sub key from new libraryfolders layout
					pszValue = pkvNode->GetString("path", "");
				}
				if ((pszValue != NULL) && (strlen(pszValue) > 0))
				{
					if ((V_strstr(pszValue, "/") != NULL) || (V_strstr(pszValue, "\\\\") != NULL))
					{
						Q_snprintf(szSteamPath, sizeof(szSteamPath), "%s/steamapps/common/%s", pszValue, szRootDir);
						V_FixDoubleSlashes(szSteamPath);
						Q_FixSlashes(szSteamPath);
						
						if (filesystem->IsDirectory(szSteamPath, NULL))
						{
							if (Q_strlen(szSubDir))
							{
								Q_snprintf(szRetPath, MAX_PATH * 2, "%s/%s", szSteamPath, szSubDir);
								Q_FixSlashes(szRetPath);
								pkvFile->deleteThis();
								return true;
							}
							else
							{
								Q_snprintf(szRetPath, MAX_PATH * 2, "%s", szSteamPath);
								Q_FixSlashes(szRetPath);
								pkvFile->deleteThis();
								return true;
							}
						}
					}
				}
				pkvNode = pkvNode->GetNextKey();
			}
			pkvNode->deleteThis();
		}
		pkvFile->deleteThis();
	}
	
	if (filesystem->FileExists("C:/Program Files (x86)/Steam/steamapps/libraryfolders.vdf", NULL))
	{
		KeyValues *pkvFile = new KeyValues("");
		if (pkvFile->LoadFromFile(filesystem, "C:/Program Files (x86)/Steam/steamapps/libraryfolders.vdf", NULL))
		{
			KeyValues *pkvNode = pkvFile->GetFirstSubKey();
			while (pkvNode)
			{
				const char *pszValue = pkvNode->GetString();
				if ((pszValue != NULL) && (strlen(pszValue) > 0))
				{
					char szValueLoc[256];
					Q_snprintf(szValueLoc, sizeof(szValueLoc), "%s", pszValue);
					
					V_FixDoubleSlashes(szValueLoc);
					if ((V_strstr(szValueLoc, "/") != NULL) || (V_strstr(szValueLoc, "\\") != NULL))
					{
						Q_snprintf(szSteamPath, sizeof(szSteamPath), "%s/steamapps/common/%s", szValueLoc, szRootDir);
						V_FixDoubleSlashes(szSteamPath);
						Q_FixSlashes(szSteamPath);
						//Msg("Check Exist \"%s\"\n", szSteamPath);
						if (filesystem->IsDirectory(szSteamPath, NULL))
						{
							if (Q_strlen(szSubDir))
							{
								if (filesystem->IsDirectory(UTIL_VarArgs("%s/%s", szSteamPath, szSubDir), NULL))
								{
									bool bFoundVPK = false;
									FileFindHandle_t vpkfh;
									char const *vpkfn = g_pFullFileSystem->FindFirst(UTIL_VarArgs("%s/%s/*.vpk", szSteamPath, szSubDir), &vpkfh);
									g_pFullFileSystem->FindClose(vpkfh);
									if (vpkfn) bFoundVPK = true;
									if ((filesystem->IsDirectory(UTIL_VarArgs("%s/maps", szSteamPath))) || (bFoundVPK))
									{
										//Msg("DirExists %i Found %i\n", filesystem->IsDirectory(UTIL_VarArgs("%s/maps", szSteamPath)), bFoundVPK);
										Q_snprintf(szRetPath, MAX_PATH * 2, "%s", szSteamPath);
										Q_FixSlashes(szRetPath);
										return true;
									}
								}
							}
							else
							{
								Q_snprintf(szRetPath, MAX_PATH * 2, "%s", szSteamPath);
								Q_FixSlashes(szRetPath);
								return true;
							}
						}
					}
				}
				pkvNode = pkvNode->GetNextKey();
			}
			pkvNode->deleteThis();
		}
		pkvFile->deleteThis();
	}
	
	// Final check in parent directory
	if (filesystem->IsDirectory(UTIL_VarArgs("../../%s", szRootDir)))
	{
		if (Q_strlen(szSubDir) > 0) Q_snprintf(szRetPath, MAX_PATH * 2, "../../%s/%s", szRootDir, szSubDir);
		else Q_snprintf(szRetPath, MAX_PATH * 2, "../../%s", szRootDir);
		Q_FixSlashes(szRetPath);
		
		return true;
	}

	if (filesystem->IsDirectory(UTIL_VarArgs("../%s", szRootDir)))
	{
		if (Q_strlen(szSubDir) > 0) Q_snprintf(szRetPath, MAX_PATH * 2, "../%s/%s", szRootDir, szSubDir);
		else Q_snprintf(szRetPath, MAX_PATH * 2, "../%s", szRootDir);
		Q_FixSlashes(szRetPath);

		return true;
	}

	return false;
}
#endif

// brute forces our search paths, reads the users steam configs
// to determine any additional steam library directories people have
// as there's no other way to currently mount a different game (css) 
// if it's located in a different library without an absolute path
void MountPathLocal( KeyValues* pGame, SearchPathAdd_t addType = PATH_ADD_TO_TAIL, bool bRemoveOld = false, const char *pMapName = NULL )
{
	const char *szGameName = pGame->GetName();
	const bool bRequired = pGame->GetBool( "required", false );
	const bool bSourceMod = pGame->GetBool("sourcemod", false);
	bool bSetVars = false;
	int iGameMode = pGame->GetInt("gamemode", -1);

	char szLocalizeUI[MAX_PATH];
	char szLocalizeCC[MAX_PATH];
	ConVarRef cc_lang("cc_lang");
	if (cc_lang.IsValid())
		Q_snprintf(szLocalizeCC, sizeof(szLocalizeCC), "_%s", cc_lang.GetString());
#ifdef CLIENT_DLL
	char tmp[MAX_PATH];
	engine->GetUILanguage(tmp, sizeof(tmp));
	Q_snprintf(szLocalizeUI, sizeof(szLocalizeUI), "_%s", tmp);
#endif

	if ( !steamapicontext || !steamapicontext->SteamApps() )
	{
		if ( bRequired )
			Error( "Failed to mount required game: %s, unable to determine app install path.\nPlease make sure Steam is running, and the game is installed properly.\n", szGameName );
		else
			Msg( "Skipping %s, unable to get app install path.\n", szGameName );

		return;
	}

	char szPath[ MAX_PATH * 2 ];
	int ccFolder = steamapicontext->SteamApps()->GetAppInstallDir( pGame->GetUint64( "appid" ), szPath, sizeof( szPath ) );

	if ( ccFolder > 0 )
	{
		ConColorMsg( Color( 90, 240, 90, 255 ), "Mounting %s (local)\n", szGameName );

		if (bSourceMod)
		{
			V_strncat(szPath, "/../../sourcemods/", ARRAYSIZE(szPath));
			if ((g_pFullFileSystem && (!g_pFullFileSystem->IsDirectory(szPath, NULL))) || (filesystem && (!filesystem->IsDirectory(szPath, NULL))))
			{
				DevMsg(1, "Failed to find in parent directory, attempting static path\n");
				Q_snprintf(szPath, sizeof(szPath), "C:/Program Files (x86)/Steam/steamapps/sourcemods/");
			}
		}

		KeyValues *pPaths = pGame->FindKey( "paths" );

		if ( !pPaths )
			return;

		for ( KeyValues *pPath = pPaths->GetFirstSubKey(); pPath; pPath = pPath->GetNextKey() )
		{
			if ( V_strncmp( pPath->GetName(), "local", 5 ) != 0 )
				continue;

			SearchPathAdd_t lAddType = addType;
			char szTempPath[ MAX_PATH * 2 ];
			Q_strncpy( szTempPath, szPath, ARRAYSIZE( szTempPath ) );

			V_AppendSlash( szTempPath, ARRAYSIZE( szTempPath ) );
			V_strncat( szTempPath, pPath->GetString(), ARRAYSIZE( szTempPath ) );

			// Add shadow of TF2
			if (!bInitShadow && (V_strstr(szGameName, "Team Fortress 2") != NULL)) //|| (V_strstr(szGameName, "Half-Life 2") != NULL))) //(V_strstr(szGameName, "Half-Life 2/hl2") != NULL) || (V_strstr(szGameName, "Half-Life 2\\hl2") != NULL)))
			{
				filesystem->AddSearchPath(szTempPath, "GAMESHADOW", addType);
			}

			if (pMapName)
			{
				if ((!V_strcmp(pPath->GetName(), "localep2")) && (!V_strncmp(pMapName, "ep2", 3)))
				{
					lAddType = PATH_ADD_TO_HEAD;
				}
				else if ((!V_strcmp(pPath->GetName(), "localep1")) && (!V_strncmp(pMapName, "ep1", 3)))
				{
					lAddType = PATH_ADD_TO_HEAD;
				}
				// TF2 overrides concrete and some other materials this doesn't need to be done on the server unless some material proxies break
				else if ((!V_strcmp(szGameName, "Half-Life 2")) && ((V_strstr(szTempPath, "hl2_misc") != NULL) || (V_strstr(szTempPath, "hl2_textures") != NULL)) &&
					((!V_strncmp(pMapName, "d1", 2)) || (!V_strncmp(pMapName, "d2", 2)) || (!V_strncmp(pMapName, "d3", 2))))
				{
					lAddType = PATH_ADD_TO_HEAD;
				}
			}

			if (bRemoveOld)
			{
				if (g_pFullFileSystem) g_pFullFileSystem->RemoveSearchPath(szTempPath, "GAME");
				else filesystem->RemoveSearchPath(szTempPath, "GAME");
				ConColorMsg(Color(90, 240, 90, 255), "\tRemoving path: %s\n", pPath->GetString());
			}
			else
			{
				if (g_pFullFileSystem) g_pFullFileSystem->AddSearchPath(szTempPath, "GAME", lAddType);
				else filesystem->AddSearchPath(szTempPath, "GAME", lAddType);
				ConColorMsg(Color(144, 238, 144, 255), "\tAdding path: %s\n", pPath->GetString());
			}

			if (filesystem->IsDirectory(szTempPath, NULL))
			{
				// Found directory, perform localization search/mount
				FileFindHandle_t vpkfh;
				char szFullDir[512];
				char szSearchCustom[512];
				Q_snprintf(szSearchCustom, sizeof(szSearchCustom), "%s/*", szTempPath);

				char const *pszSubFileName = NULL;
				if (g_pFullFileSystem) pszSubFileName = g_pFullFileSystem->FindFirst(szSearchCustom, &vpkfh);
				else pszSubFileName = filesystem->FindFirst(szSearchCustom, &vpkfh);

				if (pszSubFileName)
				{
					do
					{
						if (pszSubFileName[0] != '.')
						{
							if ((V_strstr(pszSubFileName, ".vpk") != NULL) && (V_strstr(pszSubFileName, "00") == NULL) && (V_strstr(pszSubFileName, ".cache") == NULL) && (V_strstr(pszSubFileName, ".ztmp") == NULL) && (V_strstr(pszSubFileName, ".bz2") == NULL))
							{
								if ((V_strstr(pszSubFileName, szLocalizeUI) != NULL) || (V_strstr(pszSubFileName, szLocalizeCC) != NULL))
								{
									Q_snprintf(szFullDir, sizeof(szFullDir), "%s/%s", szTempPath, pszSubFileName);
									Q_FixSlashes(szFullDir);
									if (g_pFullFileSystem) g_pFullFileSystem->AddSearchPath(szFullDir, "GAME", lAddType);
									else filesystem->AddSearchPath(szFullDir, "GAME", lAddType);
									ConColorMsg(Color(144, 238, 144, 255), "\tAdding Localization VPK: %s\n", pszSubFileName);

									break;
								}
							}
						}
						if (g_pFullFileSystem) pszSubFileName = g_pFullFileSystem->FindNext(vpkfh);
						else pszSubFileName = filesystem->FindNext(vpkfh);
					} while (pszSubFileName);
				}
				if (g_pFullFileSystem) g_pFullFileSystem->FindClose(vpkfh);
				else filesystem->FindClose(vpkfh);
				/*
				if (!bSetVars)
				{

				}
				*/
				bSetVars = true;
			}
		}
	}
	else if ( bRequired )
	{
		Error( "Failed to mount required game: %s\n", szGameName );
	}
	else
	{
		Warning( "%s not found on system. Skipping.\n", szGameName );
	}

	if (bSetVars)
	{
		// Additional vars to be determined
		if (iGameMode > -1)
		{
			ConVarRef mp_gamemode("mp_gamemode", false);
			if (mp_gamemode.IsValid())
				mp_gamemode.SetValue(iGameMode);
		}
	}
}

bool FindConflictContentTag(const char *pContentTag)
{
	if ((!pContentTag) || (g_pFullMapList.Count() < 1))
		return false;

	for (int i = 0; i < g_pFullMapList.Count(); i++)
	{
		if (!V_strcmp(g_pFullMapList[i], pContentTag))
		{
			// Conflicted with additional content tags
			if (i > 0)
			{
				if (!V_strcmp(g_pFullMapList[i - 1], "endofmaplist"))
				{
					return true;
				}
			}
			else
			{
				// Conflicted with first content tag
				return true;
			}
		}
	}

	return false;
}

#ifdef GAME_DLL
void MountPathDedicated( KeyValues* pGame, SearchPathAdd_t addType = PATH_ADD_TO_TAIL, bool bRemoveOld = false, const char *pMapName = NULL )
{
	const char* szGameName = pGame->GetName();
	//const bool bRequired = pGame->GetBool( "required", false );
	const bool bSourceMod = pGame->GetBool("sourcemod", false);
	int iGameMode = pGame->GetInt("gamemode", -1);
	bool bFoundAtRelative = false;
	bool bSetVars = false;

	ConDColorMsg( Color( 90, 240, 90, 255 ), "Mounting %s (dedicated)\n", szGameName );

	KeyValues *pPaths = pGame->FindKey( "paths" );

	if ( !pPaths )
		return;

	for ( KeyValues *pPath = pPaths->GetFirstSubKey(); pPath; pPath = pPath->GetNextKey() )
	{
		if ( V_strncmp( pPath->GetName(), "dedicated", 9 ) != 0 )
			continue;

		SearchPathAdd_t lAddType = addType;
		const char* szRelativePath = pPath->GetString();
		if (bSourceMod)
		{
			char szSMRelative[MAX_PATH * 2];
			Q_snprintf(szSMRelative, sizeof(szSMRelative), "../sourcemods/%s", pPath->GetString());
			szRelativePath = szSMRelative;
			//V_strcat((char*)szRelativePath, pPath->GetString(), COPY_ALL_CHARACTERS);
		}

		if (pMapName)
		{
			if ((V_strcmp(pPath->GetName(), "dedicatedep2") == 0) && (!V_strncmp(pMapName, "ep2", 3)))
			{
				lAddType = PATH_ADD_TO_HEAD;
			}
			else if ((V_strcmp(pPath->GetName(), "dedicatedep1") == 0) && (!V_strncmp(pMapName, "ep1", 3)))
			{
				lAddType = PATH_ADD_TO_HEAD;
			}
		}

		char gamedir[MAX_PATH * 2];
		filesystem->GetCurrentDirectory( gamedir, sizeof( gamedir ) );
		V_StripLastDir( gamedir, sizeof( gamedir ) );
		if (!bSourceMod) V_strcat( gamedir, szGameName, sizeof( gamedir ) );
		V_AppendSlash( gamedir, sizeof( gamedir ) );
		V_strcat( gamedir, szRelativePath, sizeof( gamedir ) );
		V_FixSlashes(gamedir);

		if (!bFoundAtRelative && (!filesystem->IsDirectory(gamedir, NULL)) && (!filesystem->FileExists(gamedir, NULL)))
		{
			char szRootDir[MAX_PATH * 2];
			char szRelative[MAX_PATH * 2];
			Q_snprintf(szRootDir, sizeof(szRootDir), "%s", szGameName);
			Q_snprintf(szRelative, sizeof(szRelative), "%s", szRelativePath);
			if (!AttemptFindSteamLibraries(szRootDir, gamedir, szRelative))
			{
				if (V_strstr(szRootDir, "Team Fortress 2 Dedicated") != NULL)
				{
					if (!AttemptFindSteamLibraries("Team Fortress 2", gamedir, szRelative))
					{
						Msg("Dedicated server failback to CL failed '%s' Relative: '%s'\n", gamedir, szRelative);
					}
				}
			}
		}
		else if (bSourceMod)
			bFoundAtRelative = true;

		if (!bInitShadow && (V_strstr(szGameName, "Team Fortress 2") != NULL))
		{
			g_pFullFileSystem->AddSearchPath(gamedir, "GAMESHADOW", lAddType);
		}

		if (bRemoveOld)
		{
			g_pFullFileSystem->RemoveSearchPath(gamedir, "GAME");
			ConDColorMsg(Color(90, 240, 90, 255), "\tRemoving path: %s\n", gamedir);
		}
		else
		{
			g_pFullFileSystem->AddSearchPath(gamedir, "GAME", lAddType);
			ConDColorMsg(Color(90, 240, 90, 255), "\tAdding path: %s\n", gamedir);
		}

		if (filesystem->IsDirectory(gamedir, NULL))
		{
			bSetVars = true;
		}
	}

	if (bSetVars)
	{
		if (iGameMode > -1)
		{
			ConVarRef mp_gamemode("mp_gamemode", false);
			if (mp_gamemode.IsValid())
				mp_gamemode.SetValue(iGameMode);
		}
	}
}
#endif

void AddMapsAtPath(const char *pPath)
{
	FileFindHandle_t vpkfh;
	char szFullDir[512];
	char szSearchCustom[512];
	Q_snprintf(szSearchCustom, sizeof(szSearchCustom), "%s/*", pPath);

	char const *pszSubFileName = NULL;
	if (g_pFullFileSystem) pszSubFileName = g_pFullFileSystem->FindFirst(szSearchCustom, &vpkfh);
	else pszSubFileName = filesystem->FindFirst(szSearchCustom, &vpkfh);

	if (pszSubFileName)
	{
		do
		{
			if (pszSubFileName[0] != '.')
			{
				if ((V_strstr(pszSubFileName, ".bsp") != NULL) && (V_strstr(pszSubFileName, ".ztmp") == NULL) && (V_strstr(pszSubFileName, ".bz2") == NULL))
				{
					Q_snprintf(szFullDir, sizeof(szFullDir), "%s/%s", pPath, pszSubFileName);
					Q_FixSlashes(szFullDir);
					//Q_MakeAbsolutePath(szFullDir, sizeof(szFullDir), szFullDir, NULL);

					char szBSPNoEXT[256];
					Q_StripExtension(pszSubFileName, szBSPNoEXT, sizeof(szBSPNoEXT));
#ifndef CLIENT_DLL
					if (engine->IsMapValid(szFullDir))
#endif
						g_pFullMapList.CopyAndAddToTail(szBSPNoEXT);
				}
			}
			if (g_pFullFileSystem) pszSubFileName = g_pFullFileSystem->FindNext(vpkfh);
			else pszSubFileName = filesystem->FindNext(vpkfh);
		} while (pszSubFileName);
	}
	if (g_pFullFileSystem) g_pFullFileSystem->FindClose(vpkfh);
	else filesystem->FindClose(vpkfh);
}

#ifdef CLIENT_DLL
void ParseMod(KeyValues *pGame, const char *pMounterFile)
{
	const bool bSourceMod = pGame->GetBool("sourcemod", false);
	const bool bShowInCreateServer = pGame->GetBool("showinmenu", false);
	const char *szGameName = pGame->GetName();
	const char *pContentTag = pGame->GetString("contenttag", szGameName);
	bool bMapsDirAdded = false;

	if (!steamapicontext || !steamapicontext->SteamApps())
	{
		return;
	}

	char szPath[MAX_PATH * 2];
	int ccFolder = steamapicontext->SteamApps()->GetAppInstallDir(pGame->GetUint64("appid"), szPath, sizeof(szPath));

	if (ccFolder > 0)
	{
		if (FindConflictContentTag(pContentTag))
		{
			Warning("Content Tag: '%s' is already defined!\n", pContentTag);
			return;
		}
		else
			g_pFullMapList.CopyAndAddToTail(pContentTag);

		if (bSourceMod)
		{
			V_strncat(szPath, "/../../sourcemods/", ARRAYSIZE(szPath));
			if ((g_pFullFileSystem && (!g_pFullFileSystem->IsDirectory(szPath, NULL))) || (filesystem && (!filesystem->IsDirectory(szPath, NULL))))
			{
				Q_snprintf(szPath, sizeof(szPath), "C:/Program Files (x86)/Steam/steamapps/sourcemods/");
			}
		}

		KeyValues *pPaths = pGame->FindKey("paths");

		if (!pPaths)
			return;

		for (KeyValues *pPath = pPaths->GetFirstSubKey(); pPath; pPath = pPath->GetNextKey())
		{
			if (!FStrEq(pPath->GetName(), "local"))
				continue;

			char szTempPath[MAX_PATH * 2];
			Q_strncpy(szTempPath, szPath, ARRAYSIZE(szTempPath));
			V_AppendSlash(szTempPath, ARRAYSIZE(szTempPath));
			V_strncat(szTempPath, pPath->GetString(), ARRAYSIZE(szTempPath));
			V_FixSlashes(szTempPath);

			char szCheckForMaps[MAX_PATH * 2];
			Q_snprintf(szCheckForMaps, sizeof(szCheckForMaps), "%s/maps", szTempPath);
			V_FixSlashes(szCheckForMaps);

			if ((filesystem->IsDirectory(szCheckForMaps, NULL)) && (V_strstr(szTempPath, ".vpk") == NULL))
			{
				if (bShowInCreateServer)
				{
					if (g_pGameServerPage)
						g_pGameServerPage->LoadMapsAtPath(szTempPath);
				}

				if (!bMapsDirAdded)
				{
					bMapsDirAdded = true;
					g_pFullMapList.CopyAndAddToTail(szTempPath);

					if (bSourceMod)
						ConColorMsg(Color(90, 240, 90, 255), "Found game: '%s' (sourcemod) at: '%s'\n", szGameName, pPath->GetString());
					else
						ConColorMsg(Color(90, 240, 90, 255), "Found game: '%s' (Steam) at: '%s'\n", szGameName, szTempPath);
				}
				AddMapsAtPath(szCheckForMaps);
			}
		}

		g_pFullMapList.CopyAndAddToTail("endofmaplist");
	}

	if (!bMapsDirAdded)
	{
		Warning("[CL]Unable to find mounting information for: '%s'\n", szGameName);
	}
}

#else

void ParseModDedicated(KeyValues *pGame, const char *pMounterFile)
{
	const bool bSourceMod = pGame->GetBool("sourcemod", false);
	const char *szGameName = pGame->GetName();
	const char *pContentTag = pGame->GetString("contenttag", szGameName);
	bool bMapsDirAdded = false;

	KeyValues *pPaths = pGame->FindKey("paths");

	if (!pPaths)
		return;

	if (FindConflictContentTag(pContentTag))
	{
		Warning("Content Tag: '%s' is already defined!\n", pContentTag);
		return;
	}
	else
		g_pFullMapList.CopyAndAddToTail(pContentTag);

	int iArrPos = g_pFullMapList.Count();

	for (KeyValues *pPath = pPaths->GetFirstSubKey(); pPath; pPath = pPath->GetNextKey())
	{
		if (!FStrEq(pPath->GetName(), "dedicated"))
			continue;

		const char *szRelativePath = pPath->GetString();
		if (bSourceMod)
		{
			char szSMRelative[MAX_PATH * 2];
			Q_snprintf(szSMRelative, sizeof(szSMRelative), "../sourcemods/%s", pPath->GetString());
			szRelativePath = szSMRelative;
			//V_strcat((char*)szRelativePath, pPath->GetString(), COPY_ALL_CHARACTERS);
		}

		char gamedir[MAX_PATH * 2];
		filesystem->GetCurrentDirectory(gamedir, sizeof(gamedir));
		V_StripLastDir(gamedir, sizeof(gamedir));
		if (!bSourceMod) V_strcat(gamedir, szGameName, sizeof(gamedir));
		V_AppendSlash(gamedir, sizeof(gamedir));
		V_strcat(gamedir, szRelativePath, sizeof(gamedir));
		V_FixSlashes(gamedir);

		if ((!filesystem->IsDirectory(gamedir, NULL)) && (!filesystem->FileExists(gamedir, NULL)))
		{
			char szRelative[MAX_PATH * 2];
			Q_snprintf(szRelative, sizeof(szRelative), "%s", szRelativePath);
			AttemptFindSteamLibraries((char *)szGameName, gamedir, szRelative);
		}
		
		if (filesystem->IsDirectory(gamedir, NULL) && (V_strstr(gamedir, ".vpk") == NULL))
		{
			char szCheckForMaps[MAX_PATH * 2];
			Q_snprintf(szCheckForMaps, sizeof(szCheckForMaps), "%s/maps", gamedir);
			V_FixSlashes(szCheckForMaps);

			if (filesystem->IsDirectory(szCheckForMaps, NULL))
			{
				if (!bMapsDirAdded)
				{
					bMapsDirAdded = true;
					g_pFullMapList.CopyAndAddToTail(gamedir);
				}
				AddMapsAtPath(szCheckForMaps);
			}
		}
	}

	// If we didn't find any maps from this tag, remove the entry.
	if (iArrPos == g_pFullMapList.Count())
	{
		g_pFullMapList.Remove(iArrPos-1);
	}
	else
	{
		g_pFullMapList.CopyAndAddToTail("endofmaplist");
	}
}

#endif

void ParseAvailableMods(void)
{
	// Add at head so it is used first in partial matches
	g_pFullMapList.CopyAndAddToTail("custom");
	g_pFullMapList.CopyAndAddToTail(".");
	AddMapsAtPath("maps");
	g_pFullMapList.CopyAndAddToTail("endofmaplist");

	char szFullDir[512];
	char szSearchCustom[512];
	char *pGameDir = (char *)CommandLine()->ParmValue("-game", "hl2");
	if (pGameDir)
	{
		KeyValues *pMountParse = new KeyValues("gamemounting.txt");
		FileFindHandle_t MounterFindHandle;
		Q_snprintf(szSearchCustom, sizeof(szSearchCustom), "%s/content/*", pGameDir);
		char const *szMounterFileName = NULL;
		if (g_pFullFileSystem) szMounterFileName = g_pFullFileSystem->FindFirst(szSearchCustom, &MounterFindHandle);
		else szMounterFileName = filesystem->FindFirst(szSearchCustom, &MounterFindHandle);
		
		if (szMounterFileName)
		{
			do
			{
				if (szMounterFileName[0] != '.')
				{
					if (((V_strstr(szMounterFileName, ".txt") != NULL) || (V_strstr(szMounterFileName, ".dat") != NULL)) && (V_strstr(szMounterFileName, "_mounting") == NULL))
					{
						Q_snprintf(szFullDir, sizeof(szFullDir), "content/%s", szMounterFileName);
						Q_FixSlashes(szFullDir);
						char szMounterFile[256];
						Q_StripExtension(szMounterFileName, szMounterFile, sizeof(szMounterFile));

						if (g_pFullFileSystem) pMountParse->LoadFromFile(g_pFullFileSystem, szFullDir, "MOD");
						else pMountParse->LoadFromFile(filesystem, szFullDir, "MOD");

						for (KeyValues *pGame = pMountParse->GetFirstTrueSubKey(); pGame; pGame = pGame->GetNextTrueSubKey())
						{
#ifdef CLIENT_DLL
							ParseMod(pGame, szMounterFile);
#else
							ParseModDedicated(pGame, szMounterFile);
#endif
						}

						pMountParse->deleteThis();

						pMountParse = new KeyValues("gamemounting.txt");
					}
				}
				if (g_pFullFileSystem) szMounterFileName = g_pFullFileSystem->FindNext(MounterFindHandle);
				else szMounterFileName = filesystem->FindNext(MounterFindHandle);
			} while (szMounterFileName);
		}
		if (g_pFullFileSystem) g_pFullFileSystem->FindClose(MounterFindHandle);
		else filesystem->FindClose(MounterFindHandle);

		pMountParse->deleteThis();
	}
}

#ifdef CLIENT_DLL
CON_COMMAND(content_metadata, "")
{
	// sv_content_optional
	Msg("%s : %s", engine->GetLevelName(), cl_gamemounted.GetString());
}
#endif

CON_COMMAND(listallmaps_indexes, "")
{
	for (int i = 0; i < g_pFullMapList.Count(); i++)
	{
		Msg("ENTRY %i: '%s'\n", i, g_pFullMapList[i]);
	}

	return;
}

CON_COMMAND(listallmaps, "")
{
	if (g_pFullMapList.Count() < 3)
	{
		Msg("Map list empty!\n");
	}
	char szTag[128] = "";
	Q_snprintf(szTag, sizeof(szTag), "%s", g_pFullMapList[0]);
	for (int i = 2; i < g_pFullMapList.Count(); i++)
	{
		if (!V_strcmp(g_pFullMapList[i], "endofmaplist"))
		{
			if (i + 3 < g_pFullMapList.Count())
			{
				Q_snprintf(szTag, sizeof(szTag), "%s", g_pFullMapList[i + 1]);
				i += 3;
			}
			else
				break;
		}
		Msg("%s %s\n", szTag, g_pFullMapList[i]);
	}

	return;
}

// To-Do: This Should really needs to be re-worked
// basically is the same MountPathLocal but don't check for required
// there should be a proper way to get the sourcemods folder
void MountSourceMod( KeyValues* pGame )
{
	const char* szGameName = pGame->GetName();

	char szPath[ MAX_PATH * 2 ];
	int ccFolder = steamapicontext->SteamApps()->GetAppInstallDir( pGame->GetUint64( "appid" ), szPath, sizeof( szPath ) ); // To-Do: maybe "sourcemod" bool??
	if ( ccFolder > 0 )
	{
		ConColorMsg( Color( 90, 240, 90, 255 ), "Mounting %s (sourcemod)\n", szGameName );

		KeyValues *pPaths = pGame->FindKey( "paths" );
		if ( !pPaths )
			return;

		for ( KeyValues *pPath = pPaths->GetFirstSubKey(); pPath; pPath = pPath->GetNextKey() )
		{
			if ( !FStrEq( pPath->GetName(), "local" ) )
				continue;

			char szTempPath[ MAX_PATH * 2 ];
			Q_strncpy( szTempPath, szPath, ARRAYSIZE( szTempPath ) );

			V_AppendSlash( szTempPath, ARRAYSIZE( szTempPath ) );
			V_strncat( szTempPath, pPath->GetString(), ARRAYSIZE( szTempPath ) );

			g_pFullFileSystem->AddSearchPath( szTempPath, "GAME" );
			ConColorMsg( Color( 90, 240, 90, 255 ), "\tAdding sourcemod path: %s\n", pPath->GetString() );
		}
	}
}

void AddRequiredSearchPaths(bool bMainMenu, const char *pMapName)
{
	SetExtraConditionalFunc( &EvaluateExtraConditionals ); //To-Do: Move this

	KeyValues *pMountFile = new KeyValues( "gamemounting.txt" );
	if (g_pFullFileSystem) pMountFile->LoadFromFile( g_pFullFileSystem, "gamemounting.txt", "MOD" );
	else pMountFile->LoadFromFile(filesystem, "gamemounting.txt", "MOD");

	for( KeyValues *pGame = pMountFile->GetFirstTrueSubKey(); pGame; pGame = pGame->GetNextTrueSubKey() )
	{

#ifndef CLIENT_DLL
		if ( engine->IsDedicatedServer() )
			MountPathDedicated( pGame, PATH_ADD_TO_TAIL, false, pMapName);
		else
		{
			if (bMainMenu)
			{
				if (pGame->GetBool("notmainmenu", false)) continue;
			}
			else
			{
				MountPathLocal( pGame, PATH_ADD_TO_TAIL, false, pMapName );
			}
		}

#else
		if (bMainMenu)
		{
			if (pGame->GetBool("notmainmenu", false)) continue;
		}
		MountPathLocal( pGame, PATH_ADD_TO_TAIL, false, pMapName ); // Client only mounts locally...
#endif

	}

	pMountFile->deleteThis();

	bInitShadow = true;

	KeyValues *pMountModFile = new KeyValues( "SourceMods" );
	if (g_pFullFileSystem) pMountModFile->LoadFromFile( g_pFullFileSystem, "sourcemounting.txt", "MOD" );
	else pMountModFile->LoadFromFile(filesystem, "sourcemounting.txt", "MOD");

	for( KeyValues *pGame = pMountModFile->GetFirstTrueSubKey(); pGame; pGame = pGame->GetNextTrueSubKey() )
		MountPathLocal( pGame );

	pMountModFile->deleteThis();
}

void AddBaseVPK()
{
	char *pGameDir = (char *)CommandLine()->ParmValue("-game", "hl2");
	if (pGameDir)
	{
		char szRelativeDir[MAX_PATH * 2];
		/*
		char szSDKBase[MAX_PATH * 2];
		V_GetCurrentDirectory(szSDKBase, sizeof(szSDKBase));

		if (V_strlen(szSDKBase) > 0)
		{
			Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/hl2/hl2_misc_dir.vpk", szSDKBase);
			V_FixSlashes(szRelativeDir);
			filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_HEAD);
		}
		*/

		Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/sdkbasefallback.vpk", pGameDir);
		V_FixSlashes(szRelativeDir);
		filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_HEAD);
	}
}

void AddRequiredGamePaths()
{
	// tf_coop_lambda
	// SDKBase/hl2/hl2_vpk's
	// SDKBase/hl2
	// tf_coop_lambda/download
	// TF2/tf
	// TF2/vpk's
	// Ep2 + vpk's
	// Ep1 + vpk's
	// LostCoast + vpk's
	// Poral VPK + portal
	// HL1 pak + HL1
	char *pGameDir = (char *)CommandLine()->ParmValue("-game", "hl2");
	if (pGameDir)
	{
		char szRelativeDir[MAX_PATH * 2];
		char szSDKBase[MAX_PATH * 2];
		V_GetCurrentDirectory(szSDKBase, sizeof(szSDKBase));

		if (V_strlen(szSDKBase) > 0)
		{
			Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/hl2/hl2_misc_dir.vpk", szSDKBase);
			V_FixSlashes(szRelativeDir);
			filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_TAIL);
		}

		V_FixSlashes(pGameDir);
		Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/download", pGameDir);
		V_FixSlashes(szRelativeDir);
		filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_HEAD);
		filesystem->AddSearchPath(pGameDir, "GAME", PATH_ADD_TO_HEAD);
		/*
		Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/sdkbasefallback.vpk", pGameDir);
		V_FixSlashes(szRelativeDir);
		filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_HEAD);
		*/

		FileFindHandle_t vpkfh;
		char szFullDir[512];
		char szFullDirSub[1024];
		char szSearchCustom[512];
		char szSearchVPK[512];
		Q_snprintf(szSearchCustom, sizeof(szSearchCustom), "%s/custom/*", pGameDir);
		char const *vpkfn = NULL;
		if (g_pFullFileSystem) vpkfn = g_pFullFileSystem->FindFirst(szSearchCustom, &vpkfh);
		else vpkfn = filesystem->FindFirst(szSearchCustom, &vpkfh);

		if (vpkfn)
		{
			do
			{
				if (vpkfn[0] != '.')
				{
					if (((V_strstr(vpkfn, "pak_") == NULL) && (V_strstr(vpkfn, "_0") == NULL) && (V_strstr(vpkfn, "sound.cache") == NULL)))
					{
						Q_snprintf(szFullDir, sizeof(szFullDir), "%s/custom/%s", pGameDir, vpkfn);
						Q_FixSlashes(szFullDir);
						Q_MakeAbsolutePath(szFullDir, sizeof(szFullDir), szFullDir, NULL);
						
						filesystem->AddSearchPath(szFullDir, "GAME", PATH_ADD_TO_HEAD);

						if (filesystem->IsDirectory(szFullDir, "MOD"))
						{
							Q_snprintf(szSearchVPK, sizeof(szSearchVPK), "%s/*.vpk", szFullDir);
							FileFindHandle_t VPKFindSub;
							char const *VPKSubFileName = filesystem->FindFirst(szSearchVPK, &VPKFindSub);
							if (VPKSubFileName)
							{
								do
								{
									if (VPKSubFileName[0] != '.')
									{
										if (((V_strstr(VPKSubFileName, "pak_") == NULL) && (V_strstr(VPKSubFileName, "_0") == NULL) && (V_strstr(VPKSubFileName, "sound.cache") == NULL)))
										{
											Q_snprintf(szFullDirSub, sizeof(szFullDirSub), "%s/%s", szFullDir, VPKSubFileName);
											Q_FixSlashes(szFullDirSub);
											Q_MakeAbsolutePath(szFullDirSub, sizeof(szFullDirSub), szFullDirSub, NULL);

											filesystem->AddSearchPath(szFullDirSub, "GAME", PATH_ADD_TO_HEAD);
										}
									}
									VPKSubFileName = filesystem->FindNext(VPKFindSub);
								} while (VPKSubFileName);
							}
							filesystem->FindClose(VPKFindSub);
						}
					}
				}
				if (g_pFullFileSystem) vpkfn = g_pFullFileSystem->FindNext(vpkfh);
				else vpkfn = filesystem->FindNext(vpkfh);
			} while (vpkfn);
		}
		if (g_pFullFileSystem) g_pFullFileSystem->FindClose(vpkfh);
		else filesystem->FindClose(vpkfh);

		if (V_strlen(szSDKBase) > 0)
		{
			//Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/hl2/hl2_misc_dir.vpk", szSDKBase);
			//V_FixSlashes(szRelativeDir);
			//filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_TAIL);
			Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/hl2/hl2_sound_misc_dir.vpk", szSDKBase);
			V_FixSlashes(szRelativeDir);
			filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_TAIL);
			Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/hl2/hl2_textures_dir.vpk", szSDKBase);
			V_FixSlashes(szRelativeDir);
			filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_TAIL);
			Q_snprintf(szRelativeDir, sizeof(szRelativeDir), "%s/hl2", szSDKBase);
			V_FixSlashes(szRelativeDir);
			filesystem->AddSearchPath(szRelativeDir, "GAME", PATH_ADD_TO_TAIL);
		}
	}
}

bool AddRequiredMapSearchPaths( const char *pMapName, bool bRemoveOld )
{
	if (!pMapName) return false;

	SetExtraConditionalFunc( &EvaluateExtraConditionals );

	char szMapFilename[MAX_PATH];
	szMapFilename[0] = NULL;

	if ( pMapName && *pMapName )
	{
		// First check direct tag
		Q_snprintf(szMapFilename, sizeof(szMapFilename), "content/%s.txt", pMapName);

		if (!filesystem->FileExists(szMapFilename, "MOD"))
			Q_snprintf( szMapFilename, sizeof( szMapFilename ), "maps/%s_mounting.txt", pMapName );

		// Test for prefix mounting ep1_ ep2_ bm_ etc
		if (!filesystem->FileExists(szMapFilename, "MOD"))
		{
			Q_snprintf(szMapFilename, sizeof(szMapFilename), "content/_%s_mounting.txt", pMapName);

			if (!filesystem->FileExists(szMapFilename, "MOD"))
				Q_snprintf(szMapFilename, sizeof(szMapFilename), "content/%s_mounting.txt", pMapName);

			if (!filesystem->FileExists(szMapFilename, "MOD"))
			{
				CUtlStringList pszSplit;
				V_SplitString(pMapName, "_", pszSplit);
				if (pszSplit.Count() > 1)
				{
					V_snprintf(szMapFilename, sizeof(szMapFilename), "content/%s_mounting.txt", pszSplit[0]);
				}
				else
				{
					V_snprintf(szMapFilename, sizeof(szMapFilename), "content/%s_mounting.txt", pMapName);
				}
				pszSplit.PurgeAndDeleteElements();
			}
		}
	}

	DevMsg(1, "Attempting loader: '%s'\n", szMapFilename);

#ifndef CLIENT_DLL
	if ((engine->IsDedicatedServer()) && (bRemoveOld))
	{
		filesystem->RemoveSearchPaths("GAME");
		AddRequiredSearchPaths(false, pMapName);
		AddRequiredGamePaths();
	}
#endif

	if (filesystem->FileExists(szMapFilename, "MOD"))
	{
		KeyValues *pMountFile = new KeyValues( szMapFilename );
		if (g_pFullFileSystem) pMountFile->LoadFromFile( g_pFullFileSystem, szMapFilename, "MOD" );
		else pMountFile->LoadFromFile(filesystem, szMapFilename, "MOD");

		bool bFirstSectionFound = false;

		for( KeyValues *pGame = pMountFile->GetFirstTrueSubKey(); pGame; pGame = pGame->GetNextTrueSubKey() )
		{

#ifndef CLIENT_DLL
			if ( engine->IsDedicatedServer() )
				MountPathDedicated( pGame, PATH_ADD_TO_HEAD, bRemoveOld);
			else
				MountPathLocal( pGame, PATH_ADD_TO_HEAD, bRemoveOld, pMapName);

#else
			MountPathLocal( pGame, PATH_ADD_TO_HEAD, bRemoveOld, pMapName); // Client only mounts locally...
#endif

			if (!bFirstSectionFound)
			{
				szCurrentGameName = pGame->GetName();
				bFirstSectionFound = true;
			}
		}

		pMountFile->deleteThis();

#ifndef CLIENT_DLL
		AddBaseVPK();
#endif

		return true;
	}

#ifndef CLIENT_DLL
	AddBaseVPK();
#endif

	return false;
}

void cbCLGameMount(IConVar *var, const char *pOldValue, float flOldValue)
{
	ConVar *pSetConVar = g_pCVar->FindVar("cl_gamemounted");
	if (pSetConVar)
	{
#ifdef CLIENT_DLL
		if ((pSetConVar->GetString() != NULL) && (strlen(pSetConVar->GetString()) > 0) && (pOldValue != pSetConVar->GetString()))
		{
			MDLCACHE_CRITICAL_SECTION();
			filesystem->RemoveSearchPaths("GAME");
			AddRequiredSearchPaths(false, pSetConVar->GetString());
			AddRequiredGamePaths();
			//filesystem->PrintSearchPaths();

			// Don't need to remove old path, already removed all through RemoveSearchPaths
			/*
			if (pOldValue)
			{
				if (strlen(pOldValue) > 0)
				{
					// Remove old path
					// Also set mount change occurred
					if (AddRequiredMapSearchPaths(pSetConVar->GetString(), true))
						bMapMountChange = true;
				}
			}
			*/

			if (AddRequiredMapSearchPaths(pSetConVar->GetString()))
			{
				bMapMountChange = true;
				bCustomMounted = false;
			}
			else if ((!V_strcmp(pSetConVar->GetString(), "d1")) || (!V_strcmp(pSetConVar->GetString(), "d2")) || (!V_strcmp(pSetConVar->GetString(), "d3")) || (!V_strcmp(pSetConVar->GetString(), "ep2")) || (!V_strcmp(pSetConVar->GetString(), "ep1")))
			{
				if (!bCustomMounted)
				{
					bMapMountChange = true;
					bCustomMounted = true;
				}
			}

			AddBaseVPK();
		}
#endif
	}
	return;
}

void RegisterMounterCallbacks()
{
	cl_gamemounted.InstallChangeCallback(cbCLGameMount);
}

void SetDisconnected(bool bStatus)
{
	bDisconnected = bStatus;
}

bool IsDisconnected()
{
	return bDisconnected;
}

char *GetActiveGameName()
{
	if (szCurrentGameName)
	{
		return (char*)szCurrentGameName;
	}

	return NULL;
}

#ifdef CLIENT_DLL
CON_COMMAND(disconnect, "")
{
	SetDisconnected(true);
	bMapMountChange = false;
	ConCommandBase *pDCBase = (ConCommand *)dynamic_cast<const ConCommand *>(g_pCVar->FindCommand("disconnect"));
	cvar->UnregisterConCommand(pDCBase);
	ConCommand *pCommand = (ConCommand *)dynamic_cast<const ConCommand *>(g_pCVar->FindCommand("disconnect"));
	if (pCommand)
	{
		char const *argv[] = { "disconnect" };
		CCommand cmd(1, argv);
		pCommand->Dispatch(cmd);
	}
	cvar->RegisterConCommand(pDCBase);
}
/*
CON_COMMAND(connect, "")
{
	//SetDisconnected(true);
	bMapMountChange = false;
	ConCommandBase *pDCBase = (ConCommand *)dynamic_cast<const ConCommand *>(g_pCVar->FindCommand("connect"));
	cvar->UnregisterConCommand(pDCBase);
	ConCommand *pCommand = (ConCommand *)dynamic_cast<const ConCommand *>(g_pCVar->FindCommand("connect"));
	if (pCommand)
	{
		if (args.ArgC() == 5)
		{
			if (strcmp(args.Arg(1), "localhost") != 0)
			{
				//bMapMountChange = true;
			}
			char szBuilt[512];
			Q_snprintf(szBuilt, sizeof(szBuilt), "%s%s%s", args.Arg(1), args.Arg(2), args.Arg(3));
			char const *argv[] = { "connect", szBuilt, args.Arg(4)};
			CCommand cmd(3, argv);
			pCommand->Dispatch(cmd);
		}
		else
		{
			char const *argv[] = { "connect", args.ArgS() };
			CCommand cmd(2, argv);
			pCommand->Dispatch(cmd);
		}
	}
	cvar->RegisterConCommand(pDCBase);
}
*/
#endif

CON_COMMAND(flush_anim, "")
{
	mdlcache->Flush(MDLCACHE_FLUSH_ANIMBLOCK);
}

// New changelevel and map autocompletion and mounting system
char *GetPathFromTag(const char *pszTag)
{
	if ((!pszTag) || (g_pFullMapList.Count() < 1))
	{
		Warning("MountList empty!\n");
		return NULL;
	}

	char szLowerInput[MAX_PATH * 2];
	Q_snprintf(szLowerInput, sizeof(szLowerInput), "%s", pszTag);
	V_strlower(szLowerInput);
	char szLowerArr[MAX_PATH * 2];
	for (int i = 0; i < g_pFullMapList.Count(); i++)
	{
		Q_snprintf(szLowerArr, sizeof(szLowerArr), "%s", g_pFullMapList[i]);
		V_strlower(szLowerArr);
		if (!V_strcmp(szLowerArr, szLowerInput))
		{
			if (i > 0)
			{
				if (!V_strcmp(g_pFullMapList[i - 1], "endofmaplist"))
				{
					return g_pFullMapList[i + 1];
				}
			}
			else
			{
				if ((V_strstr(g_pFullMapList[i + 1], "/") != NULL) || (V_strstr(g_pFullMapList[i + 1], "\\") != NULL))
					return g_pFullMapList[i + 1];
			}
		}
	}

	return NULL;
}

// Essentially reverse of GetPathFromTag
char *GetTagFromPath(const char *pszPath)
{
	if ((!pszPath) || (g_pFullMapList.Count() < 1))
		return NULL;

	for (int i = 0; i < g_pFullMapList.Count(); i++)
	{
		if (!V_strcmp(g_pFullMapList[i], pszPath))
		{
			if (i > 0)
			{
				return g_pFullMapList[i - 1];
			}
		}
	}

	return NULL;
}

char *GetMountPointFromMap(const char *pszMap)
{
	if ((!pszMap) || (g_pFullMapList.Count() < 1))
		return NULL;

	char *pszTag = NULL;
	for (int i = 0; i < g_pFullMapList.Count(); i++)
	{
		if ((i == 0) || (!V_strcmp(g_pFullMapList[i - 1], "endofmaplist")))
		{
			pszTag = g_pFullMapList[i];
		}

		if (!V_strcmp(g_pFullMapList[i], pszMap))
		{
			return pszTag;
		}
	}

	return NULL;
}

// Returns int position in string
static int ChkContains(const char *szFullStr, const char *szSearch)
{
	if ((szFullStr != NULL) && (szSearch != NULL))
	{
		if (V_strstr(szFullStr, szSearch) != NULL)
		{
			if (strcmp(szSearch, szFullStr) == 0) return 0;
			CUtlStringList szTagList;
			V_SplitString(szFullStr, szSearch, szTagList);
			if (szTagList.Count() < 1)
			{
				szTagList.PurgeAndDeleteElements();
				return -1;
			}
			else
			{
				if ((strcmp(szTagList[0], szFullStr) == 0) || (((strlen(szTagList[0]) + strlen(szSearch)) == strlen(szFullStr)) && (szTagList.Count() == 1))) return 0;
				return strlen(szTagList[0]) + 1;
			}
			szTagList.PurgeAndDeleteElements();
		}
		else return -1;
	}
	return -1;
}

void SetMounterCV(const char *szTagString, const char *szMap)
{
	char szCurValue[256];
	Q_snprintf(szCurValue, sizeof(szCurValue), "%s", cl_gamemounted.GetString());

	char szMounterExists[MAX_PATH * 2];
	Q_snprintf(szMounterExists, sizeof(szMounterExists), "content/%s.txt", szTagString);
	Q_FixSlashes(szMounterExists);
	if (filesystem->FileExists(szMounterExists, "MOD"))
	{
		cl_gamemounted.SetValue(szTagString);

#ifdef GAME_DLL
		if (engine->IsDedicatedServer())
		{
			// Only dedicated server should mount here
			// Check if successfully mounted
			if (V_strcmp(szCurValue, szTagString) != 0)
			{
				filesystem->RemoveSearchPaths("GAME");
				AddRequiredSearchPaths(false, szMap);
				AddRequiredGamePaths();
			}

			if (AddRequiredMapSearchPaths(szTagString))
			{
				bMapMountChange = true;
			}

			if (V_strcmp(szCurValue, szTagString) != 0)
				AddBaseVPK();
		}
#endif
	}
	else
	{
		char szMapPath[512];
		char szMapLocal[128];
		Q_snprintf(szMapLocal, sizeof(szMapLocal), "%s", szMap);
		Q_snprintf(szMapPath, sizeof(szMapPath), "content/%s_mounting.txt", szMapLocal);

		if (!filesystem->FileExists(szMapPath, NULL))
		{
			if (V_strstr(szMapLocal, "_") == NULL) V_strcat(szMapLocal, "_", COPY_ALL_CHARACTERS);
			CUtlStringList pszSplit;
			V_SplitString(szMapLocal, "_", pszSplit);
			if (pszSplit.Count() > 0)
			{
				Q_snprintf(szMapLocal, sizeof(szMapLocal), "%s", pszSplit[0]);
				Q_snprintf(szMapPath, sizeof(szMapPath), "content/%s_mounting.txt", szMapLocal);
				if (!filesystem->FileExists(szMapPath, NULL))
				{
					bool bSkip3Char = false;

					// Now test second param (if exists) for things like 1_sm_ 01_spymap_
					if ((pszSplit.Count() > 1) && (V_strstr(szMapLocal, "d1_") == NULL) && (V_strstr(szMapLocal, "d2_") == NULL) && (V_strstr(szMapLocal, "d3_") == NULL))
					{
						Q_snprintf(szMapPath, sizeof(szMapPath), "content/_%s_mounting.txt", pszSplit[1]);
						if (filesystem->FileExists(szMapPath, NULL))
						{
							Q_snprintf(szMapLocal, sizeof(szMapLocal), "_%s", pszSplit[1]);
							bSkip3Char = true;
						}
					}

					if (!bSkip3Char)
					{
						// Use first 3 char as detector
						char szMapPrefix[4];
						Q_snprintf(szMapPrefix, sizeof(szMapPrefix), "%s", szMapLocal);
						if (V_strstr(szMapPrefix, "_") != NULL)
						{
							pszSplit.RemoveAll();
							V_SplitString(szMapPrefix, "_", pszSplit);
							Q_snprintf(szMapLocal, sizeof(szMapLocal), "%s", pszSplit[0]);
						}
						else
						{
							//pszSplit[0] = szMapPrefix;
							Q_snprintf(szMapLocal, sizeof(szMapLocal), "%s", szMapPrefix);
						}
					}
				}
			}

			DevMsg(1, "AttemptToMount %i '%s'\n", pszSplit.Count(), szMapLocal);
			//if ((pszSplit.Count() >= 1) && (pszSplit[0]))

			// Crash on occasion if not removed first, then purged (freed).
			pszSplit.RemoveAll();
			pszSplit.PurgeAndDeleteElements();
		}

		if (V_strlen(szMapLocal) > 0)
		{
			cl_gamemounted.SetValue(szMapLocal);

#ifdef GAME_DLL
			if (engine->IsDedicatedServer())
			{
				// Only dedicated server should mount here
				// Check if successfully mounted
				if (V_strcmp(szCurValue, szMapLocal) != 0)
				{
					filesystem->RemoveSearchPaths("GAME");
					AddRequiredSearchPaths(false, szMap);
					AddRequiredGamePaths();
				}

				if (AddRequiredMapSearchPaths(szMapLocal))
				{
					bMapMountChange = true;
				}

				if (V_strcmp(szCurValue, szMapLocal) != 0)
					AddBaseVPK();
			}
#endif
		}
	}
}

class CChangeLevelRecon : public ICommandCallback, public ICommandCompletionCallback
{
public:
	virtual void CommandCallback(const CCommand &command)
	{
		//Msg("Changelevel or map %s %s %i\n", command[0], command[1], command.ArgC());
		if (command.ArgC() < 2)
		{
			Msg("Format: %s <contenttag> <mapname>\n", command[0]);
			return;
		}
#ifdef CLIENT_DLL
		// Might not work in some situations...
		if (V_strncmp(command[0], "changelevel", 11) == 0)
		{
			if (!engine->IsInGame())
			{
				Msg("You must start a map first...\n");
				return;
			}
		}
#endif

		char szSubString[MAX_PATH] = "";
		char szMap[MAX_PATH] = "";
		char szTagString[32] = "";
		char szTransitionPoint[256] = "";
		if (command.ArgC() == 2)
		{
			Q_snprintf(szMap, sizeof(szMap), "%s", command[1]);
			Q_snprintf(szSubString, sizeof(szSubString), "maps/%s.bsp", command[1]);
		}
		else if (command.ArgC() >= 3)
		{
			Q_snprintf(szMap, sizeof(szMap), "%s", command[2]);
			Q_snprintf(szSubString, sizeof(szSubString), "maps/%s.bsp", command[2]);
			Q_snprintf(szTagString, sizeof(szTagString), "%s", command[1]);
			Q_RemoveAllEvilCharacters(szTagString);
			if (command.ArgC() >= 4)
			{
				Q_snprintf(szTransitionPoint, sizeof(szTransitionPoint), "%s", command[3]);
				Q_RemoveAllEvilCharacters(szTransitionPoint);
			}
		}
		Q_RemoveAllEvilCharacters(szMap);
		Q_RemoveAllEvilCharacters(szSubString);

		DevMsg(1, "Check exist %s\n", szSubString);
		if ((szSubString != NULL) && (strlen(szSubString) > 0))
		{
			if (g_pFullFileSystem->FileExists(szSubString, NULL))
			{
				if ((szTagString != NULL) && (strlen(szTagString) > 0) && (V_strcmp(szTagString, "custom") != 0))
				{
					DevMsg(1, "Tag specified: '%s'\n", szTagString);
					const char *pszPath = GetPathFromTag(szTagString);
					if (pszPath != NULL)
					{
						const char *szMainMountTag = cl_gamemounted.GetString();
						if (szMainMountTag)
						{
							if (V_strncmp(szTagString, szMainMountTag, strlen(szMainMountTag)) == 0)
							{
								DevMsg(1, "Main mount is the same as specified tag, no remount required.\n");
								if (!filesystem->FileExists(szSubString, "GAME"))
								{
									Warning("map load failed: \"%s\" not found or invalid\n", szMap);
									return;
								}
								else
								{
									SetMounterCV(szTagString, szMap);
									UnRegCMD((char *)command[0], szMap, szTagString, szTransitionPoint);
									return;
								}
							}
						}
#ifdef CLIENT_DLL
						filesystem->AddSearchPath(pszPath, "TEMP", PATH_ADD_TO_HEAD);
						if ((!filesystem->FileExists(szSubString, "TEMP")) && (!filesystem->FileExists(szSubString, "GAME")))
						{
							filesystem->RemoveSearchPaths("TEMP");
							Warning("map load failed: \"%s\" not found or invalid\n", szMap);
							return;
						}
						else
						{
							filesystem->RemoveSearchPaths("TEMP");
							Q_snprintf(szTagString, sizeof(szTagString), "%s", GetTagFromPath(pszPath));
							DevMsg(1, "Map exists in current context tag specified: '%s'\n", szTagString);

							SetMounterCV(szTagString, szMap);
						}
#else
						filesystem->AddSearchPath(pszPath, "GAME", PATH_ADD_TO_HEAD);
						if (!engine->IsMapValid(UTIL_VarArgs("maps/%s.bsp", szMap)))
						{
							filesystem->RemoveSearchPath(pszPath, "GAME");
							Warning("map load failed: \"%s\" not found or invalid\n", szMap);
							return;
						}
						else
						{
							SetMounterCV(szTagString, szMap);
						}
#endif
					}
					else
					{
						if ((szTagString != NULL) && (strlen(szTagString) > 0))
						{
							if ((filesystem->FileExists(szSubString, NULL)) && (cl_gamemounted.GetString()) && (!V_strcmp(cl_gamemounted.GetString(), szTagString)))
							{
								UnRegCMD((char *)command[0], szMap, szTagString, szTransitionPoint);
							}
							else
								Warning("map load failed: \"%s\" not found in tag \"%s\"\n", szMap, szTagString);
						}
						else
							Warning("map load failed: \"%s\" not found or invalid\n", szMap);
						return;
					}
				}
				else
				{
					// No tag specified, and map exists in current context, do not perform any mount changes.
					// However, the "custom" tag should still perform an unmount if the situation is needed
					if ((filesystem->FileExists(szSubString, NULL)) && (cl_gamemounted.GetString()) && (V_strlen(cl_gamemounted.GetString()) > 0))
					{
						if ((V_strlen(szTagString) > 0) && (V_strcmp(szTagString, "custom") != 0) && (cl_gamemounted.GetString()))
						{
							if (!GetPathFromTag(cl_gamemounted.GetString()))
							{
								DevWarning(1, "Could not find a valid path from active tag: %s\n", cl_gamemounted.GetString());
							}
						}
						if (filesystem->FileExists(szSubString, "MOD"))
						{
							SetMounterCV("custom", szMap);
						}
						DevMsg(1, "No tag specified, and map exists in current context, changing level normally\n");
						UnRegCMD((char *)command[0], szMap, "", szTransitionPoint);
						return;
					}

					//NEED TO REMOUNT
					char *pszTag = GetMountPointFromMap(szMap);
					if ((szTagString != NULL) && (strlen(szTagString) > 0))
					{
						pszTag = (char *)szTagString;
					}

					if (pszTag != NULL)
					{
						DevMsg(1, "ChangeLevel map exists in current context, tag with map: '%s' Cur: '%s'\n", pszTag, cl_gamemounted.GetString());
						if (cl_gamemounted.GetString())
						{
							if (V_strstr(pszTag, cl_gamemounted.GetString()) == NULL)
							{
								if (V_strstr(pszTag, ";") != NULL)
								{
									CUtlStringList szTags;
									CUtlStringList szSplitTag;
									int iStartPos = 1;
									V_SplitString(pszTag, ";", szTags);
									char szTagList[256] = "";
									char szCat[128];

									if (V_strstr(szTags[0], " ") == NULL)
									{
										Q_snprintf(szTagList, sizeof(szTagList), "\n - \"%s\"", szTags[0]);
									}
									else
									{
										pszTag = "";
										iStartPos = 0;
									}
									for (int i = iStartPos; i < szTags.Count(); i++)
									{
										if (V_strstr(szTags[i], " ") != NULL)
										{
											V_SplitString(szTags[i], " ", szSplitTag);
											Q_snprintf(szCat, sizeof(szCat), "\n - \"%s\"", szSplitTag[0]);
											V_strcat(szTagList, szCat, COPY_ALL_CHARACTERS);
											szSplitTag.PurgeAndDeleteElements();
										}
										else
										{
											Q_snprintf(szCat, sizeof(szCat), "\n - \"%s\"", szTags[i]);
											V_strcat(szTagList, szCat, COPY_ALL_CHARACTERS);
										}
									}
									Warning("map load failed, found multiple tags for map: \"%s\"%s\n", szMap, szTagList);
									szTags.PurgeAndDeleteElements();
									szSplitTag.PurgeAndDeleteElements();
									return;
								}
								else
								{
#ifdef CLIENT_DLL
									DevMsg(1, "[CL]Map change exists in tag %s\n", pszTag);
#endif
									//char *pszPath = GetPathFromTag(pszTag);
									//if (pszPath != NULL)
									SetMounterCV(pszTag, szMap);
#ifndef CLIENT_DLL
									filesystem->AddSearchPath(szSubString, "GAME", PATH_ADD_TO_HEAD);
#endif
								}
							}
							else
							{
								SetMounterCV(pszTag, szMap);
							}
						}
					}
				}
			}
			else
			{
				char *pszTag = GetMountPointFromMap(szMap);
				
				DevMsg(1, "GetMountPointFromMap '%s' '%s' '%s'\n", szMap, pszTag, szTagString);
				if ((szTagString != NULL) && (strlen(szTagString) > 0))
				{
					pszTag = szTagString;
				}
				if ((pszTag != NULL) && (V_strstr(pszTag, ";") != NULL))
				{
					CUtlStringList szTags;
					CUtlStringList szSplitTag;
					int iStartPos = 1;
					V_SplitString(pszTag, ";", szTags);
					char szTagList[256] = "";
					char szCat[128];

					if (V_strstr(szTags[0], " ") == NULL)
					{
						Q_snprintf(szTagList, sizeof(szTagList), "\n - \"%s\"", szTags[0]);
					}
					else
					{
						pszTag = "";
						iStartPos = 0;
					}
					for (int i = iStartPos; i < szTags.Count(); i++)
					{
						if (V_strstr(szTags[i], " ") != NULL)
						{
							V_SplitString(szTags[i], " ", szSplitTag);
							Q_snprintf(szCat, sizeof(szCat), "\n - \"%s\"", szSplitTag[0]);
							V_strcat(szTagList, szCat, COPY_ALL_CHARACTERS);
							szSplitTag.PurgeAndDeleteElements();
						}
						else
						{
							Q_snprintf(szCat, sizeof(szCat), "\n - \"%s\"", szTags[i]);
							V_strcat(szTagList, szCat, COPY_ALL_CHARACTERS);
						}
					}
					Warning("map load failed, found multiple tags for map: \"%s\"%s\n", szMap, szTagList);
					szTags.PurgeAndDeleteElements();
					szSplitTag.PurgeAndDeleteElements();

					return;
				}
				if (!pszTag)
				{
					Warning("Failed to get tag for map: '%s'\n", szMap);
					return;
				}
				char *pszPath = GetPathFromTag(pszTag);
				DevMsg(1, "GetPath '%s' '%s'\n", pszTag, pszPath);
				if (pszPath != NULL)
				{
					char szFullMapPath[MAX_PATH * 2];
					Q_snprintf(szFullMapPath, sizeof(szFullMapPath), "%s/maps/%s.bsp", pszPath, szMap);
					V_FixSlashes(szFullMapPath, '/');
#ifdef CLIENT_DLL
					//filesystem->AddSearchPath(pszPath, pszTag, PATH_ADD_TO_HEAD);
					if ((!filesystem->FileExists(szFullMapPath, pszTag)) && (!filesystem->FileExists(szSubString, "GAME")))
					{
						//filesystem->RemoveSearchPaths(pszTag);
						Warning("map load failed: \"%s\" not found or invalid\n", szMap);
						return;
					}
					else
					{
						DevMsg(1, "Map exists in current context '%s'\n", pszTag);
						// Might not need to call anything here
						SetMounterCV(pszTag, szMap);

						//filesystem->AddSearchPath(szSubString, "GAME", PATH_ADD_TO_HEAD);
					}
#else
					DevMsg(1, "[SV]Mount temp fs\n");
					//filesystem->AddSearchPath(pszPath, "GAME", PATH_ADD_TO_HEAD);
					if (!engine->IsMapValid(UTIL_VarArgs("maps/%s.bsp", szMap)))
					{
						filesystem->RemoveSearchPath(pszPath, "GAME");
						Warning("map load failed: \"%s\" not found or invalid\n", szMap);
						return;
					}
					else
					{
						filesystem->RemoveSearchPath(pszPath, "GAME");
						SetMounterCV(pszTag, szMap);
					}
#endif
				}
				else
				{
					if ((pszTag != NULL) && (strlen(pszTag) > 0)) Warning("map load failed: \"%s\" not found in tag \"%s\"\n", szMap, pszTag);
					else Warning("map load failed: \"%s\" not found or invalid\n", szMap);
					return;
				}
			}
		}

		UnRegCMD((char *)command[0], szMap, szTagString, szTransitionPoint);
		return;
	}

	virtual int CommandCompletionCallback(const char *partial, CUtlVector< CUtlString > &commands)
	{
#ifdef CLIENT_DLL
		char szContains[64];
		char szCMD[16];
		char *substring = (char *)partial;
		if (V_strstr(substring, " ") != NULL)
		{
			CUtlStringList szTagList;
			V_SplitString(substring, " ", szTagList);
			Q_snprintf(szCMD, sizeof(szCMD), "%s", szTagList[0]);
			if (szTagList.Count() > 1)
			{
				Q_snprintf(szContains, sizeof(szContains), "%s", szTagList[1]);
				if (szTagList[2] != NULL)
				{
					for (int i = 2; i < szTagList.Count(); i++)
					{
						Q_snprintf(szContains, sizeof(szContains), "%s %s", szContains, szTagList[i]);
					}
				}
			}
			substring = (char *)partial + strlen(szTagList[0]) + 1;
			szTagList.PurgeAndDeleteElements();
		}
		else Q_snprintf(szCMD, sizeof(szCMD), "%s", partial);

		if (g_pFullMapList.Count() < 3) return 0;
		
		char szTmpArgs[256];
		char szCurTag[256];
		int iAddedVals = 0;
		Q_snprintf(szCurTag, sizeof(szCurTag), "%s", g_pFullMapList[0]);
		V_strlower(szCurTag);
		V_strlower(szContains);
		for (int i = 2; i < g_pFullMapList.Count(); i++)
		{
			if (V_strstr(g_pFullMapList[i], "endofmaplist") != NULL)
			{
				if (i + 2 < g_pFullMapList.Count())
				{
					i++;
					Q_snprintf(szCurTag, sizeof(szCurTag), "%s", g_pFullMapList[i]);
					V_strlower(szCurTag);
					i++;
				}
				else break;
			}
			else if ((V_strstr(g_pFullMapList[i], "/") == NULL) && (V_strstr(g_pFullMapList[i], "\\") == NULL) && (V_strstr(g_pFullMapList[i], ".") == NULL))
			{
				if (szContains != NULL)// && (strlen(szContains) > 0))
				{
					if ((ChkContains(g_pFullMapList[i], szContains) == 0) || ((strlen(szContains) > 0) && (strncmp(szCurTag, szContains, strlen(szContains)) == 0)) || (ChkContains(VarArgs("%s %s", szCurTag, g_pFullMapList[i]), szContains) == 0) || (ChkContains(szCurTag, szContains) == 0))
					{
						iAddedVals++;
						Q_snprintf(szTmpArgs, sizeof(szTmpArgs), "%s %s %s", szCMD, szCurTag, g_pFullMapList[i]);
						CUtlString command;
						command = szTmpArgs;
						commands.AddToTail(command);
					}
				}
			}
		}
		return iAddedVals;
#endif
		return 0;
	}
};

static CChangeLevelRecon g_ChangeLevelSetup;

// Only hidden so the command completion doesn't show 3 map commands.
ConCommand changelevel("changelevel", &g_ChangeLevelSetup, "Usage:\n   changelevel <contenttag> <mapname>\n", FCVAR_HIDDEN, &g_ChangeLevelSetup);
ConCommand maplevel("map", &g_ChangeLevelSetup, "Usage:\n   map <contenttag> <mapname>\n", FCVAR_HIDDEN, &g_ChangeLevelSetup);
ConCommandBase *pMapLVBase;

bool bRegisterred[2] = { false, false };

void UnRegCMD(char szCMD[24], char szMapName[MAX_PATH], char szTagString[64], char szTransitionPoint[256])
{
	//ConVar_Unregister();
	//ConCommand CLevelSetup("map", &g_ChangeLevelSetup, "Usage:\n   map <contenttag> <mapname>\n", FCVAR_NONE);
	//ConCommandBase *CLevelCmdBase;
	//cvar->RegisterConCommand(CLevelCmdBase);
	if (Q_stristr(szCMD, "changelevel")) bRegisterred[1] = false;
	else bRegisterred[0] = false;
	pMapLVBase = (ConCommand *)dynamic_cast<const ConCommand *>(g_pCVar->FindCommand(szCMD));
	cvar->UnregisterConCommand(pMapLVBase);
#ifdef CLIENT_DLL
	DevMsg(2, "[CL]UnRegCMD \"%s\"\n", szCMD);
#else
	DevMsg(2, "[SV]UnRegCMD \"%s\"\n", szCMD);
	if (engine->IsDedicatedServer())
	{
		SetMounterCV(szTagString, szMapName);
	}
#endif
	ConCommand *pCommand = (ConCommand *)dynamic_cast<const ConCommand *>(g_pCVar->FindCommand(szCMD));
	if (pCommand)
	{
#ifdef CLIENT_DLL
		if ((szTransitionPoint != NULL) && (strlen(szTransitionPoint) > 0))
		{
			char const *argv[] = { szCMD, szTagString, szMapName, szTransitionPoint };

			CCommand cmd(4, argv);
			pCommand->Dispatch(cmd);
		}
		else if ((szTagString != NULL) && (strlen(szTagString) > 0))
		{
			char const *argv[] = { szCMD, szTagString, szMapName };

			CCommand cmd(3, argv);
			pCommand->Dispatch(cmd);
		}
		else
		{
			char const *argv[] = { szCMD, szMapName };

			CCommand cmd(2, argv);
			pCommand->Dispatch(cmd);
		}
#else
		if (szTransitionPoint[0])
		{
			gpGlobals->eLoadType = MapLoad_Transition;
			char const *argv[] = { szCMD, szMapName, szTransitionPoint };

			CCommand cmd(3, argv);
			pCommand->Dispatch(cmd);
		}
		else
		{
			if (V_strstr(szCMD, "map") != NULL)
			{
				gpGlobals->eLoadType = MapLoad_NewGame;
			}

			char const *argv[] = { szCMD, szMapName };

			CCommand cmd(2, argv);
			pCommand->Dispatch(cmd);
		}
#endif
	}

	ReRegChangeCMD();
}

void ReRegChangeCMD()
{
	if (pMapLVBase)
	{
		if (Q_stristr(pMapLVBase->GetName(), "changelevel"))
		{
			if (bRegisterred[1]) return;
		}
		else
		{
			if (bRegisterred[0]) return;
		}
#ifdef CLIENT_DLL
		DevMsg(2, "[CL]ReRegChangeCMD \"%s\"\n", pMapLVBase->GetName());
#else
		DevMsg(2, "[SV]ReRegChangeCMD \"%s\"\n", pMapLVBase->GetName());
#endif
		cvar->RegisterConCommand(pMapLVBase);
	}
	return;
}