//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"

#include <stdio.h>
#include <time.h>

#include "lfe_createmultiplayergamegameplaypage.h"

#include <KeyValues.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>

#include "filesystem.h"
#include "controls/tf_cvartogglecheckbutton.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_advpanellistpanel.h"
#include "controls/tf_advslider.h"
#include "controls/tf_cvarslider.h"
#include "controls/tf_scriptobject.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

#define OPTIONS_DIR "cfg"
#define DEFAULT_OPTIONS_FILE OPTIONS_DIR "/settings_default.scr"
#define OPTIONS_FILE OPTIONS_DIR "/settings.scr"

//-----------------------------------------------------------------------------
// Purpose: class for loading/saving server config file
//-----------------------------------------------------------------------------
class CServerDescription : public CDescription
{
public:
	CServerDescription( CPanelListPanel *panel );

	void WriteScriptHeader( FileHandle_t fp );
	void WriteFileHeader( FileHandle_t fp ); 
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFCreateMultiplayerGameGameplayPage::CTFCreateMultiplayerGameGameplayPage(vgui::Panel *parent, const char *name) : CTFDialogPanelBase(parent, name)
{
	Init();
}

bool CTFCreateMultiplayerGameGameplayPage::Init()
{
	BaseClass::Init();

	m_pOptionsList = new CPanelListPanel(this, "GameOptions");

	m_pDescription = new CServerDescription(m_pOptionsList);
	m_pDescription->InitFromFile( DEFAULT_OPTIONS_FILE );
	m_pDescription->InitFromFile( OPTIONS_FILE );
	m_pList = NULL;

	LoadControlSettings("resource/UI/main_menu/CreateMultiplayerGameGameplayPage.res");

	LoadGameOptionsList();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFCreateMultiplayerGameGameplayPage::~CTFCreateMultiplayerGameGameplayPage()
{
	delete m_pDescription;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFCreateMultiplayerGameGameplayPage::GetMaxPlayers()
{
	return atoi(GetValue("maxplayers", "32"));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFCreateMultiplayerGameGameplayPage::GetPassword()
{
	return GetValue("sv_password", "");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFCreateMultiplayerGameGameplayPage::GetHostName()
{
	return GetValue("hostname", "Half-Life");	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFCreateMultiplayerGameGameplayPage::GetValue(const char *cvarName, const char *defaultValue)
{
	for (mpcontrol_t *mp = m_pList; mp != NULL; mp = mp->next)
	{
		Panel *control = mp->pControl;
		if (control && !stricmp(mp->GetName(), cvarName))
		{
			KeyValues *data = new KeyValues("GetText");
			static char buf[128];
			if (control && control->RequestInfo(data))
			{
				Q_strncpy(buf, data->GetString("text", defaultValue), sizeof(buf) - 1);
			}
			else
			{
				// no value found, copy in default text
				Q_strncpy(buf, defaultValue, sizeof(buf) - 1);
			}

			// ensure null termination of string
			buf[sizeof(buf) - 1] = 0;

			// free
			data->deleteThis();
			return buf;
		}

	}

	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: called to get data from the page
//-----------------------------------------------------------------------------
void CTFCreateMultiplayerGameGameplayPage::OnApplyChanges()
{
	// Get the values from the controls
	GatherCurrentValues();

	// Create the game.cfg file
	if ( m_pDescription )
	{
		FileHandle_t fp;

		// Add settings to config.cfg
		m_pDescription->WriteToConfig();

		g_pFullFileSystem->CreateDirHierarchy( OPTIONS_DIR, "MOD" );
		fp = g_pFullFileSystem->Open( OPTIONS_FILE, "wb", "MOD" );
		if ( fp )
		{
			m_pDescription->WriteToScriptFile( fp );
			g_pFullFileSystem->Close( fp );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates all the controls in the game options list
//-----------------------------------------------------------------------------
void CTFCreateMultiplayerGameGameplayPage::LoadGameOptionsList()
{
	BaseClass::CreateControls();

	// Go through desciption creating controls
	CScriptObject *pObj;
	pObj = m_pDescription->pObjList;
	
	mpcontrol_t	*pCtrl;
	CTFCvarToggleCheckButton *pBox;
	TextEntry *pEdit;
	ComboBox *pCombo;
	CTFCvarSlider *pScroll;
	Label *pTitle;
	CScriptListItem *pListItem;

	HFont hFont = GETSCHEME()->GetFont( m_pOptionsList->GetFontString(), true );

	Panel *objParent = m_pOptionsList;
	while ( pObj )
	{
		if ( pObj->type == O_OBSOLETE )
		{
			pObj = pObj->pNext;
			continue;
		}

		pCtrl = new mpcontrol_t( objParent, pObj->cvarname );
		pCtrl->type = pObj->type;

		switch ( pCtrl->type )
		{
		case O_BOOL:
			pBox = new CTFCvarToggleCheckButton( pCtrl, "DescCheckButton", pObj->prompt, pObj->cvarname );
			pBox->MakeReadyForUse();

			pBox->SetFont( hFont );
			pBox->SetToolTip( pObj->tooltip );

			pCtrl->pControl = pBox;
			break;
		case O_STRING:
		case O_NUMBER:
			pEdit = new TextEntry( pCtrl, "DescTextEntry");
			pEdit->MakeReadyForUse();
			pEdit->SetFont( hFont );

			pEdit->InsertString(pObj->curValue);

			pCtrl->pControl = pEdit;
			break;
		case O_SLIDER:
			pScroll = new CTFCvarSlider( pCtrl, "DescScrollEntry", pObj->prompt, pObj->fMin, pObj->fMax, pObj->cvarname );
			pScroll->MakeReadyForUse();

			pScroll->SetFont( hFont );
			pScroll->GetButton()->SetToolTip( pObj->tooltip );

			pCtrl->pControl = pScroll;
			break;
		case O_LIST:
			pCombo = new ComboBox(pCtrl, "DescComboBox", 5, false);
			pCombo->MakeReadyForUse();
			pCombo->SetFont( hFont );

			pListItem = pObj->pListItems;
			while ( pListItem )
			{
				pCombo->AddItem( pListItem->szItemText, NULL );
				pListItem = pListItem->pNext;
			}

			pCombo->ActivateItemByRow((int)pObj->fcurValue);

			pCtrl->pControl = pCombo;
			break;
		case O_CATEGORY:
			pTitle = new Label( pCtrl, "DescTextTitle", pObj->prompt );
			pTitle->MakeReadyForUse();

			pTitle->SetBorder( GETSCHEME()->GetBorder( "AdvSettingsTitleBorder" ) );
			pTitle->SetFont( GETSCHEME()->GetFont( "MenuSmallFont", true ) );
			pTitle->SetFgColor( GETSCHEME()->GetColor( ADVBUTTON_DEFAULT_COLOR, COLOR_WHITE ) );

			pCtrl->pControl = pTitle;
			break;
		default:
			break;
		}

		if ( pCtrl->type != O_BOOL && pCtrl->type != O_SLIDER && pCtrl->type != O_CATEGORY )
		{
			pCtrl->pPrompt = new Label(pCtrl, "DescLabel", "");
			pCtrl->pPrompt->MakeReadyForUse();

			pCtrl->pPrompt->SetFont( hFont );
			pCtrl->pPrompt->SetContentAlignment(vgui::Label::a_west);
			pCtrl->pPrompt->SetTextInset(5, 0);
			pCtrl->pPrompt->SetText(pObj->prompt);
		}

		pCtrl->pScrObj = pObj;
		int h = m_pOptionsList->GetTall() / 13.0; //(float)GetParent()->GetTall() / 15.0;
		pCtrl->SetSize(800, h);
		//pCtrl->SetBorder( scheme()->GetBorder(1, "DepressedButtonBorder") );
		m_pOptionsList->AddItem(pCtrl);

		// Link it in
		if ( !m_pList )
		{
			m_pList = pCtrl;
			pCtrl->next = NULL;
		}
		else
		{
			mpcontrol_t *p;
			p = m_pList;
			while ( p )
			{
				if ( !p->next )
				{
					p->next = pCtrl;
					pCtrl->next = NULL;
					break;
				}
				p = p->next;
			}
		}

		pObj = pObj->pNext;
	}
}

//-----------------------------------------------------------------------------
// Purpose: applies all the values in the page
//-----------------------------------------------------------------------------
void CTFCreateMultiplayerGameGameplayPage::GatherCurrentValues()
{
	if ( !m_pDescription )
		return;
	
	// OK
	CTFCheckButton *pBox;
	TextEntry *pEdit;
	ComboBox *pCombo;
	CTFSlider *pScroll;

	mpcontrol_t *pList;

	CScriptObject *pObj;
	CScriptListItem *pItem;

	char szValue[256];

	pList = m_pList;
	while ( pList )
	{
		pObj = pList->pScrObj;

		if ( !pList->pControl )
		{
			pObj->SetCurValue( pObj->defValue );
			pList = pList->next;
			continue;
		}

		switch ( pObj->type )
		{
		case O_BOOL:
			pBox = (CTFCheckButton *)pList->pControl;
			sprintf(szValue, "%d", pBox->IsChecked() ? 1 : 0);
			break;
		case O_NUMBER:
			pEdit = (TextEntry *)pList->pControl;
			pEdit->GetText( szValue, sizeof( szValue ) );
			break;
		case O_SLIDER:
			pScroll = (CTFSlider *)pList->pControl;
			V_strncpy( szValue, pScroll->GetFinalValue(), sizeof( szValue ) );
			break;
		case O_STRING:
			pEdit = (TextEntry *)pList->pControl;
			pEdit->GetText(szValue, sizeof(szValue));
			break;
		case O_CATEGORY:
			break;
		case O_LIST:
			pCombo = (ComboBox *)pList->pControl;
			pCombo->GetText( szValue, sizeof( szValue ) );
			int activeItem = pCombo->GetActiveItem();

			pItem = pObj->pListItems;
			//			int n = (int)pObj->fcurValue;

			while (pItem)
			{
				if (!activeItem--)
					break;

				pItem = pItem->pNext;
			}

			if (pItem)
			{
				sprintf(szValue, "%s", pItem->szValue);
			}
			else  // Couln't find index
			{
				//assert(!("Couldn't find string in list, using default value"));
				sprintf(szValue, "%s", pObj->curValue);
			}
			break;
		}

		// Remove double quotes and % characters
		UTIL_StripInvalidCharacters(szValue, sizeof(szValue));

		pObj->SetCurValue( szValue );

		pList = pList->next;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Constructor, load/save server settings object
//-----------------------------------------------------------------------------
CServerDescription::CServerDescription(CPanelListPanel *panel) : CDescription(panel)
{
	setHint( "// NOTE:  THIS FILE IS AUTOMATICALLY REGENERATED, \r\n"
"//DO NOT EDIT THIS HEADER, YOUR COMMENTS WILL BE LOST IF YOU DO\r\n"
"// Multiplayer options script\r\n"
"//\r\n"
"// Format:\r\n"
"//  Version [float]\r\n"
"//  Options description followed by \r\n"
"//  Options defaults\r\n"
"//\r\n"
"// Option description syntax:\r\n"
"//\r\n"
"//  \"cvar\" { \"Prompt\" { type [ type info ] } { default } }\r\n"
"//\r\n"
"//  type = \r\n"
"//   BOOL   (a yes/no toggle)\r\n"
"//   STRING\r\n"
"//   NUMBER\r\n"
"//   LIST\r\n"
"//\r\n"
"// type info:\r\n"
"// BOOL                 no type info\r\n"
"// NUMBER       min max range, use -1 -1 for no limits\r\n"
"// STRING       no type info\r\n"
"// LIST         "" delimited list of options value pairs\r\n"
"//\r\n"
"//\r\n"
"// default depends on type\r\n"
"// BOOL is \"0\" or \"1\"\r\n"
"// NUMBER is \"value\"\r\n"
"// STRING is \"value\"\r\n"
"// LIST is \"index\", where index \"0\" is the first element of the list\r\n\r\n\r\n" );

	setDescription ( "SERVER_OPTIONS" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CServerDescription::WriteScriptHeader( FileHandle_t fp )
{
	char am_pm[] = "AM";
	tm newtime;
	VCRHook_LocalTime(&newtime);

	g_pFullFileSystem->FPrintf(fp, (char *)getHint());

	// Write out the comment and Cvar Info:
	g_pFullFileSystem->FPrintf( fp, "// Half-Life Server Configuration Layout Script (stores last settings chosen, too)\r\n" );
	g_pFullFileSystem->FPrintf( fp, "// File generated:  %.19s %s\r\n", asctime(&newtime), am_pm );
	g_pFullFileSystem->FPrintf( fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n" );

	g_pFullFileSystem->FPrintf( fp, "VERSION %.1f\r\n\r\n", SCRIPT_VERSION );

	g_pFullFileSystem->FPrintf( fp, "DESCRIPTION SERVER_OPTIONS\r\n{\r\n" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CServerDescription::WriteFileHeader( FileHandle_t fp )
{
    char am_pm[] = "AM";
    tm newtime;
	VCRHook_LocalTime( &newtime );

	g_pFullFileSystem->FPrintf( fp, "// Half-Life Server Configuration Settings\r\n" );
	g_pFullFileSystem->FPrintf( fp, "// DO NOT EDIT, GENERATED BY HALF-LIFE\r\n" );
	g_pFullFileSystem->FPrintf( fp, "// File generated:  %.19s %s\r\n", asctime(&newtime), am_pm );
	g_pFullFileSystem->FPrintf( fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n" );
}