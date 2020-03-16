//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "tf_mainmenupanel.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_advslider.h"
#include "vgui_controls/SectionedListPanel.h"
#include "vgui_controls/ImagePanel.h"
#include "engine/IEngineSound.h"
#include "vgui_avatarimage.h"
#include "tf_gamerules.h"
#include <KeyValues.h>
#include "panels/lfe_genericconfirmation.h"
#include "panels/tf_optionsdialog.h"
#include "tier0/icommandline.h"
#include "tf_inventory.h"

using namespace vgui;
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BLOG_URL "https://www.lfe.tf/#main"

static void OnBGVideoToggle(IConVar *var, const char *pOldValue, float flOldValue)
{
	GET_MAINMENUPANEL(CTFMainMenuPanel)->ShowBGVideoPanel(((ConVar*)var)->GetBool());
}

static void OnBlogToggle(IConVar *var, const char *pOldValue, float flOldValue)
{
	GET_MAINMENUPANEL(CTFMainMenuPanel)->ShowBlogPanel(((ConVar*)var)->GetBool());
}

ConVar lfe_ui_mainmenu_music("lfe_ui_mainmenu_music", "1", FCVAR_ARCHIVE, "Toggle music in the main menu");

ConVar lfe_ui_confirmation_quit( "lfe_ui_confirmation_quit", "1", FCVAR_ARCHIVE, "Toggle quit confirmation" );
ConVar lfe_ui_confirmation_disconnect( "lfe_ui_confirmation_disconnect", "1", FCVAR_ARCHIVE, "Toggle disconnect confirmation" );
ConVar lfe_ui_mainmenu_bg_video( "lfe_ui_mainmenu_bg_video", "1", FCVAR_ARCHIVE, "Toggle video background", OnBGVideoToggle );
ConVar lfe_ui_mainmenu_news( "lfe_ui_mainmenu_news", "1", FCVAR_ARCHIVE, "Toggle news", OnBlogToggle );
ConVar lfe_ui_mainmenu_music_type( "lfe_ui_mainmenu_music_type", "0", FCVAR_ARCHIVE, "Toggle between hl2/tf2 music in the main menu." );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFMainMenuPanel::CTFMainMenuPanel(vgui::Panel* parent, const char *panelName) : CTFMenuPanelBase(parent, panelName)
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFMainMenuPanel::~CTFMainMenuPanel()
{
	m_CharacterBackgrounds.Purge();
}

bool CTFMainMenuPanel::Init()
{
	BaseClass::Init();

	m_psMusicStatus = MUSIC_FIND;
	m_pzMusicLink[0] = '\0';
	m_nSongGuid = 0;
	m_iLastSongPlayed = -1;

	if ( steamapicontext->SteamUser() )
		m_SteamID = steamapicontext->SteamUser()->GetSteamID();

	m_iShowFakeIntro = 4;
	m_pVersionLabel = NULL;
	m_pProfileAvatar = NULL;
	m_pFakeBGImage = NULL;
	m_pCharacterImage = NULL;
	m_pBlogPanel = new CTFBlogPanel(this, "BlogPanel");

	bInMenu = true;
	bInGame = false;

	KeyValues *pKV = new KeyValues( "CharacterBackgrounds" );
	if ( pKV->LoadFromFile( filesystem, "scripts/CharacterBackgrounds.txt", "MOD" ) )
	{
		for ( KeyValues *menu = pKV->GetFirstSubKey(); menu != NULL; menu = menu->GetNextKey() )
		{
			TFCharacterBackgrounds_t item;

			Q_strncpy( item.m_szImage, menu->GetString( "image", "" ), MAX_PATH );
			Q_strncpy( item.m_szHolidayRestriction, menu->GetString( "holiday_restriction", "" ), 64 );

			m_CharacterBackgrounds.AddToTail( item );
		}
	}
	pKV->deleteThis();

	return true;
}


void CTFMainMenuPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
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

	LoadControlSettings( "resource/UI/main_menu/MainMenuPanel.res", NULL, NULL, pConditions );

	m_pVersionLabel = dynamic_cast<CExLabel *>( FindChildByName( "VersionLabel" ) );
	m_pProfileAvatar = dynamic_cast<CAvatarImagePanel *>( FindChildByName( "AvatarImage" ) );
	m_pFakeBGImage = dynamic_cast<vgui::ImagePanel *>( FindChildByName( "FakeBGImage" ) );
	m_pCharacterImage = dynamic_cast<vgui::ImagePanel *>( FindChildByName( "TFCharacterImage" ) );

	if ( pConditions )
		pConditions->deleteThis();
}	

void CTFMainMenuPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_pProfileAvatar )
	{
		m_pProfileAvatar->SetPlayer( m_SteamID, k_EAvatarSize64x64 );
		m_pProfileAvatar->SetShouldDrawFriendIcon( false );
	}

	char szNickName[64];
	Q_snprintf( szNickName, sizeof( szNickName ), ( steamapicontext->SteamFriends() ? steamapicontext->SteamFriends()->GetPersonaName() : "Steam Disconnected" ) );
	SetDialogVariable( "nickname", szNickName );

	TFCharacterBackgrounds_t *pCBG = &m_CharacterBackgrounds.Random();
	if ( pCBG )
	{
		/*if ( pCBG->m_szHolidayRestriction[0] != '\0' )
		{
			if ( !Q_strcmp( UTIL_GetActiveHolidayString(), pCBG->m_szHolidayRestriction ) )
				m_pCharacterImage->SetImage( pCBG->m_szImage );
		}
		else
		{*/
			m_pCharacterImage->SetImage( pCBG->m_szImage );
		//}
	}

	AutoLayout();

	if ( m_iShowFakeIntro > 0 )
	{
		char szBGName[128];
		engine->GetMainMenuBackgroundName( szBGName, sizeof( szBGName ) );
		char szImage[128];
		Q_snprintf( szImage, sizeof( szImage ), "../console/%s", szBGName );
		int width, height;
		surface()->GetScreenSize( width, height );
		float fRatio = (float)width / (float)height;
		bool bWidescreen = ( fRatio < 1.5 ? false : true );
		if ( bWidescreen )
			Q_strcat( szImage, "_widescreen", sizeof( szImage ) );
		m_pFakeBGImage->SetImage( szImage );
		m_pFakeBGImage->SetVisible( true );
		m_pFakeBGImage->SetAlpha( 255 );
	}
};

void CTFMainMenuPanel::ShowBlogPanel( bool show )
{
	if ( m_pBlogPanel )
	{
		m_pBlogPanel->SetVisible(show);
	}
}

void CTFMainMenuPanel::ShowBGVideoPanel( bool show )
{
	if ( show )
	{
		MAINMENU_ROOT->ShowPanel(BACKGROUND_MENU);
	}
	else
	{
		MAINMENU_ROOT->HidePanel(BACKGROUND_MENU);
	}
}

void CTFMainMenuPanel::OnCommand(const char* command)
{
	if ( !Q_strcmp( command, "newquit" ) )
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
			MAINMENU_ROOT->ShowPanel(CONFIRMATION_MENU);
		}
		else
		{
			engine->ClientCmd( "quit" );
		}
	}
	else if ( !Q_strcmp( command, "newoptionsdialog" ) )
	{
		MAINMENU_ROOT->ShowPanel(OPTIONSDIALOG_MENU);
		GET_MAINMENUPANEL(CTFOptionsDialog)->SetCurrentPanel(OPTION_PANEL_ADV);
	}
	else if ( !Q_strcmp( command, "newcreategame" ) )
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

void CTFMainMenuPanel::OnTick()
{
	BaseClass::OnTick();

	if ( lfe_ui_mainmenu_music.GetBool() && !bInGameLayout && !CommandLine()->FindParm( "-nostartupsound" ) )
	{
		if ( ( m_psMusicStatus == MUSIC_FIND || m_psMusicStatus == MUSIC_STOP_FIND ) && !enginesound->IsSoundStillPlaying( m_nSongGuid ) )
		{
			GetRandomMusic( m_pzMusicLink, sizeof( m_pzMusicLink ) );
			m_psMusicStatus = MUSIC_PLAY;
		}
		else if ( ( m_psMusicStatus == MUSIC_PLAY || m_psMusicStatus == MUSIC_STOP_PLAY)&& m_pzMusicLink[0] != '\0' )
		{
			enginesound->StopSoundByGuid( m_nSongGuid );
			ConVar *snd_musicvolume = cvar->FindVar( "snd_musicvolume" );
			float fVolume = ( snd_musicvolume ? snd_musicvolume->GetFloat() : 1.0f );
			enginesound->EmitAmbientSound( m_pzMusicLink, fVolume, PITCH_NORM, 0 );	
			m_nSongGuid = enginesound->GetGuidForLastSoundEmitted();
			m_psMusicStatus = MUSIC_FIND;
		}
	}
	else if ( m_psMusicStatus == MUSIC_FIND )
	{
		enginesound->StopSoundByGuid( m_nSongGuid );
		m_psMusicStatus = ( m_nSongGuid == 0 ? MUSIC_STOP_FIND : MUSIC_STOP_PLAY );
	}
};

void CTFMainMenuPanel::OnThink()
{
	BaseClass::OnThink();

	if ( m_iShowFakeIntro > 0 )
	{
		m_iShowFakeIntro--;
		if ( m_iShowFakeIntro == 0 )
		{
			vgui::GetAnimationController()->RunAnimationCommand( m_pFakeBGImage, "Alpha", 0, 1.0f, 0.5f, vgui::AnimationController::INTERPOLATOR_SIMPLESPLINE );
		}
	}	
	if ( m_pFakeBGImage->IsVisible() && m_pFakeBGImage->GetAlpha() == 0 )
	{
		m_pFakeBGImage->SetVisible( false );
		ShowBGVideoPanel( true );
	}

	// check if user had required dx
	ConVarRef pDXLevel( "mat_dxlevel" );
	if( pDXLevel.GetInt() < 90 )
	{
		CTFGenericConfirmation* confirmation = GET_MAINMENUPANEL(CTFGenericConfirmation);
		CTFGenericConfirmation::Data_t data;

		data.pWindowTitle = "#LFE_Warning_Title";
		data.pMessageText = "#LFE_Warning_DXBelow";
		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &ConfirmQuit;
		data.pOkButtonText = "#TF_Quit_Title";
		data.bCancelButtonEnabled = true;
		data.pCancelButtonText = "#LFE_Ignore";

		confirmation->SetUsageData(data);
		MAINMENU_ROOT->ShowPanel(CONFIRMATION_MENU);
	}

	// hide all panel
	/*MAINMENU_ROOT->HidePanel(SHADEBACKGROUND_MENU);
	MAINMENU_ROOT->HidePanel(CREDIT_MENU);
	MAINMENU_ROOT->HidePanel(OPTIONSDIALOG_MENU);
	MAINMENU_ROOT->HidePanel(CREATESERVER_MENU);
	MAINMENU_ROOT->HidePanel(STATSUMMARY_MENU);
	MAINMENU_ROOT->HidePanel(TOOLTIP_MENU);
	MAINMENU_ROOT->HidePanel(ITEMTOOLTIP_MENU);*/
};

void CTFMainMenuPanel::Show()
{
	BaseClass::Show();
	//vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 255, 0.0f, 0.5f, vgui::AnimationController::INTERPOLATOR_SIMPLESPLINE);
};

void CTFMainMenuPanel::Hide()
{
	BaseClass::Hide();
	//vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 0, 0.0f, 0.1f, vgui::AnimationController::INTERPOLATOR_LINEAR);
};


void CTFMainMenuPanel::DefaultLayout()
{
	BaseClass::DefaultLayout();
};

void CTFMainMenuPanel::GameLayout()
{
	BaseClass::GameLayout();
};

void CTFMainMenuPanel::PlayMusic()
{

}

void CTFMainMenuPanel::StopMusic()
{
	enginesound->StopSoundByGuid( m_nSongGuid );
}

void CTFMainMenuPanel::GetRandomMusic( char *pszBuf, int iBufLength )
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

void CTFMainMenuPanel::ConfirmQuit()
{
	engine->ClientCmd( "quit" );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFBlogPanel::CTFBlogPanel(vgui::Panel* parent, const char *panelName) : CTFMenuPanelBase(parent, panelName)
{
	m_pHTMLPanel = new vgui::HTML(this, "HTMLPanel");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFBlogPanel::~CTFBlogPanel()
{
}

void CTFBlogPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings( "resource/UI/main_menu/BlogPanel.res" );
}

void CTFBlogPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	LoadBlogPost(BLOG_URL);
}

void CTFBlogPanel::LoadBlogPost(const char* URL)
{
	if (m_pHTMLPanel)
	{
		m_pHTMLPanel->SetVisible(true);
		m_pHTMLPanel->OpenURL(URL, NULL);
	}
}

void CTFBlogPanel::Show()
{
	BaseClass::Show();
	vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 255, 0.0f, 0.5f, vgui::AnimationController::INTERPOLATOR_SIMPLESPLINE);
};

void CTFBlogPanel::Hide()
{
	BaseClass::Hide();
	vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 0, 0.0f, 0.1f, vgui::AnimationController::INTERPOLATOR_LINEAR);
};
