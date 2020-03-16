//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "lfe_quitconfirmation.h"
#include "vgui_controls/Label.h"
#include "vgui/ISurface.h"
#include "controls/tf_advbutton.h"
#include "cdll_util.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFQuitConfirmation::CTFQuitConfirmation( vgui::Panel* parent, const char *panelName ) : CTFDialogPanelBase( parent, panelName )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFQuitConfirmation::~CTFQuitConfirmation()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFQuitConfirmation::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "resource/UI/main_menu/QuitConfirmation.res" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFQuitConfirmation::OnCommand( const char* command )
{
	if (!Q_strcmp(command, "vguicancel"))
	{
		PostActionSignal( new KeyValues( "CancelPressed" ) );
		Hide();
	}
	else if (!stricmp(command, "Ok"))
	{
		if ( InGame() )
			engine->ExecuteClientCmd( "disconnect" );
		else
			engine->ExecuteClientCmd( "quit" );
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFQuitConfirmation::Show()
{
	BaseClass::Show();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFQuitConfirmation::Hide()
{
	BaseClass::Hide();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFQuitConfirmation::DefaultLayout()
{
	BaseClass::DefaultLayout();
	Hide();

	CExLabel *pLblTitle = dynamic_cast< CExLabel* >( FindChildByName( "LblTitle" ) );
	if ( pLblTitle )
		pLblTitle->SetText( "#MMenu_PromptQuit_Title" );

	CExLabel *pLblMessage = dynamic_cast< CExLabel* >( FindChildByName( "LblMessage" ) );
	if ( pLblMessage )
		pLblMessage->SetText( "#MMenu_PromptQuit_Body" );

	CTFButton *pOkButton = dynamic_cast< CTFButton* >( FindChildByName( "BtnOk" ) );
	if ( pOkButton )
		pOkButton->SetText( "#TF_Quit_Title" );

	CTFButton *pCancelButton = dynamic_cast< CTFButton* >( FindChildByName( "BtnCancel" ) );
	if ( pCancelButton )
		pCancelButton->SetText( "#TF_Cancel_NoKey" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFQuitConfirmation::GameLayout()
{
	BaseClass::GameLayout();
	Hide();

	CExLabel *pLblTitle = dynamic_cast< CExLabel* >( FindChildByName( "LblTitle" ) );
	if ( pLblTitle )
		pLblTitle->SetText( "#TF_MM_Disconnect_Title" );

	CExLabel *pLblMessage = dynamic_cast< CExLabel* >( FindChildByName( "LblMessage" ) );
	if ( pLblMessage )
		pLblMessage->SetText( "#TF_MM_Disconnect" );

	CTFButton *pOkButton = dynamic_cast< CTFButton* >( FindChildByName( "BtnOk" ) );
	if ( pOkButton )
		pOkButton->SetText( "#TF_Disconnect" );

	CTFButton *pCancelButton = dynamic_cast< CTFButton* >( FindChildByName( "BtnCancel" ) );
	if ( pCancelButton )
		pCancelButton->SetText( "#TF_Cancel_NoKey" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFQuitConfirmation::OnKeyCodeTyped( vgui::KeyCode code )
{
	BaseClass::OnKeyCodeTyped( code );
}