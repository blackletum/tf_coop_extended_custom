//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/AnimationController.h>
#include "achievement_notification_panel.h"
#include "steam/steam_api.h"
#include "iachievementmgr.h"
#include "fmtstr.h"
#include "achievementmgr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define ACHIEVEMENT_NOTIFICATION_DURATION 10.0f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

DECLARE_HUDELEMENT_DEPTH( CAchievementNotificationPanel, 100 );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAchievementNotificationPanel::CAchievementNotificationPanel( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "AchievementNotificationPanel" )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_flHideTime = 0;
	m_pPanelBackground = new EditablePanel( this, "Notification_Background" );
	m_pIcon = new ImagePanel( this, "Notification_Icon" );
	m_pLabelHeading = new CExLabel( this, "HeadingLabel", "" );
	m_pLabelTitle = new CExLabel( this, "TitleLabel", "" );

	m_pIcon->SetShouldScaleImage( true );

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::Init()
{
	ListenForGameEvent( "achievement_event" );
	ListenForGameEvent( "achievement_earned" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::ApplySchemeSettings( IScheme *pScheme )
{
	// load control settings...
	LoadControlSettings( "resource/UI/AchievementNotification.res" );
	
	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::PerformLayout( void )
{
	BaseClass::PerformLayout();

	SetBgColor( Color( 0, 0, 0, 0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::FireGameEvent( IGameEvent * event )
{
	const char *name = event->GetName();
	/*if ( Q_strcmp( "achievement_event", name ) == 0 )
	{
		const char *pchName = event->GetString( "achievement_name" );
		int iCur = event->GetInt( "cur_val" );
		int iMax = event->GetInt( "max_val" );
		wchar_t szLocalizedName[256]=L"";

		if ( Q_strcmp( "LFE_BEAT_HL2", pchName ) == 0 || Q_strcmp( "LFE_BEAT_HL2EP1", pchName ) == 0 || Q_strcmp( "LFE_BEAT_HL2EP2", pchName ) == 0 || Q_strcmp( "LFE_BEAT_HLS", pchName ) == 0 || Q_strcmp( "LFE_BEAT_P1", pchName ) == 0 )
			return;

		const wchar_t *pchLocalizedName = ACHIEVEMENT_LOCALIZED_NAME_FROM_STR( pchName );
		Assert( pchLocalizedName );
		if ( !pchLocalizedName || !pchLocalizedName[0] )
			return;
		Q_wcsncpy( szLocalizedName, pchLocalizedName, sizeof( szLocalizedName ) );

		// this is achievement progress, compose the message of form: "<name> (<#>/<max>)"
		wchar_t szFmt[128]=L"";
		wchar_t szText[512]=L"";
		wchar_t szNumFound[16]=L"";
		wchar_t szNumTotal[16]=L"";
		_snwprintf( szNumFound, ARRAYSIZE( szNumFound ), L"%i", iCur );
		_snwprintf( szNumTotal, ARRAYSIZE( szNumTotal ), L"%i", iMax );

		const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#GameUI_Achievement_Progress_Fmt" );
		if ( !pchFmt || !pchFmt[0] )
			return;
		Q_wcsncpy( szFmt, pchFmt, sizeof( szFmt ) );

		g_pVGuiLocalize->ConstructString( szText, sizeof( szText ), szFmt, 3, szLocalizedName, szNumFound, szNumTotal );
		AddNotification( pchName, g_pVGuiLocalize->Find( "#GameUI_Achievement_Progress" ), szText, false );

		vgui::surface()->PlaySound( "ui/quest_status_tick_expert.wav" );
			
	}
	else*/ if ( Q_strcmp( "achievement_earned", name ) == 0 )
	{
		CAchievementMgr *pAchievementMgr = dynamic_cast<CAchievementMgr *>( engine->GetAchievementMgr() );
		if ( pAchievementMgr )
		{
			IAchievement *pAchievement = pAchievementMgr->GetAchievementByID( event->GetInt( "achievement" ) );
			if ( pAchievement )
			{
				int iPlayerIndex = event->GetInt( "player" );
				C_BasePlayer *pPlayer = UTIL_PlayerByIndex( iPlayerIndex );
				if ( pPlayer && pPlayer->IsLocalPlayer() )
				{
					wchar_t szLocalizedName[256]=L"";
					const wchar_t *pchLocalizedName = ACHIEVEMENT_LOCALIZED_NAME_FROM_STR( pAchievement->GetName() );
					Assert( pchLocalizedName );
					if ( !pchLocalizedName || !pchLocalizedName[0] )
						return;

					Q_wcsncpy( szLocalizedName, pchLocalizedName, sizeof( szLocalizedName ) );
					const char *pchName = pAchievement->GetName();

					AddNotification( pchName, g_pVGuiLocalize->Find( "#GameUI_Achievement_Awarded" ), szLocalizedName, true );
					vgui::surface()->PlaySound( "ui/quest_turn_in_accepted_light.wav" );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called on each tick
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::OnTick( void )
{
	if ( ( m_flHideTime > 0 ) && ( m_flHideTime < gpGlobals->curtime ) )
	{
		m_flHideTime = 0;
		ShowNextNotification();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAchievementNotificationPanel::ShouldDraw( void )
{
	return ( ( m_flHideTime > 0 ) && ( m_flHideTime > gpGlobals->curtime ) && CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::AddNotification( const char *szIconBaseName, const wchar_t *pHeading, const wchar_t *pTitle, bool bComplete )
{
	// put this notification in our queue
	int iQueueItem = m_queueNotification.AddToTail();
	Notification_t &notification = m_queueNotification[iQueueItem];
	Q_strncpy( notification.szIconBaseName, szIconBaseName, ARRAYSIZE( notification.szIconBaseName ) );
	Q_wcsncpy( notification.szHeading, pHeading, sizeof( notification.szHeading ) );
	Q_wcsncpy( notification.szTitle, pTitle, sizeof( notification.szTitle ) );
	notification.bComplete = bComplete;

	// if we are not currently displaying a notification, go ahead and show this one
	if ( 0 == m_flHideTime )
	{
		ShowNextNotification();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shows next notification in queue if there is one
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::ShowNextNotification()
{
	// see if we have anything to do
	if ( 0 == m_queueNotification.Count() )
	{
		m_flHideTime = 0;
		return;
	}

	Notification_t &notification = m_queueNotification[ m_queueNotification.Head() ];

	m_flHideTime = gpGlobals->curtime + ACHIEVEMENT_NOTIFICATION_DURATION;

	// set the text and icon in the dialog
	SetDialogVariable( "heading", notification.szHeading );
	SetDialogVariable( "title", notification.szTitle );
	const char *pchIconBaseName = notification.szIconBaseName;
	if ( pchIconBaseName && pchIconBaseName[0] )
	{
		m_pIcon->SetImage( CFmtStr( notification.bComplete ? "achievements/%s" : "achievements/%s_bw", pchIconBaseName ) );
	}

	// resize the panel so it always looks good

	// get fonts
	HFont hFontHeading = m_pLabelHeading->GetFont();
	HFont hFontTitle = m_pLabelTitle->GetFont();
	// determine how wide the text strings are
	int iHeadingWidth = UTIL_ComputeStringWidth( hFontHeading, notification.szHeading );
	int iTitleWidth = UTIL_ComputeStringWidth( hFontTitle, notification.szTitle );
	// use the widest string
	int iTextWidth = MAX( iHeadingWidth, iTitleWidth );
	// don't let it be insanely wide
	iTextWidth = MIN( iTextWidth, XRES( 300 ) );
	int iIconWidth = m_pIcon->GetWide();
	int iSpacing = XRES( 10 );
	int iPanelWidth = iSpacing + iIconWidth + iSpacing + iTextWidth + iSpacing;
	int iPanelX = GetWide() - iPanelWidth;
	int iIconX = iPanelX + iSpacing;
	int iTextX = iIconX + iIconWidth + iSpacing;
	// resize all the elements
	SetXAndWide( m_pPanelBackground, iPanelX, iPanelWidth );
	SetXAndWide( m_pIcon, iIconX, iIconWidth );
	SetXAndWide( m_pLabelHeading, iTextX, iTextWidth );
	SetXAndWide( m_pLabelTitle, iTextX, iTextWidth );

	vgui::surface()->PlaySound( "ui/hint.wav" );
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( this, "AchievementNotificationBounce" );

	m_queueNotification.Remove( m_queueNotification.Head() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementNotificationPanel::SetXAndWide( Panel *pPanel, int x, int wide )
{
	int xCur, yCur;
	pPanel->GetPos( xCur, yCur );
	pPanel->SetPos( x, yCur );
	pPanel->SetWide( wide );
}

CON_COMMAND_F( achievement_notification_test, "Test the hud notification UI", FCVAR_CHEAT /*| FCVAR_DEVELOPMENTONLY*/ )
{
	static int iCount=0;

	CAchievementNotificationPanel *pPanel = GET_HUDELEMENT( CAchievementNotificationPanel );
	if ( pPanel )
	{
		if ( 0 == ( iCount % 2 ) )
			pPanel->AddNotification( "TF_GET_HEADSHOTS", g_pVGuiLocalize->Find( "#GameUI_Achievement_Progress" ), L"Test Notification Message A (1/10)", false );
		else
			pPanel->AddNotification( "TF_GET_HEADSHOTS", g_pVGuiLocalize->Find( "#GameUI_Achievement_Awarded" ), L"Test Message B", true );
	}

	iCount++;
}