//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_shovel.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Shovel tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFShovel, tf_weapon_shovel )

//=============================================================================
//
// Weapon Shovel functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFShovel::CTFShovel()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFShovel::GetCustomDamageType() const
{
	int nShovelWeaponMode = 0;
	CALL_ATTRIB_HOOK_INT( nShovelWeaponMode, set_weapon_mode );
	if ( nShovelWeaponMode == 1 || nShovelWeaponMode == 2 )
		return TF_DMG_CUSTOM_PICKAXE;

	return TF_DMG_CUSTOM_NONE;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFShovel::GetSpeedMod( void ) const
{
	if ( m_bLowered )
		return 1.0f;

	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return 1.0f;

	int nMode = 0;
	CALL_ATTRIB_HOOK_INT( nMode, set_weapon_mode );
	if ( nMode != 2 )
		return 1.0f;

	float flFraction = (float)pOwner->GetHealth() / pOwner->GetMaxHealth();
	if ( flFraction > 0.8f )
		return 1.0f;
	else if ( flFraction > 0.6f )
		return 1.1f;
	else if ( flFraction > 0.4f )
		return 1.2f;
	else if ( flFraction > 0.2f )
		return 1.4f;
	else
		return 1.6f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFShovel::GetMeleeDamage( CBaseEntity *pTarget, int &iDamageType, int &iCustomDamage )
{
	float flDamage = BaseClass::GetMeleeDamage( pTarget, iDamageType, iCustomDamage );
	iCustomDamage = GetCustomDamageType();

	int nMode = 0;
	CALL_ATTRIB_HOOK_INT( nMode, set_weapon_mode );
	if ( nMode != 1 )
		return flDamage;

	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return 0.0f;

	float flFraction = Clamp( (float)pOwner->GetHealth() / pOwner->GetMaxHealth(), 0.0f, 1.0f );
	flDamage *= flFraction * -1.15 + 1.65;

	return flDamage;
}
