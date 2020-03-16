//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "tf_hud_mediccallers.h"
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

#define MEDICCALLER_WIDE		(XRES(56))
#define MEDICCALLER_TALL		(YRES(30))
#define MEDICCALLER_ARROW_WIDE	(XRES(16))
#define MEDICCALLER_ARROW_TALL	(YRES(24))

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFMedicCallerPanel::CTFMedicCallerPanel( Panel *parent, const char *name ) : EditablePanel(parent,name)
{
	m_pArrowMaterial = NULL;
	m_iDrawArrow = DRAW_ARROW_UP;
	m_bOnscreen = false;
	m_pCallerBG = new CTFImagePanel( this, "CallerBG" );
	m_pCallerBurning = new CTFImagePanel( this, "CallerBurning" );
	m_pCallerHealth = new CTFImagePanel( this, "CallerHealth" );
	m_pCallerBleeding = new CTFImagePanel( this, "CallerBleeding" );
	m_pCallerAuto = new CTFImagePanel( this, "CallerAuto" );
	m_pCallerReviveEasy = new CTFImagePanel( this, "CallerReviveEasy" );
	m_pCallerReviveMedium = new CTFImagePanel( this, "CallerReviveMedium" );
	m_pCallerReviveHard = new CTFImagePanel( this, "CallerReviveHard" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFMedicCallerPanel::~CTFMedicCallerPanel( void )
{
	if ( m_pArrowMaterial )
	{
		m_pArrowMaterial->DecrementReferenceCount();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Applies scheme settings
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/UI/MedicCallerPanel.res" );

	if ( m_pArrowMaterial )
	{
		m_pArrowMaterial->DecrementReferenceCount();
	}
	m_pArrowMaterial = materials->FindMaterial( "HUD/medic_arrow", TEXTURE_GROUP_VGUI );
	m_pArrowMaterial->IncrementReferenceCount();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::PerformLayout( void )
{
	BaseClass::PerformLayout();

	if ( m_pCallerBG )
		m_pCallerBG->SetPos( (GetWide() - m_pCallerBG->GetWide()) * 0.5, (GetTall() - m_pCallerBG->GetTall()) * 0.5 );

	if ( m_pCallerBurning )
		m_pCallerBurning->SetPos( (GetWide() - m_pCallerBurning->GetWide()) * 0.5, (GetTall() - m_pCallerBurning->GetTall()) * 0.5 );

	if ( m_pCallerHealth )
		m_pCallerHealth->SetPos( (GetWide() - m_pCallerHealth->GetWide()) * 0.5, (GetTall() - m_pCallerHealth->GetTall()) * 0.5 );

	if ( m_pCallerBleeding )
		m_pCallerBleeding->SetPos( (GetWide() - m_pCallerBleeding->GetWide()) * 0.5, (GetTall() - m_pCallerBleeding->GetTall()) * 0.5 );

	if ( m_pCallerAuto )
		m_pCallerAuto->SetPos( (GetWide() - m_pCallerAuto->GetWide()) * 0.5, (GetTall() - m_pCallerAuto->GetTall()) * 0.5 );

	if ( m_pCallerReviveEasy )
		m_pCallerReviveEasy->SetPos( (GetWide() - m_pCallerReviveEasy->GetWide()) * 0.5, (GetTall() - m_pCallerReviveEasy->GetTall()) * 0.5 );

	if ( m_pCallerReviveMedium )
		m_pCallerReviveMedium->SetPos( (GetWide() - m_pCallerReviveMedium->GetWide()) * 0.5, (GetTall() - m_pCallerReviveMedium->GetTall()) * 0.5 );

	if ( m_pCallerReviveHard )
		m_pCallerReviveHard->SetPos( (GetWide() - m_pCallerReviveHard->GetWide()) * 0.5, (GetTall() - m_pCallerReviveHard->GetTall()) * 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::GetCallerPosition( const Vector &vecDelta, float flRadius, float *xpos, float *ypos, float *flRotation )
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
void CTFMedicCallerPanel::OnTick( void )
{
	C_TFPlayer *pTFPlayer = ToTFPlayer( m_hCaller );
	if ( ( m_hCaller->IsPlayer() && pTFPlayer && !pTFPlayer->IsAlive() ) || gpGlobals->curtime > m_flRemoveAt )
	{
		MarkForDeletion();
		return;
	}

	// If the local player has started healing this guy, remove it too.
	//Also don't draw it if we're dead.
	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pLocalTFPlayer )
	{
		CBaseEntity *pHealTarget = pLocalTFPlayer->MedicGetHealTarget();
		if ( ( pHealTarget && pHealTarget == pTFPlayer) || !pLocalTFPlayer->IsAlive() )
		{
			MarkForDeletion();
			return;
		}

		// If we're pointing to an enemy spy and they are no longer disguised, remove ourselves
		if ( pTFPlayer->IsPlayerClass( TF_CLASS_SPY ) && 
			!( pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED ) && pTFPlayer->m_Shared.GetDisguiseTeam() == pLocalTFPlayer->GetTeamNumber() ) )
		{
			MarkForDeletion();
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::PaintBackground( void )
{
	C_TFPlayer *pTFPlayer = ToTFPlayer( m_hCaller );
	// If the local player has started healing this guy, remove it too.
	//Also don't draw it if we're dead.
	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalTFPlayer )
		return;

	if ( m_hCaller->IsPlayer() && ( !pTFPlayer || pTFPlayer->IsDormant() ) )
	{
		SetAlpha(0);
		return;
	}

	// Reposition the callout based on our target's position
	int iX, iY;
	Vector vecTarget = (m_hCaller->GetAbsOrigin() + m_vecOffset);
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
		GetCallerPosition( vecDelta, flRadius, &xpos, &ypos, &flRotation );

		iX = xpos;
		iY = ypos;

		Vector vCenter = m_hCaller->WorldSpaceCenter();
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
	else
	{
		// On screen
		// If our target isn't visible, we draw transparently
		trace_t	tr;
		UTIL_TraceLine( vecTarget, MainViewOrigin(), MASK_OPAQUE, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction >= 1.0f )
		{
			m_bOnscreen = true;
			SetAlpha( 0 );
			return;
		}

		m_iDrawArrow = DRAW_ARROW_UP;
		SetAlpha( 92 );
		SetPos( iX - halfWidth, iY - (GetTall() / 2) );
	}

	m_bOnscreen = false;
	BaseClass::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::Paint( void )
{
	// Don't draw if our target is visible. The particle effect will be doing it for us.
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
		x += GetWide() - MEDICCALLER_ARROW_WIDE;
	}

	int iyindent = (GetTall() - MEDICCALLER_ARROW_TALL) * 0.5;
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

	meshBuilder.Position3f( x + MEDICCALLER_ARROW_WIDE, y, 0.0f );
	meshBuilder.TexCoord2f( 0, uB, yA );
	meshBuilder.Color4ub( 255, 255, 255, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( x + MEDICCALLER_ARROW_WIDE, y + MEDICCALLER_ARROW_TALL, 0.0f );
	meshBuilder.TexCoord2f( 0, uB, yB );
	meshBuilder.Color4ub( 255, 255, 255, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( x, y + MEDICCALLER_ARROW_TALL, 0.0f );
	meshBuilder.TexCoord2f( 0, uA, yB );
	meshBuilder.Color4ub( 255, 255, 255, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::FireGameEvent( IGameEvent * event )
{
	/*if ( FStrEq( "", event->GetName() ) )
	{
	}*/
}


//-----------------------------------------------------------------------------
// Purpose: player, revive marker or something else?
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::SetCaller( C_BaseEntity *pCaller, float flDuration, Vector &vecOffset )
{
	m_hCaller = pCaller;
	m_flRemoveAt = gpGlobals->curtime + flDuration;
	m_vecOffset = vecOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMedicCallerPanel::AddMedicCaller( C_BaseEntity *pCaller, float flDuration, Vector &vecOffset, MedicCallerType eType )
{
	CTFMedicCallerPanel *pCallerPanel = new CTFMedicCallerPanel( g_pClientMode->GetViewport(), "MedicCallerPanel" );
	vgui::SETUP_PANEL( pCallerPanel );
	pCallerPanel->SetBounds( 0,0, MEDICCALLER_WIDE, MEDICCALLER_TALL );

	pCallerPanel->SetCaller( pCaller, flDuration, vecOffset );

	switch ( eType )
	{
	case TF_CALL_BURNING:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( true );
		if ( pCallerPanel->m_pCallerBurning )
			pCallerPanel->m_pCallerBurning->SetVisible( true );
		break;

	case TF_CALL_HEALTH:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( false );
		if ( pCallerPanel->m_pCallerHealth )
			pCallerPanel->m_pCallerHealth->SetVisible( true );
		break;

	case TF_CALL_BLEEDING:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( true );
		if ( pCallerPanel->m_pCallerBurning )
			pCallerPanel->m_pCallerBurning->SetVisible( true );
		break;

	case TF_CALL_AUTO:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( false );
		if ( pCallerPanel->m_pCallerAuto )
			pCallerPanel->m_pCallerAuto->SetVisible( true );
		break;

	case TF_CALL_REVIVE_EASY:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( false );
		if ( pCallerPanel->m_pCallerReviveEasy )
			pCallerPanel->m_pCallerReviveEasy->SetVisible( true );
		break;

	case TF_CALL_REVIVE_MEDIUM:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( false );
		if ( pCallerPanel->m_pCallerReviveMedium )
			pCallerPanel->m_pCallerReviveMedium->SetVisible( true );
		break;

	case TF_CALL_REVIVE_HARD:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( false );
		if ( pCallerPanel->m_pCallerReviveHard )
			pCallerPanel->m_pCallerReviveHard->SetVisible( true );
		break;

	case TF_CALL_MEDIC:
	default:
		if ( pCallerPanel->m_pCallerBG )
			pCallerPanel->m_pCallerBG->SetVisible( true );
		break;
	}

	pCallerPanel->SetVisible( true );
	vgui::ivgui()->AddTickSignal( pCallerPanel->GetVPanel() );
}
