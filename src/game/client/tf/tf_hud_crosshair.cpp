//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "clientmode.h"
#include "c_tf_player.h"
#include "tf_hud_crosshair.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialvar.h"
#include "mathlib/mathlib.h"
#include "basecombatweapon_shared.h"
#include "tf_weapon_portalgun.h"

ConVar cl_crosshair_red( "cl_crosshair_red", "200", FCVAR_ARCHIVE );
ConVar cl_crosshair_green( "cl_crosshair_green", "200", FCVAR_ARCHIVE );
ConVar cl_crosshair_blue( "cl_crosshair_blue", "200", FCVAR_ARCHIVE );
ConVar cl_crosshair_alpha( "cl_crosshair_alpha", "200", FCVAR_ARCHIVE );

ConVar cl_crosshair_file( "cl_crosshair_file", "", FCVAR_ARCHIVE );

ConVar cl_crosshair_scale( "cl_crosshair_scale", "32.0", FCVAR_ARCHIVE );
ConVar cl_crosshair_approach_speed( "cl_crosshair_approach_speed", "0.015" );

using namespace vgui;

DECLARE_HUDELEMENT(CHudTFCrosshair);

CHudTFCrosshair::CHudTFCrosshair(const char *pElementName) :
	CHudCrosshair("CHudCrosshair")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_pCrosshair = 0;
	m_szPreviousCrosshair[0] = '\0';

	m_pFrameVar = NULL;

	m_clrCrosshair = Color(0, 0, 0, 0);

	m_vecCrossHairOffsetAngle.Init();

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR);
}

void CHudTFCrosshair::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings( scheme );

	SetSize( ScreenWidth(), ScreenHeight() );
}

void CHudTFCrosshair::LevelShutdown(void)
{
	// forces m_pFrameVar to recreate next map
	m_szPreviousCrosshair[0] = '\0';

	if (m_pCrosshairOverride)
	{
		delete m_pCrosshairOverride;
		m_pCrosshairOverride = NULL;
	}

	if ( m_pFrameVar )
	{
		delete m_pFrameVar;
		m_pFrameVar = NULL;
	}
}

void CHudTFCrosshair::Init()
{
	m_iCrosshairTextureID = vgui::surface()->CreateNewTextureID();

	m_icon_rb = gHUD.GetIcon( "portal_crosshair_right_valid" );
	m_icon_lb = gHUD.GetIcon( "portal_crosshair_left_valid" );
	m_icon_rbe = gHUD.GetIcon( "portal_crosshair_last_placed" );
	m_icon_lbe = gHUD.GetIcon( "portal_crosshair_last_placed" );
	m_icon_rbn = gHUD.GetIcon( "portal_crosshair_right_invalid" );
	m_icon_lbn = gHUD.GetIcon( "portal_crosshair_left_invalid" );
	m_icon_rbnone = gHUD.GetIcon( "crosshair_right" );
	m_icon_lbnone = gHUD.GetIcon( "crosshair_left" );
}

void CHudTFCrosshair::SetCrosshair(CHudTexture *texture, Color& clr)
{
	m_pCrosshair = texture;
	m_clrCrosshair = clr;
}

bool CHudTFCrosshair::ShouldDraw()
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	CTFWeaponBase *pWeapon = pPlayer->GetActiveTFWeapon();
	if ( !pWeapon )
		return false;

	if ( pPlayer->m_Shared.InCond( TF_COND_TAUNTING ) || pPlayer->m_Shared.IsControlStunned() || pPlayer->m_Shared.IsLoser() )
		return false;

	if ( pPlayer->m_Shared.IsInCutScene() )
		return false;

	return pWeapon->ShouldDrawCrosshair();
}

void CHudTFCrosshair::Paint()
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return;

	const char *crosshairfile = cl_crosshair_file.GetString();
	CTFWeaponBase *pWeapon = pPlayer->GetActiveTFWeapon();
	if ( !pWeapon )
		return;

	/*C_WeaponPortalgun *pPortalgun = dynamic_cast<C_WeaponPortalgun*>( pWeapon );

	int		xCenter	= ( ScreenWidth() - cl_crosshair_scale.GetInt() ) / 2;
	int		yCenter = ( ScreenHeight() - cl_crosshair_scale.GetInt() ) / 2;

	bool bPortalPlacability[2];
	if ( ( pWeapon->GetWeaponID() == TF_WEAPON_PORTALGUN ) && pPortalgun )
	{
		bPortalPlacability[0] = pPortalgun->GetPortal1Placablity() > 0.5f;
		bPortalPlacability[1] = pPortalgun->GetPortal2Placablity() > 0.5f;

		const unsigned char iAlphaStart = 150;	   

		Color portal1Color = UTIL_Portal_Color( 1 );
		Color portal2Color = UTIL_Portal_Color( 2 );

		portal1Color[ 3 ] = iAlphaStart;
		portal2Color[ 3 ] = iAlphaStart;

		const int iBaseLastPlacedAlpha = 128;
		Color lastPlaced1Color = Color( portal1Color[0], portal1Color[1], portal1Color[2], iBaseLastPlacedAlpha );
		Color lastPlaced2Color = Color( portal2Color[0], portal2Color[1], portal2Color[2], iBaseLastPlacedAlpha );

		const float fLastPlacedAlphaLerpSpeed = 300.0f;

		float fLeftPlaceBarFill = 0.0f;
		float fRightPlaceBarFill = 0.0f;

		if ( pPortalgun->CanFirePortal1() && pPortalgun->CanFirePortal2() )
		{
			int iDrawLastPlaced = 0;

			//do last placed indicator effects
			if ( pPortalgun->GetLastFiredPortal() == 1 )
			{
				iDrawLastPlaced = 0;
				fLeftPlaceBarFill = 1.0f;
			}
			else if ( pPortalgun->GetLastFiredPortal() == 2 )
			{
				iDrawLastPlaced = 1;
				fRightPlaceBarFill = 1.0f;			
			}

			if( m_bLastPlacedAlphaCountingUp[iDrawLastPlaced] )
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] += gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed * 2.0f;
				if( m_fLastPlacedAlpha[iDrawLastPlaced] > 255.0f )
				{
					m_bLastPlacedAlphaCountingUp[iDrawLastPlaced] = false;
					m_fLastPlacedAlpha[iDrawLastPlaced] = 255.0f - (m_fLastPlacedAlpha[iDrawLastPlaced] - 255.0f);
				}
			}
			else
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
				if( m_fLastPlacedAlpha[iDrawLastPlaced] < (float)iBaseLastPlacedAlpha )
				{
					m_fLastPlacedAlpha[iDrawLastPlaced] = (float)iBaseLastPlacedAlpha;
				}
			}

			//reset the last placed indicator on the other side
			m_fLastPlacedAlpha[1 - iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
			if( m_fLastPlacedAlpha[1 - iDrawLastPlaced] < 0.0f )
			{
				m_fLastPlacedAlpha[1 - iDrawLastPlaced] = 0.0f;
			}
			m_bLastPlacedAlphaCountingUp[1 - iDrawLastPlaced] = true;

			if ( pPortalgun->GetLastFiredPortal() != 0 )
			{
				lastPlaced1Color[3] = m_fLastPlacedAlpha[0];
				lastPlaced2Color[3] = m_fLastPlacedAlpha[1];
			}
			else
			{
				lastPlaced1Color[3] = 0.0f;
				lastPlaced2Color[3] = 0.0f;
			}
		}
		//can't fire both portals, and we want the crosshair to remain somewhat symmetrical without being confusing
		else if ( !pPortalgun->CanFirePortal1() )
		{
			// clone portal2 info to portal 1
			portal1Color = portal2Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[0] = bPortalPlacability[1];
		}
		else if ( !pPortalgun->CanFirePortal2() )
		{
			// clone portal1 info to portal 2
			portal2Color = portal1Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[1] = bPortalPlacability[0];
		}

		if ( pPortalgun->IsHoldingObject() )
		{
			// Change the middle to orange 
			portal1Color = portal2Color = UTIL_Portal_Color( 0 );
			bPortalPlacability[0] = bPortalPlacability[1] = false;
		}

		if ( bPortalPlacability[0] )
			m_icon_lb->DrawSelf(xCenter - (m_icon_lb->Width() * 0.64f ), yCenter - ( m_icon_rb->Height() * 0.17f ), portal1Color);
		else
			m_icon_lbn->DrawSelf(xCenter - (m_icon_lbn->Width() * 0.64f ), yCenter - ( m_icon_rb->Height() * 0.17f ), portal1Color);

		if ( bPortalPlacability[1] )
			m_icon_rb->DrawSelf(xCenter + ( m_icon_rb->Width() * -0.35f ), yCenter + ( m_icon_rb->Height() * 0.17f ), portal2Color);
		else
			m_icon_rbn->DrawSelf(xCenter + ( m_icon_rbn->Width() * -0.35f ), yCenter + ( m_icon_rb->Height() * 0.17f ), portal2Color);

		//last placed portal indicator
		m_icon_lbe->DrawSelf( xCenter - (m_icon_lbe->Width() * 1.85f), yCenter, lastPlaced1Color );
		m_icon_rbe->DrawSelf( xCenter + (m_icon_rbe->Width() * 0.75f), yCenter, lastPlaced2Color );
	}*/

	if ( crosshairfile[0] == '\0' )
	{
		m_pCrosshair = pWeapon->GetWpnData().iconCrosshair;
		BaseClass::Paint();
		return;
	}
	else
	{
		if ( Q_stricmp( m_szPreviousCrosshair, crosshairfile ) != 0 )
		{
			char buf[256];
			Q_snprintf( buf, sizeof( buf ), "vgui/crosshairs/%s", crosshairfile );

			vgui::surface()->DrawSetTextureFile( m_iCrosshairTextureID, buf, true, false );

			if ( m_pCrosshairOverride )
			{
				delete m_pCrosshairOverride;
			}

			m_pCrosshairOverride = vgui::surface()->DrawGetTextureMatInfoFactory( m_iCrosshairTextureID );

			if ( !m_pCrosshairOverride )
				return;

			if ( m_pFrameVar )
			{
				delete m_pFrameVar;
			}

			bool bFound = false;
			m_pFrameVar = m_pCrosshairOverride->FindVarFactory( "$frame", &bFound );
			Assert( bFound );

			m_nNumFrames = m_pCrosshairOverride->GetNumAnimationFrames();

			// save the name to compare with the cvar in the future
			Q_strncpy( m_szPreviousCrosshair, crosshairfile, sizeof( m_szPreviousCrosshair ) );
		}

		Color clr( cl_crosshair_red.GetInt(), cl_crosshair_green.GetInt(), cl_crosshair_blue.GetInt(), 255 );

		float x, y;
		bool bBehindCamera;
		GetDrawPosition( &x, &y, &bBehindCamera, m_vecCrossHairOffsetAngle );

		if ( bBehindCamera )
			return;

		int screenWide, screenTall;
		GetHudSize( screenWide, screenTall );

		int iWidth, iHeight;

		// Vintage Ambassador crosshair scaling
		float flCrosshairScale = 1.0f;
		if ( pWeapon->GetWeaponID() == TF_WEAPON_REVOLVER )
		{
			int iMode = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iMode, set_weapon_mode );
			if ( iMode == 1 )
			{
				float flFireInterval = min( gpGlobals->curtime - pWeapon->GetLastFireTime(), 1.25f );
				flCrosshairScale = clamp( ( flFireInterval / 1.25f ), 0.334, 1.0f );
			}
		}

		iWidth = iHeight = cl_crosshair_scale.GetInt();
		int iX = (int)( x + 0.5f );
		int iY = (int)( y + 0.5f );

		vgui::surface()->DrawSetColor( clr );
		vgui::surface()->DrawSetTexture( m_iCrosshairTextureID );
		vgui::surface()->DrawTexturedRect( iX - iWidth, iY - iHeight, iX + iWidth, iY + iHeight );
		vgui::surface()->DrawSetTexture( 0 );
	}
}

