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
CREATE_SIMPLE_WEAPON_TABLE(TFWeaponCustomPrimary, tf_weapon_custom_primary)
// Custom Weapon Functions
CTFWeaponCustom::CTFWeaponCustom()
{
	m_bReloadsSingly = false;

}
void CTFWeaponCustom::Equip(CBaseCombatCharacter *pEquipTo)
{
	BaseClass::Equip(pEquipTo);
}
int	CTFWeaponCustom::GetDamageType() const
{
	int iDmgType = BaseClass::GetDamageType();

	int iType = 0;
	CALL_ATTRIB_HOOK_INT(iType, cw_add_dmgtype);
	if (iType << 0)
		iDmgType |= iType;

		

	return iDmgType;
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
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if (!CanAttack())
		return;
	int iSecondaryMode = 0;
	CALL_ATTRIB_HOOK_INT(iSecondaryMode, cw_secondary_attack_mode);
if (iSecondaryMode == 1){ //Abandon switches because for whatever reason they just didn't work.. at all
	if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0) //UNFINISHED
	{
		FireHLAR2Grenade(pOwner, 0);
		SetSecondaryAmmoCount(pOwner->GetAmmoCount(m_iSecondaryAmmoType) - 1);
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
		WeaponSound(WPN_DOUBLE);
	}
	else{
		WeaponSound(EMPTY);
		DevMsg("skill issue 2");
	}
}
if (iSecondaryMode == 2){
	if (m_iClip1 >= 2 && m_flNextSecondaryAttack >= gpGlobals->curtime) //UNFINISHED
	{
		PrimaryAttack();
		m_flNextPrimaryAttack = gpGlobals->curtime;
		PrimaryAttack();
		//m_iClip1 -= 2;
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
		WeaponSound(WPN_DOUBLE);
		
	}
	else{
		WeaponSound(EMPTY);
		DevMsg("skill issue 2");
	}

}



	// Set the weapon mode.
	//m_iWeaponMode = TF_WEAPON_SECONDARY_MODE;
	
	BaseClass::SecondaryAttack();
	float flSecondaryAttackDelay = 0.5;
	CALL_ATTRIB_HOOK_FLOAT(flSecondaryAttackDelay, secondary_atk_fire_rate);
	m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
}
bool CTFWeaponCustom::Reload(void)
{
	int iReloadSingle = 0;
	CALL_ATTRIB_HOOK_INT(iReloadSingle, cw_reload_singly);
	if (iReloadSingle == 1){
		m_bReloadsSingly = true;
		//DevMsg("Weapon reloads singly (1)");
	}
	if (iReloadSingle == 0){
		m_bReloadsSingly = false;
		//DevMsg("Weapon does not reload singly (0)");
	}
	return BaseClass::Reload();
}
void CTFWeaponCustomPrimary::Precache(void)
{
#ifndef CLIENT_DLL
	// Set the proper classname so it loads the correct script file.
	SetClassname("tf_weapon_custom_primary");
#endif

	BaseClass::Precache();

}
