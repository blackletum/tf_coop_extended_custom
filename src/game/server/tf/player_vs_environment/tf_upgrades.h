//======= Copyright 1996-2020, Valve Corporation, All rights reserved. ========//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#ifndef TF_UPGRADES_H
#define TF_UPGRADES_H

#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"
#include "GameEventListener.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CUpgrades : public CBaseTrigger
{
public:
	DECLARE_CLASS( CUpgrades, CBaseTrigger );
	DECLARE_DATADESC();

	CUpgrades();
	virtual ~CUpgrades();
	
	virtual void	Spawn( void );
	void			UpgradeTouch( CBaseEntity *pOther );
	virtual void	EndTouch( CBaseEntity *pOther );
	void			Touch( CBaseEntity *pOther ) { return UpgradeTouch( pOther ); }

	virtual void	SetDisabled( bool bDisabled );

	bool			IsDisabled( void ) { return m_bDisabled; };

	// Input handlers
	virtual void	InputEnable( inputdata_t &inputdata );
	virtual void	InputDisable( inputdata_t &inputdata );

	//virtual void	FireGameEvent( IGameEvent *event );

	/*void			ApplyUpgradeToItem( CTFPlayer *pBuyer, CEconItemView*, int, int, bool, bool );
	const char		*GetUpgradeAttributeName( int iIndex ) const;
	void			GrantOrRemoveAllUpgrades( CTFPlayer *pBuyer, bool, bool );
	void			NotifyItemOnUpgrade( CTFPlayer*, unsigned short, bool );
	void			PlayerPurchasingUpgrade( CTFPlayer *pBuyer, int, int, bool, bool, bool );
	void			ReportUpgrade( CTFPlayer *pBuyer, int, int, int, int, bool, bool, bool );*/

private:
	bool	m_bDisabled;

};

#endif // TF_UPGRADES_H
