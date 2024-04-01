#include "cbase.h"
#include "class_loadout_panel.h"
#include "tf_playerclass_shared.h"
#include "tf_inventory.h"
#include "tf_weaponbase.h"
#include "activitylist.h"
#include "tf_mainmenu.h"

CClassLoadoutPanel::CClassLoadoutPanel( vgui::Panel* parent ) : vgui::EditablePanel( parent, "class_loadout_panel" )
{
	m_pMouseOverItemPanel = new CItemModelPanel( this, "mouseoveritempanel" );
}

CClassLoadoutPanel::~CClassLoadoutPanel()
{
}

void CClassLoadoutPanel::ApplySchemeSettings( vgui::IScheme* pScheme )
{
	LoadControlSettings( "Resource/UI/ClassLoadoutPanel.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	Panel* pCaratLabel = FindChildByName( "CaratLabel" );
	if ( pCaratLabel )
		m_pCaratLabel = dynamic_cast< CExLabel* >( pCaratLabel );

	Panel* pTauntCaratLabel = FindChildByName( "TauntCaratLabel" );
	if ( pTauntCaratLabel )
		m_pTauntCaratLabel = dynamic_cast< CExLabel* >( pTauntCaratLabel );

	Panel* pClassLabel = FindChildByName( "ClassLabel" );
	if ( pClassLabel )
		m_pClassLabel = dynamic_cast< CExLabel* >( pClassLabel );

	Panel* pTauntLabel = FindChildByName( "TauntLabel" );
	if ( pTauntLabel )
		m_pTauntLabel = dynamic_cast< CExLabel* >( pTauntLabel );

	Panel *pTFModelPanel = FindChildByName( "classmodelpanel" );
	if ( pTFModelPanel )
		m_pTFModelPanel = dynamic_cast< CTFPlayerModelPanel* >( pTFModelPanel );

	m_pMouseOverItemPanel->SetVisible( false );

	CreateItemPanels();
}

void CClassLoadoutPanel::ApplySettings( KeyValues* inResourceData )
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

void CClassLoadoutPanel::CreateItemPanels()
{
	for ( ; m_pItemModelPanels.Count() < GetNumItemPanels(); )
	{
		AddNewItemPanel( m_pItemModelPanels.Count() );
	}
}

void CClassLoadoutPanel::AddNewItemPanel( int index )
{
	CItemModelPanel* pItemPanel = new CItemModelPanel( this, VarArgs( "modelpanel%d", index ) );
	if ( pItemPanel )
		m_pItemModelPanels.AddToTail( pItemPanel );
}

void CClassLoadoutPanel::ApplyKVsToItemPanels()
{
	FOR_EACH_VEC( m_pItemModelPanels, iPanel )
	{
		m_pItemModelPanels[iPanel]->ApplySettings( m_pItemModelPanelKeyValues );
		m_pItemModelPanels[iPanel]->InvalidateLayout();
	}
}

int CClassLoadoutPanel::GetNumItemPanels()
{
	return 8;
}

void CClassLoadoutPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	ApplyKVsToItemPanels();

	FOR_EACH_VEC( m_pItemModelPanels, iPanel )
	{
		int ypos = m_iItemYPos + iPanel * m_iItemYDelta;
		int xpos = GetWide() / 2 + m_iItemXPosOffCenterA;
		int column = iPanel / 4;
		if ( column == 1 )
		{
			xpos = GetWide() / 2 + m_iItemXPosOffCenterB;
			ypos = m_iItemYPos + ( iPanel - 4 ) * m_iItemYDelta;
		}

		m_pItemModelPanels[iPanel]->SetPos( xpos, ypos );
		m_pItemModelPanels[iPanel]->SetVisible( true );

		if ( m_iClass == TF_CLASS_SPY )
		{
			m_pItemModelPanels[0]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_SECONDARY, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_SECONDARY ) ) );
			m_pItemModelPanels[1]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_MELEE, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_MELEE ) ) );
			m_pItemModelPanels[2]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_PDA2, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_PDA2 ) ) );
			m_pItemModelPanels[3]->SetVisible( true );
		}
		else if ( m_iClass == TF_CLASS_ENGINEER )
		{
			m_pItemModelPanels[0]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_PRIMARY, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_PRIMARY ) ) );
			m_pItemModelPanels[1]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_SECONDARY, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_SECONDARY ) ) );
			m_pItemModelPanels[2]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_MELEE, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_MELEE ) ) );
			m_pItemModelPanels[3]->SetVisible( true );
		}
		else
		{
			m_pItemModelPanels[0]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_PRIMARY, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_PRIMARY ) ) );
			m_pItemModelPanels[1]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_SECONDARY, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_SECONDARY ) ) );
			m_pItemModelPanels[2]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_MELEE, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_MELEE ) ) );
			m_pItemModelPanels[3]->SetVisible( false );
		}

		m_pItemModelPanels[3]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_BUILDING, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_BUILDING ) ) );

		m_pItemModelPanels[4]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_HAT, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_HAT ) ) );
		m_pItemModelPanels[5]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_MISC, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_MISC ) ) );
		m_pItemModelPanels[6]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_MISC2, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_MISC2 ) ) );
		m_pItemModelPanels[7]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_ACTION, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_ACTION ) ) );

		if ( m_bTauntLoadout )
		{
			m_pItemModelPanels[0]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT ) ) );
			m_pItemModelPanels[1]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT2, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT2 ) ) );
			m_pItemModelPanels[2]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT3, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT3 ) ) );
			m_pItemModelPanels[3]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT4, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT4 ) ) );
			m_pItemModelPanels[4]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT5, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT5 ) ) );
			m_pItemModelPanels[5]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT6, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT6 ) ) );
			m_pItemModelPanels[6]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT7, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT7 ) ) );
			m_pItemModelPanels[7]->SetItem( GetTFInventory()->GetItem( m_iClass, LOADOUT_POSITION_TAUNT8, GetTFInventory()->GetWeaponPreset( m_iClass, LOADOUT_POSITION_TAUNT8 ) ) );

			if ( m_pTauntCaratLabel && !m_pTauntCaratLabel->IsVisible() )
				m_pTauntCaratLabel->SetVisible( true );

			if ( m_pTauntLabel && !m_pTauntLabel->IsVisible() )
				m_pTauntLabel->SetVisible( true );
		}
		else
		{
			if ( m_pTauntCaratLabel && m_pTauntCaratLabel->IsVisible() )
				m_pTauntCaratLabel->SetVisible( false );

			if ( m_pTauntLabel && m_pTauntLabel->IsVisible() )
				m_pTauntLabel->SetVisible( false );
		}

		m_pItemModelPanels[iPanel]->InvalidateLayout();
	}

	SetSelectedItemPanel( m_pItemModelPanels[0] );
	//MAINMENU_ROOT->HideItemToolTip();
}

void CClassLoadoutPanel::CloseSelectionPanel()
{
	if ( m_pItemSelectionPanel )
	{
		m_pItemSelectionPanel->DeletePanel();
		m_pItemSelectionPanel = NULL;
	}
}

void CClassLoadoutPanel::OnCommand( const char* command )
{
	if ( V_strnicmp( command, "characterloadout", 16 ) == 0 )
	{
		m_bTauntLoadout = false;
		PerformLayout();
	}
	else if ( V_strnicmp( command, "tauntloadout", 12 ) == 0 )
	{
		m_bTauntLoadout = true;
		PerformLayout();
	}
	else
	{
		return BaseClass::OnCommand( command );
	}
}

void CClassLoadoutPanel::OnKeyCodePressed( vgui::KeyCode code )
{
	if( code == KEY_G )
	{
		if ( m_pTFModelPanel )
		{
			char szScene[128];
			V_snprintf( szScene, sizeof( szScene ), "scenes/player/%s/low/taunt01.vcd", g_aPlayerClassNames_NonLocalized[m_iClass] );
			m_pTFModelPanel->SetVCD( szScene );
		}
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void CClassLoadoutPanel::SetTeam( int iTeam )
{
	m_iTeam = iTeam;
}

void CClassLoadoutPanel::SetClass( int iClass )
{
	m_iClass = iClass;

	InvalidateLayout();

	SetDialogVariable( "loadoutclass", g_pVGuiLocalize->FindAsUTF8( g_aPlayerClassNames[m_iClass] ) );

	if ( m_pTFModelPanel )
	{
		m_pTFModelPanel->SetModelName( GetPlayerClassData( iClass )->GetModelName() );
		m_pTFModelPanel->Update();		
	}
	
	if ( m_iClass == TF_CLASS_SCOUT )
	{	
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0); 
	}
	else if (m_iClass == TF_CLASS_SOLDIER)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	else if (m_iClass == TF_CLASS_PYRO)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	else if (m_iClass == TF_CLASS_HEAVYWEAPONS)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	else if (m_iClass == TF_CLASS_ENGINEER)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	else if (m_iClass == TF_CLASS_DEMOMAN)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	else if (m_iClass == TF_CLASS_MEDIC)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	else if (m_iClass == TF_CLASS_SNIPER)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	else if (m_iClass == TF_CLASS_SPY)
	{
		m_pTFModelPanel->SetPoseParameterByName("r_hand_grip", 0.0);
	}
	
	m_bTauntLoadout = false;
	PerformLayout();
}

void CClassLoadoutPanel::SetSelectedItemPanel( CItemModelPanel* pPanel )
{
	if ( !pPanel->GetItem() )
		return;

	m_pTFModelPanel->ClearMergeMDLs();
	m_pTFModelPanel->SetVCD( "" );

	int iAnimationIndex = pPanel->GetItem()->GetAnimationSlot();
	if ( iAnimationIndex == -1 )
		iAnimationIndex = pPanel->GetItem()->GetStaticData()->item_slot;

	CStudioHdr studiohdr( m_pTFModelPanel->GetStudioHdr(), mdlcache );

	if ( !V_stricmp( pPanel->GetItem()->GetEntityName(), "tf_weapon_invis" ) || !V_stricmp( pPanel->GetItem()->GetEntityName(), "tf_wearable" ) || !V_stricmp( pPanel->GetItem()->GetEntityName(), "tf_wearable_demoshield" ) || !V_stricmp( pPanel->GetItem()->GetEntityName(), "tf_weapon_parachute" ) || ( iAnimationIndex == -1 ) || ( iAnimationIndex == -2 ) )
	{
		if ( ActivityList_NameForIndex( ACT_MP_COMPETITIVE_LOSERSTATE ) )
			m_pTFModelPanel->SetSequence( m_pTFModelPanel->FindSequenceFromActivity( &studiohdr, ActivityList_NameForIndex( ACT_MP_COMPETITIVE_LOSERSTATE ) ) );
	}
	else if ( !V_stricmp( pPanel->GetItem()->GetEntityName(), "tf_weapon_katana" ) )
	{
		if ( m_iClass == TF_CLASS_DEMOMAN )
		{
			if ( ActivityList_NameForIndex( ACT_MP_STAND_ITEM1 ) )
				m_pTFModelPanel->SetSequence( m_pTFModelPanel->FindSequenceFromActivity( &studiohdr, ActivityList_NameForIndex( ACT_MP_STAND_ITEM1 ) ) );
		}
		else
		{
			if ( ActivityList_NameForIndex( ACT_MP_STAND_MELEE ) )
				m_pTFModelPanel->SetSequence( m_pTFModelPanel->FindSequenceFromActivity( &studiohdr, ActivityList_NameForIndex( ACT_MP_STAND_MELEE ) ) );
		}
	}
	else
	{
		int iCount = 0;
		acttable_t* actlist = CTFWeaponBase::ActivityList( iCount, iAnimationIndex );
		for ( int i = 0; i <= iCount; i++ )
		{
			if ( actlist[i].baseAct != ACT_MP_STAND_IDLE )
				continue;

			m_pTFModelPanel->SetSequence( m_pTFModelPanel->FindSequenceFromActivity( &studiohdr, ActivityList_NameForIndex( actlist[i].weaponAct ) ) );
			break;
		}
	}
	
	for ( int i = 0; i < m_pTFModelPanel->GetNumBodyGroups(); i++ )
	{
		m_pTFModelPanel->SetBodygroup( i, 0 );
	}

	const char *pszModel = pPanel->GetItem()->GetWorldDisplayModel( m_iClass );
	if ( pszModel && pszModel[0] != '\0' )
	{
		m_pTFModelPanel->SetMergeMDL( pszModel );
	}

	const char *pszExtraWearableModel = pPanel->GetItem()->GetStaticData()->extra_wearable;
	if (pszExtraWearableModel && pszExtraWearableModel[0] != '\0')
	{
		m_pTFModelPanel->SetMergeMDL( pszExtraWearableModel );
	}

	if ( pPanel->GetItem()->GetStaticData()->GetVisuals() )
	{
		for ( int i = 0; i < m_pTFModelPanel->GetNumBodyGroups(); i++ )
		{
			unsigned int index = pPanel->GetItem()->GetStaticData()->GetVisuals()->player_bodygroups.Find( m_pTFModelPanel->GetBodygroupName(i) );
			if ( pPanel->GetItem()->GetStaticData()->GetVisuals()->player_bodygroups.IsValidIndex( index ) )
				m_pTFModelPanel->SetBodygroup( i , 1 );
		}

		if ( pPanel->GetItem()->GetStaticData()->GetVisuals()->wm_bodygroup_override > 0 )
			m_pTFModelPanel->SetBodygroup( pPanel->GetItem()->GetStaticData()->GetVisuals()->wm_bodygroup_override, pPanel->GetItem()->GetStaticData()->GetVisuals()->wm_bodygroup_state_override );
	}

	/*if ( pPanel->GetItem()->GetStaticData()->custom_taunt_scene_per_class[m_iClass][0] != '\0' )
	{
		m_pTFModelPanel->SetVCD( pPanel->GetItem()->GetStaticData()->custom_taunt_scene_per_class[m_iClass] );
	}*/
	return;
}

void CClassLoadoutPanel::OnItemPanelEntered( Panel* pPanel )
{
	CItemModelPanel* pItemPanel = (CItemModelPanel* )pPanel;
	if ( !pItemPanel )
		return;

	SetSelectedItemPanel( pItemPanel );

	/*if ( pItemPanel->GetItem() && pItemPanel->GetItem()->GetStaticData() )
		MAINMENU_ROOT->ShowItemToolTip( pItemPanel->GetItem()->GetStaticData() );*/
}

void CClassLoadoutPanel::OnItemPanelExited( Panel* pPanel )
{
	//MAINMENU_ROOT->HideItemToolTip();
}

void CClassLoadoutPanel::OnItemPanelMouseReleased( Panel* pPanel )
{
	FOR_EACH_VEC( m_pItemModelPanels, iPanel )
	{
		if ( m_pItemModelPanels[iPanel] == pPanel )
		{
			int iActualPanel = 0;

			if ( m_bTauntLoadout )
			{
				if ( iPanel == 0 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT;
				}
				else if ( iPanel == 1 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT2;
				}
				else if ( iPanel == 2 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT3;
				}
				else if ( iPanel == 3 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT4;
				}
				else if ( iPanel == 4 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT5;
				}
				else if ( iPanel == 5 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT6;
				}
				else if ( iPanel == 6 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT7;
				}
				else if ( iPanel == 7 )
				{
					iActualPanel = LOADOUT_POSITION_TAUNT8;
				}
			}
			else
			{
				if ( iPanel == 0 )
				{
					if ( m_iClass == TF_CLASS_SPY )
						iActualPanel = LOADOUT_POSITION_SECONDARY;
					else
						iActualPanel = LOADOUT_POSITION_PRIMARY;
				}
				else if ( iPanel == 1 )
				{
					if ( m_iClass == TF_CLASS_SPY )
						iActualPanel = LOADOUT_POSITION_MELEE;
					else
						iActualPanel = LOADOUT_POSITION_SECONDARY;
				}
				else if ( iPanel == 2 )
				{
					if ( m_iClass == TF_CLASS_SPY )
						iActualPanel = LOADOUT_POSITION_PDA2;
					else
						iActualPanel = LOADOUT_POSITION_MELEE;
				}
				else if ( iPanel == 3 )
				{
					iActualPanel = LOADOUT_POSITION_BUILDING;
				}
				else if ( iPanel == 4 )
				{
					iActualPanel = LOADOUT_POSITION_HAT;
				}
				else if ( iPanel == 5 )
				{
					iActualPanel = LOADOUT_POSITION_MISC;
				}
				else if ( iPanel == 6 )
				{
					iActualPanel = LOADOUT_POSITION_MISC2;
				}
				else if ( iPanel == 7 )
				{
					iActualPanel = LOADOUT_POSITION_ACTION;
				}
			}

			m_pItemSelectionPanel = new CEquipSlotItemSelectionPanel( this, iActualPanel );

			m_pItemSelectionPanel->SetClass( m_iClass );
			m_pItemSelectionPanel->SetTeam( m_iTeam );
			m_pItemSelectionPanel->SetVisible( true );
			break;
		}
	}

	//MAINMENU_ROOT->HideItemToolTip();
}

void CClassLoadoutPanel::OnCloseItemSelection()
{
	CloseSelectionPanel();
}