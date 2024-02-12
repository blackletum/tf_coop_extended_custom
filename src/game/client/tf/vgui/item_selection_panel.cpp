#include "cbase.h"
#include "item_selection_panel.h"
#include "character_info_panel.h"
#include "tf_inventory.h"
#include "tf_mainmenu.h"
#include "ienginevgui.h"


CEquipSlotItemSelectionPanel::CEquipSlotItemSelectionPanel( vgui::Panel* parent, int iSlot ) : vgui::EditablePanel( parent, "ItemSelectionPanel" )
{
	m_pMouseOverItemPanel = new CItemModelPanel( this, "mouseoveritempanel" );
	m_iSlot = iSlot;
	
}

CEquipSlotItemSelectionPanel::~CEquipSlotItemSelectionPanel()
{
}

void CEquipSlotItemSelectionPanel::ApplySchemeSettings( vgui::IScheme* pScheme )
{
	LoadControlSettings( "Resource/UI/itemselectionpanel.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	Panel* pCaratLabel = FindChildByName( "CaratLabel" );
	if ( pCaratLabel )
		m_pCaratLabel = dynamic_cast< CExLabel* >( pCaratLabel );

	Panel* pClassLabel = FindChildByName( "ClassLabel" );
	if ( pClassLabel )
		m_pClassLabel = dynamic_cast< CExLabel* >( pClassLabel );

	Panel* pItemSlotLabel = FindChildByName( "ItemSlotLabel" );
	if ( pItemSlotLabel )
		m_pItemSlotLabel = dynamic_cast< CExLabel* >( pItemSlotLabel );
	
	SetKeyBoardInputEnabled(true);

	m_pMouseOverItemPanel->SetVisible( false );

	CreateItemPanels();
}

void CEquipSlotItemSelectionPanel::ApplySettings( KeyValues* inResourceData )
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

	pModelPanelsKV = inResourceData->FindKey( "modelpanels_selection_kv" );
	if ( pModelPanelsKV )
	{
		if ( m_pItemModelPanelSelectionKeyValues )
		{
			m_pItemModelPanelSelectionKeyValues->deleteThis();
		}

		m_pItemModelPanelSelectionKeyValues = new KeyValues( "modelpanels_selection_kv" );
		pModelPanelsKV->CopySubkeys( m_pItemModelPanelSelectionKeyValues );
	}
}

void CEquipSlotItemSelectionPanel::CreateItemPanels()
{
	for ( ; m_pItemModelPanels.Count() < GetNumItemPanels(); )
	{
		AddNewItemPanel( m_pItemModelPanels.Count() );
	}
}

void CEquipSlotItemSelectionPanel::AddNewItemPanel( int index )
{
	CItemModelPanel* pItemPanel = new CItemModelPanel( this, VarArgs( "modelpanel%d", index ) );
	if ( pItemPanel )
		m_pItemModelPanels.AddToTail( pItemPanel );
}

void CEquipSlotItemSelectionPanel::ApplyKVsToItemPanels()
{
	FOR_EACH_VEC( m_pItemModelPanels, iPanel )
	{
		m_pItemModelPanels[iPanel]->ApplySettings( m_pItemModelPanelKeyValues );
		m_pItemModelPanels[iPanel]->ApplySettings( m_pItemModelPanelSelectionKeyValues );
		m_pItemModelPanels[iPanel]->InvalidateLayout();
	}
}

int CEquipSlotItemSelectionPanel::GetNumItemPanels()
{
	return GetTFInventory()->GetNumPresets( m_iClass, m_iSlot );
}

void CEquipSlotItemSelectionPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	ApplyKVsToItemPanels();

	FOR_EACH_VEC( m_pItemModelPanels, iPanel )
	{
		int ypos = m_iItemYPos;
		int xpos = GetWide() / 2 + m_iItemBackpackOffCenterX + iPanel * g_pVGuiSchemeManager->GetProportionalScaledValue(100);
		int column = iPanel / 6;
		if ( column >= 1 )
		{
			ypos = m_iItemYPos + m_iItemYDelta * column;
			xpos = GetWide() / 2 + m_iItemBackpackOffCenterX + ( iPanel - ( 6 * column ) ) * g_pVGuiSchemeManager->GetProportionalScaledValue(100);
		}

		m_pItemModelPanels[iPanel]->SetPos( xpos, ypos );
		m_pItemModelPanels[iPanel]->SetVisible( true );

		m_pItemModelPanels[iPanel]->SetItem( GetTFInventory()->GetItem( m_iClass, m_iSlot, iPanel ) );
		m_pItemModelPanels[iPanel]->InvalidateLayout();
	}

	if ( m_pItemSlotLabel )
		m_pItemSlotLabel->SetText( g_LoadoutTranslations[m_iSlot] );
}

void CEquipSlotItemSelectionPanel::SetTeam( int iTeam )
{
	m_iTeam = iTeam;
}

void CEquipSlotItemSelectionPanel::SetClass( int iClass )
{
	m_iClass = iClass;

	InvalidateLayout();

	SetDialogVariable( "loadoutclass", g_pVGuiLocalize->FindAsUTF8( g_aPlayerClassNames[m_iClass] ) );
}

void CEquipSlotItemSelectionPanel::OnItemPanelEntered( Panel* pPanel )
{
	CItemModelPanel* pItemPanel = (CItemModelPanel* )pPanel;
	if ( !pItemPanel )
		return;

	if ( pItemPanel->GetItem() && pItemPanel->GetItem()->GetStaticData() )
		MAINMENU_ROOT->ShowItemToolTip( pItemPanel->GetItem()->GetStaticData() );
}

void CEquipSlotItemSelectionPanel::OnItemPanelExited( Panel* pPanel )
{
	MAINMENU_ROOT->HideItemToolTip();
}

void CEquipSlotItemSelectionPanel::OnCommand(const char *command)
{
	if (!stricmp(command, "loadout_scrollup"))
	{
		DevMsg("Pootis UP! \n");
		if (GetYPos() <= -70)
			SetPos(GetXPos(), GetYPos() + 70);
	}
	if (!stricmp(command, "loadout_scrolldown"))
	{
		DevMsg("Pootis DOWN! \n");
		SetPos(GetXPos(), GetYPos() - 70);
	}
}


void CEquipSlotItemSelectionPanel::OnItemPanelMouseReleased( Panel* pPanel )
{
	FOR_EACH_VEC( m_pItemModelPanels, iPanel )
	{
		if ( m_pItemModelPanels[iPanel] == pPanel )
		{
			GetTFInventory()->SetWeaponPreset( m_iClass, m_iSlot, iPanel );
			C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
			if ( pPlayer )
			{
				char szCmd[64];
				Q_snprintf( szCmd, sizeof( szCmd ), "weaponpresetclass %d %d %d", m_iClass, m_iSlot, iPanel );
				engine->ExecuteClientCmd( szCmd );
			}

			if ( m_pItemModelPanels[iPanel]->GetItem()->GetStaticData()->mouse_pressed_sound[0] != '\0' )
				g_pVGuiSurface->PlaySound( m_pItemModelPanels[iPanel]->GetItem()->GetStaticData()->mouse_pressed_sound );

			GetParent()->InvalidateLayout();
			g_pVGui->PostMessage( GetVParent(), new KeyValues( "CloseItemSelection" ), GetVPanel() );
			break;
		}
	}

	MAINMENU_ROOT->HideItemToolTip();
}