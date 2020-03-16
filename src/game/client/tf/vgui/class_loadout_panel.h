#ifndef CLASS_LOADOUT_PANEL_H
#define CLASS_LOADOUT_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui/ILocalize.h>
#include "tf_controls.h"
#include "tf_playermodelpanel.h"
#include "item_model_panel.h"
#include "utlvector.h"
#include "item_selection_panel.h"

class CClassLoadoutPanel : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CClassLoadoutPanel, vgui::EditablePanel );

	CClassLoadoutPanel( vgui::Panel* parent );
	~CClassLoadoutPanel();
	virtual void	ApplySchemeSettings( vgui::IScheme* pScheme );
	virtual void	ApplySettings( KeyValues* inResourceData );
	virtual void	CreateItemPanels();
	virtual void	AddNewItemPanel( int index );
	virtual void	ApplyKVsToItemPanels();
	virtual int		GetNumItemPanels();
	virtual void	PerformLayout();
	virtual bool	InSelectionPanel() { return m_pItemSelectionPanel != NULL; }
	virtual void	CloseSelectionPanel();
	void			SetTeam( int iTeam );
	void			SetClass( int iClass );
	void			SetSelectedItemPanel( CItemModelPanel* pPanel );
	virtual void	OnCommand( const char* command );
	virtual void	OnKeyCodePressed( vgui::KeyCode code );
private:

	CPanelAnimationVarAliasType( int, m_iItemXPosOffCenterA, "item_xpos_offcenter_a", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemXPosOffCenterB, "item_xpos_offcenter_b", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemYPos, "item_ypos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemYDelta, "item_ydelta", "0", "proportional_int" );
	MESSAGE_FUNC_PTR( OnItemPanelEntered, "ItemPanelEntered", panel );
	MESSAGE_FUNC_PTR( OnItemPanelExited, "ItemPanelExited", panel );
	MESSAGE_FUNC_PTR( OnItemPanelMouseReleased, "ItemPanelMouseReleased", panel );
	MESSAGE_FUNC( OnCloseItemSelection, "CloseItemSelection" );

	CExLabel* m_pCaratLabel = NULL;
	CExLabel* m_pTauntCaratLabel = NULL;
	CExLabel* m_pClassLabel = NULL;
	CExLabel* m_pTauntLabel = NULL;
	CTFPlayerModelPanel* m_pTFModelPanel = NULL;
	CEquipSlotItemSelectionPanel* m_pItemSelectionPanel = NULL;
	CUtlVector<CItemModelPanel *> m_pItemModelPanels;
	KeyValues* m_pItemModelPanelKeyValues = NULL;
	vgui::Panel* m_pMouseOverItemPanel;
	int		m_iTeam = TF_TEAM_RED;
	int		m_iClass = TF_CLASS_SCOUT;
	bool	m_bTauntLoadout = false;
};

#endif 