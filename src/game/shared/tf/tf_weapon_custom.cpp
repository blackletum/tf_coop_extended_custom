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
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;
	if (!CanAttack())
		return;
	float flSecondaryAttackDelay = 0;
	int bNoSeparatePrimaryFire = 0;
	float flPrimaryPunchMax = 0.0f;
	float flPrimaryPunchMin = 0.0f;
	CTFPlayer *pOwner = GetTFPlayerOwner();
	QAngle angle = pOwner->GetPunchAngle();
	CALL_ATTRIB_HOOK_FLOAT(flPrimaryPunchMax, cw_primaryfire_punch_maxangle);
	CALL_ATTRIB_HOOK_FLOAT(flPrimaryPunchMin, cw_primaryfire_punch_minangle);
	CALL_ATTRIB_HOOK_FLOAT(flSecondaryAttackDelay, secondary_atk_fire_rate);
	CALL_ATTRIB_HOOK_INT(bNoSeparatePrimaryFire, cw_separate_primary_secondaryfire);
	if (bNoSeparatePrimaryFire == 1){	
		m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
	//	DevMsg("Test");
		//DevMsg("(P) Secondary Fire Rate: %.2f", flSecondaryAttackDelay);
	}
	if (flPrimaryPunchMax && flPrimaryPunchMin && pOwner)
	{
		angle.x -= SharedRandomInt("CWPunchAngle", (flPrimaryPunchMax), (flPrimaryPunchMin));
		pOwner->SetPunchAngle(angle);
	}
	// Set the weapon mode.
	

	BaseClass::PrimaryAttack();
}
void CTFWeaponCustom::SecondaryAttack()
{
	m_iWeaponMode = TF_WEAPON_SECONDARY_MODE;
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if (!CanAttack())
		return;
	int iSecondaryMode = 0;
	float flSecondaryAttackDelay = 0;
	int bNoSeparatePrimaryFire = 0;
	DevMsg("Secondary Fire Rate Before: %.2f", flSecondaryAttackDelay);
	CALL_ATTRIB_HOOK_FLOAT(flSecondaryAttackDelay, secondary_atk_fire_rate);
	DevMsg("Secondary Fire Rate After: %.2f", flSecondaryAttackDelay);
	CALL_ATTRIB_HOOK_INT(iSecondaryMode, cw_secondary_attack_mode);
	CALL_ATTRIB_HOOK_INT(bNoSeparatePrimaryFire, cw_separate_primary_secondaryfire);

if (iSecondaryMode == 1){ //Abandon switches because for whatever reason they just didn't work.. at all
	if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0) //UNFINISHED
	{
		FireHLAR2Grenade(pOwner, 0);
		pOwner->SetAmmoCount(pOwner->GetAmmoCount(m_iSecondaryAmmoType) - 1, m_iSecondaryAmmoType);
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
		WeaponSound(WPN_DOUBLE);
		m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
		if (bNoSeparatePrimaryFire == 1)
			m_flNextPrimaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
	
	}
	else{
		WeaponSound(EMPTY);
		DevMsg("skill issue 2");
		m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
		if (bNoSeparatePrimaryFire == 1)
			m_flNextPrimaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
	}
}
if (iSecondaryMode == 2 && m_flNextSecondaryAttack <= gpGlobals->curtime){
	if (m_iClip1 >= 2) //Finished now :D
	{
		PrimaryAttack();
		m_flNextPrimaryAttack = gpGlobals->curtime;
		PrimaryAttack();
		//m_iClip1 -= 2;
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
		WeaponSound(WPN_DOUBLE);

		

	//	DevMsg("Secondary Fire Rate After: %.2f", flSecondaryAttackDelay);
		m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
		if (bNoSeparatePrimaryFire == 1)
			m_flNextPrimaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
	}
	else if (m_iClip1 == 1) //Just primary attack bruz
	{
		PrimaryAttack();
		m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
	}
	else{
		WeaponSound(EMPTY);
		DevMsg("skill issue 2");
		m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
		if (bNoSeparatePrimaryFire == 1)
			m_flNextPrimaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
	}

}
if (iSecondaryMode == 3 && m_flNextSecondaryAttack <= gpGlobals->curtime){
	if (pOwner->m_Shared.InCond(TF_COND_ZOOMED))
	{
		ZoomOut();
		pOwner->m_Shared.RemoveCond(TF_COND_AIMING);
	}
	else{
		ZoomIn();
		pOwner->m_Shared.AddCond(TF_COND_AIMING);
	}
	m_flNextSecondaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
	if (bNoSeparatePrimaryFire == 1)
		m_flNextPrimaryAttack = gpGlobals->curtime + flSecondaryAttackDelay;
}


	// Set the weapon mode.
	
	
	BaseClass::SecondaryAttack();
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
float CTFWeaponCustom::GetProjectileSpeed(void)
{
	float flAttribSpeedMult = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT(flAttribSpeedMult, mult_projectile_speed);
	return 1000.0f * flAttribSpeedMult;
}
float CTFWeaponCustom::GetProjectileGravity(void)
{
	float flAttribGravAdd = 0.0f;
	CALL_ATTRIB_HOOK_FLOAT(flAttribGravAdd, add_projectile_gravity);
	CALL_ATTRIB_HOOK_FLOAT(flAttribGravAdd, mult_projectile_gravity);
	

	return flAttribGravAdd;

}
void CTFWeaponCustomPrimary::Precache(void)
{
#ifndef CLIENT_DLL
	// Set the proper classname so it loads the correct script file.
	SetClassname("tf_weapon_custom_primary");
#endif

	BaseClass::Precache();

}
