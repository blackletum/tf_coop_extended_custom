//========= Copyright  1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TF_HUD_ITEMEFFECTMETER_H
#define TF_HUD_ITEMEFFECTMETER_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include "iclientmode.h"
#include "ienginevgui.h"
#include "tf_controls.h"
#include "c_tf_player.h"
#include "hud.h"
#include "hudelement.h"
#include "tf_weapon_invis.h"
#include "tf_weapon_invis.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudItemEffectMeter : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudItemEffectMeter, vgui::EditablePanel );

public:
	CHudItemEffectMeter( const char *pElementName );

	virtual void    ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void    PerformLayout( void );
	virtual bool	ShouldDraw( void );

	virtual void    Update( C_TFPlayer *pPlayer );

	virtual int     GetCount( void )                    { return -1; }
	virtual float   GetProgress( void );
	virtual const char* GetLabelText( void );
	virtual const char* GetResourceName( void )         { return "resource/UI/HudItemEffectMeter.res"; }
	virtual const char* GetIconName( void );
	virtual bool    ShouldBeep( void );
	virtual bool    ShouldFlash( void )                 { return false; }
	virtual bool    IsEnabled( void )                   { return m_bEnabled; }

	virtual C_EconEntity* GetItem( void ) const       { return m_hItem.Get(); }
	void            SetItem( C_EconEntity *pItem )    { m_hItem = pItem; }

private:
	vgui::ContinuousProgressBar *m_pEffectMeter;
	CExLabel *m_pEffectMeterLabel;
	CTFImagePanel	*m_pEffectMeterIcon;

	CPanelAnimationVarAliasType( int, m_iXOffset, "x_offset", "0", "proportional_int" );

	CHandle<C_EconEntity> m_hItem;
	float m_flOldCharge;

protected:
	bool m_bEnabled;
};

#endif	// TF_HUD_ITEMEFFECTMETER_H
