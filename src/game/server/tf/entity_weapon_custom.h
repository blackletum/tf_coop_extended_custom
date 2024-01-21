//=========================                         ==========================//
//
// Purpose: Custom Weapon with Custom Attributes.
//
//=============================================================================//
#ifndef ENTITY_WEAPON_CUSTOM_H
#define ENTITY_WEAPON_CUSTOM_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_powerup.h"

#define MAX_NUM_CUSTOM_ATTRIBUTES		16

//=============================================================================

class CTFLFEWeaponCustom : public CTFPowerup
{
public:
	DECLARE_CLASS( CTFLFEWeaponCustom, CTFPowerup );
	DECLARE_DATADESC();

	CTFLFEWeaponCustom();

	void					Spawn( void );
	void					Precache( void );
	virtual bool			KeyValue( const char *szKeyName, const char *szValue );
	virtual CBaseEntity*	Respawn( void );
	virtual void			Materialize( void );
	bool					MyTouch( CBasePlayer *pPlayer );
	void					EndTouch( CBaseEntity *pOther );

private:
	CEconItemView			m_Item;

	int						m_nAttributes[MAX_NUM_CUSTOM_ATTRIBUTES];
	float					m_nAttributesValue[MAX_NUM_CUSTOM_ATTRIBUTES];
	bool					m_bSkipBaseAttributes;

	int						m_nItemID;
	const char				*m_szItemClassname;
	char					m_szItemModelW[128];
	char					m_szItemModelV[128];

};

#endif // ENTITY_WEAPON_CUSTOM_H