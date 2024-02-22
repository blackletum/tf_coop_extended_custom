//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#ifndef WEAPON_CROSSBOW_H
#define WEAPON_CROSSBOW_H

#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "tf_weaponbase.h"

#define BOLT_AIR_VELOCITY	2500
#define BOLT_WATER_VELOCITY	1500

//-----------------------------------------------------------------------------
// Crossbow Bolt
//-----------------------------------------------------------------------------
class CCrossbowBolt : public CBaseCombatCharacter
{
	DECLARE_CLASS( CCrossbowBolt, CBaseCombatCharacter );

public:
	CCrossbowBolt() { };
	~CCrossbowBolt();

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );
	void BoltTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	virtual float	GetDamage(void) { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }
	unsigned int PhysicsSolidMaskForEntity() const;
	void CCrossbowBolt::SetLauncher(EHANDLE hLauncher);
	void CCrossbowBolt::SetCrit(bool bCritYesNo);
	void CCrossbowBolt::SetMiniCrit(bool bMiniCritYesNo);
	static CCrossbowBolt *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL, CBaseCombatWeapon *pLauncher = NULL, bool bCrit = false, bool bMiniCrit = false);
	CHandle<CBaseCombatWeapon>			m_hLauncher;
	bool m_bCrit;
	bool m_bMiniCrit;
protected:

	float					m_flDamage;
	bool	CreateSprites( void );

	CHandle<CSprite>		m_pGlowSprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

//-----------------------------------------------------------------------------
// CWeaponCrossbow
//-----------------------------------------------------------------------------
class CWeaponCrossbow : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponCrossbow, CBaseHLCombatWeapon );
public:

	CWeaponCrossbow( void );

	virtual int				GetWeaponID( void ) const			{ return TF_WEAPON_HL2_CROSSBOW; }

	virtual void	Precache( void );
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual bool	Deploy( void );
	virtual void	Drop( const Vector &vecVelocity );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual bool	Reload( void );
	virtual void	ItemPostFrame( void );
	virtual void	ItemBusyFrame( void );
	virtual void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	virtual bool	SendWeaponAnim( int iActivity );
	virtual bool	IsWeaponZoomed() { return m_bInZoom; }
	
	bool	ShouldDisplayHUDHint() { return true; }


	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	
	void	StopEffects( void );
	void	SetSkin( int skinNum );
	void	CheckZoomToggle( void );
	void	FireBolt( void );
	void	ToggleZoom( void );
	
	// Various states for the crossbow's charger
	enum ChargerState_t
	{
		CHARGER_STATE_START_LOAD,
		CHARGER_STATE_START_CHARGE,
		CHARGER_STATE_READY,
		CHARGER_STATE_DISCHARGE,
		CHARGER_STATE_OFF,
	};

	void	CreateChargerEffects( void );
	void	SetChargerState( ChargerState_t state );
	void	DoLoadEffect( void );

private:
	
	// Charger effects
	ChargerState_t		m_nChargeState;
	CHandle<CSprite>	m_hChargerSprite;

	bool				m_bInZoom;
	bool				m_bMustReload;
};

#endif // WEAPON_CROSSBOW_H
