//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//========================================================================//

#ifndef ACHIEVEMENT_NOTIFICATION_PANEL_H
#define ACHIEVEMENT_NOTIFICATION_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include "hudelement.h"
#include "tf_controls.h"
#include "c_tf_player.h"

using namespace vgui;

class CExLabel;

class CAchievementNotificationPanel : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CAchievementNotificationPanel, vgui::EditablePanel );

public:
	CAchievementNotificationPanel( const char *pElementName );

	virtual void	Init();
	virtual void	ApplySchemeSettings( IScheme *scheme );
	virtual bool	ShouldDraw( void );
	virtual void	PerformLayout( void );
	virtual void	LevelInit( void ) { m_flHideTime = 0; }
	virtual void	FireGameEvent( IGameEvent * event );
	virtual void	OnTick( void );

	void AddNotification( const char *szIconBaseName, const wchar_t *pHeading, const wchar_t *pTitle, bool bComplete = false );

private:
	void ShowNextNotification();
	void SetXAndWide( Panel *pPanel, int x, int wide );

	float m_flHideTime;

	CExLabel *m_pLabelHeading;
	CExLabel *m_pLabelTitle;
	vgui::EditablePanel *m_pPanelBackground;
	vgui::ImagePanel *m_pIcon;

	struct Notification_t
	{
		char szIconBaseName[255];
		wchar_t szHeading[255];
		wchar_t szTitle[255];
		bool bComplete;
	};

	CUtlLinkedList<Notification_t> m_queueNotification;
};

#endif	// ACHIEVEMENT_NOTIFICATION_PANEL_H