//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
#include "cbase.h"
#include "lfe_carouselslider.h"
#include "fmtstr.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IInput.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY( CTFCarouselSlider );

// There is only one game mode carousel, so safe to save this state.
// This allows the children panels that "throw" back to the main menu to have
// the carousel in the same state when it navigated to them.
CUtlString g_CurrentModeIdSave;

CTFCarouselSlider::CTFCarouselSlider( Panel *pParent, const char *pName ) : BaseClass( pParent, pName, "" )
{
	SetProportional( true );
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( true );

	m_nPicWidth = 0;
	m_nPicHeight = 0;

	m_nSubPics = 0;
	m_nSubPicGap = 0;
	m_nSubPicWidth = 0;
	m_nSubPicHeight = 0;

	m_nLeftArrowX = 0;
	m_nLeftArrowY = 0;
	m_nRightArrowX = 0;
	m_nRightArrowY = 0;
	m_nRightArrowOffsetX = 0;

	m_nActive = 0;
	m_startScrollTime = 0;
	m_bLeftScroll = false;
	m_bHideLabels = false;
	m_nScrollMultipleCount = 0;

	m_nLeftArrowId = -1;
	m_nRightArrowId = -1;
	m_nBorderImageId = -1;
	m_nTopBorderImageId = -1;
	m_nBottomBorderImageId = -1;
}

CTFCarouselSlider::~CTFCarouselSlider()
{
	// Purposely not destroying our texture IDs, they are finds.
	// The naive create/destroy pattern causes excessive i/o for no benefit.
	// These images will always have to be there anyways.

	m_CarouselSliderInfos.Purge();
}

void CTFCarouselSlider::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	const char *pTopImageName = pScheme->GetResourceString( "Frame.TopBorderImage" );
	m_nTopBorderImageId = vgui::surface()->DrawGetTextureId( pTopImageName );
	if ( m_nTopBorderImageId == -1 )
	{
		m_nTopBorderImageId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nTopBorderImageId, pTopImageName, true, false );	
	}

	const char *pBottomImageName = pScheme->GetResourceString( "Frame.BottomBorderImage" );
	m_nBottomBorderImageId = vgui::surface()->DrawGetTextureId( pBottomImageName );
	if ( m_nBottomBorderImageId == -1 )
	{
		m_nBottomBorderImageId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nBottomBorderImageId, pBottomImageName, true, false );	
	}

	m_smearColor = pScheme->GetColor( "Frame.SmearColor", Color( 0, 0, 0, 200 ) );
}

void CTFCarouselSlider::OnKeyCodePressed( vgui::KeyCode code )
{
	bool bHandled = false;

	switch( code )
	{
	case KEY_LEFT:
		if ( !IsScrollBusy() && m_nSubPics )
		{
			ScrollLeft();
			bHandled = true;
		}
		break;
	case KEY_RIGHT:
		if ( !IsScrollBusy() && m_nSubPics )
		{
			ScrollRight();
			bHandled = true;
		}
		break;

	case KEY_UP:
		if ( NavigateUp() )
		{
			bHandled = true;
		}
		break;
	case KEY_DOWN:
		if ( NavigateDown() )
		{
			bHandled = true;
		}
		break;
	}

	if ( !bHandled )
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void CTFCarouselSlider::ApplySettings( KeyValues *pInResourceData )
{
	BaseClass::ApplySettings( pInResourceData );

	vgui::HScheme hScheme = vgui::scheme()->GetScheme( "SwarmScheme" );
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( hScheme );
	if ( !pScheme )
		return;

	const char *pImageName = pInResourceData->GetString( "borderimage", "" );
	m_nBorderImageId = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_nBorderImageId == -1 )
	{
		m_nBorderImageId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nBorderImageId, pImageName, true, false );	
	}

	pImageName = pInResourceData->GetString( "leftarrow", "" );
	m_nLeftArrowId = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_nLeftArrowId == -1 )
	{
		m_nLeftArrowId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nLeftArrowId, pImageName, true, false );	
	}

	pImageName = pInResourceData->GetString( "rightarrow", "" );
	m_nRightArrowId = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_nRightArrowId == -1 )
	{
		m_nRightArrowId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nRightArrowId, pImageName, true, false );	
	}

	m_nPicOffsetX = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "picoffsetx", 0 ) );
	m_nPicWidth = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "picwidth", 0 ) );
	m_nPicHeight = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "picheight", 0 ) );

	m_nMenuTitleX = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "menutitlex", 0 ) );
	m_nMenuTitleY = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "menutitley", 0 ) );
	m_nMenuTitleWide = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "menutitlewide", 0 ) );
	m_nMenuTitleTall = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "menutitletall", 0 ) );

	m_nSubPics = pInResourceData->GetInt( "subpics", 0 );
	m_nSubPicGap = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "subpicgap", 0 ) );
	m_nSubPicOffsetX = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "subpicoffsetx", 0 ) );
	m_nSubPicOffsetY = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "subpicoffsety", 0 ) );
	m_nSubPicWidth = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "subpicwidth", 0 ) );
	m_nSubPicHeight = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "subpicheight", 0 ) );
	m_bHideLabels = !!vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "hidelabels", 0 ) );

	m_nArrowWidth = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "arrowwidth", 0 ) );
	m_nArrowHeight = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "arrowheight", 0 ) );
	m_nArrowOffsetY = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "arrowoffsety", 0 ) );
	m_nRightArrowOffsetX = vgui::scheme()->GetProportionalScaledValue( pInResourceData->GetInt( "rightarrowoffsetx", 0 ) );

	m_hNameFont = pScheme->GetFont( pInResourceData->GetString( "subpicnamefont", "" ), true );
	m_nNameFontHeight = vgui::surface()->GetFontTall( m_hNameFont );

	int wideAtOpen = pInResourceData->GetInt( "wideatopen", 0 );

	// need to reset due to video mode change, alt+tab, etc
	m_CarouselSliderInfos.Purge();

	// find all modes
	for ( KeyValues *pModeKey = pInResourceData->GetFirstTrueSubKey(); pModeKey; pModeKey = pModeKey->GetNextTrueSubKey() )
	{
		pImageName = pModeKey->GetString( "image", "" );
		int nImageId = vgui::surface()->DrawGetTextureId( pImageName );
		if ( nImageId == -1 )
		{
			nImageId = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile( nImageId, pImageName, true, false );	
		}

		int iIndex = m_CarouselSliderInfos.AddToTail();

		m_CarouselSliderInfos[iIndex].m_NameId = pModeKey->GetString( "id", "" );
		m_CarouselSliderInfos[iIndex].m_NameText = pModeKey->GetString( "name", "" );
		m_CarouselSliderInfos[iIndex].m_CommandText = pModeKey->GetString( "command", "" );
		m_CarouselSliderInfos[iIndex].m_TitleText = pModeKey->GetString( "menutitle", "" );
		m_CarouselSliderInfos[iIndex].m_HintText = pModeKey->GetString( "menuhint", "" );
		m_CarouselSliderInfos[iIndex].m_HintTextDisabled = pModeKey->GetString( "menuhintdisabled", "" );
		m_CarouselSliderInfos[iIndex].m_nImageId = nImageId;
		m_CarouselSliderInfos[iIndex].m_bEnabled = pModeKey->GetBool( "enabled", true );

		m_CarouselSliderInfos[iIndex].m_pHybridButton = new BaseModHybridButton( 
					this, 
					m_CarouselSliderInfos[iIndex].m_NameId, 
					m_CarouselSliderInfos[iIndex].m_TitleText, 
					this->GetParent(), 
					m_CarouselSliderInfos[iIndex].m_CommandText );
		KeyValues *pKV = new KeyValues( "BtnCarouselSlider" );

		int buttonX = vgui::scheme()->GetProportionalNormalizedValue( m_nMenuTitleX );
		int buttonY = vgui::scheme()->GetProportionalNormalizedValue( m_nPicHeight + m_nMenuTitleY );
		int buttonW = vgui::scheme()->GetProportionalNormalizedValue( m_nMenuTitleWide );
		int buttonH = vgui::scheme()->GetProportionalNormalizedValue( m_nMenuTitleTall );

		pKV->SetInt( "xpos", buttonX );
		pKV->SetInt( "ypos", buttonY );
		pKV->SetInt( "wide", buttonW );
		pKV->SetInt( "tall", buttonH );
		pKV->SetInt( "autoResize", 1 );
		pKV->SetInt( "pinCorner", 0 );
		pKV->SetInt( "visible", 0 );
		pKV->SetInt( "enabled", m_CarouselSliderInfos[iIndex].m_bEnabled );
		pKV->SetInt( "tabPosition", 0 );
		pKV->SetString( "tooltiptext", m_CarouselSliderInfos[iIndex].m_HintText );
		pKV->SetString( "disabled_tooltiptext", m_CarouselSliderInfos[iIndex].m_HintTextDisabled );
		pKV->SetString( "style", "GameModeButton" );
		pKV->SetInt( "ActivationType", 1 );
		pKV->SetString( "EnableCondition", pModeKey->GetString( "EnableCondition", "" ) );
		pKV->SetInt( "wideatopen", wideAtOpen );

		m_CarouselSliderInfos[iIndex].m_pHybridButton->ApplySettings( pKV );
		pKV->deleteThis();
	}
	
	m_nMenuTitleActualTall = m_nMenuTitleTall;
	if ( m_CarouselSliderInfos.Count() )
	{
		// get the real size
		m_nMenuTitleActualTall = m_CarouselSliderInfos[0].m_pHybridButton->GetTall();

		// fixup the number of subpics to what we actually have
		// one active plus any subpics can only show the modes with no repeat
		m_nSubPics = MIN( m_nSubPics, m_CarouselSliderInfos.Count() - 1 );
	}
	else
	{
		m_nSubPics = 0;
	}

	// exact size fixup width to get hardware clipping on rhs
	int panelWidth, panelHeight;
	GetSize( panelWidth, panelHeight );
	panelWidth = m_nPicOffsetX + m_nPicWidth + m_nSubPicOffsetX + ( m_nSubPics + 1 ) * m_nSubPicWidth + m_nSubPics * m_nSubPicGap;
	panelHeight = m_nPicHeight + m_nMenuTitleY + ( m_nMenuTitleActualTall - m_nMenuTitleTall )/2 + m_nMenuTitleTall;
	SetSize( panelWidth, panelHeight );

	// calc the arrow position for drawing and hit testing
	m_nSubPicX = m_nPicOffsetX + m_nPicWidth + m_nSubPicOffsetX;
	m_nSubPicY = ( m_nPicHeight + m_nMenuTitleY + m_nMenuTitleTall - m_nSubPicHeight )/2 + m_nSubPicOffsetY;
	m_nLeftArrowX = m_nPicOffsetX - m_nSubPicGap - m_nArrowWidth;
	m_nLeftArrowY = m_nSubPicY + m_nSubPicHeight - m_nArrowHeight + m_nArrowOffsetY;

	if ( m_nRightArrowOffsetX )
	{
		m_nRightArrowX = m_nPicOffsetX + m_nPicWidth + m_nSubPicGap + m_nRightArrowOffsetX;
	}
	else
	{
		m_nRightArrowX = m_nSubPicX + m_nSubPics * ( m_nSubPicWidth + m_nSubPicGap );
	}

	m_nRightArrowY = m_nLeftArrowY;

	// try to put back our last known game mode
	// this is to solve the navigation to children when the navigation has been lost
	// or alt+tab, video mode resize
	m_nActive = 0;
	if ( !g_CurrentModeIdSave.IsEmpty() )
	{
		m_nActive = NameIdToModeInfo( g_CurrentModeIdSave.String() );
		if ( m_nActive == -1 )
		{
			// no longer available
			g_CurrentModeIdSave = NULL;
			m_nActive = 0;
		}
	}

	SetActiveCarousel( m_nActive, true );
}

void CTFCarouselSlider::SetActiveCarousel( int nActive, bool bKeepFocus )
{
	if ( !m_CarouselSliderInfos.Count() )
		return;

	//int nPrevActive = m_nActive;
	m_nActive = nActive;

	g_CurrentModeIdSave = m_CarouselSliderInfos[m_nActive].m_NameId;

	for ( int i = 0; i < m_CarouselSliderInfos.Count(); i++ )
	{
		m_CarouselSliderInfos[i].m_pHybridButton->SetVisible( false );
	}

	m_CarouselSliderInfos[m_nActive].m_pHybridButton->SetVisible( true );
	if ( bKeepFocus )
	{
		//CBaseModPanel::GetSingletonPtr()->SafeNavigateTo( m_CarouselSliderInfos[nPrevActive].m_pHybridButton, m_CarouselSliderInfos[m_nActive].m_pHybridButton, false );
	}
}

bool CTFCarouselSlider::GetLastActiveNameId( char *pOutBuffer, int nOutBufferSize )
{
	if ( g_CurrentModeIdSave.IsEmpty() )
		return false;
	
	int nActive = NameIdToModeInfo( g_CurrentModeIdSave.String() );
	if ( nActive == -1 )
		return false;

	V_strncpy( pOutBuffer, g_CurrentModeIdSave.String(), nOutBufferSize );
	return true;
}

void CTFCarouselSlider::PaintBackground()
{
	if ( !m_CarouselSliderInfos.Count() )
		return;

	BaseModHybridButton *pHybridButton = m_CarouselSliderInfos[m_nActive].m_pHybridButton;
	bool bHasFocus = pHybridButton->HasFocus() || 
					( pHybridButton->GetCurrentState() == BaseModHybridButton::Focus ) ||
					( pHybridButton->GetCurrentState() == BaseModHybridButton::FocusDisabled );
	bool bIsOpen = ( pHybridButton->GetCurrentState() == BaseModHybridButton::Open );

	// update scroll
	// varies between [0..1] or [1..0]
	float t = 0;
	if ( m_startScrollTime )
	{
		float rate = ( m_nScrollMultipleCount >= 1 ) ? 8.0f : 5.0f;
		t = ( Plat_FloatTime() - m_startScrollTime ) * rate;
		if ( t >= 1.0f )
		{
			// finished, scroll is circular warp around
			t = 1.0f;
			m_startScrollTime = 0;

			int nNewActive = 0;
			if ( m_bLeftScroll )
			{
				nNewActive = ( m_nActive + 1 ) % m_CarouselSliderInfos.Count();
			}
			else
			{
				nNewActive = ( m_nActive - 1 + m_CarouselSliderInfos.Count() ) % m_CarouselSliderInfos.Count();
			}
			SetActiveCarousel( nNewActive, bHasFocus );

			if ( --m_nScrollMultipleCount > 0 )
			{
				m_startScrollTime = Plat_FloatTime();
				t = 0;
			}
		}
	}

	int panelWidth, panelHeight;
	GetSize( panelWidth, panelHeight );

// need this to see panel bounds during debugging
//vgui::surface()->DrawSetColor( Color( 255, 0, 0, 255 ) );
//vgui::surface()->DrawFilledRect( 0, 0, panelWidth, panelHeight );

	// the main pic and all the subpics
	int nPicCount = 1 + m_nSubPics;
	if ( m_startScrollTime && m_bLeftScroll )
	{
		// scrolling means one extra sub pic must be drawn at the far right edge
		// either coming/going
		nPicCount++;
	}

	int nOffscreenSubPicX = m_nPicOffsetX - m_nSubPicOffsetX - m_nSubPicWidth;
	int nOffscreenSubPicY = m_nSubPicY;
	int nOffscreenSubPicW = m_nSubPicWidth;
	int nOffscreenSubPicH = m_nSubPicHeight;

	int nActivePicX = m_nPicOffsetX;
	int nActivePicY = 0;
	int nActivePicW = m_nPicWidth;
	int nActivePicH = m_nPicHeight;
	float nActiveAngle = 0; //-3;

	int x, y;
	int w, h;

	int iPrevActiveMode = ( m_nActive - 1 + m_CarouselSliderInfos.Count() ) % m_CarouselSliderInfos.Count(); 
	int iNextActiveMode = ( m_nActive + 1 ) % m_CarouselSliderInfos.Count();

	// center the vertical smear
	y = nActivePicY + nActivePicH + m_nMenuTitleY + ( m_nMenuTitleActualTall - m_nMenuTitleTall )/2;
	vgui::surface()->DrawSetColor( m_smearColor );
	DrawSmearBackgroundFade( 
		m_nMenuTitleX - ( 0.30f * m_nMenuTitleX ), 
		y, 
		m_nSubPicX + m_nSubPics * ( m_nSubPicWidth + m_nSubPicGap ), 
		y + m_nMenuTitleTall ); 

	// cyclical carousel
	// the first pic to be drawn is the active main pic, followed by all the sub pics
	for ( int i = 0; i < nPicCount; i++ )
	{
		int iGameMode = ( m_nActive + i ) % m_CarouselSliderInfos.Count();

		// in between scrolling, this image transition gets handled by specialized lerp drawing
		bool bSkipSubPicDraw = m_startScrollTime && ( m_bLeftScroll && ( iGameMode == iNextActiveMode ) );

		if ( !i )
		{
			// active pic
			x = nActivePicX;
			y = nActivePicY;
			w = nActivePicW;
			h = nActivePicH;
			
			if ( !m_startScrollTime )
			{
				Color picColor;
				picColor.SetColor( 255, 255, 255, 255 );
				vgui::surface()->DrawSetColor( picColor );
				vgui::surface()->DrawSetTexture( m_CarouselSliderInfos[m_nActive].m_nImageId );
				vgui::surface()->DrawTexturedRect( x, y, x+w, y+h );

				/*DrawTexturedRectParms_t parms;
				parms.x0 = x;
				parms.y0 = y;
				parms.x1 = x+w;
				parms.y1 = y+h;
				parms.angle = nActiveAngle;
				vgui::surface()->DrawTexturedRectEx( &parms );*/

				if ( bHasFocus || bIsOpen )
				{
					picColor.SetColor( 255, 255, 255, 255 );
				}
				else
				{
					picColor.SetColor( 0, 0, 0, 255 );
				}
				vgui::surface()->DrawSetColor( picColor );
				vgui::surface()->DrawSetTexture( m_nBorderImageId );
				//vgui::surface()->DrawTexturedRectEx( &parms );
			}
			else
			{
				// draw the lerping pics
				// lerp active pic to sub pic on left edge	
				int iMode = m_nActive;
				float tt = 1 - t;
				if ( !m_bLeftScroll )
				{
					tt = t;
					iMode = iPrevActiveMode;
				}

				x = (float)nOffscreenSubPicX + tt * ( (float)nActivePicX - nOffscreenSubPicX );
				y = (float)nOffscreenSubPicY + tt * ( (float)nActivePicY - nOffscreenSubPicY );
				w = (float)nOffscreenSubPicW + tt * ( (float)nActivePicW - nOffscreenSubPicW );
				h = (float)nOffscreenSubPicH + tt * ( (float)nActivePicH - nOffscreenSubPicH );
				float a = tt * 255.0f;
				float ang = tt * nActiveAngle;

				Color picColor;
				picColor.SetColor( 255, 255, 255, a );
				vgui::surface()->DrawSetColor( picColor );
				vgui::surface()->DrawSetTexture( m_CarouselSliderInfos[iMode].m_nImageId );
				vgui::surface()->DrawTexturedRect( x, y, x+w, y+h );

				/*DrawTexturedRectParms_t parms;
				parms.x0 = x;
				parms.y0 = y;
				parms.x1 = x+w;
				parms.y1 = y+h;
				parms.angle = ang;
				vgui::surface()->DrawTexturedRectEx( &parms );*/

				vgui::surface()->DrawSetTexture( m_nBorderImageId );
				//vgui::surface()->DrawTexturedRectEx( &parms );

				// lerp active pic to sub pic on right edge	
				iMode = iNextActiveMode;
				tt = t;
				if ( !m_bLeftScroll )
				{
					tt = 1 - t;
					iMode = m_nActive;
				}
		
				x = (float)m_nSubPicX + tt * ( (float)nActivePicX - m_nSubPicX );
				y = (float)m_nSubPicY + tt * ( (float)nActivePicY - m_nSubPicY );
				w = (float)m_nSubPicWidth + tt * ( (float)nActivePicW - m_nSubPicWidth );
				h = (float)m_nSubPicHeight + tt * ( (float)nActivePicH - m_nSubPicHeight );
				float c = 125.0f + tt * ( 255.0f - 125.0f );
				ang = tt * nActiveAngle;

				picColor.SetColor( c, c, c, 255 );
				vgui::surface()->DrawSetColor( picColor );
				vgui::surface()->DrawSetTexture( m_CarouselSliderInfos[iMode].m_nImageId );
				vgui::surface()->DrawTexturedRect( x, y, x+w, y+h );

				/*parms.x0 = x;
				parms.y0 = y;
				parms.x1 = x+w;
				parms.y1 = y+h;
				parms.s0 = 0;
				parms.s1 = 1;
				parms.t0 = 0;
				parms.t1 = 1;
				parms.angle = ang;
				vgui::surface()->DrawTexturedRectEx( &parms );*/
			}
		}
		else if ( !bSkipSubPicDraw )
		{
			// sub pics
			x = m_nSubPicX + ( i - 1 ) * ( m_nSubPicWidth + m_nSubPicGap );
			y = m_nSubPicY;
			w = m_nSubPicWidth;
			h = m_nSubPicHeight;
			int alpha = 255;
			int focusColor = ( bHasFocus || bIsOpen ) ? 255 : 125;

			if ( m_startScrollTime )
			{
				int x1 = x;
				if ( m_bLeftScroll )
				{
					x1 -= m_nSubPicWidth + m_nSubPicGap;
				}
				else
				{
					x1 += m_nSubPicWidth + m_nSubPicGap;
				}

				x = (float)x + t * (float)( x1 - x );

				if ( i == nPicCount - 1 )
				{
					if ( m_bLeftScroll )
					{
						alpha = t * (float)alpha;
					}
					else
					{
						alpha = ( 1 - t ) * (float)alpha;
					}
				}
			}

			Color picColor;
			picColor.SetColor( focusColor, focusColor, focusColor, alpha );
			vgui::surface()->DrawSetColor( picColor );
			vgui::surface()->DrawSetTexture( m_CarouselSliderInfos[iGameMode].m_nImageId );
			vgui::surface()->DrawTexturedSubRect( x, y, x+w, y+h, 0.0f, 0.0f, 1.0f, 1.0f );

			if ( !m_bHideLabels )
			{
				// sub pics have a label on top of an inscribed rect
				vgui::surface()->DrawSetColor( Color( 0, 0, 0, alpha ) );
				vgui::surface()->DrawFilledRect( x, y + m_nSubPicHeight - m_nNameFontHeight, x + w, y + m_nSubPicHeight );
				Color textColor( focusColor, focusColor, focusColor, alpha );
				int gap = vgui::surface()->GetCharacterWidth( m_hNameFont, ' ' );
				DrawColoredText( m_hNameFont, x + gap, y + m_nSubPicHeight - m_nNameFontHeight/2, textColor, m_CarouselSliderInfos[iGameMode].m_NameText );
			}
		}
	}

	// draw arrows
	if ( m_nSubPics )
	{
		// pc always shows the arrows because mouse can move over them at any time
		// xbox hides the arrows when the control does not have focus
		if ( IsPC() || ( IsX360() && bHasFocus && !bIsOpen ) )
		{
			// xbox highlight when scroll active
			bool bLeftHighlight = IsX360() && m_startScrollTime && !m_bLeftScroll;
			bool bRightHightlight = IsX360() && m_startScrollTime && m_bLeftScroll;

			// pc highlights when mouse over
			if ( IsPC() && !m_startScrollTime )
			{
				int iPosX;
				int iPosY;
				input()->GetCursorPos( iPosX, iPosY );
				ScreenToLocal( iPosX, iPosY );

				if ( ( iPosX >= m_nLeftArrowX && iPosX <= m_nLeftArrowX + m_nArrowWidth ) &&
					( iPosY >= m_nLeftArrowY && iPosY <= m_nLeftArrowY + m_nArrowHeight ) )
				{
					bLeftHighlight = true;
				}
				else if ( ( iPosX >= m_nRightArrowX && iPosX <= m_nRightArrowX + m_nArrowWidth ) &&
					( iPosY >= m_nRightArrowY && iPosY <= m_nRightArrowY + m_nArrowHeight ) )
				{
					bRightHightlight = true;
				}
			}

			Color leftArrowColor;
			leftArrowColor.SetColor( 125, 125, 125, 255 );
			if ( bLeftHighlight )
			{
				leftArrowColor.SetColor( 255, 255, 255, 255 );
			}
			vgui::surface()->DrawSetColor( leftArrowColor );
			vgui::surface()->DrawSetTexture( m_nLeftArrowId );
			vgui::surface()->DrawTexturedRect( m_nLeftArrowX, m_nLeftArrowY, m_nLeftArrowX + m_nArrowWidth, m_nLeftArrowY + m_nArrowHeight );

			Color rightArrowColor;
			rightArrowColor.SetColor( 125, 125, 125, 255 );
			if ( bRightHightlight )
			{
				rightArrowColor.SetColor( 255, 255, 255, 255 );
			}
			vgui::surface()->DrawSetColor( rightArrowColor );
			vgui::surface()->DrawSetTexture( m_nRightArrowId );
			vgui::surface()->DrawTexturedRect( m_nRightArrowX, m_nRightArrowY, m_nRightArrowX + m_nArrowWidth, m_nRightArrowY + m_nArrowHeight );
		}
	}
}

//-----------------------------------------------------------------------------
// Centers vertically about y 
//-----------------------------------------------------------------------------
int CTFCarouselSlider::DrawColoredText( vgui::HFont hFont, int x, int y, Color color, const char *pAnsiText, float alphaScale )
{
	int len;
	wchar_t szconverted[1024];
	wchar_t *pUnicodeString;
	if ( pAnsiText && pAnsiText[0] == '#' )
	{
		pUnicodeString = g_pVGuiLocalize->Find( pAnsiText );
		if ( !pUnicodeString )
		{
			return x;
		}
		len = V_wcslen( pUnicodeString );
	}
	else
	{
		len = g_pVGuiLocalize->ConvertANSIToUnicode( pAnsiText, szconverted, sizeof( szconverted ) );
		if ( len <= 0 )
		{
			return x;
		}
		pUnicodeString = szconverted;
	}	

	int wide, tall;
	vgui::surface()->GetTextSize( hFont, szconverted, wide, tall );

	int a = color.a();
	a = (float)a * alphaScale;
	a = clamp( a, 0, 255 );

	vgui::surface()->DrawSetTextFont( hFont );
	vgui::surface()->DrawSetTextPos( x, y - tall/2 );
	vgui::surface()->DrawSetTextColor( color.r(), color.g(), color.b(), a );
	vgui::surface()->DrawPrintText( pUnicodeString, len );

	return x + wide;
}

bool CTFCarouselSlider::ScrollLeft()
{
	if ( m_startScrollTime || !m_CarouselSliderInfos.Count() )
	{
		// already scrolling
		return false;
	}

	// only scroll if we have something to scroll to
	if ( m_nSubPics )
	{
		m_startScrollTime = Plat_FloatTime();
		m_bLeftScroll = false;
		//CBaseModPanel::GetSingleton().PlayUISound( UISOUND_CLICK );
	}

	return true;
}

bool CTFCarouselSlider::ScrollRight( int nCount )
{
	if ( m_startScrollTime || !m_CarouselSliderInfos.Count() )
	{
		// already scrolling
		return false;
	}

	// only scroll if we have something to scroll to
	if ( m_nSubPics )
	{
		m_startScrollTime = Plat_FloatTime();
		m_bLeftScroll = true;
		//CBaseModPanel::GetSingleton().PlayUISound( UISOUND_CLICK );
		// a normal single scroll accounts for 1 slot, this is for sliding in > 1
		m_nScrollMultipleCount = nCount > 1 ? nCount : 0;
	}

	return true;
}

int CTFCarouselSlider::NameIdToModeInfo( const char *pNameId )
{
	for ( int i = 0; i < m_CarouselSliderInfos.Count(); i++ )
	{
		if ( !V_stricmp( m_CarouselSliderInfos[i].m_NameId.String(), pNameId ) )
		{
			// found
			return i;
		}
	}

	// not found
	return -1;
}

void CTFCarouselSlider::SetEnabled( const char *pNameId, bool bEnabled )
{
	int iIndex = NameIdToModeInfo( pNameId );
	if ( iIndex == -1 )
		return;

	m_CarouselSliderInfos[iIndex].m_bEnabled = bEnabled;
}

bool CTFCarouselSlider::SetActive( const char *pNameId, bool bForce )
{
	int nIndex = NameIdToModeInfo( pNameId );
	if ( nIndex == -1 )
	{
		// unknown
		return false;
	}

	m_startScrollTime = 0;
	SetActiveCarousel( nIndex, true );
	return true;
}

int	CTFCarouselSlider::GetNumGameInfos()
{
	return m_CarouselSliderInfos.Count();
}

BaseModHybridButton *CTFCarouselSlider::GetHybridButton( int nIndex )
{
	if ( !m_CarouselSliderInfos.IsValidIndex( nIndex ) )
		return NULL;

	return m_CarouselSliderInfos[nIndex].m_pHybridButton;
}

void CTFCarouselSlider::NavigateFrom()
{
	for ( int i = 0; i < m_CarouselSliderInfos.Count(); i++ )
	{
		m_CarouselSliderInfos[i].m_pHybridButton->NavigateFrom();
	}

	BaseClass::NavigateFrom();
}

void CTFCarouselSlider::NavigateTo()
{
	BaseClass::NavigateTo();

	SetActiveCarousel( m_nActive, true );
}

#define TOP_BORDER_HEIGHT		21
#define BOTTOM_BORDER_HEIGHT	21
int CTFCarouselSlider::DrawSmearBackgroundFade( int x0, int y0, int x1, int y1 )
{
	// TODO: enable when we have appropriate art
	return 0;

	int wide = x1 - x0;
	int tall = y1 - y0;

	int topTall = scheme()->GetProportionalScaledValue( TOP_BORDER_HEIGHT );
	int bottomTall = scheme()->GetProportionalScaledValue( BOTTOM_BORDER_HEIGHT );

	float f1 = 0.05f;
	float f2 = 0.20f;

	topTall  = 1.00f * topTall;
	bottomTall = 1.00f * bottomTall;

	int middleTall = tall - ( topTall + bottomTall );
	if ( middleTall < 0 )
	{
		middleTall = 0;
	}

	surface()->DrawSetColor( m_smearColor );
	
	// top
	surface()->DrawSetTexture( m_nTopBorderImageId );

	vgui::surface()->DrawTexturedRect( x0, y0, x0 + f1 * wide, y0 + topTall );
	vgui::surface()->DrawTexturedRect( x0 + f1 * wide, y0, x0 + f2 * wide, y0 + topTall );
	vgui::surface()->DrawTexturedRect( x0 + f2 * wide, y0, x0 + wide, y0 + topTall );

	/*DrawTexturedRectParms_t parms;
	parms.x0 = x0;
	parms.y0 = y0;
	parms.x1 = x0 + f1 * wide;
	parms.y1 = y0 + topTall;
	parms.s0 = 0;
	parms.s1 = f1;
	parms.alpha_ul = parms.alpha_ll = 0.0f;
	parms.alpha_ur = parms.alpha_lr = 255.0f;
	vgui::surface()->DrawTexturedRectEx( &parms );

	parms.x0 = x0 + f1 * wide;
	parms.x1 = x0 + f2 * wide;
	parms.s0 = f1;
	parms.s1 = f2;
	parms.alpha_ul = parms.alpha_ll = 255.0f;
	parms.alpha_ur = parms.alpha_lr = 255.0f;
	vgui::surface()->DrawTexturedRectEx( &parms );

	parms.x0 = x0 + f2 * wide;
	parms.x1 = x0 + wide;
	parms.s0 = f2;
	parms.s1 = 1.0f;
	parms.alpha_ul = parms.alpha_ll = 255.0f;
	parms.alpha_ur = parms.alpha_lr = 0;
	vgui::surface()->DrawTexturedRectEx( &parms );*/
	y0 += topTall;

	if ( middleTall )
	{
		// middle
		surface()->DrawFilledRectFade( x0, y0, x0 + f1*wide, y0 + middleTall, 0, 255, true );
		surface()->DrawFilledRectFade( x0 + f1*wide, y0, x0 + f2*wide, y0 + middleTall, 255, 255, true );
		surface()->DrawFilledRectFade( x0 + f2*wide, y0, x0 + wide, y0 + middleTall, 255, 0, true );
		y0 += middleTall;
	}

	// bottom
	surface()->DrawSetTexture( m_nBottomBorderImageId );

	vgui::surface()->DrawTexturedRect( x0, y0, x0 + f1 * wide, y0 + bottomTall );
	vgui::surface()->DrawTexturedRect( x0 + f1 * wide, y0, x0 + f2 * wide, y0 + bottomTall );
	vgui::surface()->DrawTexturedRect( x0 + f2 * wide, y0, x0 + wide, y0 + bottomTall );

	/*parms.x0 = x0;
	parms.y0 = y0;
	parms.x1 = x0 + f1 * wide;
	parms.y1 = y0 + bottomTall;
	parms.s0 = 0;
	parms.s1 = f1;
	parms.alpha_ul = parms.alpha_ll = 0;
	parms.alpha_ur = parms.alpha_lr = 255.0f;
	vgui::surface()->DrawTexturedRectEx( &parms );

	parms.x0 = x0 + f1 * wide;
	parms.x1 = x0 + f2 * wide;
	parms.s0 = f1;
	parms.s1 = f2;
	parms.alpha_ul = parms.alpha_ll = 255.0f;
	parms.alpha_ur = parms.alpha_lr = 255.0f;
	vgui::surface()->DrawTexturedRectEx( &parms );

	parms.x0 = x0 + f2 * wide;
	parms.x1 = x0 + wide;
	parms.s0 = f2;
	parms.s1 = 1.0f;
	parms.alpha_ul = parms.alpha_ll = 255.0f;
	parms.alpha_ur = parms.alpha_lr = 0;
	y0 += bottomTall;
	vgui::surface()->DrawTexturedRectEx( &parms );*/

	return topTall + middleTall + bottomTall;
}

void CTFCarouselSlider::OnCommand( const char *command )
{
	// we got in the way of the hybrid/flyouts, these are meant for our parent
	GetParent()->OnCommand( command );
}

bool CTFCarouselSlider::IsScrollBusy()
{
	return ( m_startScrollTime != 0 );
}

void CTFCarouselSlider::OnMousePressed( vgui::MouseCode code )
{
	BaseClass::OnMousePressed( code );

	if ( code != MOUSE_LEFT )
		return;		

	if ( IsScrollBusy() )
		return;

	int iPosX;
	int iPosY;
	input()->GetCursorPos( iPosX, iPosY );
	ScreenToLocal( iPosX, iPosY );

	if ( m_nSubPics )
	{
		bool bRightScroll = false;
		bool bLeftScroll = false;
		int nSubPic = 0;

		if ( ( iPosX >= m_nPicOffsetX && iPosX <= m_nPicOffsetX + m_nPicWidth ) &&
			( iPosY >= 0 && iPosY <= m_nPicHeight ) )
		{
			BaseModHybridButton *pHybridButton = GetHybridButton( m_nActive );
			if ( pHybridButton && pHybridButton->GetCurrentState() != BaseModHybridButton::Open )
			{
				// open it
				if ( pHybridButton->IsEnabled() )
				{
					pHybridButton->DoClick();
				}
			}
		}
		else if ( ( iPosX >= m_nLeftArrowX && iPosX <= m_nLeftArrowX + m_nArrowWidth ) &&
			( iPosY >= m_nLeftArrowY && iPosY <= m_nLeftArrowY + m_nArrowHeight ) )
		{
			bLeftScroll = true;
		}
		else if ( ( iPosX >= m_nRightArrowX && iPosX <= m_nRightArrowX + m_nArrowWidth ) &&
			( iPosY >= m_nRightArrowY && iPosY <= m_nRightArrowY + m_nArrowHeight ) )
		{
			bRightScroll = true;
		}
		else
		{
			// determine if sub pic selected
			if ( iPosY >= m_nSubPicY && iPosY <= m_nSubPicY + m_nSubPicHeight )
			{
				int x = m_nSubPicX;
				for ( int i = 1; i <= m_nSubPics; i++ )
				{
					if ( iPosX >= x && iPosX <= x + m_nSubPicWidth )
					{
						nSubPic = i;
						break;
					}
					x += m_nSubPicWidth + m_nSubPicGap;
				}
			}
		}

		if ( bLeftScroll || bRightScroll || nSubPic )
		{
			if ( bLeftScroll )
			{
				ScrollLeft();
			}
			else if ( bRightScroll )
			{
				ScrollRight();
			}
			else if ( nSubPic )
			{
				ScrollRight( nSubPic );
			}
		}
	}
}
