//============== Copyright LFE-TEAM Not All rights reserved. ==================//
//
// Purpose: medic become super useful
//
//
//=============================================================================//

#ifndef TF_REVIVE_H
#define TF_REVIVE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef GAME_DLL
#include "baseanimating.h"
#include "tf_player.h"
#else
#include "c_baseanimating.h"
#include "c_tf_player.h"
#endif

#ifdef CLIENT_DLL
#define CTFReviveMarker C_TFReviveMarker
#endif // CLIENT_DLL

//=============================================================================
//
//
//
//=============================================================================
//m_nRevives speech_revivecall   speech_revivecall_medium    speech_revivecall_hard
//revive_player_stopped revive_player_complete revive_player_notify marker_entindex
class CTFReviveMarker : public CBaseAnimating
{
public:
	DECLARE_CLASS( CTFReviveMarker, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CTFReviveMarker();
	~CTFReviveMarker();

	virtual void	Spawn();
	virtual void	Precache();

	virtual bool	IsCombatItem( void ) const { return true; }
	bool			IsReviveInProgress( void );

	virtual	bool	ShouldCollide( int collisionGroup, int contentsMask ) const;
	void			ReviveThink( void );

	void			ReviveOwner( void );
	//void			PromptOwner( void );
#ifdef GAME_DLL
	DECLARE_DATADESC();
	virtual int		UpdateTransmitState( void );
	virtual int		ShouldTransmit( const CCheckTransmitInfo *pInfo );
	static CTFReviveMarker *Create( CTFPlayer *pOwner );
	void			AddMarkerHealth( float flAmount );
#else
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual bool	IsVisibleToTargetID( void ) const { return true; }

	// Medic callout particle effect
	CNewParticleEffect	*m_pReviveMeEffect;
#endif
#ifdef GAME_DLL
	void	SetOwner( CTFPlayer *pOwner );
	CNetworkHandle( CTFPlayer,	m_hOwner );
#else
	CHandle< C_TFPlayer > m_hOwner;
#endif
	virtual int		GetHealth( void ) { return m_iMarkerHealth; }
	virtual int		GetMaxHealth( void ) { return m_iMarkerMaxHealth; }
protected:

	CNetworkVar( int, m_nRevives );

#ifdef GAME_DLL
	CNetworkVar( int, m_iMarkerHealth );
	CNetworkVar( int, m_iMarkerMaxHealth );

#else
	int				m_iMarkerHealth;
	int				m_iMarkerMaxHealth;

#endif
};

#endif // TF_REVIVE_H
