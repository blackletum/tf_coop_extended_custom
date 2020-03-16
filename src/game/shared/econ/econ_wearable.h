//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//========================================================================//

#ifndef ECON_WEARABLE_H
#define ECON_WEARABLE_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#include "particles_new.h"
#endif

#define MAX_WEARABLES_SENT_FROM_SERVER	5

#if defined( CLIENT_DLL )
#define CEconWearable C_EconWearable
#endif


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CEconWearable : public CEconEntity
{
	DECLARE_CLASS( CEconWearable, CEconEntity );
	DECLARE_NETWORKCLASS();

public:

	virtual void			Spawn( void );
	virtual bool			IsWearable( void ) { return true; }
	virtual int				GetSkin( void );
	virtual void			UpdateWearableBodyGroups( CBaseEntity *pEntity );
	virtual void			GiveTo( CBaseEntity *pEntity );

	virtual bool			IsViewModelWearable( void ) { return false; }

	virtual bool			IsExtraWearable( void ) { return m_bExtraWearable; }
#ifdef GAME_DLL
	virtual void			Equip( CBaseEntity *pEntity );
	virtual void			UnEquip( CBaseEntity *pEntity );
	virtual void			SetExtraWearable( bool bExtraWearable ) { m_bExtraWearable = bExtraWearable; }
	void					Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
#else
	virtual void			OnDataChanged(DataUpdateType_t type);
	virtual	ShadowType_t	ShadowCastType( void );
	virtual bool			ShouldDraw( void );
#endif

protected:
 #ifdef GAME_DLL
	CNetworkVar( bool, m_bExtraWearable );
#else
	bool m_bExtraWearable;
#endif

private:

};

#endif
