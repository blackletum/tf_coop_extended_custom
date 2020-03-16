#ifndef TF_ITEM_POWERUP_BOTTLE_H
#define TF_ITEM_POWERUP_BOTTLE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_wearable.h"

#if defined CLIENT_DLL
#define CTFPowerupBottle C_TFPowerupBottle
#endif

class CTFPowerupBottle : public CTFWearable
{
	DECLARE_CLASS( CTFPowerupBottle, CTFWearable )
public:
	DECLARE_NETWORKCLASS()

	CTFPowerupBottle();
	virtual ~CTFPowerupBottle();

	virtual void	Precache( void );

	virtual void	ReapplyProvision( void );

	virtual int		GetSkin( void );

#ifdef CLIENT_DLL
	virtual const char	*GetEffectLabelText( void );
	virtual const char	*GetEffectIconName( void );
#else
	bool			Use( void );
	void			RemoveEffect( void );
	void			Reset( void );
	void			SetNumCharges( int iNum );
	//void			StatusThink( void );

	virtual void	UnEquip( CBaseEntity *pEntity );
#endif

	//GetWorldModelIndex // alternative canteen models

	int		GetPowerupType( void ) const;
	int		GetNumCharges( void ) const;
	int		GetMaxNumCharges( void ) const;

private:
	enum PowerupType
	{
		EMPTY			= 0,
		CRITBOOST,
		UBERCHARGE,
		RECALL,
		REFILL_AMMO,
		BUILDING_UPGRADE,

		/* ? */
		//RADIUS_STEALTH   = 6,
	};

	//CNetworkVar(bool,          m_bActive);
	CNetworkVar( int, m_usNumCharges );

	IntervalTimer	m_itCooldown;

	CTFPowerupBottle( const CTFPowerupBottle& ) {}
};

#endif
