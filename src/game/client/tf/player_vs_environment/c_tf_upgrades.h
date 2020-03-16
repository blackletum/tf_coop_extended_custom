//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_C_UPGRADES_H
#define TF_C_UPGRADES_H
#ifdef _WIN32
#pragma once
#endif

#include "hud.h"
#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>
#include "item_model_panel.h"
#include "utlvector.h"
#include "tf_inventory.h"
#include "tf_hud_playerstatus.h"

//-----------------------------------------------------------------------------
// Purpose:  Displays weapon ammo data
//-----------------------------------------------------------------------------
class CHudUpgradePanel : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudUpgradePanel, vgui::EditablePanel );

public:

	CHudUpgradePanel( const char *pElementName );

	virtual void	ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void	ApplySettings( KeyValues* inResourceData );
	virtual void	PerformLayout( void );
	virtual bool	ShouldDraw( void );

	virtual void	OnCommand( const char* command );
	virtual void	FireGameEvent( IGameEvent *event );

	void			CreateItemModelPanel( void );
	void			CancelUpgrades( void );
	//GetLocalPlayerBottleFromInventory

	//void		PlayerInventoryChanged( C_TFPlayer *pPlayer )
	//void		QuickEquipBottle( void )

	virtual void	SetActive( bool bActive );
	virtual void	SetVisible( bool bState );

	void	SetBorderForItem( CItemModelPanel *pPanel, bool bState );

	//void	UpdateButtonStates( int, int, int );
	//void	UpdateHighlights( void );
	void	UpdateItemStatsLabel( void );
	//void	UpdateJoystickControls( void );
	void	UpdateModelPanels( void );
	//void	UpdateMouseOverHighlight( void );
	//void	UpdateUpgradeButtons( void );
	//void	UpgradeItemInSlot( int iSlot );

protected:

	virtual void OnTick();

private:

	CPanelAnimationVarAliasType( int, m_iItemPanelXPos, "itempanel_xpos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemPanelYPos, "itempanel_ypos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemPanelXDelta, "itempanel_xdelta", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iItemPanelYDelta, "itempanel_ydelta", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iUpgradeBuyPanelXPos, "upgradebuypanel_xpos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iUpgradeBuyPanelYPos, "upgradebuypanel_ypos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iUpgradeBuyPanelDelta, "upgradebuypanel_delta", "0", "proportional_int" );

	MESSAGE_FUNC_PTR( OnItemPanelEntered, "ItemPanelEntered", panel );
	MESSAGE_FUNC_PTR( OnItemPanelExited, "ItemPanelExited", panel );
	MESSAGE_FUNC_PTR( OnItemPanelMousePressed, "ItemPanelMouseReleased", panel );

	CUtlVector<CItemModelPanel *> m_pItemModelPanels;
	KeyValues* m_pItemModelPanelKeyValues = NULL;

	vgui::EditablePanel			*m_pSelectWeaponPanel;
	CTFClassImage				*m_pClassImage;
	vgui::ImagePanel			*m_pSentryIcon;
};


class CUpgradeBuyPanel : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CUpgradeBuyPanel, vgui::EditablePanel );

	CUpgradeBuyPanel( vgui::Panel* parent, const char* panelName );
	virtual ~CUpgradeBuyPanel();

	virtual void OnCommand( const char* command );

	//PerformLayout();

	//void	SetNumLevelImages( int i );
	//void	UpdateImages( int i );
	//void	ValidateUpgradeStepData( void );
protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void ApplySettings( KeyValues* inResourceData );
private:
	CPanelAnimationVarAliasType( int, m_iUpgradeButtonXPos, "upgradebutton_xpos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iUpgradeButtonYPos, "upgradebutton_ypos", "0", "proportional_int" );
/*m_rgbaArmedBG
m_rgbaArmedFG
m_rgbaDefaultBG
m_rgbaDefaultFG
m_rgbaDepressedBG
m_rgbaDepressedFG
m_rgbaDisabledBG
m_rgbaDisabledFG
m_rgbaSelectedBG
m_rgbaSelectedFG*/
};

#endif	// TF_C_UPGRADES_H