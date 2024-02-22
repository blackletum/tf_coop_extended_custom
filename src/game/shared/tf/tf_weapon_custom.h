//
//	Hot Garbage Fortress
//	Custom weapon entity, for use with the econ system and attributes.
//
//


#ifndef TF_WEAPON_CUSTOM_H
#define TF_WEAPON_CUSTOM_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

#if defined( CLIENT_DLL )
#define CTFWeaponCustom C_TFWeaponCustom
#define CTFWeaponCustomPrimary C_TFWeaponCustomPrimary
#endif


class CTFWeaponCustom : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS(CTFWeaponCustom, CTFWeaponBaseGun);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFWeaponCustom();

	virtual int		GetWeaponID(void) const			{ return TF_WEAPON_CUSTOM; }
	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack(); 
	virtual bool	Reload(void);
	virtual void	Equip(CBaseCombatCharacter *pOwner);
	virtual int		GetDamageType() const;
//	virtual float   GetProjectileSpeed(void);
//	virtual float   GetProjectileGravity(void);

protected:

	void		Fire(CTFPlayer *pPlayer);
	void		UpdatePunchAngles(CTFPlayer *pPlayer);

private:

	CTFWeaponCustom(const CTFWeaponCustom &) {}
};
class CTFWeaponCustomPrimary : public CTFWeaponCustom
{
public:
	DECLARE_CLASS(CTFWeaponCustomPrimary, CTFWeaponCustom);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	void	Precache();
};









#endif //TF_WEAPON_CUSTOM