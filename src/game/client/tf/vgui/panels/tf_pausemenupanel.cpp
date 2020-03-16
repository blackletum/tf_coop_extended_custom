//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "tf_pausemenupanel.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_advslider.h"
#include "vgui_controls/SectionedListPanel.h"
#include "vgui_controls/ImagePanel.h"
#include "engine/IEngineSound.h"
#include "vgui_avatarimage.h"
#include "panels/lfe_genericconfirmation.h"
#include "panels/tf_optionsdialog.h"
#include "tf_gamerules.h"

using namespace vgui;
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar lfe_pausemenu_music( "lfe_ui_pausemenu_music", "0", FCVAR_ARCHIVE, "Toggle music in-game main menu" );

extern ConVar lfe_ui_confirmation_quit;
extern ConVar lfe_ui_confirmation_disconnect;
extern ConVar lfe_ui_mainmenu_music_type;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFPauseMenuPanel::CTFPauseMenuPanel( vgui::Panel* parent, const char *panelName ) : CTFMenuPanelBase( parent, panelName )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFPauseMenuPanel::~CTFPauseMenuPanel()
{

}

bool CTFPauseMenuPanel::Init()
{
	BaseClass::Init();

	m_psMusicStatus = PAUSE_MUSIC_FIND;
	m_pzMusicLink[0] = '\0';
	m_nSongGuid = 0;
	m_iLastSongPlayed = -1;

	if ( steamapicontext->SteamUser() )
		m_SteamID = steamapicontext->SteamUser()->GetSteamID();

	m_pVersionLabel = NULL;
	m_pProfileAvatar = NULL;

	bInMenu = false;
	bInGame = true;
	return true;
};


void CTFPauseMenuPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	KeyValues *pConditions = NULL;

	if ( UTIL_IsHolidayActive( kHoliday_Halloween ) )
	{
		pConditions = new KeyValues( "conditions" );
		AddSubKeyNamed( pConditions, "if_halloween" );
	}
	else if ( UTIL_IsHolidayActive( kHoliday_Christmas ) )
	{
		pConditions = new KeyValues( "conditions" );
		AddSubKeyNamed( pConditions, "if_smissmas" );
	}

	LoadControlSettings( "resource/UI/main_menu/PauseMenuPanel.res", NULL, NULL, pConditions );

	m_pVersionLabel = dynamic_cast<CExLabel *>( FindChildByName( "VersionLabel" ) );
	m_pProfileAvatar = dynamic_cast<CAvatarImagePanel *>( FindChildByName( "AvatarImage" ) );

	if ( pConditions )
		pConditions->deleteThis();
}

void CTFPauseMenuPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	if (m_pProfileAvatar)
	{
		m_pProfileAvatar->SetPlayer( m_SteamID, k_EAvatarSize64x64 );
		m_pProfileAvatar->SetShouldDrawFriendIcon( false );
	}

	char szNickName[64];
	Q_snprintf( szNickName, sizeof( szNickName ),
		( steamapicontext->SteamFriends() ? steamapicontext->SteamFriends()->GetPersonaName() : "Steam Disconnected" ) );
	SetDialogVariable( "nickname", szNickName );

	AutoLayout();
};

void CTFPauseMenuPanel::OnCommand( const char* command )
{
	if ( !Q_strcmp(command, "newquit" ) )
	{
		if ( lfe_ui_confirmation_quit.GetBool() )
		{
			CTFGenericConfirmation* confirmation = GET_MAINMENUPANEL(CTFGenericConfirmation);

			CTFGenericConfirmation::Data_t data;

			data.pWindowTitle = "#MMenu_PromptQuit_Title";
			data.pMessageText = "#MMenu_PromptQuit_Body";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &ConfirmQuit;
			data.pOkButtonText = "#TF_Quit_Title";
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData(data);
			MAINMENU_ROOT->ShowPanel( CONFIRMATION_MENU );
		}
		else
		{
			engine->ClientCmd( "quit" );
		}
	}
	else if ( !Q_strcmp( command, "newdisconnect" ) )
	{
		if ( lfe_ui_confirmation_disconnect.GetBool() )
		{
			CTFGenericConfirmation* confirmation = GET_MAINMENUPANEL(CTFGenericConfirmation);

			CTFGenericConfirmation::Data_t data;

			data.pWindowTitle = "#TF_MM_Disconnect_Title";
			data.pMessageText = "#TF_MM_Disconnect";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &ConfirmDisconnect;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData(data);
			MAINMENU_ROOT->ShowPanel( CONFIRMATION_MENU );
		}
		else
		{
			engine->ClientCmd( "disconnect" );
		}
	}
	else if ( !Q_strcmp(command, "newoptionsdialog" ) )
	{
		MAINMENU_ROOT->ShowPanel( OPTIONSDIALOG_MENU );
		GET_MAINMENUPANEL(CTFOptionsDialog)->SetCurrentPanel( OPTION_PANEL_ADV );
	}
	else if ( !Q_strcmp(command, "newcreategame" ) )
	{
		MAINMENU_ROOT->ShowPanel( CREATESERVER_MENU );
	}
	else if ( !Q_strcmp( command, "newloadout" ) )
	{
		engine->ClientCmd( "open_charinfo" );
	}
	else if ( !Q_strcmp( command, "newstats" ) )
	{
		MAINMENU_ROOT->ShowPanel( STATSUMMARY_MENU );
	}
	else if ( !Q_strcmp( command, "newcredits" ) )
	{
		MAINMENU_ROOT->ShowPanel( CREDIT_MENU );
	}
	else if ( !Q_strcmp( command, "newachievements" ) )
	{
		MAINMENU_ROOT->ShowPanel( ACHIEVEMENTS_MENU );
	}
	else if ( !Q_strcmp( command, "randommusic" ) )
	{
		StopMusic();
	}
	else if ( !Q_strcmp( command, "callvote" ) )
	{
		engine->ExecuteClientCmd( "callvote" );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CTFPauseMenuPanel::OnTick()
{
	BaseClass::OnTick();

	if ( lfe_pausemenu_music.GetBool() && bInGameLayout )
	{
		if ( ( m_psMusicStatus == PAUSE_MUSIC_FIND || m_psMusicStatus == PAUSE_MUSIC_STOP_FIND) && !enginesound->IsSoundStillPlaying( m_nSongGuid ) )
		{
			GetRandomMusic( m_pzMusicLink, sizeof( m_pzMusicLink ) );
			m_psMusicStatus = PAUSE_MUSIC_PLAY;
		}
		else if ( ( m_psMusicStatus == PAUSE_MUSIC_PLAY || m_psMusicStatus == PAUSE_MUSIC_STOP_PLAY)&& m_pzMusicLink[0] != '\0' )
		{
			enginesound->StopSoundByGuid( m_nSongGuid );
			ConVar *snd_musicvolume = cvar->FindVar( "snd_musicvolume" );
			float fVolume = ( snd_musicvolume ? snd_musicvolume->GetFloat() : 1.0f );
			enginesound->EmitAmbientSound( m_pzMusicLink, fVolume, PITCH_NORM, 0 );			
			m_nSongGuid = enginesound->GetGuidForLastSoundEmitted();
			m_psMusicStatus = PAUSE_MUSIC_FIND;
		}
	}
	else if ( m_psMusicStatus == PAUSE_MUSIC_FIND )
	{
		enginesound->StopSoundByGuid( m_nSongGuid );
		m_psMusicStatus = ( m_nSongGuid == 0 ? PAUSE_MUSIC_STOP_FIND : PAUSE_MUSIC_STOP_PLAY );
	}
};

void CTFPauseMenuPanel::OnThink()
{
	BaseClass::OnThink();
};

void CTFPauseMenuPanel::Show()
{
	BaseClass::Show();
	//vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 255, 0.0f, 0.5f, vgui::AnimationController::INTERPOLATOR_SIMPLESPLINE);
};

void CTFPauseMenuPanel::Hide()
{
	BaseClass::Hide();
	//vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 0, 0.0f, 0.1f, vgui::AnimationController::INTERPOLATOR_LINEAR);
};

void CTFPauseMenuPanel::DefaultLayout()
{
	BaseClass::DefaultLayout();
};

void CTFPauseMenuPanel::GameLayout()
{
	BaseClass::GameLayout();
};

void CTFPauseMenuPanel::PlayMusic()
{

}

void CTFPauseMenuPanel::StopMusic()
{
	enginesound->StopSoundByGuid(m_nSongGuid);
}

void CTFPauseMenuPanel::GetRandomMusic( char *pszBuf, int iBufLength )
{
	Assert( iBufLength );

	char szPath[MAX_PATH];

	if ( lfe_ui_mainmenu_music_type.GetInt() == 1 )
	{
		// Check that there's music available
		if ( !g_pFullFileSystem->FileExists( "sound/ui/gamestartup1.mp3", "GAME" ) )
		{
			Assert(false);
			*pszBuf = '\0';
		}

		// Discover tracks, 1 through n
		int iLastTrack = 0;

		do
		{
			Q_snprintf( szPath, sizeof( szPath ), "sound/ui/gamestartup%d.mp3", ++iLastTrack );
		} while ( g_pFullFileSystem->FileExists( szPath, "MOD" ) );

		// Pick a random one
		int iRandSong = RandomInt(1, iLastTrack - 1);

		// Prevent the same song from being played twice in a row
		while (iRandSong == m_iLastSongPlayed) iRandSong = RandomInt(1, iLastTrack - 1);
		m_iLastSongPlayed = iRandSong;

		Q_snprintf( szPath, sizeof( szPath ), "ui/gamestartup%d.mp3", RandomInt( 1, iLastTrack - 1 ) );
		Q_strncpy( pszBuf, szPath, iBufLength );
	}
	else
	{
		// Check that there's music available
		if ( !g_pFullFileSystem->FileExists( "sound/ui/gamestartup/gamestartup1.mp3", "MOD" ) )
		{
			Assert(false);
			*pszBuf = '\0';
		}

		// Discover tracks, 1 through n
		int iLastTrack = 0;

		do
		{
			Q_snprintf( szPath, sizeof( szPath ), "sound/ui/gamestartup/gamestartup%d.mp3", ++iLastTrack );
		} while ( g_pFullFileSystem->FileExists( szPath, "MOD" ) );

		// Pick a random one
		int iRandSong = RandomInt(1, iLastTrack - 1);

		// Prevent the same song from being played twice in a row
		while (iRandSong == m_iLastSongPlayed) iRandSong = RandomInt(1, iLastTrack - 1);
		m_iLastSongPlayed = iRandSong;

		Q_snprintf( szPath, sizeof( szPath ), "ui/gamestartup/gamestartup%d.mp3", RandomInt( 1, iLastTrack - 1 ) );
		Q_strncpy( pszBuf, szPath, iBufLength );
	}
}

void CTFPauseMenuPanel::ConfirmQuit()
{
	engine->ClientCmd( "quit" );
}

void CTFPauseMenuPanel::ConfirmDisconnect()
{
	COM_TimestampedLog( "Exit Game" );
	engine->ExecuteClientCmd( "disconnect" );
	MAINMENU_ROOT->HidePanel( CONFIRMATION_MENU );
}