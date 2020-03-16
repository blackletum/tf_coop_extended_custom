#include "cbase.h"
#include "charinfo_loadout_subpanel.h"

enum CharInfoLoadoutPanels
{
	CharPanel_Loadout
};

CCharInfoLoadoutSubPanel::CCharInfoLoadoutSubPanel( vgui::Panel *parent ) : vgui::PropertyPage( parent, "CharInfoLoadoutSubPanel" )
{
	g_pVGui->AddTickSignal( GetVPanel() );

	m_pClassLoadout = new CClassLoadoutPanel( this );
}

void CCharInfoLoadoutSubPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "Resource/UI/CharInfoLoadoutSubPanel.res" );
}

void CCharInfoLoadoutSubPanel::OnCommand( const char* command )
{
	if ( V_strnicmp( command, "loadout ", 8 ) == 0 )
	{
		int iClass = GetClassIndexFromString( command + 8, TF_LAST_NORMAL_CLASS+1 );
		if ( iClass )
		{
			if ( m_iCurrentClass != iClass )
			{
				m_iCurrentClass = iClass;
				m_iActivePanel = CharPanel_Loadout;
				UpdateModelPanels( true );
			//	RequestInventoryRefresh()
			}
		}
	}
	else
	{
		return BaseClass::OnCommand( command );
	}
}

void CCharInfoLoadoutSubPanel::UpdateModelPanels( bool unknown )
{
	switch ( m_iActivePanel )
	{
	case CharPanel_Loadout:
	{
		//Hide other panels here if we ever implement them

		m_pClassLoadout->SetTeam( TF_TEAM_RED );
		m_pClassLoadout->SetClass( m_iCurrentClass );
		m_pClassLoadout->SetVisible( true );
	}
	default:
		break;
	}
}

void CCharInfoLoadoutSubPanel::CloseLoadout()
{
	if ( m_pClassLoadout->InSelectionPanel() )
		m_pClassLoadout->CloseSelectionPanel();
	else
		m_pClassLoadout->SetVisible( false );

	m_iActivePanel = -1;
	m_iCurrentClass = TF_CLASS_UNDEFINED;
}
