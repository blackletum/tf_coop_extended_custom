//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_ADDONSPANEL_H
#define TF_ADDONSPANEL_H

#include "tf_menupanelbase.h"
#include "steam/steam_api.h"
#include "tf_dialogpanelbase.h"
#include "tf_controls.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFAddonsPanel : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE( CTFAddonsPanel, CTFDialogPanelBase );

public:
	CTFAddonsPanel( vgui::Panel* parent, const char *panelName );
	virtual ~CTFAddonsPanel();

	void Show();
	void Hide();
	void OnCommand( const char* command );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

void UpdateAddonsSearchPaths();

#endif // TF_ADDONSPANEL_H