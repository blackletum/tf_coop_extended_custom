//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_revolver.h"
#include "tf_fx_shared.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Revolver tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFRevolver, tf_weapon_revolver )

//=============================================================================
//
// Weapon Revolver functions.
//

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRevolver::PrimaryAttack( void )
{
	if ( !CanAttack() )
		return;

	BaseClass::PrimaryAttack();

	int iKillsCollectCrit = 0;
	CALL_ATTRIB_HOOK_INT( iKillsCollectCrit, sapper_kills_collect_crits );
	if ( iKillsCollectCrit > 0 )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( pOwner && pOwner->IsAlive() )
		{
			if ( pOwner->m_Shared.GetDecapitationCount() > 0 )
				pOwner->m_Shared.SetDecapitationCount( pOwner->m_Shared.GetDecapitationCount() - 1 );

			if ( pOwner->m_Shared.GetDecapitationCount() == 0 )
				pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFRevolver::CanFireCriticalShot( CBaseEntity *pEntity, bool bIsHeadshot )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (pEntity)
	{
		flDistanceToTarget = pOwner->GetAbsOrigin().DistTo(pEntity->GetAbsOrigin());
	}
	// can only fire a crit shot if this is a headshot
	if ( !bIsHeadshot || !pEntity || (!pEntity->IsPlayer() && !pEntity->IsNPC()) || flDistanceToTarget > 512 )
		return false;

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );
	if ( iType == 1 )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFRevolver::GetDamageType( void ) const
{
	int iDmgType = BaseClass::GetDamageType();

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );
	if ( iType == 1 )
		iDmgType |= DMG_USE_HITLOCATIONS;

	return iDmgType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFRevolver::Deploy( void )
{
	int iKillsCollectCrit = 0;
	CALL_ATTRIB_HOOK_INT( iKillsCollectCrit, sapper_kills_collect_crits );
	if ( iKillsCollectCrit > 0 )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( pOwner && BaseClass::Deploy() )
		{
			if ( pOwner->m_Shared.GetDecapitationCount() > 0 )
				pOwner->m_Shared.AddCond( TF_COND_CRITBOOSTED );

			return true;
		}
	}

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFRevolver::Holster( CBaseCombatWeapon *pSwitchTo )
{
	int iKillsCollectCrit = 0;
	CALL_ATTRIB_HOOK_INT( iKillsCollectCrit, sapper_kills_collect_crits );
	if ( iKillsCollectCrit > 0 )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( pOwner && BaseClass::Holster( pSwitchTo ) )
		{
			pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );
			return true;
		}
	}

	return BaseClass::Holster( pSwitchTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRevolver::Detach( void )
{
	int iKillsCollectCrit = 0;
	CALL_ATTRIB_HOOK_INT( iKillsCollectCrit, sapper_kills_collect_crits );
	if ( iKillsCollectCrit > 0 )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( pOwner )
			pOwner->m_Shared.SetDecapitationCount( 0 );
	}

	BaseClass::Detach();
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
int CTFRevolver::GetCount( void ) const
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return 0;
	
	return pOwner->m_Shared.GetDecapitationCount();
}
#endif