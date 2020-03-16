//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_smg.h"

//=============================================================================
//
// Weapon SMG tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFSMG, tf_weapon_smg )

//=============================================================================
//
// Weapon SMG functions.
//

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFSMG::GetDamageType( void ) const
{
	int iDmgType = BaseClass::GetDamageType();

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );

	if ( iType == 1 )
	{
		iDmgType |= DMG_USE_HITLOCATIONS;
	}
	else
	{
		iDmgType |= DMG_BULLET;	
	}

	return iDmgType;
}

//=============================================================================
//
// Weapon Charged SMG tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFChargedSMG, DT_WeaponChargedSMG )

BEGIN_NETWORK_TABLE( CTFChargedSMG, DT_WeaponChargedSMG )
#ifndef CLIENT_DLL
	SendPropFloat( SENDINFO( m_flMinicritCharge ), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
#else
	RecvPropFloat( RECVINFO( m_flMinicritCharge ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CTFChargedSMG )
	DEFINE_PRED_FIELD( m_flMinicritCharge, FIELD_FLOAT, FTYPEDESC_INSENDTABLE )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( tf_weapon_charged_smg, CTFChargedSMG );
PRECACHE_WEAPON_REGISTER( tf_weapon_charged_smg );
