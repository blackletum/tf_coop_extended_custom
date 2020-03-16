//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: Hammer model browser in-game
//
//=============================================================================

#include "cbase.h"

#include "vgui/lfe_mdlpicker.h"
#include "tier1/KeyValues.h"
#include "tier1/utldict.h"
#include "filesystem.h"
#include "studio.h"
#include "matsys_controls/matsyscontrols.h"
#include "matsys_controls/mdlpanel.h"
#include "vgui_controls/Splitter.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/MessageBox.h"
#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/CheckButton.h"
#include "vgui/IVGui.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "vgui/Cursor.h"
#include "datamodel/dmelement.h"
#include "matsys_controls/assetpicker.h"
#include "c_tf_player.h"
#include "ienginevgui.h"
#include "tf_weapon_cheatgun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


using namespace vgui;


//-----------------------------------------------------------------------------
//
// MDL Picker
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Sort by MDL name
//-----------------------------------------------------------------------------
static int __cdecl TFMDLBrowserSortFunc( vgui::ListPanel *pPanel, const ListPanelItem &item1, const ListPanelItem &item2 )
{
	const char *string1 = item1.kv->GetString("mdl");
	const char *string2 = item2.kv->GetString("mdl");
	return stricmp( string1, string2 );
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFMDLPicker::CTFMDLPicker( vgui::Panel *pParent, int nFlags ) : 
	BaseClass( pParent, "MDL Files", "mdl", "models", "mdlName" )
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/SourceScheme.res", "SourceScheme" ) );

	m_hSelectedMDL = MDLHANDLE_INVALID;

	m_nFlags = nFlags;	// remember what we show and what not

	m_pRenderPage = NULL;
	m_pSequencesPage = NULL;
	m_pActivitiesPage = NULL;
	m_pSkinsPage = NULL;
	m_pInfoPage = NULL;

	m_pSequencesList = NULL;
	m_pActivitiesList = NULL;

	// Horizontal splitter for mdls
	m_pFileBrowserSplitter = new Splitter( this, "FileBrowserSplitter", SPLITTER_MODE_VERTICAL, 1 );

	float flFractions[] = { 0.33f, 0.67f };

	m_pFileBrowserSplitter->RespaceSplitters( flFractions );

	vgui::Panel *pSplitterLeftSide = m_pFileBrowserSplitter->GetChild( 0 );
	vgui::Panel *pSplitterRightSide = m_pFileBrowserSplitter->GetChild( 1 );

	// Standard browser controls
	pSplitterLeftSide->RequestFocus();
	CreateStandardControls( pSplitterLeftSide, false );

	// property sheet - revisions, changes, etc.
	m_pPreviewSplitter = new Splitter( pSplitterRightSide, "PreviewSplitter", SPLITTER_MODE_HORIZONTAL, 1 );

	vgui::Panel *pSplitterTopSide = m_pPreviewSplitter->GetChild( 0 );
	vgui::Panel *pSplitterBottomSide = m_pPreviewSplitter->GetChild( 1 );

	// MDL preview
	m_pMDLPreview = new CMDLPanel( pSplitterTopSide, "MDLPreview" );
	SetSkipChildDuringPainting( m_pMDLPreview );

	m_pViewsSheet = new vgui::PropertySheet( pSplitterBottomSide, "ViewsSheet" );
	m_pViewsSheet->AddActionSignalTarget( this );

	// now add wanted features
	if ( nFlags & PAGE_SEQUENCES )
	{
		m_pSequencesPage = new vgui::PropertyPage( m_pViewsSheet, "SequencesPage" );

		m_pSequencesList = new vgui::ListPanel( m_pSequencesPage, "SequencesList" );
		m_pSequencesList->AddColumnHeader( 0, "sequence", "sequence", 52, 0 );
		m_pSequencesList->AddActionSignalTarget( this );
		m_pSequencesList->SetSelectIndividualCells( true );
		m_pSequencesList->SetEmptyListText("No .MDL file currently selected.");
		m_pSequencesList->SetDragEnabled( true );
		m_pSequencesList->SetAutoResize( Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 6, -6, -6 );
	}

	if ( nFlags & PAGE_ACTIVITIES )
	{
		m_pActivitiesPage = new vgui::PropertyPage( m_pViewsSheet, "ActivitiesPage" );

		m_pActivitiesList = new vgui::ListPanel( m_pActivitiesPage, "ActivitiesList" );
		m_pActivitiesList->AddColumnHeader( 0, "activity", "activity", 52, 0 );
		m_pActivitiesList->AddActionSignalTarget( this );
		m_pActivitiesList->SetSelectIndividualCells( true );
		m_pActivitiesList->SetEmptyListText( "No .MDL file currently selected." );
		m_pActivitiesList->SetDragEnabled( true );
		m_pActivitiesList->SetAutoResize( Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 6, -6, -6 );
	}

	if ( nFlags & PAGE_SKINS )
	{
		m_pSkinsPage = new vgui::PropertyPage( m_pViewsSheet, "SkinsPage" );

		m_pSkinsList = new vgui::ListPanel( m_pSkinsPage, "SkinsList" );
		m_pSkinsList->AddColumnHeader( 0, "skin", "skin", 52, 0 );
		m_pSkinsList->AddActionSignalTarget( this );
		m_pSkinsList->SetSelectIndividualCells( true );
		m_pSkinsList->SetEmptyListText( "No .MDL file currently selected." );
		m_pSkinsList->SetDragEnabled( true );
		m_pSkinsList->SetAutoResize( Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 6, -6, -6 );		
	}

	if ( nFlags & PAGE_INFO )
	{
		m_pInfoPage = new vgui::PropertyPage( m_pViewsSheet, "InfoPage" );

		m_pInfoPage->AddActionSignalTarget( this );

		m_pInfoPage->LoadControlSettingsAndUserConfig( "Resource/ModelBrowserInfo.res" );

		CheckButton * pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName( "PhysicsObject" );
		if (pTempCheck)
		{
			pTempCheck->SetDisabledFgColor1(pTempCheck->GetFgColor());
			pTempCheck->SetDisabledFgColor2(pTempCheck->GetFgColor());
			pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("StaticObject");
			pTempCheck->SetDisabledFgColor1(pTempCheck->GetFgColor());
			pTempCheck->SetDisabledFgColor2(pTempCheck->GetFgColor());
			pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("DynamicObject");
			pTempCheck->SetDisabledFgColor1(pTempCheck->GetFgColor());
			pTempCheck->SetDisabledFgColor2(pTempCheck->GetFgColor());
		}

		m_pPropDataList = new vgui::ListPanel( m_pInfoPage, "PropData" );
		m_pPropDataList->AddColumnHeader( 0, "key", "key", 250, ListPanel::COLUMN_FIXEDSIZE );		
		m_pPropDataList->AddColumnHeader( 1, "value", "value", 52, 0 );
		m_pPropDataList->AddActionSignalTarget( this );
		m_pPropDataList->SetSelectIndividualCells( false );
		m_pPropDataList->SetEmptyListText( "No prop_data available." );
		m_pPropDataList->SetDragEnabled( true );
		m_pPropDataList->SetAutoResize( Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 72, -6, -6 );
	}

	// Load layout settings; has to happen before pinning occurs in code
	LoadControlSettingsAndUserConfig( "Resource/ModelBrowser.res" );

	// Pages must be added after control settings are set up
	if ( m_pRenderPage )
	{
		m_pViewsSheet->AddPage( m_pRenderPage, "Render" );
	}
	if ( m_pSequencesPage )
	{
		m_pViewsSheet->AddPage( m_pSequencesPage, "Sequences" );
	}
	if ( m_pActivitiesPage )
	{
		m_pViewsSheet->AddPage( m_pActivitiesPage, "Activities" );
	}
	if ( m_pSkinsPage )
	{
		m_pViewsSheet->AddPage( m_pSkinsPage, "Skins" );
	}
	if ( m_pInfoPage )
	{
		m_pViewsSheet->AddPage( m_pInfoPage, "Info" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFMDLPicker::~CTFMDLPicker()
{
}


//-----------------------------------------------------------------------------
// Performs layout
//-----------------------------------------------------------------------------
void CTFMDLPicker::PerformLayout()
{
	// NOTE: This call should cause auto-resize to occur
	// which should fix up the width of the panels
	BaseClass::PerformLayout();

	int w, h;
	GetSize( w, h );

	// Layout the mdl splitter
	m_pFileBrowserSplitter->SetBounds( 0, 0, w, h );
}


//-----------------------------------------------------------------------------
// Buttons on various pages
//-----------------------------------------------------------------------------
void CTFMDLPicker::OnAssetSelected( KeyValues *pParams )
{
	/*const char *pAsset = pParams->GetString( "asset" );

	char pProbeBuf[MAX_PATH];
	Q_snprintf( pProbeBuf, sizeof(pProbeBuf), "materials/lightprobes/%s", pAsset );

	CDisableUndoScopeGuard sg;

	CDmxElement *pLightProbe = NULL; 
	g_pDataModel->RestoreFromFile( pProbeBuf, "GAME", NULL, &pLightProbe );
	if ( !pLightProbe )
	{
		char pBuf[1024];
		Q_snprintf( pBuf, sizeof(pBuf), "Error loading lightprobe file '%s'!\n", pProbeBuf ); 
		vgui::MessageBox *pMessageBox = new vgui::MessageBox( "Error Loading File!\n", pBuf, GetParent() );
		pMessageBox->DoModal( );
		return;
	}

	m_pMDLPreview->SetLightProbe( pLightProbe );
	g_pDataModel->RemoveFileId( pLightProbe->GetFileId() );*/
}


//-----------------------------------------------------------------------------
// Buttons on various pages
//-----------------------------------------------------------------------------
void CTFMDLPicker::OnCommand( const char *pCommand )
{
	if ( !Q_stricmp( pCommand, "ChooseLightProbe" ) )
	{
		CAssetPickerFrame *pPicker = new CAssetPickerFrame( this, "Select Light Probe (.prb) File",
			"Light Probe", "prb", "materials/lightprobes", "lightprobe" );
		pPicker->DoModal();
		return;
	}

	BaseClass::OnCommand( pCommand );
}


//-----------------------------------------------------------------------------
// Purpose: rebuilds the list of activities
//-----------------------------------------------------------------------------
void CTFMDLPicker::RefreshActivitiesAndSequencesList()
{
	m_pActivitiesList->RemoveAll();
	m_pSequencesList->RemoveAll();
	m_pMDLPreview->SetSequence( 0 );

	if ( m_hSelectedMDL == MDLHANDLE_INVALID )
	{
		m_pActivitiesList->SetEmptyListText("No .MDL file currently selected");
		m_pSequencesList->SetEmptyListText("No .MDL file currently selected");
		return;
	}

	m_pActivitiesList->SetEmptyListText(".MDL file contains no activities");
	m_pSequencesList->SetEmptyListText(".MDL file contains no sequences");

	studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL );
	
	CUtlDict<int, unsigned short> activityNames( true, 0, hdr->GetNumSeq() );

	for (int j = 0; j < hdr->GetNumSeq(); j++)
	{
		if ( /*g_viewerSettings.showHidden ||*/ !(hdr->pSeqdesc(j).flags & STUDIO_HIDDEN))
		{
			const char *pActivityName = hdr->pSeqdesc(j).pszActivityName();
			if ( pActivityName && pActivityName[0] )
			{
				// Multiple sequences can have the same activity name; only add unique activity names
				if ( activityNames.Find( pActivityName ) == activityNames.InvalidIndex() )
				{
					KeyValues *pkv = new KeyValues("node", "activity", pActivityName );
					int nItemID = m_pActivitiesList->AddItem( pkv, 0, false, false );

					KeyValues *pDrag = new KeyValues( "drag", "text", pActivityName );
					pDrag->SetString( "texttype", "activityName" );
					pDrag->SetString( "mdl", vgui::MDLCache()->GetModelName( m_hSelectedMDL ) );
					m_pActivitiesList->SetItemDragData( nItemID, pDrag );

					activityNames.Insert( pActivityName, j );
				}
			}

			const char *pSequenceName = hdr->pSeqdesc(j).pszLabel();
			if ( pSequenceName && pSequenceName[0] )
			{
				KeyValues *pkv = new KeyValues("node", "sequence", pSequenceName);
				int nItemID = m_pSequencesList->AddItem( pkv, 0, false, false );

				KeyValues *pDrag = new KeyValues( "drag", "text", pSequenceName );
				pDrag->SetString( "texttype", "sequenceName" );
				pDrag->SetString( "mdl", vgui::MDLCache()->GetModelName( m_hSelectedMDL ) );
				m_pSequencesList->SetItemDragData( nItemID, pDrag );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// A MDL was selected
//-----------------------------------------------------------------------------
void CTFMDLPicker::OnSelectedAssetPicked( const char *pMDLName )
{
	char pRelativePath[MAX_PATH];
	if ( pMDLName )
	{
		Q_snprintf( pRelativePath, sizeof(pRelativePath), "models\\%s", pMDLName );
		SelectMDL( pRelativePath );
	}
	else
	{
		SelectMDL( NULL );
	}
}


//-----------------------------------------------------------------------------
// Allows external apps to select a MDL
//-----------------------------------------------------------------------------
void CTFMDLPicker::SelectMDL( const char *pRelativePath )
{
	MDLHandle_t hSelectedMDL = pRelativePath ? vgui::MDLCache()->FindMDL( pRelativePath ) : MDLHANDLE_INVALID;
	 
	// We didn't change models after all...
	if ( hSelectedMDL == m_hSelectedMDL )
	{
		// vgui::MDLCache()->FindMDL adds a reference by default we don't use, release it again
		if ( hSelectedMDL != MDLHANDLE_INVALID )
		{
			vgui::MDLCache()->Release( hSelectedMDL );
		}
		return;
	}

	m_hSelectedMDL = hSelectedMDL;

	if ( vgui::MDLCache()->IsErrorModel( m_hSelectedMDL ) )
	{
		m_hSelectedMDL = MDLHANDLE_INVALID;
	}
	m_pMDLPreview->SetMDL( m_hSelectedMDL );

	m_pMDLPreview->LookAtMDL();


	if ( m_nFlags & ( PAGE_SKINS ) )
	{
		UpdateSkinsList();
	}

	if ( m_nFlags & ( PAGE_INFO ) )
	{
		UpdateInfoTab();
	}

	if ( m_nFlags & (PAGE_ACTIVITIES|PAGE_SEQUENCES) )
	{
		RefreshActivitiesAndSequencesList();
	}

	// vgui::MDLCache()->FindMDL adds a reference by default we don't use, release it again
	if ( hSelectedMDL != MDLHANDLE_INVALID )
	{
		vgui::MDLCache()->Release( hSelectedMDL );
	}

	PostActionSignal( new KeyValues( "MDLPreviewChanged", "mdl", pRelativePath ? pRelativePath : "" ) );
}


//-----------------------------------------------------------------------------
// Purpose: updates revision view on a file being selected
//-----------------------------------------------------------------------------
void CTFMDLPicker::OnCheckButtonChecked(KeyValues *kv)
{
//    RefreshMDLList();
	BaseClass::OnCheckButtonChecked( kv );
}


void CTFMDLPicker::GetSelectedMDLName( char *pBuffer, int nMaxLen )
{
	Assert( nMaxLen > 0 );
	if ( GetSelectedAssetCount() > 0 )
	{
		Q_snprintf( pBuffer, nMaxLen, "models\\%s", GetSelectedAsset( ) );
	}
	else
	{
		pBuffer[0] = 0;
	}
}
	
//-----------------------------------------------------------------------------
// Gets the selected activity/sequence
//-----------------------------------------------------------------------------
int CTFMDLPicker::GetSelectedPage( )
{
	if ( m_pSequencesPage && ( m_pViewsSheet->GetActivePage() == m_pSequencesPage ) )
		return PAGE_SEQUENCES;

	if ( m_pActivitiesPage && ( m_pViewsSheet->GetActivePage() == m_pActivitiesPage ) )
		return PAGE_ACTIVITIES;

	return PAGE_NONE;
}

const char *CTFMDLPicker::GetSelectedSequenceName()
{
	if ( !m_pSequencesPage  )
		return NULL;

	int nIndex = m_pSequencesList->GetSelectedItem( 0 );
	if ( nIndex >= 0 )
	{
		KeyValues *pkv = m_pSequencesList->GetItem( nIndex );
		return pkv->GetString( "sequence", NULL );
	}

	return NULL;
}

const char *CTFMDLPicker::GetSelectedActivityName()
{
	if ( !m_pActivitiesPage  )
		return NULL;

	int nIndex = m_pActivitiesList->GetSelectedItem( 0 );
	if ( nIndex >= 0 )
	{
		KeyValues *pkv = m_pActivitiesList->GetItem( nIndex );
		return pkv->GetString( "activity", NULL );
	}
	return NULL;
}

int	CTFMDLPicker::GetSelectedSkin()
{
	if ( !m_pSkinsPage )
		return NULL;

	int nIndex = m_pSkinsList->GetSelectedItem( 0 );
	if ( nIndex >= 0 )
	{
		return nIndex;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Plays the selected activity
//-----------------------------------------------------------------------------
void CTFMDLPicker::SelectActivity( const char *pActivityName )
{
	studiohdr_t *pstudiohdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL );
	for ( int i = 0; i < pstudiohdr->GetNumSeq(); i++ )
	{
		mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );
		if ( stricmp( seqdesc.pszActivityName(), pActivityName ) == 0 )
		{
			// FIXME: Add weighted sequence selection logic?
			m_pMDLPreview->SetSequence( i );
			break;
		}
	}

	PostActionSignal( new KeyValues( "SequenceSelectionChanged", "activity", pActivityName ) );
}


//-----------------------------------------------------------------------------
// Plays the selected sequence
//-----------------------------------------------------------------------------
void CTFMDLPicker::SelectSequence( const char *pSequenceName )
{
	studiohdr_t *pstudiohdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL );
	for (int i = 0; i < pstudiohdr->GetNumSeq(); i++)
	{
		mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );
		if ( !Q_stricmp( seqdesc.pszLabel(), pSequenceName ) )
		{
			m_pMDLPreview->SetSequence( i );
			break;
		}
	}

	PostActionSignal( new KeyValues( "SequenceSelectionChanged", "sequence", pSequenceName ) );
}

void CTFMDLPicker::SelectSkin( int nSkin )
{
	m_pMDLPreview->SetSkin( nSkin );
	PostActionSignal( new KeyValues( "SkinSelectionChanged", "skin", nSkin));
}

	
//-----------------------------------------------------------------------------
// Purpose: Updates preview when an item is selected
//-----------------------------------------------------------------------------
void CTFMDLPicker::OnItemSelected( KeyValues *kv )
{
	Panel *pPanel = (Panel *)kv->GetPtr("panel", NULL);
	if ( m_pSequencesList && (pPanel == m_pSequencesList ) )
	{
		const char *pSequenceName = GetSelectedSequenceName();
		if ( pSequenceName )
		{
			SelectSequence( pSequenceName );
		}
		return;
	}

	if ( m_pActivitiesList && ( pPanel == m_pActivitiesList ) )
	{
		const char *pActivityName = GetSelectedActivityName();
		if ( pActivityName )
		{
			SelectActivity( pActivityName );
		}
		return;
	}

	if ( m_pSkinsList && ( pPanel == m_pSkinsList ) )
	{
		int nSelectedSkin = GetSelectedSkin();
		SelectSkin( nSelectedSkin );
	
		return;
	}

	BaseClass::OnItemSelected( kv );
}


//-----------------------------------------------------------------------------
// Purpose: Called when a page is shown
//-----------------------------------------------------------------------------
void CTFMDLPicker::OnPageChanged( )
{
	if ( m_pSequencesPage && ( m_pViewsSheet->GetActivePage() == m_pSequencesPage ) )
	{
		m_pSequencesList->RequestFocus();

		const char *pSequenceName = GetSelectedSequenceName();

		if ( pSequenceName )
		{
			SelectSequence( pSequenceName );
		}
		return;
	}
	
	if ( m_pActivitiesPage && ( m_pViewsSheet->GetActivePage() == m_pActivitiesPage ) )
	{
		m_pActivitiesList->RequestFocus();

		const char *pActivityName = GetSelectedActivityName();

		if ( pActivityName )
		{
			SelectActivity( pActivityName );
		}
		return;
	}
}


//-----------------------------------------------------------------------------
//
// Purpose: Modal picker frame
//
//-----------------------------------------------------------------------------
CTFMDLPickerFrame::CTFMDLPickerFrame( vgui::Panel *pParent, const char *pTitle, int nFlags ) : 
	BaseClass( pParent )
{
	SetAssetPicker( new CTFMDLPicker( this, nFlags ) );
	LoadControlSettingsAndUserConfig( "Resource/ModelBrowserFrame.res" );
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/SourceScheme.res", "SourceScheme" ) );
	SetTitle( pTitle, false );
}

CTFMDLPickerFrame::~CTFMDLPickerFrame()
{
}


//-----------------------------------------------------------------------------
// Allows external apps to select a MDL
//-----------------------------------------------------------------------------
void CTFMDLPickerFrame::SelectMDL( const char *pRelativePath )
{
	static_cast<CTFMDLPicker*>( GetAssetPicker() )->SelectMDL( pRelativePath );
}

int CTFMDLPicker::UpdateSkinsList()
{
	int nNumSkins = 0;

	if ( m_pSkinsList )
	{
		m_pSkinsList->RemoveAll();

		studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL );
		if ( hdr )
		{
			nNumSkins = hdr->numskinfamilies;
			for ( int i = 0; i < nNumSkins; i++ )
			{
				char skinText[25] = "";
				sprintf( skinText, "skin%i", i );
				KeyValues *pkv = new KeyValues("node", "skin", skinText );
				m_pSkinsList->AddItem( pkv, 0, false, false );
			}
		}
	}
		
	return nNumSkins;
}

void CTFMDLPicker::UpdateInfoTab()
{
	studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL );
	if ( !hdr )
		return;
	
	int nMass = hdr->mass;
	Panel *pTempPanel = m_pInfoPage->FindChildByName("MassValue");
	char massBuff[10];
	V_snprintf( massBuff, sizeof(massBuff), "%d", nMass );
	if (pTempPanel)
	{
		((vgui::Label *)pTempPanel)->SetText(massBuff);
	}
	bool bIsStatic = hdr->flags & STUDIOHDR_FLAGS_STATIC_PROP;
	bool bIsPhysics = false;
	const char* buf = hdr->KeyValueText();
	Label * pTempLabel = (Label *)m_pInfoPage->FindChildByName("StaticText");
	if (pTempLabel)		//I will actually beat you for these crashes.
	{
		pTempLabel->SetVisible(false);
	}
	if( buf )
	{
		buf = Q_strstr( buf, "prop_data" );
		if ( buf )
		{
			int iPropDataCount = UpdatePropDataList( buf, bIsStatic );
			if( iPropDataCount )
			{
				bIsPhysics = true;
			}
		}
		else
		{
			m_pPropDataList->RemoveAll();
		}
	}
	else
	{
		m_pPropDataList->RemoveAll();
	}
	
	CheckButton * pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("StaticObject");
	if (pTempCheck)
	{
		pTempCheck->SetCheckButtonCheckable(true);
		pTempCheck->SetSelected(bIsStatic);
		pTempCheck->SetCheckButtonCheckable(false);
		pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("PhysicsObject");
		pTempCheck->SetCheckButtonCheckable(true);
		pTempCheck->SetSelected(bIsPhysics);
		pTempCheck->SetCheckButtonCheckable(false);
		pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("DynamicObject");
		pTempCheck->SetCheckButtonCheckable(true);
		pTempCheck->SetSelected(!bIsPhysics);
		pTempCheck->SetCheckButtonCheckable(false);
	}


}

int CTFMDLPicker::UpdatePropDataList( const char* pszPropData, bool &bIsStatic )
{
	int iCount = 0;  

	if ( m_pPropDataList )
	{
		m_pPropDataList->RemoveAll();

		const char * endPropData = strchr( pszPropData, '}' );
		char keyText[255] = "";
		char valueText[255] = "";
		const char *beginChunk = strchr( pszPropData, '\"' );
		if ( !beginChunk )
		{
			return 0;
		}
		beginChunk++;
		const char *endChunk = strchr( beginChunk, '\"' );
		while( endChunk )
		{
			Q_memcpy( keyText, beginChunk, endChunk - beginChunk );
			beginChunk = endChunk + 1;
			beginChunk = strchr( beginChunk, '\"' ) + 1;
			endChunk = strchr( beginChunk, '\"' );
			Q_memcpy( valueText, beginChunk, endChunk - beginChunk );		
			if( !Q_strcmp( keyText, "allowstatic" ) && !Q_strcmp( valueText , "1" ) )
			{
				if ( !bIsStatic )
				{					
					Label * pTempLabel = (Label *)m_pInfoPage->FindChildByName("StaticText");
					pTempLabel->SetVisible( true );
				}
				bIsStatic &= true;
			}
			KeyValues *pkv = new KeyValues("node", "key", keyText, "value", valueText );
			m_pPropDataList->AddItem( pkv, 0, false, false );
			Q_memset( keyText, 0, 255 );
			Q_memset( valueText, 0, 255 );
			iCount++;
			beginChunk = endChunk + 1;
			beginChunk = strchr( beginChunk, '\"' );
			if ( !beginChunk || beginChunk > endPropData )
			{
				return iCount;
			}
			beginChunk++;
			endChunk = strchr( beginChunk, '\"' );		
		}
	}
	return iCount;
}

static CTFMDLPickerDialog *g_pMDLPickerDialog = NULL;

CTFMDLPickerDialog *GetMDLPickerDialog()
{
	Assert( g_pMDLPickerDialog );
	return g_pMDLPickerDialog;
}

static void CC_MDLPickerDialog()
{
	CTFMDLPickerDialog *pDialog = GetMDLPickerDialog();
	if ( pDialog )
	{
		pDialog->SetVisible( pDialog->IsVisible() ? false : true );
		pDialog->MoveToFront();
		pDialog->InvalidateLayout( true, true );
	}
}
void MDLPickerDialog_Create( vgui::VPANEL parent )
{
	Assert( !g_pMDLPickerDialog );

	new CTFMDLPickerDialog( parent, "MDLPickerDialog" );
}

void MDLPickerDialog_Destroy()
{
	if ( g_pMDLPickerDialog )
	{
		g_pMDLPickerDialog->MarkForDeletion();
		g_pMDLPickerDialog = NULL;
	}
}

static ConCommand mdlpickerdialog( "mdlpickerdialog", CC_MDLPickerDialog, "Show/hide model browser UI." );


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFMDLPickerDialog::CTFMDLPickerDialog( VPANEL parent, char const *panelName ) : BaseClass( NULL, panelName )
{
	g_pMDLPickerDialog = this;
	SetParent( parent );

	SetMoveable( false );
	SetSizeable( false );

	m_pPicker = new CTFMDLPicker( this );
	m_pButtonOK = new vgui::Button( this, "OpenButton", "OK", this, "OK" );
	m_pButtonCancel = new vgui::Button( this, "CancelButton", "Cancel", this, "Cancel" );
	m_pStatusLine = new vgui::TextEntry( NULL, "StatusLine" );

	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/SourceScheme.res", "SourceScheme" ) );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFMDLPickerDialog::~CTFMDLPickerDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetProportional( true );

	LoadControlSettings( "Resource/ModelBrowserDialog.res" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings( inResourceData );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pPicker->SetBounds( 0,0, GetWide(), GetTall() - 30 );
	m_pStatusLine->SetBounds( 160, GetWide() - 30, max( 100, GetTall() - 166 ), 24 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::Activate()
{
	InvalidateLayout( true, true );

	BaseClass::Activate();

	m_pPicker->Activate();
	m_pStatusLine->SetParent( this );
	m_pStatusLine->SetEditable( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::OnClose()
{
	BaseClass::OnClose();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "OK" ) )
	{
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pPlayer )
		{
			char szModel[256];
			char szCmd[256];
			GetModelName( szModel, sizeof(szModel) );
			Q_snprintf( szCmd, sizeof(szCmd), "cheatgun_build prop_dynamic model %s", szModel );
			engine->ExecuteClientCmd( szCmd );
			SetVisible( false );
			engine->ExecuteClientCmd( "gameui_hide" );
		}
	}
	else if ( !V_stricmp( command, "Cancel" ) )
	{
		SetVisible( false );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::SetModelName( const char *pModelName )
{
	char pszTempModelName[255];
	strcpy( pszTempModelName, pModelName );

	char * pszSelectedModel = strchr( pszTempModelName, '/' );
	if( pszSelectedModel)
	{
		pszSelectedModel += 1;
		Q_FixSlashes( pszSelectedModel, '\\' );
	}

	m_pPicker->SelectMDL( pModelName );
	m_pPicker->SetInitialSelection( pszSelectedModel );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::GetModelName( char *pModelName, int length )
{
	m_pPicker->GetSelectedMDLName( pModelName, length );

	Q_FixSlashes( pModelName, '/' );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::GetSkin( int &nSkin )
{
	nSkin = m_pPicker->GetSelectedSkin();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::SetSkin( int nSkin )
{
	m_pPicker->SelectSkin( nSkin );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFMDLPickerDialog::UpdateStatusLine()
{
	char szModel[1024];

	m_pPicker->GetSelectedMDLName( szModel, sizeof(szModel) );

	m_pStatusLine->SetText( szModel );

/*	MDLHandle_t hMDL = vgui::MDLCache()->FindMDL( szModel );
	studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr( hMDL );
	vgui::MDLCache()->Release( hMDL ); */
}
