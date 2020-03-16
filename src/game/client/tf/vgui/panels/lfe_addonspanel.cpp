//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "lfe_addonspanel.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_advslider.h"
#include "vgui_controls/SectionedListPanel.h"
#include "vgui_controls/ImagePanel.h"
#include "engine/IEngineSound.h"
#include "vgui_avatarimage.h"
#include "tf_gamerules.h"
#include <KeyValues.h>
#include "tier0/icommandline.h"

using namespace vgui;
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// LF:E Addons panel
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFAddonsPanel::CTFAddonsPanel( vgui::Panel* parent, const char *panelName ) : CTFDialogPanelBase( parent, panelName )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFAddonsPanel::~CTFAddonsPanel()
{
}

void CTFAddonsPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "resource/UI/main_menu/AddonsPanel.res" );
}

void CTFAddonsPanel::OnCommand(const char* command)
{
	BaseClass::OnCommand( command );
}


void CTFAddonsPanel::Show()
{
	BaseClass::Show();
};

void CTFAddonsPanel::Hide()
{
	BaseClass::Hide();
};



void UpdateAddonsSearchPaths()
{
	char addonlistFilename[MAX_PATH];
	char modPath[MAX_PATH];

	g_pFullFileSystem->GetSearchPath( "MOD", false, modPath, MAX_PATH );
	char *pSemi = V_strrchr( modPath, ';');
	if ( pSemi )
		V_strncpy( modPath, ++pSemi, MAX_PATH );

	V_snprintf( addonlistFilename, sizeof( addonlistFilename ), "%s%s", modPath, "addons/*" );

	//g_pFullFileSystem->AddSearchPath( addonlistFilename, "MOD" );
	ConColorMsg( Color( 77, 116, 85, 255 ), "[Addons] Found path: '%s'\n", addonlistFilename );
}