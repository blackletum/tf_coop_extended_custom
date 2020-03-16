//=============================================================================//
//
// Purpose: Mannpower Imbalance crits
//
//=============================================================================//
#ifndef FUNC_POWERUPVOLUME_H
#define FUNC_POWERUPVOLUME_H

#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"
#include "tf_player.h"

//=============================================================================

DECLARE_AUTO_LIST( IFuncPowerupVolumeAutoList );

class CPowerupVolume : public CBaseTrigger, public IFuncPowerupVolumeAutoList
{
public:
	DECLARE_CLASS( CPowerupVolume, CBaseTrigger );
	DECLARE_DATADESC();

	virtual void	Spawn( void );
	virtual void	Precache( void );
	virtual void	Touch( CBaseEntity *pOther );

private:
	bool	m_bDisabled;
};

#endif // FUNC_POWERUPVOLUME_H



