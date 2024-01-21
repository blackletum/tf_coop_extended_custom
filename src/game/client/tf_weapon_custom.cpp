//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================


#include "cbase.h"
#include "tf_weapon_custom.h"
#include "decals.h"
#include "tf_fx_shared.h"
#include "tf_gamerules.h"

// Client specific.
#if defined( CLIENT_DLL )
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_obj_sentrygun.h"
#include "sceneentity.h"
#endif

CREATE_SIMPLE_WEAPON_TABLE(TFWeaponCustom, tf_weapon_custom)
// Custom Weapon Functions
CTFWeaponCustom::CTFWeaponCustom()
{
	m_bReloadsSingly = false;
}
void CTFWeaponCustom::Equip(CBaseCombatCharacter *pEquipTo)
{
	BaseClass::Equip(pEquipTo);
}
void CTFWeaponCustom::PrimaryAttack()
{
	if (!CanAttack())
		return;

	// Set the weapon mode.
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

	BaseClass::PrimaryAttack();
}
void CTFWeaponCustom::SecondaryAttack()
{
	if (!CanAttack())
		return;

	// Set the weapon mode.
	m_iWeaponMode = TF_WEAPON_SECONDARY_MODE;

	BaseClass::SecondaryAttack();
}
