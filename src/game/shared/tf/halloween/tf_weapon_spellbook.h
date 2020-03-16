//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_WEAPON_SPELLBOOK_H
#define TF_WEAPON_SPELLBOOK_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFSpellBook C_TFSpellBook
#endif

//=============================================================================
//
// Bottle class.
//
class CTFSpellBook : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFSpellBook, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFSpellBook();
	~CTFSpellBook();

	virtual void		Precache();

	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_SPELLBOOK; }
	virtual void		PrimaryAttack();
	bool				Deploy( void );

	virtual void		ItemPostFrame( void );
	virtual void		ItemBusyFrame( void );
	virtual void		ItemHolsterFrame( void );

	virtual bool		CanBeSelected( void )				{ return true; }
	virtual bool		VisibleInWeaponSelection( void )	{ return true; }

	/*CanCastSpell(CTFPlayer*)
	CanThrowUnderWater()
	CastKartBombHead(CTFPlayer*)
	CastKartRocketJump(CTFPlayer*)
	CastKartSpell()
	CastKartUber(CTFPlayer*)
	CastRocketJump(CTFPlayer*)
	CastSelfHeal(CTFPlayer*)
	CastSelfSpeedBoost(CTFPlayer*)
	CastSelfStealth(CTFPlayer*)
	CastSpell(CTFPlayer*, int)
	ClearSpell()
	CreateSpellJar(Vector const&, QAngle const&, Vector const&, Vector const&, CBaseCombatCharacter*, CTFWeaponInfo const&)
	CreateSpellRocket(Vector const&, QAngle const&, Vector const&, Vector const&, CBaseCombatCharacter*, CTFWeaponInfo const&)
	FireJar(CTFPlayer*)
	GetEffectLabelText()
	HasASpellWithCharges()
	IsEnergyWeapon() const
	PaySpellCost(CTFPlayer*)
	RollNewSpell(int, bool)
	RollNewSpellFinish()
	SetSelectedSpell(int)
	ShowHudElement()
	TossJarThink()*/

#ifdef CLIENT_DLL
	//GetHandEffect(C_EconItemView*, int)
#endif

private:
#ifdef CLIENT_DLL
	CNewParticleEffect		*m_pSpellEffect;
#endif

	CNetworkVar( bool, m_bFiredAttack );
	CNetworkVar( float, m_flTimeNextSpell );
	CNetworkVar( int, m_iSelectedSpellIndex );
	CNetworkVar( int, m_iSpellCharges );

	CTFSpellBook( const CTFSpellBook & ) {}
};

#endif // TF_WEAPON_SPELLBOOK_H
