//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef OPTIONS_SUB_VGUI_H
#define OPTIONS_SUB_VGUI_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_dialogpanelbase.h"

class CPanelListPanel;
class CTFButton;
class CTFCvarToggleCheckButton;
class CTFCvarSlider;

namespace vgui
{
    class Label;
    class Panel;
}

//-----------------------------------------------------------------------------
// Purpose: Mouse Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class CTFOptionsVGUIPanel : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFOptionsVGUIPanel, CTFDialogPanelBase);

public:
	CTFOptionsVGUIPanel(vgui::Panel* parent, const char *panelName);
	~CTFOptionsVGUIPanel();
	virtual bool Init();
	virtual void OnResetData();
	virtual void OnApplyChanges();
	virtual void OnCommand(const char *command);
protected:
    void ApplySchemeSettings(vgui::IScheme *pScheme);
	void CreateControls();
	void DestroyControls();
	void UpdateSensitivityLabel();
	void UpdatePanels();

private:
	MESSAGE_FUNC_PTR( OnControlModified, "ControlModified", panel );
    MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	MESSAGE_FUNC_INT( OnCheckButtonChecked, "CheckButtonChecked", panel )
	{
		OnControlModified( NULL );
	}

	vgui::Label					*pTitleMainMenu;
	CTFCvarToggleCheckButton		*m_pMainMenuConfirmationQuit;
	CTFCvarToggleCheckButton		*m_pMainMenuConfirmationDisconnect;
	CTFCvarToggleCheckButton		*m_pMainMenuShowNews;

	vgui::Label					*pTitleLoadingScreen;
	//CTFCvarToggleCheckButton		*m_pLoadingProgressStatSummary;
	CTFCvarToggleCheckButton		*m_pLoadingProgressTips;
	CTFCvarToggleCheckButton		*m_pLoadingProgressBanner;
};



#endif // OPTIONS_SUB_VGUI_H