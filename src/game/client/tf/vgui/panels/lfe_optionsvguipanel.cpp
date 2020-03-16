//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "lfe_optionsvguipanel.h"
#include "tf_mainmenu.h"
#include "controls/tf_scriptobject.h"
#include "controls/tf_cvartogglecheckbutton.h"
#include "controls/tf_cvarslider.h"
#include "controls/tf_advpanellistpanel.h"
#include "controls/tf_advbutton.h"
#include "vgui_controls/ComboBox.h"
#include <KeyValues.h>
#include <vgui/IScheme.h>
#include "tier1/convar.h"
#include <stdio.h>
#include <vgui_controls/TextEntry.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

CTFOptionsVGUIPanel::CTFOptionsVGUIPanel(vgui::Panel *parent, const char *panelName) : CTFDialogPanelBase(parent, panelName)
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFOptionsVGUIPanel::~CTFOptionsVGUIPanel()
{
}

bool CTFOptionsVGUIPanel::Init()
{
	BaseClass::Init();

	m_pListPanel = new CPanelListPanel(this, "PanelListPanel");
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: sets background color & border
//-----------------------------------------------------------------------------
void CTFOptionsVGUIPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/UI/main_menu/OptionsVGUI.res");
}

void CTFOptionsVGUIPanel::OnCommand(const char* command)
{
	BaseClass::OnCommand(command);
}

void CTFOptionsVGUIPanel::CreateControls()
{
	BaseClass::CreateControls();

	pTitleMainMenu = new Label( this, "DescTextTitle", "#MainMenu" );
	m_pMainMenuConfirmationQuit = new CTFCvarToggleCheckButton( this, "MainConfirmQuit", "#LFEOption_ConfirmationQuit", "lfe_ui_confirmation_quit" );
	m_pMainMenuConfirmationDisconnect = new CTFCvarToggleCheckButton( this, "MainConfirmDisconnect", "#LFEOption_ConfirmationDisconnect", "lfe_ui_confirmation_disconnect" );
	m_pMainMenuShowNews = new CTFCvarToggleCheckButton( this, "MainShowNews", "#LFEOption_ShowBlog", "lfe_ui_mainmenu_news" );

	pTitleLoadingScreen = new Label( this, "DescTextTitle", "#LFEOption_Title_LoadingScreen" );
	//m_pLoadingProgressStatSummary = new CTFCvarToggleCheckButton( this, "LoadingStatSummary", "#LFEOption_StatSummary", "lfe_ui_loadingprogress_statsummary" );
	m_pLoadingProgressTips = new CTFCvarToggleCheckButton( this, "LoadingTips", "#LFEOption_LoadingTips", "lfe_ui_loadingprogress_tips" );
	m_pLoadingProgressBanner = new CTFCvarToggleCheckButton( this, "LoadingBanner", "#LFEOption_LoadingBanner", "lfe_ui_loadingprogress_banner" );

	AddControl(pTitleMainMenu, O_CATEGORY);
	AddControl(m_pMainMenuConfirmationQuit, O_BOOL);
	AddControl(m_pMainMenuConfirmationDisconnect, O_BOOL);
	AddControl(m_pMainMenuShowNews, O_BOOL);

	AddControl(pTitleLoadingScreen, O_CATEGORY);
	//AddControl(m_pLoadingProgressStatSummary, O_BOOL);
	AddControl(m_pLoadingProgressTips, O_BOOL);
	AddControl(m_pLoadingProgressBanner, O_BOOL);

	UpdatePanels();
}

void CTFOptionsVGUIPanel::DestroyControls()
{
	BaseClass::DestroyControls();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsVGUIPanel::OnResetData()
{
	BaseClass::OnResetData();

	m_pMainMenuConfirmationQuit->Reset();
	m_pMainMenuConfirmationDisconnect->Reset();
	m_pMainMenuShowNews->Reset();

	//m_pLoadingProgressStatSummary->Reset();
	m_pLoadingProgressTips->Reset();
	m_pLoadingProgressBanner->Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsVGUIPanel::OnApplyChanges()
{
	BaseClass::OnApplyChanges();

	m_pMainMenuConfirmationQuit->ApplyChanges();
	m_pMainMenuConfirmationDisconnect->ApplyChanges();
	m_pMainMenuShowNews->ApplyChanges();

	//m_pLoadingProgressStatSummary->ApplyChanges();
	m_pLoadingProgressTips->ApplyChanges();
	m_pLoadingProgressBanner->ApplyChanges();

	engine->ClientCmd_Unrestricted("lfe_mainmenu_reload");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsVGUIPanel::OnControlModified(Panel *panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
	//if (panel == m_pJoystickCheckBox || panel == m_pMouseAccelCheckBox)
	UpdatePanels();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsVGUIPanel::UpdatePanels()
{
	/*bool bEnabled = m_pMouseAccelCheckBox->IsSelected();
	m_pMouseAccelSlider->SetVisible(bEnabled);


	bEnabled = m_pJoystickCheckBox->IsSelected();
	m_pReverseJoystickCheckBox->SetVisible( bEnabled );
	m_pJoystickSouthpawCheckBox->SetVisible( bEnabled );
	m_pJoyYawSensitivitySlider->SetVisible( bEnabled );
	m_pJoyPitchSensitivitySlider->SetVisible( bEnabled );*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsVGUIPanel::OnTextChanged(Panel *panel)
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsVGUIPanel::UpdateSensitivityLabel()
{

}
