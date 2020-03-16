//=============================================================================
//
// Purpose: COME OVER HERE
//
//=============================================================================

#include "cbase.h"
#include "lfe_hud_pingsystem.h"
#include "iclientmode.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include "view.h"
#include "ivieweffects.h"
#include "viewrender.h"
#include "prediction.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PINGSYSTEM_WIDE			(XRES(56))
#define PINGSYSTEM_TALL			(YRES(30))
#define PINGSYSTEM_ARROW_WIDE	(XRES(16))
#define PINGSYSTEM_ARROW_TALL	(YRES(24))

enum
{
	DRAW_ARROW_UP,
	DRAW_ARROW_LEFT,
	DRAW_ARROW_RIGHT
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFPingSystemPanel::CTFPingSystemPanel( Panel *parent, const char *name ) : EditablePanel(parent,name)
{
	m_pArrowMaterial = NULL;
	m_iDrawArrow = DRAW_ARROW_UP;
	m_bOnscreen = false;

	m_pPingSpy = new CTFImagePanel( this, "PingSpy" );
	m_pPingNeedSentry = new CTFImagePanel( this, "PingNeedSentry" );
	m_pPingNeedDispenser = new CTFImagePanel( this, "PingNeedDispenser" );
	m_pPingNeedTeleporter = new CTFImagePanel( this, "PingNeedTeleporter" );
	m_pPingItem = new CTFImagePanel( this, "PingItem" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFPingSystemPanel::~CTFPingSystemPanel( void )
{
	if ( m_pArrowMaterial )
	{
		m_pArrowMaterial->DecrementReferenceCount();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Applies scheme settings
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/UI/PingSystemPanel.res" );

	if ( m_pArrowMaterial )
		m_pArrowMaterial->DecrementReferenceCount();

	m_pArrowMaterial = materials->FindMaterial( "HUD/medic_arrow", TEXTURE_GROUP_VGUI );
	m_pArrowMaterial->IncrementReferenceCount();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::PerformLayout( void )
{
	BaseClass::PerformLayout();

	if ( m_pPingSpy )
		m_pPingSpy->SetPos( (GetWide() - m_pPingSpy->GetWide()) * 0.5, (GetTall() - m_pPingSpy->GetTall()) * 0.5 );

	if ( m_pPingNeedSentry )
		m_pPingNeedSentry->SetPos( (GetWide() - m_pPingNeedSentry->GetWide()) * 0.5, (GetTall() - m_pPingNeedSentry->GetTall()) * 0.5 );

	if ( m_pPingNeedDispenser )
		m_pPingNeedDispenser->SetPos( (GetWide() - m_pPingNeedDispenser->GetWide()) * 0.5, (GetTall() - m_pPingNeedDispenser->GetTall()) * 0.5 );

	if ( m_pPingNeedTeleporter )
		m_pPingNeedTeleporter->SetPos( (GetWide() - m_pPingNeedTeleporter->GetWide()) * 0.5, (GetTall() - m_pPingNeedTeleporter->GetTall()) * 0.5 );

	if ( m_pPingItem )
		m_pPingItem->SetPos( (GetWide() - m_pPingItem->GetWide()) * 0.5, (GetTall() - m_pPingItem->GetTall()) * 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::GetPingPosition( const Vector &vecDelta, float flRadius, float *xpos, float *ypos, float *flRotation )
{
	// Player Data
	Vector playerPosition = MainViewOrigin();
	QAngle playerAngles = MainViewAngles();

	Vector forward, right, up(0,0,1);
	AngleVectors (playerAngles, &forward, NULL, NULL );
	forward.z = 0;
	VectorNormalize(forward);
	CrossProduct( up, forward, right );
	float front = DotProduct(vecDelta, forward);
	float side = DotProduct(vecDelta, right);
	*xpos = flRadius * -side;
	*ypos = flRadius * -front;

	// Get the rotation (yaw)
	*flRotation = atan2(*xpos,*ypos) + M_PI;
	*flRotation *= 180 / M_PI;

	float yawRadians = -(*flRotation) * M_PI / 180.0f;
	float ca = cos( yawRadians );
	float sa = sin( yawRadians );

	// Rotate it around the circle
	*xpos = (int)((ScreenWidth() / 2) + (flRadius * sa));
	*ypos = (int)((ScreenHeight() / 2) - (flRadius * ca));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::OnTick( void )
{
	if ( gpGlobals->curtime > m_flRemoveAt )
	{
		MarkForDeletion();
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::PaintBackground( void )
{
	//C_TFPlayer *pTFPlayer = ToTFPlayer( m_hPingTarget );
	// If the local player has started healing this guy, remove it too.
	//Also don't draw it if we're dead.
	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalTFPlayer )
		return;

	/*if ( !pTFPlayer || pTFPlayer->IsDormant() )
	{
		SetAlpha(0);
		return;
	}*/

	// Reposition the callout based on our target's position
	int iX, iY;
	Vector vecTarget = ( m_vecPingOrigin );
	Vector vecDelta = vecTarget - MainViewOrigin();
	bool bOnscreen = GetVectorInScreenSpace( vecTarget, iX, iY );

	int halfWidth = GetWide() / 2;
	if( !bOnscreen || iX < halfWidth || iX > ScreenWidth()-halfWidth )
	{
		// It's off the screen. Position the callout.
		VectorNormalize(vecDelta);
		float xpos, ypos;
		float flRotation;
		float flRadius = YRES(100);
		GetPingPosition( vecDelta, flRadius, &xpos, &ypos, &flRotation );

		iX = xpos;
		iY = ypos;

		Vector vCenter = m_hPingTarget ? m_hPingTarget->WorldSpaceCenter() : vecTarget;
		if( MainViewRight().Dot( vCenter - MainViewOrigin() ) > 0 )
		{
			m_iDrawArrow = DRAW_ARROW_RIGHT;
		}
		else
		{
			m_iDrawArrow = DRAW_ARROW_LEFT;
		}

		// Move the icon there
		SetPos( iX - halfWidth, iY - (GetTall() / 2) );
		SetAlpha( 255 );
	}
	else // On screen
	{
		// If our target isn't visible, we draw transparently
		trace_t	tr;
		UTIL_TraceLine( vecTarget, MainViewOrigin(), MASK_OPAQUE, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction >= 1.0f )
		{
			m_bOnscreen = true;
			m_iDrawArrow = DRAW_ARROW_UP;
			SetAlpha( 255 );
			SetPos( iX - halfWidth, iY - (GetTall() / 2) );
			return;
		}

		m_iDrawArrow = DRAW_ARROW_UP;
		SetAlpha( 60 );
		SetPos( iX - halfWidth, iY - (GetTall() / 2) );
	}

	m_bOnscreen = false;
	BaseClass::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::Paint( void )
{
	// Don't draw if our target is visible.
	if ( m_bOnscreen )
		return;

	BaseClass::Paint();

	if ( m_iDrawArrow == DRAW_ARROW_UP )
		return;

	float uA,uB,yA,yB;
	int x,y;
	GetPos( x,y );
	if ( m_iDrawArrow == DRAW_ARROW_LEFT )
	{
		uA = 1.0;
		uB = 0.0;
		yA = 0.0;
		yB = 1.0;
	}
	else
	{
		uA = 0.0;
		uB = 1.0;
		yA = 0.0;
		yB = 1.0;
		x += GetWide() - PINGSYSTEM_ARROW_WIDE;
	}

	int iyindent = (GetTall() - PINGSYSTEM_ARROW_TALL) * 0.5;
	y += iyindent;

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( m_pArrowMaterial );
	IMesh* pMesh = pRenderContext->GetDynamicMesh( true );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	meshBuilder.Position3f( x, y, 0.0f );
	meshBuilder.TexCoord2f( 0, uA, yA );
	meshBuilder.Color4ub( 255, 255, 255, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( x + PINGSYSTEM_ARROW_WIDE, y, 0.0f );
	meshBuilder.TexCoord2f( 0, uB, yA );
	meshBuilder.Color4ub( 255, 255, 255, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( x + PINGSYSTEM_ARROW_WIDE, y + PINGSYSTEM_ARROW_TALL, 0.0f );
	meshBuilder.TexCoord2f( 0, uB, yB );
	meshBuilder.Color4ub( 255, 255, 255, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( x, y + PINGSYSTEM_ARROW_TALL, 0.0f );
	meshBuilder.TexCoord2f( 0, uA, yB );
	meshBuilder.Color4ub( 255, 255, 255, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::FireGameEvent( IGameEvent * event )
{
	/*if ( FStrEq( "", event->GetName() ) )
	{
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: usable stuff
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::SetPingTarget( C_BaseEntity *pTarget )
{
	m_hPingTarget = pTarget;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::SetPingDuration( float flDuration )
{
	m_flRemoveAt = gpGlobals->curtime + flDuration;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::SetPingOrigin( Vector &vecOrigin )
{
	m_vecPingOrigin = vecOrigin;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPingSystemPanel::AddPing( int nType, Vector &vecOrigin, float flDuration, C_BaseEntity *pTarget )
{
	CTFPingSystemPanel *pPingSystemPanel = new CTFPingSystemPanel( g_pClientMode->GetViewport(), "PingSystemPanel" );
	vgui::SETUP_PANEL( pPingSystemPanel );
	pPingSystemPanel->SetBounds( 0,0, PINGSYSTEM_WIDE, PINGSYSTEM_TALL );

	if ( pTarget )
		pPingSystemPanel->SetPingTarget( pTarget );

	pPingSystemPanel->SetPingDuration( flDuration );
	pPingSystemPanel->SetPingOrigin( vecOrigin );

	switch ( nType )
	{
	case TF_PING_SPY:
		if ( pPingSystemPanel->m_pPingSpy )
			pPingSystemPanel->m_pPingSpy->SetVisible( true );
		break;

	case TF_PING_SENTRY:
		if ( pPingSystemPanel->m_pPingNeedSentry )
			pPingSystemPanel->m_pPingNeedSentry->SetVisible( true );
		break;

	case TF_PING_DISPENSER:
		if ( pPingSystemPanel->m_pPingNeedDispenser )
			pPingSystemPanel->m_pPingNeedDispenser->SetVisible( true );
		break;

	case TF_PING_TELEPORTER:
		if ( pPingSystemPanel->m_pPingNeedTeleporter )
			pPingSystemPanel->m_pPingNeedTeleporter->SetVisible( true );
		break;

	case TF_PING_GO:
		/*if ( m_hPingTarget )
		{
			if ( pPingSystemPanel->m_pPingItem )
				pPingSystemPanel->m_pPingItem->SetVisible( true );
		}*/
		pPingSystemPanel->MarkForDeletion();
		return;
		break;

	case TF_PING_NONE:
	default:
		pPingSystemPanel->MarkForDeletion();
		return;
		break;
	}

	pPingSystemPanel->SetVisible( true );
	vgui::ivgui()->AddTickSignal( pPingSystemPanel->GetVPanel() );
}