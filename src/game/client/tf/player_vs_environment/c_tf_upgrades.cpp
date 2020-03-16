//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include "iclientmode.h"
#include "tf_shareddefs.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IImage.h>
#include <vgui_controls/Label.h>

#include "ihudlcd.h"
#include "tf_controls.h"
#include "in_buttons.h"
#include "tf_imagepanel.h"
#include "tf_tips.h"
#include "c_team.h"
#include "c_tf_player.h"
#include "c_tf_upgrades.h"
#include "tf_gamerules.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudUpgradePanel );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudUpgradePanel::CHudUpgradePanel( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudUpgradePanel" )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
	SetMouseInputEnabled( true );

	m_pClassImage = new CTFClassImage( this, "ClassImage" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// load control settings...
	LoadControlSettings( "resource/UI/HudUpgradePanel.res" );

	Panel* pSelectWeaponPanel = FindChildByName( "SelectWeaponPanel" );
	if ( pSelectWeaponPanel )
	{
		m_pSelectWeaponPanel = dynamic_cast< vgui::EditablePanel* >( pSelectWeaponPanel );
		m_pSelectWeaponPanel->SetMouseInputEnabled( true );
	}

	if ( m_pSelectWeaponPanel )
	{
		Panel* pSentryIcon = m_pSelectWeaponPanel->FindChildByName( "SentryIcon" );
		if ( pSentryIcon )
			m_pSentryIcon = dynamic_cast< vgui::ImagePanel* >( pSentryIcon );

		Panel* pClassImage = m_pSelectWeaponPanel->FindChildByName( "ClassImage" );
		if ( pClassImage )
			m_pClassImage = dynamic_cast< CTFClassImage* >( pClassImage );
	}

	CreateItemModelPanel();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::ApplySettings( KeyValues* inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	KeyValues* pModelPanelsKV = inResourceData->FindKey( "modelpanels_kv" );
	if( pModelPanelsKV )
	{
		if ( m_pItemModelPanelKeyValues )
		{
			m_pItemModelPanelKeyValues->deleteThis();
		}

		m_pItemModelPanelKeyValues = new KeyValues( "modelpanels_kv" );
		pModelPanelsKV->CopySubkeys( m_pItemModelPanelKeyValues );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudUpgradePanel::ShouldDraw( void )
{
	// Get the player and active weapon.
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	return pPlayer->m_Shared.InUpgradeZone();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::SetActive( bool bActive )
{
	SetMouseInputEnabled( bActive );
	CHudElement::SetActive( bActive );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::SetVisible( bool bState )
{
	BaseClass::SetVisible( bState );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::OnCommand( const char* command )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();

	if ( V_strnicmp( command, "cancel", 6 ) == 0 )
	{
		CancelUpgrades();
	}
	else if ( V_strnicmp( command, "close", 5 ) == 0 )
	{
		// confirm upgrade
	}
	else if ( V_strnicmp( command, "respec", 6 ) == 0 )
	{
		// refund everything
	}
	else if ( V_strnicmp( command, "PlayerUpgrade", 13 ) == 0 )
	{
	}
	else if ( V_strnicmp( command, "quick_equip_bottle", 18 ) == 0 )
	{
	}
	else if ( V_strnicmp( command, "open_charinfo_direct", 20 ) == 0 )
	{
		engine->ClientCmd( "open_charinfo_direct" );
	}
	else if ( V_strnicmp( command, "nexttip", 7 ) == 0 )
	{
		if ( pPlayer && m_pSelectWeaponPanel )
		{
			int iClass = TF_CLASS_UNDEFINED;
			if ( pPlayer->GetPlayerClass() )
				iClass = pPlayer->GetPlayerClass()->GetClassIndex();

			m_pSelectWeaponPanel->SetDialogVariable( "tiptext", g_TFTips.GetRandomTip( iClass ) );
		}
	}
	else
	{
		return BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudUpgradePanel::PerformLayout( void )
{
	BaseClass::PerformLayout();

	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return;

	int iClass = TF_CLASS_UNDEFINED;
	if ( pPlayer->GetPlayerClass() )
		iClass = pPlayer->GetPlayerClass()->GetClassIndex();

	if ( m_pSelectWeaponPanel )
	{
		if ( m_pClassImage )
			m_pClassImage->SetClass( pPlayer->GetTeamNumber(), iClass, 0 );

		m_pSelectWeaponPanel->SetDialogVariable( "tiptext", g_TFTips.GetRandomTip( iClass ) );

		FOR_EACH_VEC( m_pItemModelPanels, iPanel )
		{
			m_pItemModelPanels[iPanel]->ApplySettings( m_pItemModelPanelKeyValues );

			int ypos, xpos;

			m_pSelectWeaponPanel->GetPos( xpos, ypos );

			ypos += m_iItemPanelYPos + iPanel * m_iItemPanelYDelta;
			xpos += m_iItemPanelXPos + iPanel * m_iItemPanelXDelta;

			m_pItemModelPanels[iPanel]->SetPos( xpos, ypos );

			if ( !m_pItemModelPanels[iPanel]->IsVisible() )
				m_pItemModelPanels[iPanel]->SetVisible( true );

			m_pItemModelPanels[0]->SetItem( GetTFInventory()->GetItem( iClass, LOADOUT_POSITION_PRIMARY, GetTFInventory()->GetWeaponPreset( iClass, LOADOUT_POSITION_PRIMARY ) ) );
			m_pItemModelPanels[1]->SetItem( GetTFInventory()->GetItem( iClass, LOADOUT_POSITION_SECONDARY, GetTFInventory()->GetWeaponPreset( iClass, LOADOUT_POSITION_SECONDARY ) ) );
			m_pItemModelPanels[2]->SetItem( GetTFInventory()->GetItem( iClass, LOADOUT_POSITION_MELEE, GetTFInventory()->GetWeaponPreset( iClass, LOADOUT_POSITION_MELEE ) ) );

			if ( iClass == TF_CLASS_SPY )
			{
				m_pItemModelPanels[0]->SetItem( GetTFInventory()->GetItem( iClass, LOADOUT_POSITION_SECONDARY, GetTFInventory()->GetWeaponPreset( iClass, LOADOUT_POSITION_SECONDARY ) ) );
				m_pItemModelPanels[1]->SetItem( GetTFInventory()->GetItem( iClass, LOADOUT_POSITION_MELEE, GetTFInventory()->GetWeaponPreset( iClass, LOADOUT_POSITION_MELEE ) ) );
				m_pItemModelPanels[2]->SetItem( GetTFInventory()->GetItem( iClass, LOADOUT_POSITION_PDA2, GetTFInventory()->GetWeaponPreset( iClass, LOADOUT_POSITION_PDA2 ) ) );
			}

			if ( iClass == TF_CLASS_ENGINEER )
			{
				if ( !m_pSentryIcon->IsVisible() )
					m_pSentryIcon->SetVisible( true );
			}
			else
			{
				if ( m_pSentryIcon->IsVisible() )
					m_pSentryIcon->SetVisible( false );
			}

			m_pItemModelPanels[iPanel]->InvalidateLayout();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudUpgradePanel::FireGameEvent( IGameEvent *event )
{
	if ( !Q_strcmp( event->GetName(), "post_inventory_application" ) )
	{
		int iUserID = event->GetInt( "userid" );
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pPlayer && pPlayer->GetUserID() == iUserID )
		{
			//PlayerInventoryChanged( pPlayer );
		}
	}
	else if ( !Q_strcmp( event->GetName(), "localplayer_respawn" ) )
	{
		PerformLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::OnTick()
{
	if ( !IsVisible() )
		return;

	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return;

	if ( m_pSelectWeaponPanel )
	{
		m_pSelectWeaponPanel->SetDialogVariable( "credits", pPlayer->m_Shared.GetCurrency() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::CancelUpgrades( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
		pPlayer->m_Shared.SetInUpgradeZone( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::CreateItemModelPanel()
{
	for ( ; m_pItemModelPanels.Count() < 4; )
	{
		CItemModelPanel* pItemPanel = new CItemModelPanel( this, VarArgs( "modelpanel%d", m_pItemModelPanels.Count() ) );
		if ( pItemPanel )
			m_pItemModelPanels.AddToTail( pItemPanel );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudUpgradePanel::UpdateModelPanels( void )
{
	FOR_EACH_VEC( m_pItemModelPanels, iPanel )
	{
		m_pItemModelPanels[iPanel]->ApplySettings( m_pItemModelPanelKeyValues );
		m_pItemModelPanels[iPanel]->InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudUpgradePanel::UpdateItemStatsLabel( void )
{

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudUpgradePanel::SetBorderForItem( CItemModelPanel *pPanel, bool bState )
{
	if ( !pPanel->GetItem() )
		return;

	UpdateItemStatsLabel();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::OnItemPanelEntered( Panel* pPanel )
{
	CItemModelPanel* pItemPanel = (CItemModelPanel* )pPanel;
	if ( !pItemPanel )
		return;

	SetBorderForItem( pItemPanel, true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::OnItemPanelExited( Panel* pPanel )
{
	CItemModelPanel* pItemPanel = (CItemModelPanel* )pPanel;
	if ( !pItemPanel )
		return;

	SetBorderForItem( pItemPanel, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUpgradePanel::OnItemPanelMousePressed( Panel* pPanel )
{
	CItemModelPanel* pItemPanel = (CItemModelPanel* )pPanel;
	if ( !pItemPanel )
		return;

	SetBorderForItem( pItemPanel, false );
}


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
CUpgradeBuyPanel::CUpgradeBuyPanel( vgui::Panel* parent, const char* panelName ) : BaseClass( parent, panelName )
{
	SetProportional( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CUpgradeBuyPanel::~CUpgradeBuyPanel()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CUpgradeBuyPanel::OnCommand( const char* command )
{
	BaseClass::OnCommand( command );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CUpgradeBuyPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// load control settings...
	LoadControlSettings( "resource/UI/UpgradeBuyPanel.res" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CUpgradeBuyPanel::ApplySettings( KeyValues* inResourceData )
{
	BaseClass::ApplySettings( inResourceData );
}
