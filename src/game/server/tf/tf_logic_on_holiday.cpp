//============ Copyright Valve Corporation, All rights reserved. ===============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "baseanimating.h"
#include "tf_gamerules.h"

#include "tier0/memdbgon.h"

class CLogicOnHoliday : public CBaseEntity
{
public:
	DECLARE_CLASS( CLogicOnHoliday, CBaseEntity );
	DECLARE_DATADESC();

	CLogicOnHoliday();

	virtual void	InputRoundSpawn( inputdata_t &inputdata );

	void			InputFire( inputdata_t &inputdata );

	COutputEvent	m_IsAprilFools;
	COutputEvent	m_IsFullMoon;
	COutputEvent	m_IsHalloween;
	COutputEvent	m_IsSmissmas;
	COutputEvent	m_IsTFBirthday;
	COutputEvent	m_IsValentines;
	COutputEvent	m_IsNothing;
};

//-----------------------------------------------------------------------------
// Purpose: merasmus's spooky paper output
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CLogicOnHoliday )
	DEFINE_INPUTFUNC( FIELD_VOID, "Fire", InputFire ),

	DEFINE_OUTPUT( m_IsAprilFools,	"IsAprilFools" ),
	DEFINE_OUTPUT( m_IsFullMoon,	"IsFullMoon" ),
	DEFINE_OUTPUT( m_IsHalloween,	"IsHalloween" ),
	DEFINE_OUTPUT( m_IsSmissmas,	"IsSmissmas" ),
	DEFINE_OUTPUT( m_IsTFBirthday,	"IsTFBirthday" ),
	DEFINE_OUTPUT( m_IsValentines,	"IsValentines" ),
	DEFINE_OUTPUT( m_IsNothing,		"IsNothing" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_logic_on_holiday, CLogicOnHoliday );

CLogicOnHoliday::CLogicOnHoliday()
{
}

void CLogicOnHoliday::InputFire( inputdata_t &inputdata )
{
	m_IsAprilFools.FireOutput( this, this );
	m_IsFullMoon.FireOutput( this, this );
	m_IsHalloween.FireOutput( this, this );
	m_IsSmissmas.FireOutput( this, this );
	m_IsTFBirthday.FireOutput( this, this );
	m_IsValentines.FireOutput( this, this );
	m_IsNothing.FireOutput( this, this );
}

void CLogicOnHoliday::InputRoundSpawn( inputdata_t &inputdata )
{
	if ( TFGameRules()->IsHolidayActive( kHoliday_None ) )
		m_IsNothing.FireOutput( this, this );

	if ( TFGameRules()->IsHolidayActive( kHoliday_ValentinesDay ) )
		m_IsValentines.FireOutput( this, this );

	if ( TFGameRules()->IsHolidayActive( kHoliday_AprilFools ) )
		m_IsAprilFools.FireOutput( this, this );

	if ( TFGameRules()->IsHolidayActive( kHoliday_Christmas ) )
		m_IsSmissmas.FireOutput( this, this );

	if ( TFGameRules()->IsBirthday() )
		m_IsTFBirthday.FireOutput( this, this );

	if ( TFGameRules()->IsHolidayActive( kHoliday_FullMoon ) || TFGameRules()->IsHolidayActive( kHoliday_HalloweenOrFullMoon ) || TFGameRules()->IsHolidayActive( kHoliday_HalloweenOrFullMoonOrValentines ) )
		m_IsFullMoon.FireOutput( this, this );

	if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) || TFGameRules()->IsHolidayActive( kHoliday_HalloweenOrFullMoon ) || TFGameRules()->IsHolidayActive( kHoliday_HalloweenOrFullMoonOrValentines ) )
		m_IsHalloween.FireOutput( this, this );
}
