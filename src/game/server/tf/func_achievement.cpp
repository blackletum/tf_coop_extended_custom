//=============================================================================//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "func_achievement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CAchievementZone )

	DEFINE_KEYFIELD( m_iZoneID, FIELD_INTEGER, "zone_id" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( func_achievement, CAchievementZone );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAchievementZone::CAchievementZone()
{
	m_bDisabled = false;
	m_iZoneID = 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAchievementZone::Spawn( void )
{
	Precache();
	BaseClass::Spawn();
	InitTrigger();

	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS ); // so we can keep track of who is touching us
	AddEffects( EF_NODRAW );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementZone::Precache( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: Return true if the specified entity is touching this zone
//-----------------------------------------------------------------------------
bool CAchievementZone::IsTouching( CBaseEntity *pEntity )
{
	return BaseClass::IsTouching( pEntity );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAchievementZone::InputEnable( inputdata_t &inputdata )
{
	SetDisabled( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAchievementZone::InputDisable( inputdata_t &inputdata )
{
	SetDisabled( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CAchievementZone::IsDisabled( void )
{
	return m_bDisabled;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAchievementZone::InputToggle( inputdata_t &inputdata )
{
	if ( m_bDisabled )
	{
		SetDisabled( false );
	}
	else
	{
		SetDisabled( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAchievementZone::SetDisabled( bool bDisabled )
{
	m_bDisabled = bDisabled;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the specified entity is in an achievement zone
//-----------------------------------------------------------------------------
bool InAchievementZone( CBaseEntity *pEntity )
{
	CBaseEntity *pTempEnt = NULL;
	while ( ( pTempEnt = gEntList.FindEntityByClassname( pTempEnt, "func_achievement" ) ) != NULL )
	{
		CAchievementZone *pZone = dynamic_cast<CAchievementZone *>(pTempEnt);

		if ( !pZone->IsDisabled() && pZone->IsTouching( pEntity ) )
		{
			return true;
		}
	}

	return false;
}
