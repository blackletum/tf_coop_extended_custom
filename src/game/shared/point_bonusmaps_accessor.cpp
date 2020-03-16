//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"

#include "fmtstr.h"
#include "igameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPointBonusMapsAccessor : public CPointEntity
{
public:
	DECLARE_CLASS( CPointBonusMapsAccessor, CPointEntity );
	DECLARE_DATADESC();

	virtual void	Activate( void );

	void InputUnlock( inputdata_t& inputdata );
	void InputComplete( inputdata_t& inputdata );
	void InputSave( inputdata_t& inputdata );

private:
	string_t	m_String_tFileName;
	string_t	m_String_tMapName;
};

BEGIN_DATADESC( CPointBonusMapsAccessor )
	DEFINE_KEYFIELD( m_String_tFileName, FIELD_STRING, "filename" ),
	DEFINE_KEYFIELD( m_String_tMapName, FIELD_STRING, "mapname" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Unlock", InputUnlock ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Complete", InputComplete ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Save", InputSave ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( point_bonusmaps_accessor, CPointBonusMapsAccessor );

void CPointBonusMapsAccessor::Activate( void )
{
	BaseClass::Activate();
}

void CPointBonusMapsAccessor::InputUnlock( inputdata_t& inputdata )
{
}

void CPointBonusMapsAccessor::InputComplete( inputdata_t& inputdata )
{
}

void CPointBonusMapsAccessor::InputSave( inputdata_t& inputdata )
{
}

#endif

void BonusMapChallengeUpdate( const char *pchFileName, const char *pchMapName, const char *pchChallengeName, int iBest )
{
}

void BonusMapChallengeNames( char *pchFileName, char *pchMapName, char *pchChallengeName )
{
}

void BonusMapChallengeObjectives( int &iBronze, int &iSilver, int &iGold )
{
}