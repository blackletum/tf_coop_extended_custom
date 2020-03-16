//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "tf_optionsdialog.h"
#include "tf_mainmenu.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/QueryBox.h"
#include "controls/tf_advbutton.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"

#include "KeyValues.h"
#include "tf_optionsmousepanel.h"
#include "tf_optionskeyboardpanel.h"
#include "tf_optionsaudiopanel.h"
#include "tf_optionsvideopanel.h"
#include "tf_optionsadvancedpanel.h"
#include "lfe_optionsvguipanel.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CTFOptionsDialog::CTFOptionsDialog(vgui::Panel *parent, const char *panelName) : CTFDialogPanelBase(parent, panelName)
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFOptionsDialog::~CTFOptionsDialog()
{}


bool CTFOptionsDialog::Init()
{
	BaseClass::Init();

	m_pPanels.SetSize(OPTION_PANEL_COUNT);

	AddPanel(new CTFOptionsAdvancedPanel(this, "MultiplayerOptions"), OPTION_PANEL_ADV);
	AddPanel(new CTFOptionsMousePanel(this, "MouseOptions"), OPTION_PANEL_MOUSE);
	AddPanel(new CTFOptionsKeyboardPanel(this, "KeyboardOptions"), OPTION_PANEL_KEYBOARD);
	AddPanel(new CTFOptionsAudioPanel(this, "AudioOptions"), OPTION_PANEL_AUDIO);
	AddPanel(new CTFOptionsVideoPanel(this, "VideoOptions"), OPTION_PANEL_VIDEO);
	AddPanel(new CTFOptionsVGUIPanel(this, "VGUIOptions"), OPTION_PANEL_VGUI);

	m_pApplyButton = new CTFButton( this, "Apply", "" );
	m_pDefaultsButton = new CTFButton( this, "Defaults", "" );

	m_pOptionsCurrent = OPTION_PANEL_ADV;

	return true;
}


void CTFOptionsDialog::AddPanel(CTFDialogPanelBase *m_pPanel, int iPanel)
{
	m_pPanels[iPanel] = m_pPanel;
	m_pPanel->SetEmbedded(true);
	m_pPanel->Hide();
}

void CTFOptionsDialog::SetCurrentPanel(OptionPanel pCurrentPanel)
{
	if (m_pOptionsCurrent == pCurrentPanel)
		return;
	GetPanel(m_pOptionsCurrent)->Hide();
	GetPanel(pCurrentPanel)->Show();
	m_pOptionsCurrent = pCurrentPanel;

	m_pDefaultsButton->SetVisible( ( pCurrentPanel == OPTION_PANEL_KEYBOARD ) );
}

CTFDialogPanelBase*	CTFOptionsDialog::GetPanel(int iPanel)
{
	return m_pPanels[iPanel];
}

void CTFOptionsDialog::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/UI/main_menu/OptionsDialog.res");
}

void CTFOptionsDialog::Show()
{
	BaseClass::Show();
	GetPanel(m_pOptionsCurrent)->Show();

	for (int i = 0; i < OPTION_PANEL_COUNT; i++)
		GetPanel(i)->OnCreateControls();

	// Disable apply button, it'll get enabled once any controls change.
	m_pApplyButton->SetEnabled( false );
}

void CTFOptionsDialog::Hide()
{
	BaseClass::Hide();
	GetPanel(m_pOptionsCurrent)->Hide();

	for (int i = 0; i < OPTION_PANEL_COUNT; i++)
		GetPanel(i)->OnDestroyControls();
};

void CTFOptionsDialog::OnCommand(const char* command)
{
	if (!Q_strcmp(command, "vguicancel"))
	{
		OnCancelPressed();
	}
	else if (!stricmp(command, "Ok"))
	{
		OnOkPressed();
	}
	else if (!stricmp(command, "Apply"))
	{
		OnApplyPressed();
	}
	else if (!stricmp(command, "DefaultsOK"))
	{
		OnDefaultPressed();
	}
	else if (!Q_strcmp(command, "newoptionsadv"))
	{
		SetCurrentPanel( OPTION_PANEL_ADV );
		MAINMENU_ROOT->HidePanel( CURRENT_MENU );
	}
	else if (!Q_strcmp(command, "newoptionsmouse"))
	{
		SetCurrentPanel( OPTION_PANEL_MOUSE );
		MAINMENU_ROOT->HidePanel( CURRENT_MENU );
	}
	else if (!Q_strcmp(command, "newoptionskeyboard"))
	{
		SetCurrentPanel( OPTION_PANEL_KEYBOARD );
		MAINMENU_ROOT->HidePanel( CURRENT_MENU );
	}
	else if (!Q_strcmp(command, "newoptionsaudio"))
	{
		SetCurrentPanel( OPTION_PANEL_AUDIO );
		MAINMENU_ROOT->HidePanel( CURRENT_MENU );
	}
	else if (!Q_strcmp(command, "newoptionsvideo"))
	{
		SetCurrentPanel( OPTION_PANEL_VIDEO );
		MAINMENU_ROOT->HidePanel( CURRENT_MENU );
	}
	else if (!Q_strcmp(command, "newoptionsvgui"))
	{
		SetCurrentPanel( OPTION_PANEL_VGUI );
		MAINMENU_ROOT->HidePanel( CURRENT_MENU );
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsDialog::OnOkPressed()
{
	OnApplyPressed();
	Hide();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsDialog::OnApplyPressed()
{
	m_pApplyButton->SetEnabled( false );

	for (int i = 0; i < OPTION_PANEL_COUNT; i++)
		GetPanel(i)->OnApplyChanges();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsDialog::OnCancelPressed()
{
	for (int i = 0; i < OPTION_PANEL_COUNT; i++)
		GetPanel(i)->OnResetData();

	Hide();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsDialog::OnDefaultPressed()
{
	for (int i = 0; i < OPTION_PANEL_COUNT; i++)
		GetPanel(i)->OnSetDefaults();
}

//-----------------------------------------------------------------------------
// Purpose: Brings the dialog to the fore
//-----------------------------------------------------------------------------
void CTFOptionsDialog::Activate()
{
	//BaseClass::Activate();
	//EnableApplyButton(false);
}

//-----------------------------------------------------------------------------
// Purpose: Opens the dialog
//-----------------------------------------------------------------------------
void CTFOptionsDialog::Run()
{
	//SetTitle("#GameUI_Options", true);
	//Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the GameUI is hidden
//-----------------------------------------------------------------------------
void CTFOptionsDialog::OnGameUIHidden()
{
	// tell our children about it
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *pChild = GetChild(i);
		if (pChild)
		{
			PostMessage(pChild, new KeyValues("GameUIHidden"));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Enable apply button when something is changed.
//-----------------------------------------------------------------------------
void CTFOptionsDialog::OnApplyButtonEnabled( void )
{
	if ( !m_pApplyButton->IsEnabled() )
	{
		m_pApplyButton->SetEnabled( true );
	}
}

void CTFOptionsDialog::OnKeyCodeTyped( vgui::KeyCode code )
{
	// force ourselves to be closed if the escape key it pressed
	if ( code == KEY_ESCAPE )
	{
		Hide();
		MAINMENU_ROOT->ShowPanel( CURRENT_MENU );
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}