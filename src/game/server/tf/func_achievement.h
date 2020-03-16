//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef FUNC_ACHIEVEMENT_H
#define FUNC_ACHIEVEMENT_H

#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"
#include "tf_player.h"

//=============================================================================

class CAchievementZone : public CBaseTrigger
{
public:
	DECLARE_CLASS( CAchievementZone, CBaseTrigger );
	DECLARE_DATADESC();

	CAchievementZone();

	void	Spawn( void );
	void	Precache( void );
	
	// Return true if the specified entity is touching this zone
	bool	IsTouching( CBaseEntity *pEntity );

	bool	IsDisabled( void );
	void	SetDisabled( bool bDisabled );

	// Input handlers
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );
	void	InputToggle( inputdata_t &inputdata );

private:
	bool	m_bDisabled;
	bool	m_iZoneID;
};

// Return true if the specified entity is in an achievement zone
bool InAchievementZone( CBaseEntity *pEntity );

#endif // FUNC_ACHIEVEMENT_H
