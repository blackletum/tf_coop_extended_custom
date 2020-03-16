#ifndef ITEM_MODEL_PANEL_H
#define ITEM_MODEL_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Image.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "tf_controls.h"
#include "basemodel_panel.h"

class CEmbeddedItemModelPanel;

class CItemModelPanel : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CItemModelPanel, vgui::EditablePanel );

	CItemModelPanel( vgui::Panel* parent, const char* name );
	~CItemModelPanel();
	virtual void ApplySchemeSettings( vgui::IScheme* pScheme );
	virtual void ApplySettings( KeyValues* inResourceData );
	virtual void PerformLayout();
	virtual void OnCursorEntered();
	virtual void OnCursorExited();
	virtual void OnMouseReleased( vgui::MouseCode code );
	virtual void SetWeapon( void* p, bool b, int i) {};
	virtual void SetItem( CEconItemView* pItem );
	virtual CEconItemView* GetItem() { return m_pItem; }
private:
	CPanelAnimationVarAliasType( int, m_iTextYPos, "text_ypos", "0", "proportional_int" );

	vgui::EditablePanel* m_pMainContentsContainer = NULL;
	CEmbeddedItemModelPanel* m_pItemModelPanel = NULL;
	CExLabel* m_pNameLabel = NULL;
	CExLabel* m_pEquippedLabel = NULL;
	CEconItemView* m_pItem = NULL;
	vgui::IBorder* m_pBorder = NULL;
	vgui::IBorder* m_pBorderSelected = NULL;
};

//Currently only uses backpack textures
class CEmbeddedItemModelPanel : public CBaseModelPanel
{
public:
	DECLARE_CLASS_SIMPLE( CEmbeddedItemModelPanel, CBaseModelPanel );

	CEmbeddedItemModelPanel( vgui::Panel* parent, const char* name );
	~CEmbeddedItemModelPanel();

	virtual void PerformLayout();
	virtual void SetItem( CEconItemView* pItem );
	virtual void Paint();
private:
	CEconItemView* m_pItem = NULL;
	int m_iBackPackTexture = -1;
	bool m_bUseModel = false;
};


//tf2c
class CItemModelPanelHud : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CItemModelPanelHud, vgui::EditablePanel );

public:
	CItemModelPanelHud( Panel *parent, const char* name );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout( void );

	virtual void SetWeapon( C_BaseCombatWeapon *pWeapon, int iBorderStyle = -1, int ID = -1 );
	virtual void SetWeapon( CEconItemDefinition *pItemDefinition, int iBorderStyle = -1, int ID = -1 );
	virtual void SetWeapon( CEconItemView *pItem, int iBorderStyle = 0, int ID = -1 );
	virtual CEconItemView* GetItem() { return m_pItem; }

private:
	CEconItemView* m_pItem = NULL;
	C_BaseCombatWeapon	*m_pWeapon;
	vgui::Label			*m_pWeaponName;
	vgui::Label			*m_pSlotID;
	vgui::ImagePanel	*m_pWeaponImage;
	vgui::HFont			m_pDefaultFont;
	vgui::HFont			m_pSelectedFont;
	vgui::HFont			m_pNumberDefaultFont;
	vgui::HFont			m_pNumberSelectedFont;
	vgui::IBorder		*m_pDefaultBorder;
	vgui::IBorder		*m_pSelectedRedBorder;
	vgui::IBorder		*m_pSelectedBlueBorder;
	int					m_iBorderStyle;
	int					m_ID;
	bool				m_bOldStyleIcon;
};

#endif