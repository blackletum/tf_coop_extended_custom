//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_QUITCONFIRMATION_H
#define TF_QUITCONFIRMATION_H

#include "tf_dialogpanelbase.h"

class CTFCvarToggleCheckButton;
class CTFButton;
class CExLabel;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFQuitConfirmation : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE( CTFQuitConfirmation, CTFDialogPanelBase );

public:
	CTFQuitConfirmation( vgui::Panel* parent, const char *panelName );
	virtual ~CTFQuitConfirmation();

	void Show();
	void Hide();
	void OnCommand( const char* command );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void DefaultLayout();
	virtual void GameLayout();
	virtual void OnKeyCodeTyped( vgui::KeyCode code );

};

#endif // TF_QUITCONFIRMATION_H