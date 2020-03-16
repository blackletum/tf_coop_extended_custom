//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_dialogpanelbase.h"
#include <vgui/KeyCode.h>

class CTFButton;

enum OptionPanel
{
	OPTION_PANEL_ADV,
	OPTION_PANEL_MOUSE,
	OPTION_PANEL_KEYBOARD,
	OPTION_PANEL_AUDIO,
	OPTION_PANEL_VIDEO,
	OPTION_PANEL_VGUI,

	OPTION_PANEL_COUNT
};

//-----------------------------------------------------------------------------
// Purpose: Holds all the game option pages
//-----------------------------------------------------------------------------
class CTFOptionsDialog : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFOptionsDialog, CTFDialogPanelBase);

public:
	CTFOptionsDialog(vgui::Panel *parent, const char *panelName);
	~CTFOptionsDialog();
	bool Init();
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void Run();
	virtual void Activate();
	virtual void OnCommand(const char *command);
	virtual void Show();
	virtual void Hide();

	MESSAGE_FUNC(OnCancelPressed, "CancelPressed");
	MESSAGE_FUNC(OnOkPressed, "OkPressed");
	MESSAGE_FUNC(OnApplyPressed, "OnApplyPressed");	
	MESSAGE_FUNC(OnDefaultPressed, "OnDefaultPressed");
	MESSAGE_FUNC(OnGameUIHidden, "GameUIHidden");	// called when the GameUI is hidden
	MESSAGE_FUNC(OnApplyButtonEnabled, "ApplyButtonEnable" );

	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	void SetCurrentPanel(OptionPanel pCurrentPanel);
private:
	void AddPanel(CTFDialogPanelBase *m_pPanel, int iPanel);
	CTFDialogPanelBase* GetPanel(int iPanel);
	CUtlVector<CTFDialogPanelBase*>	m_pPanels;
	OptionPanel	m_pOptionsCurrent;

	CTFButton *m_pApplyButton;
	CTFButton *m_pDefaultsButton;
};


#define OPTIONS_MAX_NUM_ITEMS 15

struct OptionData_t;


#endif // OPTIONSDIALOG_H