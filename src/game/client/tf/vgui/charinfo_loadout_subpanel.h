#ifndef CHAR_INFO_LOADOUT_SUBPANEL_H
#define CHAR_INFO_LOADOUT_SUBPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/PropertyPage.h>
#include "tf_controls.h"
#include "class_loadout_panel.h"

using namespace vgui;

class CCharInfoLoadoutSubPanel : public vgui::PropertyPage
{
public:
	DECLARE_CLASS_SIMPLE( CCharInfoLoadoutSubPanel, vgui::PropertyPage );

	CCharInfoLoadoutSubPanel( vgui::Panel *parent );
	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void OnCommand( const char* command );
	void UpdateModelPanels( bool unknown );
	bool IsLoadoutActive() { return m_pClassLoadout->IsVisible(); }
	void CloseLoadout();
private:
	int m_iCurrentClass = TF_CLASS_UNDEFINED;
	int m_iActivePanel = -1;
	CClassLoadoutPanel *m_pClassLoadout;
};

#endif