#include "cbase.h"
#include "item_model_panel.h"

DECLARE_BUILD_FACTORY( CItemModelPanel );
DECLARE_BUILD_FACTORY( CEmbeddedItemModelPanel );
DECLARE_BUILD_FACTORY( CItemModelPanelHud );

using namespace vgui;

CItemModelPanel::CItemModelPanel( vgui::Panel* parent, const char* name ) : vgui::EditablePanel( parent, name )
{
}

CItemModelPanel::~CItemModelPanel()
{
}

void CItemModelPanel::ApplySchemeSettings( vgui::IScheme* pScheme )
{
	//engine->ClientCmd( "gameui_activate" );
	LoadControlSettings( "Resource/UI/econ/ItemModelPanel.res" );
	BaseClass::ApplySchemeSettings( pScheme );

	Panel* pMainContentsContainer = FindChildByName( "MainContentsContainer" );
	if ( pMainContentsContainer )
	{
		m_pMainContentsContainer = dynamic_cast< vgui::EditablePanel* >( pMainContentsContainer );
		m_pMainContentsContainer->SetMouseInputEnabled( false );
	}

	Panel* pItemModelPanel = FindChildByName( "itemmodelpanel", true );
	if ( pItemModelPanel )
	{
		m_pItemModelPanel = dynamic_cast< CEmbeddedItemModelPanel* >( pItemModelPanel );
		m_pItemModelPanel->SetVisible( true );
	}

	Panel* pNameLabel = FindChildByName( "namelabel", true );
	if ( pNameLabel )
	{
		m_pNameLabel = dynamic_cast< CExLabel* >( pNameLabel );
		m_pNameLabel->SetMouseInputEnabled( false );
	}

	Panel* pEquippedLabel = FindChildByName( "equippedlabel", true );
	if ( pEquippedLabel )
	{
		m_pEquippedLabel = dynamic_cast< CExLabel* >( pEquippedLabel );
		m_pEquippedLabel->SetVisible( false );
	}
	m_pBorder = pScheme->GetBorder( "BackpackItemBorder" );
	m_pBorderSelected = pScheme->GetBorder( "BackpackItemMouseOverBorder" );
	SetBorder( m_pBorder );
}

void CItemModelPanel::ApplySettings( KeyValues* inResourceData )
{
	BaseClass::ApplySettings( inResourceData );
}

void CItemModelPanel::PerformLayout()
{
	if ( m_pNameLabel )
	{
		int wide, tall;
		GetSize( wide, tall );

		int ypos = m_iTextYPos;

		if ( m_pItem == NULL )
		{
			m_pMainContentsContainer->SetDialogVariable( "itemname", "NO ITEM YET" );
			ypos = m_iTextYPos / 2;
		}
		else
		{
			if ( m_pItem->GetStaticData() )
			{
				m_pMainContentsContainer->SetDialogVariable( "itemname", m_pItem->GetStaticData()->GenerateLocalizedFullItemName() );

				if ( m_pItem->GetStaticData()->item_quality > QUALITY_UNIQUE )
				{
					const char *pszColor = EconQuality_GetColorString( m_pItem->GetStaticData()->item_quality );
					if ( pszColor )
						m_pNameLabel->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( pszColor, Color( 255, 255, 255, 255 ) ) );
					else
						m_pNameLabel->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( "TanLight", Color( 255, 255, 255, 255 ) ) );
				}
				else
				{
					m_pNameLabel->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( "TanLight", Color( 255, 255, 255, 255 ) ) );
				}
			}
		}

		m_pNameLabel->InvalidateLayout( true );
		m_pNameLabel->SizeToContents();
		m_pNameLabel->SetContentAlignment( vgui::Label::a_south );
		m_pNameLabel->SetCenterWrap( true );
		m_pNameLabel->SetPos( wide / 2 - m_pNameLabel->GetWide() / 2, ypos );
	}
	BaseClass::PerformLayout();
}

void CItemModelPanel::OnCursorEntered()
{
	BaseClass::OnCursorEntered();
	KeyValues* msg = new KeyValues( "ItemPanelEntered" );
	msg->SetPtr( "panel", this );
	g_pVGui->PostMessage( GetVParent(), msg, GetVPanel() );
	SetBorder( m_pBorderSelected );
}

void CItemModelPanel::OnCursorExited()
{
	BaseClass::OnCursorExited();
	KeyValues* msg = new KeyValues( "ItemPanelExited" );
	msg->SetPtr( "panel", this );
	g_pVGui->PostMessage( GetVParent(), msg, GetVPanel() );
	SetBorder( m_pBorder );
}

void CItemModelPanel::OnMouseReleased( vgui::MouseCode code )
{
	if ( GetItem() )
	{
		BaseClass::OnMouseReleased( code );
		KeyValues* msg = new KeyValues( "ItemPanelMouseReleased" );
		msg->SetPtr( "panel", this );
		g_pVGui->PostMessage( GetVParent(), msg, GetVPanel() );

		g_pVGuiSurface->PlaySound( "ui/buttonclickrelease.wav" );
	}
}

void CItemModelPanel::SetItem( CEconItemView* pItem )
{
	if ( !pItem )
		return;

	m_pItem = pItem;
	if ( m_pItemModelPanel )
		m_pItemModelPanel->SetItem( pItem );
}

CEmbeddedItemModelPanel::CEmbeddedItemModelPanel( vgui::Panel* parent, const char* name ) : CBaseModelPanel( parent, name )
{
}

CEmbeddedItemModelPanel::~CEmbeddedItemModelPanel()
{
}

void CEmbeddedItemModelPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CEmbeddedItemModelPanel::SetItem( CEconItemView* pItem )
{
	if ( !pItem )
		return;

	m_pItem = pItem;

	if ( m_iBackPackTexture == -1 )
		m_iBackPackTexture = g_pVGuiSurface->CreateNewTextureID();

	g_pVGuiSurface->DrawSetTextureFile( m_iBackPackTexture, m_pItem->GetStaticData()->image_inventory, 1, false );
}

void CEmbeddedItemModelPanel::Paint()
{
	if ( m_bUseModel )
	{
		BaseClass::Paint();
		return;
	}
	if ( m_pItem )
	{
		int x, y, wide, tall;
		GetBounds( x, y, wide, tall );
		int imgwide, imgtall;
		g_pVGuiSurface->DrawGetTextureSize( m_iBackPackTexture, imgwide, imgtall );

		x = x + wide * 0.5 - imgwide * 0.5;

		g_pVGuiSurface->DrawSetTexture( m_iBackPackTexture );
		g_pVGuiSurface->DrawSetColor( 255, 255, 255, 255 );
		g_pVGuiSurface->DrawTexturedRect( x, y, x + imgwide, y + imgtall );
	}
}




//-----------------------------------------------------------------------------
// tf2c
//-----------------------------------------------------------------------------
CItemModelPanelHud::CItemModelPanelHud( Panel *parent, const char* name ) : EditablePanel( parent, name )
{
	m_pWeapon = NULL;
	m_pWeaponName = new vgui::Label ( this, "WeaponName", "" );
	m_pSlotID = new vgui::Label( this, "SlotID", "" );
	m_pWeaponImage = new vgui::ImagePanel( this, "WeaponImage" );
	m_iBorderStyle = -1;
	m_ID = -1;
	m_bOldStyleIcon = false;
}

void CItemModelPanelHud::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pDefaultFont = pScheme->GetFont( "ItemFontNameSmallest", true );
	m_pSelectedFont = pScheme->GetFont( "ItemFontNameSmall", true );
	m_pNumberDefaultFont = pScheme->GetFont( "FontStorePromotion", true );
	m_pNumberSelectedFont = pScheme->GetFont( "HudFontSmall", true );
	m_pDefaultBorder = pScheme->GetBorder( "TFFatLineBorder" );
	m_pSelectedRedBorder = pScheme->GetBorder( "TFFatLineBorderRedBG" );
	m_pSelectedBlueBorder = pScheme->GetBorder( "TFFatLineBorderBlueBG" );

	SetPaintBorderEnabled( false );

	m_pWeaponImage->SetShouldScaleImage( true );
	m_pWeaponImage->SetZPos( -1 );

	m_pWeaponName->SetFont( m_pDefaultFont );
	m_pWeaponName->SetContentAlignment( Label::a_south );
	m_pWeaponName->SetCenterWrap( true );

	m_pSlotID->SetFont( m_pNumberDefaultFont );
	m_pSlotID->SetContentAlignment( Label::a_northeast );
	m_pSlotID->SetFgColor( pScheme->GetColor( "TanLight", Color( 255, 255, 255, 255 ) ) );
}

void CItemModelPanelHud::PerformLayout( void )
{
	// Set border.
	if ( m_iBorderStyle == -1 )
	{
		SetPaintBorderEnabled( false );
	}
	else if ( m_iBorderStyle == 0 )
	{
		SetPaintBorderEnabled( true );
		SetBorder( m_pDefaultBorder );
	}
	else if ( m_iBorderStyle == 1 )
	{
		int iTeam = GetLocalPlayerTeam();

		if ( iTeam == TF_TEAM_RED )
		{
			SetBorder( m_pSelectedRedBorder );
		}
		else
		{
			SetBorder( m_pSelectedBlueBorder );
		}
	}

	// Position image.
	if ( m_bOldStyleIcon )
	{
		m_pWeaponImage->SetBounds( YRES( 5 ), -1 * ( GetTall() / 10.0 ) + YRES( 5 ), ( GetWide() * 1.5 ) - YRES( 10 ), ( GetWide() * 0.75 ) - YRES( 10 ) );
	}
	else
	{
		m_pWeaponImage->SetBounds( YRES( 5 ), -1 * ( GetTall() / 5.0 ) + YRES( 5 ), GetWide() - YRES( 10 ), GetWide() - YRES( 10 ) );
	}

	// Position weapon name.
	m_pWeaponName->SetBounds( YRES( 5 ), GetTall() - YRES( 25 ), GetWide() - YRES( 5 ), YRES( 20 ) );
	m_pWeaponName->SetFont( m_iBorderStyle ? m_pSelectedFont : m_pDefaultFont );

	if ( m_pWeapon ) 
	{
		if ( !m_pWeapon->HasAnyAmmo() )
		{
			m_pWeaponName->SetText( "#TF_OUT_OF_AMMO" );
			//m_pWeaponName->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( "RedSolid", Color( 255, 255, 255, 255 ) ) );
		}
		else
		{
			if ( m_pWeapon->GetItem() && m_pWeapon->GetItem()->GetStaticData() )
			{
				if ( m_pWeapon->GetItem()->GetStaticData()->item_quality > QUALITY_UNIQUE )
				{
					const char *pszColor = EconQuality_GetColorString( m_pWeapon->GetItem()->GetStaticData()->item_quality );
					if ( pszColor )
						m_pWeaponName->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( pszColor, Color( 255, 255, 255, 255 ) ) );
				}
			}
			else
			{
				m_pWeaponName->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( "TanLight", Color( 255, 255, 255, 255 ) ) );
			}
		}
	}

	// Position slot number.
	m_pSlotID->SetBounds( 0, YRES( 5 ), GetWide() - YRES( 5 ), YRES( 10 ) );
	m_pSlotID->SetFont( m_iBorderStyle ? m_pNumberSelectedFont : m_pNumberDefaultFont );
}

void CItemModelPanelHud::SetWeapon( C_BaseCombatWeapon *pWeapon, int iBorderStyle, int ID )
{
	m_pWeapon = pWeapon;
	m_ID = ID;
	m_iBorderStyle = iBorderStyle;

	int iItemID = m_pWeapon->GetItemID();
	CEconItemDefinition *pItemDefinition = GetItemSchema()->GetItemDefinition( iItemID );
	const char *pszName = "";
	char szImage[128] = { '\0' };
	if ( pItemDefinition )
	{
		pszName = pItemDefinition->item_name;
		V_snprintf( szImage, sizeof( szImage ), "../%s_large", pItemDefinition->image_inventory );

		m_bOldStyleIcon = false;

		if ( pItemDefinition->item_quality > QUALITY_UNIQUE )
		{
			const char *pszColor = EconQuality_GetColorString( pItemDefinition->item_quality );
			if ( pszColor )
				m_pWeaponName->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( pszColor, Color( 255, 255, 255, 255 ) ) );
		}
	}
	else
	{
		pszName = m_pWeapon->GetWpnData().szPrintName;
		const CHudTexture *pTexture = m_pWeapon->GetSpriteInactive(); // red team
		if ( pTexture )
		{
			V_snprintf( szImage, sizeof( szImage ), "../%s", pTexture->szTextureFile );
		}

		m_bOldStyleIcon = true;
	}

	m_pWeaponName->SetText( pszName );

	m_pWeaponImage->SetImage( szImage );

	if ( ID != -1 )
	{
		char szSlotID[8];
		V_snprintf( szSlotID, sizeof( szSlotID ), "%d", m_ID + 1 );
		m_pSlotID->SetText( szSlotID );
	}
	else
	{
		m_pSlotID->SetText( "" );
	}

	InvalidateLayout( true );
	PerformLayout();
}

void CItemModelPanelHud::SetWeapon( CEconItemDefinition *pItemDefinition, int iBorderStyle, int ID )
{
	m_pWeapon = NULL;
	m_ID = ID;
	m_iBorderStyle = iBorderStyle;

	if ( pItemDefinition )
	{
		char szImage[128];
		Q_snprintf( szImage, sizeof( szImage ), "../%s_large", pItemDefinition->image_inventory );
		m_pWeaponImage->SetImage( szImage );
		m_bOldStyleIcon = false;

		if ( pItemDefinition->item_quality > QUALITY_UNIQUE )
		{
			const char *pszColor = EconQuality_GetColorString( pItemDefinition->item_quality );
			if ( pszColor )
				m_pWeaponName->SetFgColor( vgui::scheme()->GetIScheme( GetScheme() )->GetColor( pszColor, Color( 255, 255, 255, 255 ) ) );
		}
	}

	m_pWeaponName->SetText( pItemDefinition->GenerateLocalizedFullItemName() );

	if ( ID != -1 )
	{
		char szSlotID[8];
		Q_snprintf( szSlotID, sizeof( szSlotID ), "%d", m_ID + 1 );
		m_pSlotID->SetText( szSlotID );
	}
	else
	{
		m_pSlotID->SetText( "" );
	}

	InvalidateLayout( true );
	PerformLayout();
}

void CItemModelPanelHud::SetWeapon( CEconItemView *pItem, int iBorderStyle, int ID )
{
	m_pItem = pItem;
	SetWeapon( pItem->GetStaticData(), iBorderStyle, ID );
	m_pWeaponName->SetText( "" );
}
