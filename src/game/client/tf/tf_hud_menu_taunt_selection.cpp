//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "tf_hud_menu_taunt_selection.h"
#include "c_tf_player.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Label.h>
#include "tf_inventory.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//======================================

DECLARE_HUDELEMENT_DEPTH( CHudMenuTauntSelection, 40 );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudMenuTauntSelection::CHudMenuTauntSelection( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudMenuTauntSelection" )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	RegisterForRenderGroup( "mid" );

	ListenForGameEvent( "tauntmenupleaseshow" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudMenuTauntSelection::ApplySchemeSettings( IScheme *pScheme )
{
	for ( ; m_pItemModelPanels.Count() < 8; )
	{
		CItemModelPanelHud* pItemPanel = new CItemModelPanelHud( this, VarArgs( "TauntModelPanel%d", m_pItemModelPanels.Count()+1 ) );
		if ( pItemPanel )
			m_pItemModelPanels.AddToTail( pItemPanel );
	}

	LoadControlSettings( "resource/ui/hudmenutauntselection.res" );

	SetVisible( false );

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudMenuTauntSelection::FireGameEvent( IGameEvent *event )
{
	if ( Q_strcmp( "tauntmenupleaseshow", event->GetName() ) == 0 )
	{
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pPlayer && ( pPlayer->GetUserID() == event->GetInt( "userid" ) ) )
		{
			SetVisible( true );
			InvalidateLayout( true );
		}
	}
	else
	{
		CHudElement::FireGameEvent( event );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudMenuTauntSelection::ShouldDraw( void )
{
	return ( IsVisible() );
}

//-----------------------------------------------------------------------------
// Purpose: Keyboard input hook. Return 0 if handled
//-----------------------------------------------------------------------------
int	CHudMenuTauntSelection::HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if ( !down )
		return 1;

	if ( !ShouldDraw() )
		return 1;

	if ( pszCurrentBinding )
	{
		if ( FStrEq( pszCurrentBinding, "taunt" ) || FStrEq( pszCurrentBinding, "+taunt" ) )
		{
			engine->ClientCmd( "taunt_by_name default" );
			SetVisible( false );
			return 0;
		}
		else if ( FStrEq( pszCurrentBinding, "lastinv" ) )
		{
			SetVisible( false );
			return 0;
		}
	}

	int iSlot = 0;

	switch( keynum )
	{
	case KEY_1:
		iSlot = 1;
		break;
	case KEY_2:
		iSlot = 2;
		break;
	case KEY_3:
		iSlot = 3;
		break;
	case KEY_4:
		iSlot = 4;
		break;
	case KEY_5:
		iSlot = 5;
		break;
	case KEY_6:
		iSlot = 6;
		break;
	case KEY_7:
		iSlot = 7;
		break;
	case KEY_8:
		iSlot = 8;
		break;

	case KEY_9:
		// Eat these keys
		return 0;

	case KEY_0:
	case KEY_XBUTTON_B:
		// cancel, close the menu
		SetVisible( false );
		engine->ExecuteClientCmd( "lastinv" );
		return 0;

	default:
		return 1;	// key not handled
	}

	if ( iSlot > 0 )
	{
		C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pLocalPlayer )
		{
			if ( m_pItemModelPanels[iSlot-1]->GetItem() && m_pItemModelPanels[iSlot-1]->GetItem()->GetStaticData() )
			{
				char szCmd[128];
				Q_snprintf( szCmd, sizeof(szCmd), "taunt_by_name %i", m_pItemModelPanels[iSlot-1]->GetItem()->GetStaticData()->index );
				engine->ClientCmd( szCmd );
				SetVisible( false );
			}
			else
			{
				pLocalPlayer->EmitSound( "Player.DenyWeaponSelection" );
				SetVisible( false );
			}
		}
		return 0;
	}

	return 1;	// key not handled
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudMenuTauntSelection::SetVisible( bool state )
{
	if ( state == true )
	{
		// close the weapon selection menu
		engine->ClientCmd( "cancelselect" );

		// set the %lastinv% dialog var to our binding
		const char *key = engine->Key_LookupBinding( "lastinv" );
		if ( !key )
		{
			key = "< not bound >";
		}

		SetDialogVariable( "lastinv", key );

		// set the %taunt% dialog var
		key = engine->Key_LookupBinding( "+taunt" );
		if ( !key )
		{
			key = "< not bound >";
		}

		SetDialogVariable( "taunt", key );

		CEconItemDefinition *pItemDef_Schadenfreude = GetItemSchema()->GetItemDefinition( 463 );
		if ( pItemDef_Schadenfreude )
		{
			m_pItemModelPanels[0]->SetWeapon( GetTFInventory()->GetItem( 463 ) );
		}

		CEconItemDefinition *pItemDef_DirectorsVision = GetItemSchema()->GetItemDefinition( 438 );
		if ( pItemDef_DirectorsVision )
		{
			m_pItemModelPanels[1]->SetWeapon( GetTFInventory()->GetItem( 438 ) );
		}

		CEconItemDefinition *pItemDef_ShredAlert = GetItemSchema()->GetItemDefinition( 1015 );
		if ( pItemDef_ShredAlert )
		{
			m_pItemModelPanels[2]->SetWeapon( GetTFInventory()->GetItem( 1015 ) );
		}

		CEconItemDefinition *pItemDef_Burstchester = GetItemSchema()->GetItemDefinition( 30621 );
		if ( pItemDef_Burstchester )
		{
			m_pItemModelPanels[3]->SetWeapon( GetTFInventory()->GetItem( 30621 ) );
		}

		CEconItemDefinition *pItemDef_SecondRateSorcery = GetItemSchema()->GetItemDefinition( 30816 );
		if ( pItemDef_SecondRateSorcery )
		{
			m_pItemModelPanels[4]->SetWeapon( GetTFInventory()->GetItem( 30816 ) );
		}

		int  iClassSpcificOne = 1182;
		int  iClassSpcificTwo = 1182;
		int  iClassSpcificTee = 1157;
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pPlayer )
		{
			if ( pPlayer->IsPlayerClass( TF_CLASS_SCOUT ) )
			{
				iClassSpcificOne = 1119;
				iClassSpcificTwo = 30921;
				iClassSpcificTee = 30572;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_SOLDIER ) )
			{
				iClassSpcificOne = 30673;
				iClassSpcificTwo = 30761;
				iClassSpcificTee = 1113;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_PYRO ) )
			{
				iClassSpcificOne = 1112;
				iClassSpcificTwo = 30876;
				iClassSpcificTee = 30570;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_DEMOMAN ) )
			{
				iClassSpcificOne = 1120;
				iClassSpcificTwo = 30671;
				iClassSpcificTee = 1114;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_HEAVYWEAPONS ) )
			{
				iClassSpcificOne = 30616;
				iClassSpcificTwo = 30843;
				iClassSpcificTee = 30844;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_ENGINEER ) )
			{
				iClassSpcificOne = 1115;
				iClassSpcificTwo = 30842;
				iClassSpcificTee = 1157;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_MEDIC ) )
			{
				iClassSpcificOne = 1109;
				iClassSpcificTwo = 30918;
				iClassSpcificTee = 477;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_SNIPER ) )
			{
				iClassSpcificOne = 30609;
				iClassSpcificTwo = 30839;
				iClassSpcificTee = 1116;
			}
			else if ( pPlayer->IsPlayerClass( TF_CLASS_SPY ) )
			{
				iClassSpcificOne = 1108;
				iClassSpcificTwo = 30922;
				iClassSpcificTee = 30762;
			}
		}

		CEconItemDefinition *pItemDef_ClassSpecific = GetItemSchema()->GetItemDefinition( iClassSpcificOne );
		if ( pItemDef_ClassSpecific )
		{
			m_pItemModelPanels[5]->SetWeapon( GetTFInventory()->GetItem( iClassSpcificOne ) );
		}

		CEconItemDefinition *pItemDef_ClassSpecificTwo = GetItemSchema()->GetItemDefinition( iClassSpcificTwo );
		if ( pItemDef_ClassSpecificTwo )
		{
			m_pItemModelPanels[6]->SetWeapon( GetTFInventory()->GetItem( iClassSpcificTwo ) );
		}

		CEconItemDefinition *pItemDef_ClassSpecificTee = GetItemSchema()->GetItemDefinition( iClassSpcificTee );
		if ( pItemDef_ClassSpecificTee )
		{
			m_pItemModelPanels[7]->SetWeapon( GetTFInventory()->GetItem( iClassSpcificTee ) );
		}

		FOR_EACH_VEC( m_pItemModelPanels, iPanel )
		{
			m_pItemModelPanels[iPanel]->SetVisible( true );
		}

		HideLowerPriorityHudElementsInGroup( "mid" );
	}
	else
	{
		FOR_EACH_VEC( m_pItemModelPanels, iPanel )
		{
			m_pItemModelPanels[iPanel]->SetVisible( false );
		}

		UnhideLowerPriorityHudElementsInGroup( "mid" );
	}

	BaseClass::SetVisible( state );
}


void CHudMenuTauntSelection::PerformLayout()
{
	BaseClass::PerformLayout();
}
