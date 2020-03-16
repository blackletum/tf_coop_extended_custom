//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include <vgui_controls/ScrollBarSlider.h>
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "tf_particlepanel.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"


using namespace vgui;

DECLARE_BUILD_FACTORY( CTFParticlePanel );

//-----------------------------------------------------------------------------
// Purpose: Reverse engie live TF2 CTFParticlePanel
//-----------------------------------------------------------------------------
CTFParticlePanel::CTFParticlePanel( Panel *parent, const char *panelName ) : BaseClass( parent, panelName ) 
{
	m_pParticleInfo = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFParticlePanel::~CTFParticlePanel()
{
	if ( m_pParticleInfo )
	{
		delete m_pParticleInfo;
		m_pParticleInfo = NULL;
	}

	if ( m_pParticleSystem )
	{
		delete m_pParticleSystem;
		m_pParticleSystem = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticlePanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticlePanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

 	for ( KeyValues *pData = inResourceData->GetFirstSubKey() ; pData != NULL ; pData = pData->GetNextKey() )
 	{
 		if ( !Q_stricmp( pData->GetName(), "ParticleEffects" ) )
 		{
	 		if ( m_pParticleInfo )
	 		{
	 			delete m_pParticleInfo;
	 			m_pParticleInfo = NULL;
	 		}

	 		m_pParticleInfo = new CParticlePanelInfo;
	 		if ( !m_pParticleInfo )
	 			return;

	 		m_pParticleInfo->m_pszParticleName = ReadAndAllocStringValue( inResourceData, "particleName" );

			m_pParticleSystem = g_pParticleSystemMgr->CreateParticleCollection( m_pParticleInfo->m_pszParticleName );

			m_pParticleInfo->m_bStartActive = inResourceData->GetInt( "start_activated", 1 );
			if ( m_pParticleInfo->m_bStartActive )
				m_pParticleSystem->StartEmission();

	 		m_pParticleInfo->m_vecOriginOffset.Init( inResourceData->GetFloat( "particle_xpos", 110.0 ), inResourceData->GetFloat( "particle_ypos", 5.0 ) );
 		}
 	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticlePanel::PaintBackground( void )
{
	BaseClass::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticlePanel::Paint( void )
{
	int x, y, w, h;
	GetBounds( x, y, w, h );
	ParentLocalToScreen( x, y );

	float flWidthRatio = engine->GetScreenAspectRatio() / ( 4.0f / 3.0f );

	CMatRenderContextPtr pRenderContext( materials );

	// figure out what our viewport is right now
	int viewportX, viewportY, viewportWidth, viewportHeight;
	pRenderContext->GetViewport( viewportX, viewportY, viewportWidth, viewportHeight );

	CViewSetup view;
	view.x = x + m_pParticleInfo->m_vecOriginOffset.x + viewportX;
	view.y = y + m_pParticleInfo->m_vecOriginOffset.y + viewportY;
	view.width = w;
	view.height = h;

	view.m_bOrtho = false;

	// scale the FOV for aspect ratios other than 4/3
	view.fov = ScaleFOVByWidthRatio( 90, flWidthRatio );

	view.origin = vec3_origin;
	view.angles.Init();
	view.zNear = VIEW_NEARZ;
	view.zFar = 1000;

	Frustum frustum;
	render->Push3DView( view, 0, NULL, frustum );

	m_pParticleSystem->Render( pRenderContext );

	render->PopView( frustum );
}

void CTFParticlePanel::OnTick()
{
	m_pParticleSystem->Simulate( gpGlobals->frametime, false );
}

void CTFParticlePanel::OnCommand(const char* command)
{
	BaseClass::OnCommand( command );
}