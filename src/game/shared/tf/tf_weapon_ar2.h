//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	TF_WEAPONAR2_H
#define	TF_WEAPONAR2_H

#include "basegrenade_shared.h"
#include "tf_weaponbase_gun.h"

#ifdef CLIENT_DLL
    #define CTFWeaponAR2 C_TFWeaponAR2
#endif

class CTFWeaponAR2 : public CTFWeaponBaseGun
{
public:
	DECLARE_CLASS( CTFWeaponAR2, CTFWeaponBaseGun );

	CTFWeaponAR2();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int GetWeaponID(void) const { return TF_WEAPON_HL2_AR2; }

	void	ItemPostFrame( void );
	void	Precache( void );
	
	void	SecondaryAttack( void );
	void	DelayedAttack( void );

	//const char *GetTracerType( void ) { return "AR2Tracer"; }
	void	AddViewKick( void );

#ifndef CLIENT_DLL
    int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void	FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void	Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	int		GetMinBurst( void ) { return 2; }
	int		GetMaxBurst( void ) { return 5; }
	float	GetFireRate( void ) { return 0.1f; }

	bool	CanHolster( void );
	bool	Reload( void );

	//Activity	GetPrimaryAttackActivity( void );
	
	void	DoImpactEffect( trace_t &tr, int nDamageType );

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;
		
		cone = VECTOR_CONE_3DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

protected:

    int	m_nShotsFired;	// Number of consecutive shots fired

	float					m_flDelayedFire;
	bool					m_bShotDelayed;
	int						m_nVentPose;

    //DECLARE_ACTTABLE();
	
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
};


#endif	//TF_WEAPONAR2_H