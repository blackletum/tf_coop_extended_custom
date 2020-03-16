//=============================================================================
//
// Purpose: COME OVER HERE
//
//=============================================================================

#ifndef LFE_HUD_PINGSYSTEM_H
#define LFE_HUD_PINGSYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <game/client/iviewport.h>
#include <vgui/IScheme.h>
#include "hud.h"
#include "hudelement.h"
#include "tf_imagepanel.h"
#include "c_tf_player.h"
#include "GameEventListener.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFPingSystemPanel : public vgui::EditablePanel, public CGameEventListener
{
	DECLARE_CLASS_SIMPLE( CTFPingSystemPanel, vgui::EditablePanel );
public:
	CTFPingSystemPanel( vgui::Panel *parent, const char *name );
	~CTFPingSystemPanel( void );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout( void );
	virtual void OnTick( void );
	virtual void PaintBackground( void );
	virtual void Paint( void );

	virtual void FireGameEvent( IGameEvent *event );

	void		GetPingPosition( const Vector &vecDelta, float flRadius, float *xpos, float *ypos, float *flRotation );
	void		SetPingTarget( C_BaseEntity *pTarget );
	void		SetPingDuration( float flDuration );
	void		SetPingOrigin( Vector &vecOrigin );
	static void AddPing( int nType, Vector &vecOrigin, float flDuration = 5.0f, C_BaseEntity *pTarget = NULL );

protected:
	CTFImagePanel	*m_pPingSpy;
	CTFImagePanel	*m_pPingNeedSentry;
	CTFImagePanel	*m_pPingNeedDispenser;
	CTFImagePanel	*m_pPingNeedTeleporter;
	CTFImagePanel	*m_pPingItem;

private:
	IMaterial		*m_pArrowMaterial;
	float			m_flRemoveAt;
	Vector			m_vecPingOrigin;
	CHandle<C_BaseEntity> m_hPingTarget;
	int				m_iDrawArrow;
	bool			m_bOnscreen;
};

#endif // LFE_HUD_PINGSYSTEM_H
