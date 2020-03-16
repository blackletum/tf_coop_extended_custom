#include "hl1_basecombatweapon_shared.h"

#ifndef HL1MP_BASECOMBATWEAPON_SHARED_H
#define HL1MP_BASECOMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif


#if defined( CLIENT_DLL )
#define CBaseHL1MPCombatWeapon C_BaseHL1MPCombatWeapon
#endif


class CBaseHL1MPCombatWeapon : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CBaseHL1MPCombatWeapon, CBaseHL1CombatWeapon );
public :
	CBaseHL1MPCombatWeapon();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

public :
	void EjectShell( CBaseEntity *pPlayer, int iType );

	CBasePlayer* GetPlayerOwner() const;
	virtual void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );

#ifdef CLIENT_DLL
	void OnDataChanged( DataUpdateType_t type );
	bool ShouldPredict();

	void ApplyBoneMatrixTransform( matrix3x4_t& transform );
#endif
	bool IsPredicted() const;

};


#endif	// #ifndef HL1MP_BASECOMBATWEAPON_SHARED_H