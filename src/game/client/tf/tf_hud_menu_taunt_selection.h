//========= Copyright © 1996-2019, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TF_HUD_MENU_TAUNT_SELECTION_H
#define TF_HUD_MENU_TAUNT_SELECTION_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include "IconPanel.h"
#include "tf_controls.h"
#include "item_model_panel.h"
#include "utlvector.h"

using namespace vgui;

#define ALL_BUILDINGS	-1

class CHudMenuTauntSelection : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudMenuTauntSelection, EditablePanel );

public:
	CHudMenuTauntSelection( const char *pElementName );

	virtual void	ApplySchemeSettings( IScheme *scheme );
	virtual void	PerformLayout();

	virtual bool	ShouldDraw( void );
	virtual void	SetVisible( bool state );

	int				HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );

	virtual int		GetRenderGroupPriority() { return 50; }

	virtual void	FireGameEvent( IGameEvent *event );

	void			UpdateItemModelPanels( void );
private:

	CUtlVector<CItemModelPanelHud *> m_pItemModelPanels;
};

#endif	// TF_HUD_MENU_TAUNT_SELECTION_H