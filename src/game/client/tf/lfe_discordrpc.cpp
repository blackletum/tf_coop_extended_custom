//=============================================================================
//
// Purpose: Discord Presence support.
//
//=============================================================================

#include "cbase.h"
#include "lfe_discordrpc.h"
#include "c_team_objectiveresource.h"
#include "tf_gamerules.h"
#include "c_tf_team.h"
#include "c_tf_playerresource.h"
#include <inetchannelinfo.h>
#include "discord/discord_rpc.h"
#include "discord/discord_register.h"
#include "tf_gamerules.h"
#include <ctime>
#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_richpresence_printmsg( "cl_richpresence_printmsg", "0", FCVAR_ARCHIVE, "" );

#define DISCORD_APP_ID	"460822696951939073"

// update once every 10 seconds. discord has an internal rate limiter of 15 seconds as well
#define DISCORD_UPDATE_RATE 10.0f

extern const char *GetMapDisplayName( const char *mapName );

const char *g_aClassImage[] =
{
	"",
	"lfe_class_scout",
	"lfe_class_sniper",
	"lfe_class_soldier",
	"lfe_class_demo",
	"lfe_class_medic",
	"lfe_class_heavy",
	"lfe_class_pyro",
	"lfe_class_spy",
	"lfe_class_engineer",
	"",
	"",
	"",
	""
};

CTFDiscordRPC g_discordrpc;

CTFDiscordRPC::CTFDiscordRPC()
{
	Q_memset(m_szLatchedMapname, 0, MAX_MAP_NAME);
	m_bInitializeRequested = false;
}

CTFDiscordRPC::~CTFDiscordRPC()
{
}

void CTFDiscordRPC::Shutdown()
{
	Discord_Shutdown();
}

void CTFDiscordRPC::Init()
{
	InitializeDiscord();
	m_bInitializeRequested = true;

	// make sure to call this after game system initialized
	ListenForGameEvent( "server_spawn" );
}

void CTFDiscordRPC::RunFrame()
{
	if (m_bErrored)
		return;

	// NOTE: we want to run this even if they have use_discord off, so we can clear
	// any previous state that may have already been sent
	UpdateRichPresence();
	Discord_RunCallbacks();

	// always run this, otherwise we will chicken & egg waiting for ready
	//if (Discord_RunCallbacks)
	//	Discord_RunCallbacks();
}

void CTFDiscordRPC::OnReady( const DiscordUser* user )
{
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Ready!\n" );
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] User %s#%s - %s\n", user->username, user->discriminator, user->userId );
	
	g_discordrpc.Reset();
}

void CTFDiscordRPC::OnDiscordError( int errorCode, const char *szMessage )
{
	g_discordrpc.m_bErrored = true;
	char buff[1024];
	Q_snprintf(buff, 1024, "[Rich Presence] Init failed. code %d - error: %s\n", errorCode, szMessage);
	Warning(buff);
}


void CTFDiscordRPC::OnJoinGame( const char *joinSecret )
{
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Join Game: %s\n", joinSecret );

	char szCommand[128];
	Q_snprintf( szCommand, sizeof( szCommand ), "connect %s\n", joinSecret );
	engine->ExecuteClientCmd( szCommand );
}

void CTFDiscordRPC::OnSpectateGame( const char *spectateSecret )
{
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Spectate Game: %s\n", spectateSecret );
}

void CTFDiscordRPC::OnJoinRequest( const DiscordUser *joinRequest )
{
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Join Request: %s#%s\n", joinRequest->username, joinRequest->discriminator );
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Join Request Accepted\n" );
	Discord_Respond( joinRequest->userId, DISCORD_REPLY_YES );
}

void CTFDiscordRPC::SetLogo( void )
{
	const char *pszGameType = "";
	const char *pszImageLarge = "lfe_large";

	// april fools
	/*pszGameType = "WTF!";
	pszImageLarge = "lfe_cfs_large";*/

	//string for setting the picture of the class
	//you should name the small picture affter the class itself ex: Scout.jpg, Soldier.jpg, Pyro.jpg ...
	//you get it
	//-Nbc66
	const char *pszImageSmall = "";
	const char *pszImageText = "";
	C_TFPlayer *pTFPlayer = C_TFPlayer::GetLocalTFPlayer();

	if ( TFGameRules() && engine->IsConnected() )
	{
		if ( TFGameRules()->GetGameType() == TF_GAMETYPE_UNDEFINED )
		{
			pszGameType = "";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_CTF )
		{
			pszGameType = "Capture The Flag";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_CP )
		{
			if (TFGameRules()->IsInKothMode())
				pszGameType = "King of the Hill";
			else
				pszGameType = "Control Point";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_ARENA )
		{
			pszGameType = "Arena";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_ESCORT )
		{
			pszGameType = "Payload";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_COOP )
		{
			if ( TFGameRules()->IsHolidayActive( kHoliday_AprilFools ) )
				pszGameType = "Yes";
			else
				pszGameType = "Co-Op";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_BLUCOOP )
		{
			if ( TFGameRules()->IsHolidayActive( kHoliday_AprilFools ) )
				pszGameType = "Yesn't";
			else
				pszGameType = "BLU Co-Op";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_VS )
		{
			pszGameType = "Versus";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_HORDE )
		{
			pszGameType = "Horde";
		}
		else if ( TFGameRules()->GetGameType() == TF_GAMETYPE_FREE )
		{
			if ( TFGameRules()->IsHolidayActive( kHoliday_AprilFools ) )
				pszGameType = "Garryn't Mod";
			else
				pszGameType = "Free";
		}

		const char *map = engine->GetLevelName();
		char mapfile[32];
		V_FileBase(map, mapfile, sizeof(mapfile));
		V_StripExtension(mapfile, mapfile, sizeof(mapfile));
		map = mapfile;

		if (V_strnicmp(map, "d1_trainstation_", 16) == 0)
		{
			pszImageLarge = "lfe_hl2_background01";
		}
		else if (V_strnicmp(map, "d1_canals_", 10) == 0)
		{
			pszImageLarge = "lfe_hl2_background02";
		}
		else if (V_strnicmp(map, "d1_eli_", 7) == 0)
		{
			pszImageLarge = "lfe_hl2_background03";
		}
		else if (V_strnicmp(map, "d1_town_", 8) == 0)
		{
			pszImageLarge = "lfe_hl2_background03";
		}
		else if (V_strnicmp(map, "d2_coast_", 9) == 0)
		{
			pszImageLarge = "lfe_hl2_background04";
		}
		else if (V_strnicmp(map, "d2_prison_", 10) == 0)
		{
			pszImageLarge = "lfe_hl2_background05";
		}
		else if (V_strnicmp(map, "d3_c17_", 7) == 0)
		{
			pszImageLarge = "lfe_hl2_background06";
		}
		else if (V_strnicmp(map, "d3_citadel_", 11) == 0)
		{
			pszImageLarge = "lfe_hl2_background07";
		}
		else if (V_strnicmp(map, "d3_breen_", 9) == 0)
		{
			pszImageLarge = "lfe_hl2_background07";
		}
		else if (V_strnicmp(map, "ep1_citadel_", 12) == 0)
		{
			pszImageLarge = "lfe_ep1_background01";
		}
		else if (V_strnicmp(map, "ep1_c17_00", 10) == 0)
		{
			pszImageLarge = "lfe_ep1_background01a";
		}
		else if (V_strnicmp(map, "ep1_c17_00a", 11) == 0)
		{
			pszImageLarge = "lfe_ep1_background01a";
		}
		else if ( V_strnicmp(map, "ep1_c17_01", 10) == 0 || V_strnicmp(map, "ep1_c17_01a", 11) == 0 || V_strnicmp(map, "ep1_c17_02", 10) == 0 || V_strnicmp(map, "ep1_c17_02b", 11) == 0 || V_strnicmp(map, "ep1_c17_02a", 11) == 0 || V_strnicmp(map, "ep1_c17_05", 10) == 0 )
		{
			pszImageLarge = "lfe_ep1_background02";
		}
		else if ( V_strnicmp(map, "ep1_c17_06", 10) == 0 )
		{
			pszImageLarge = "lfe_ep1_background02";
		}
		else if ( V_strnicmp(map, "ep2_outland_01", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background01";
		}
		else if ( V_strnicmp(map, "ep2_outland_01a", 15) == 0 )
		{
			pszImageLarge = "lfe_ep2_background01";
		}
		else if ( V_strnicmp(map, "ep2_outland_02", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02";
		}
		else if ( V_strnicmp(map, "ep2_outland_03", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02";
		}
		else if ( V_strnicmp(map, "ep2_outland_04", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02";
		}
		else if ( V_strnicmp(map, "ep2_outland_05", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_06", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_06a", 15) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_07", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_08", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_09", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_10", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_10a", 15) == 0 )
		{
			pszImageLarge = "lfe_ep2_background03";
		}
		else if ( V_strnicmp(map, "ep2_outland_11", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02a";
		}
		else if ( V_strnicmp(map, "ep2_outland_11a", 15) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02a";
		}
		else if ( V_strnicmp(map, "ep2_outland_11b", 15) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02a";
		}
		else if ( V_strnicmp(map, "ep2_outland_12", 14) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02a";
		}
		else if ( V_strnicmp(map, "ep2_outland_12a", 15) == 0 )
		{
			pszImageLarge = "lfe_ep2_background02a";
		}
		else if ( V_strnicmp(map, "testchmb_a_", 11) == 0 )
		{
			pszImageLarge = "lfe_portal_background1";
		}
		else if ( V_strnicmp(map, "escape_00", 9) == 0 )
		{
			pszImageLarge = "lfe_portal_background1";
		}
		else if ( V_strnicmp(map, "escape_01", 9) == 0 )
		{
			pszImageLarge = "lfe_portal_background1";
		}
		else if ( V_strnicmp(map, "escape_02", 9) == 0 )
		{
			pszImageLarge = "lfe_portal_background1";
		}
	}

	//checks the players class
	if ( pTFPlayer )
	{
		int iClass = pTFPlayer->GetPlayerClass()->GetClassIndex();

		if ( pTFPlayer->GetTeamNumber() != TEAM_SPECTATOR )
		{
			pszImageSmall = g_aClassImage[iClass];
			pszImageText = g_aPlayerClassNames_NonLocalized[iClass];
		}
		else
		{
			pszImageSmall = "";
			pszImageText = "Spectating";
		}
	}

	m_sDiscordRichPresence.largeImageKey = pszImageLarge;
	m_sDiscordRichPresence.largeImageText = pszGameType;
	m_sDiscordRichPresence.smallImageKey = pszImageSmall;
	m_sDiscordRichPresence.smallImageText = pszImageText;
}

void CTFDiscordRPC::InitializeDiscord()
{
	DiscordEventHandlers handlers;
	Q_memset(&handlers, 0, sizeof(handlers));
	handlers.ready			= &CTFDiscordRPC::OnReady;
	handlers.errored		= &CTFDiscordRPC::OnDiscordError;
	handlers.joinGame		= &CTFDiscordRPC::OnJoinGame;
	//handlers.spectateGame = &CTFDiscordRPC::OnSpectateGame;
	handlers.joinRequest	= &CTFDiscordRPC::OnJoinRequest;

	char command[512];
	V_snprintf( command, sizeof( command ), "%s -game \"%s\" -novid -steam\n", CommandLine()->GetParm( 0 ), CommandLine()->ParmValue( "-game" ) );
	Discord_Register( DISCORD_APP_ID, command );
	Discord_Initialize( DISCORD_APP_ID, &handlers, false, "" );
	Reset();
}

bool CTFDiscordRPC::NeedToUpdate()
{
	if ( m_bErrored || m_szLatchedMapname[0] == '\0')
		return false;

	return gpGlobals->realtime >= m_flLastUpdatedTime + DISCORD_UPDATE_RATE;
}

void CTFDiscordRPC::Reset()
{
	Q_memset( &m_sDiscordRichPresence, 0, sizeof( m_sDiscordRichPresence ) );
	m_sDiscordRichPresence.details = "Main Menu";
	const char *pszState = "";

	m_sDiscordRichPresence.state = pszState;

	m_sDiscordRichPresence.endTimestamp;
	
	SetLogo();
	Discord_UpdatePresence( &m_sDiscordRichPresence );
}

void CTFDiscordRPC::UpdatePlayerInfo()
{
	C_TF_PlayerResource *pResource = GetTFPlayerResource();
	if ( !pResource )
		return;

	int maxPlayers = gpGlobals->maxClients;
	int curPlayers = 0;

	const char *pzePlayerName = NULL;

	// This is used for setting the discord decription name
	// We are using the discord description for the chapter name
	// -Nbc66

	//HL2 Chapters
	if ( engine->IsConnected() )
	{
		const char *szMapName = GetMapDisplayName( m_szLatchedMapname );
		if ( szMapName[0] != '\0' )
		{
			m_sDiscordRichPresence.details = szMapName;
		}
		else
		{
			m_sDiscordRichPresence.details = m_szLatchedMapname;
		}
	}

	for ( int i = 1; i < maxPlayers; i++ )
	{
		if ( pResource->IsConnected( i ) )
		{
			curPlayers++;
			if ( pResource->IsLocalPlayer( i ) )
			{
				pzePlayerName = pResource->GetPlayerName( i );
			}
		}
	}

	//int iTimeLeft = TFGameRules()->GetTimeLeft();

	if ( m_szLatchedHostname[0] != '\0' )
	{
		if ( cl_richpresence_printmsg.GetBool() )
		{
			ConColorMsg( Color( 114, 137, 218, 255 ), "[Discord] sending details of\n '%s'\n", m_szServerInfo );
		}
		m_sDiscordRichPresence.partySize = curPlayers;
		m_sDiscordRichPresence.partyMax = maxPlayers;
		m_sDiscordRichPresence.state = m_szLatchedHostname;
		//m_sDiscordRichPresence.state = szStateBuffer;
	}
}

void CTFDiscordRPC::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp( type, "server_spawn" ) == 0 )
	{
		Q_strncpy( m_szLatchedHostname, event->GetString( "hostname" ), 255 );
	}
}

void CTFDiscordRPC::UpdateRichPresence()
{
	//The elapsed timer function using <ctime>
	//this is for setting up the time when the player joins a server
	//-Nbc66
	time_t iSysTime;
	time(&iSysTime);
	struct tm *tStartTime = NULL;
	tStartTime = localtime(&iSysTime);
	tStartTime->tm_sec += 0 - gpGlobals->curtime;

	if (!NeedToUpdate())
		return;

	m_flLastUpdatedTime = gpGlobals->realtime;

	if ( engine->IsConnected() )
	{
		UpdatePlayerInfo();
		UpdateNetworkInfo();
		//starts the elapsed timer for discord rpc
		//-Nbc66
		m_sDiscordRichPresence.startTimestamp = mktime(tStartTime);
	}

	//checks if the loading bar is being drawn
	//and sets the discord status to "Currently is loading..."
	//-Nbc66
	if ( engine->IsDrawingLoadingImage() == true )
	{
		m_sDiscordRichPresence.state = "";
		m_sDiscordRichPresence.details = "Currently loading...";
	}

	SetLogo();

	Discord_UpdatePresence(&m_sDiscordRichPresence);
}


void CTFDiscordRPC::UpdateNetworkInfo()
{
	INetChannelInfo *ni = engine->GetNetChannelInfo();
	if ( ni->IsLoopback() )
		return;

	char partyId[128];
	sprintf( partyId, "ip %s", ni->GetAddress() ); // adding -party here because secrets cannot match the party id

	m_sDiscordRichPresence.partyId = partyId;

	m_sDiscordRichPresence.joinSecret = ni->GetAddress();
	m_sDiscordRichPresence.spectateSecret = "Spectate";
}

void CTFDiscordRPC::LevelInit( const char *szMapname )
{
	Reset();
	// we cant update our presence here, because if its the first map a client loaded,
	// discord api may not yet be loaded, so latch
	Q_strcpy(m_szLatchedMapname, szMapname);
	//V_snprintf(szStateBuffer, sizeof(szStateBuffer), "MAP: %s", m_szLatchedMapname);
	// important, clear last update time as well
	m_flLastUpdatedTime = max(0, gpGlobals->realtime - DISCORD_UPDATE_RATE);
}