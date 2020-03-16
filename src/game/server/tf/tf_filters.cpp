//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "filters.h"
#include "team_control_point.h"
#include "tf_gamerules.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Team Fortress Team Filter
//
class CFilterTFTeam : public CBaseFilter
{
	DECLARE_CLASS( CFilterTFTeam, CBaseFilter );

public:

	void InputRoundSpawn( inputdata_t &inputdata );
	void InputRoundActivate( inputdata_t &inputdata );

	inline bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );

private:

	string_t m_iszControlPointName;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CFilterTFTeam )

DEFINE_KEYFIELD( m_iszControlPointName, FIELD_STRING, "controlpoint" ),

// Inputs.
DEFINE_INPUTFUNC( FIELD_VOID, "RoundSpawn", InputRoundSpawn ),
DEFINE_INPUTFUNC( FIELD_VOID, "RoundActivate", InputRoundActivate ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( filter_activator_tfteam, CFilterTFTeam );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CFilterTFTeam::PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
{
	// is the entity we're asking about on the winning 
	// team during the bonus time? (winners pass all filters)
	if (  TFGameRules() &&
		( TFGameRules()->State_Get() == GR_STATE_TEAM_WIN ) && 
		( TFGameRules()->GetWinningTeam() == pEntity->GetTeamNumber() ) )
	{
		// this should open all doors for the winners
		if ( m_bNegated )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return ( pEntity->GetTeamNumber() == GetTeamNumber() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFilterTFTeam::InputRoundSpawn( inputdata_t &input )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFilterTFTeam::InputRoundActivate( inputdata_t &input )
{
	if ( m_iszControlPointName != NULL_STRING )
	{
		CTeamControlPoint *pControlPoint = dynamic_cast<CTeamControlPoint*>( gEntList.FindEntityByName( NULL, m_iszControlPointName ) );
		if ( pControlPoint )
		{
			ChangeTeam( pControlPoint->GetTeamNumber() );
		}
		else
		{
			Warning( "%s failed to find control point named '%s'\n", GetClassname(), STRING(m_iszControlPointName) );
		}
	}
}

//=============================================================================
//
// Class filter
//

class CFilterTFClass : public CBaseFilter
{
	DECLARE_CLASS( CFilterTFClass, CBaseFilter );

public:

	inline bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );

private:

	int	m_iAllowedClass;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CFilterTFClass )

DEFINE_KEYFIELD( m_iAllowedClass, FIELD_INTEGER, "tfclass" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( filter_tf_class, CFilterTFClass );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CFilterTFClass::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
	CTFPlayer *pPlayer = dynamic_cast< CTFPlayer * >( pEntity );

	if ( !pPlayer )
		return false;

	if ( m_bNegated )
	{
		return ( !pPlayer->IsPlayerClass( m_iAllowedClass ));
	}

	return ( pPlayer->IsPlayerClass( m_iAllowedClass ) );
}

//=============================================================================
//
// Condition filter
//

class CFilterTFCondition : public CBaseFilter
{
	DECLARE_CLASS( CFilterTFCondition, CBaseFilter );

public:

	inline bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );

private:

	int	m_iCond;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CFilterTFCondition )

DEFINE_KEYFIELD( m_iCond, FIELD_INTEGER, "condition" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( filter_tf_condition, CFilterTFCondition );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CFilterTFCondition::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
	CTFPlayer *pPlayer = ToTFPlayer( pEntity );
	CAI_BaseNPC *pNPC = dynamic_cast< CAI_BaseNPC * >( pEntity );

	if ( m_bNegated )
	{
		if ( pPlayer )
			return ( !pPlayer->m_Shared.InCond( m_iCond ));

		if ( pNPC )
			return ( !pNPC->InCond( m_iCond ));
	}
	else
	{
		if ( pPlayer )
			return ( pPlayer->m_Shared.InCond( m_iCond ) );

		if ( pNPC )
			return ( pNPC->InCond( m_iCond ) );
	}

	return false;
}


//=============================================================================
//
// longname filter
//
class FilterDamagedByWeaponInSlot : public CBaseFilter
{
	DECLARE_CLASS( FilterDamagedByWeaponInSlot, CBaseFilter );
	DECLARE_DATADESC();

protected:

	bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		ASSERT( false );
	 	return true;
	}

	bool PassesDamageFilterImpl( const CTakeDamageInfo &info );

	int m_iWeaponSlot;
};

LINK_ENTITY_TO_CLASS( filter_tf_damaged_by_weapon_in_slot, FilterDamagedByWeaponInSlot );

BEGIN_DATADESC( FilterDamagedByWeaponInSlot )

	// Keyfields
	DEFINE_KEYFIELD( m_iWeaponSlot,	FIELD_INTEGER,	"weaponSlot" ),

END_DATADESC()

bool FilterDamagedByWeaponInSlot::PassesDamageFilterImpl( const CTakeDamageInfo &info )
{
	bool bPass = false;
	CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( info.GetWeapon() );
	if ( !pWeapon )
		bPass = false;

	CTFPlayer *pAttacker = ToTFPlayer( info.GetAttacker() );
	if ( !pAttacker )
		bPass = false;

	CTFWeaponBase *pWeaponSlot = pAttacker->Weapon_GetWeaponByType( m_iWeaponSlot );
	if ( !pWeaponSlot )
		bPass = false;

	if ( pWeapon == pWeaponSlot )
		bPass = true;

	return bPass;
}

//=============================================================================
//
// Squadname filter
//

class CFilterLFENPCHasSquad : public CBaseFilter
{
	DECLARE_CLASS( CFilterLFENPCHasSquad, CBaseFilter );

public:

	inline bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );

private:

	string_t	m_iszSquadName;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CFilterLFENPCHasSquad )

DEFINE_KEYFIELD( m_iszSquadName, FIELD_STRING, "squadname" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( filter_lfe_npc_has_squad, CFilterLFENPCHasSquad );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CFilterLFENPCHasSquad::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
	CAI_BaseNPC *pNPC = dynamic_cast< CAI_BaseNPC * >( pEntity );
	if ( !pNPC )
		return false;

	if ( m_bNegated )
	{
		if ( pNPC )
			return ( !pNPC->IsInSquadName( m_iszSquadName ));
	}

	return ( pNPC->IsInSquadName( m_iszSquadName ) );
}
