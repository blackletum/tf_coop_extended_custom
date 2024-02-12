#ifndef ITEM_SELECTION_PANEL_H
#define ITEM_SELECTION_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui/ILocalize.h>
#include "tf_controls.h"
#include "tf_playermodelpanel.h"
#include "item_model_panel.h"
#include "utlvector.h"

class CEquipSlotItemSelectionPanel : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CEquipSlotItemSelectionPanel, vgui::EditablePanel );

	CEquipSlotItemSelectionPanel( vgui::Panel* parent, int iSlot = LOADOUT_POSITION_PRIMARY );
	~CEquipSlotItemSelectionPanel();
	virtual void ApplySchemeSettings( vgui::IScheme* pScheme );
	virtual void ApplySettings( KeyValues* inResourceData );
	virtual void CreateItemPanels();
	virtual void AddNewItemPanel( int index );
	virtual void ApplyKVsToItemPanels();
	virtual void OnCommand(const char *command);
	virtual int GetNumItemPanels();
	virtual void PerformLayout();
	void SetTeam( int iTeam );
	void SetClass( int iClass );

private:

	CPanelAnimationVarAliasType( int, m_iItemYPos, "item_ypos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemYDelta, "item_ydelta", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemBackpackOffCenterX, "item_backpack_offcenter_x", "0", "proportional_int" );
	MESSAGE_FUNC_PTR( OnItemPanelEntered, "ItemPanelEntered", panel );
	MESSAGE_FUNC_PTR( OnItemPanelExited, "ItemPanelExited", panel );
	MESSAGE_FUNC_PTR( OnItemPanelMouseReleased, "ItemPanelMouseReleased", panel );

	CExLabel* m_pCaratLabel = NULL;
	CExLabel* m_pClassLabel = NULL;
	CExLabel* m_pItemSlotLabel = NULL;
	CUtlVector<CItemModelPanel *> m_pItemModelPanels;
	KeyValues* m_pItemModelPanelKeyValues = NULL;
	KeyValues* m_pItemModelPanelSelectionKeyValues = NULL;
	vgui::Panel* m_pMouseOverItemPanel;
	int	m_iTeam = TF_TEAM_RED;
	int m_iClass = TF_CLASS_SCOUT;
	int m_iSlot;
};

#endif 