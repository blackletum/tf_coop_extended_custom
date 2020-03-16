//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Antlion Grub - cannon fodder
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef	NPC_ANTLIONGRUB_H
#define	NPC_ANTLIONGRUB_H

#include "items.h"
#include "ai_utils.h"
#include "Sprite.h"

enum GrubState_e
{
	GRUB_STATE_IDLE,
	GRUB_STATE_AGITATED,
};

enum
{
	NUGGET_NONE,
	NUGGET_SMALL = 1,
	NUGGET_MEDIUM,
	NUGGET_LARGE
};

//
//  Grub nugget
//

class CGrubNugget : public CItem
{
public:
	DECLARE_CLASS( CGrubNugget, CItem );

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void Event_Killed( const CTakeDamageInfo &info ); 
	virtual bool VPhysicsIsFlesh( void );
	
	bool	MyTouch( CBasePlayer *pPlayer );
	void	SetDenomination( int nSize ) { Assert( nSize <= NUGGET_LARGE && nSize >= NUGGET_SMALL ); m_nDenomination = nSize; }

	DECLARE_DATADESC();

private:
	int		m_nDenomination;	// Denotes size and health amount given
};

//
//  Simple grub
//

class CAntlionGrub : public CBaseAnimating
{
public:
	DECLARE_CLASS( CAntlionGrub, CBaseAnimating );

	virtual void	Activate( void );
	virtual void	Spawn( void );
	virtual void	Precache( void );
	virtual void	UpdateOnRemove( void );
	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);

	void	InputSquash( inputdata_t &data );

	void	IdleThink( void );
	void	FlinchThink( void );
	void	GrubTouch( CBaseEntity *pOther );

	DECLARE_DATADESC();

protected:

	inline bool InPVS( void );
	void		SetNextThinkByDistance( void );

	int		GetNuggetDenomination( void );
	void	CreateNugget( void );
	void	MakeIdleSounds( void );
	void	MakeSquashDecals( const Vector &vecOrigin );
	void	AttachToSurface( void );
	void	CreateGlow( void );
	void	FadeGlow( void );
	void	Squash( CBaseEntity *pOther, bool bDealDamage, bool bSpawnBlood );
	void	SpawnSquashedGrub( void );
	void	InputAgitate( inputdata_t &inputdata );

	inline bool ProbeSurface( const Vector &vecTestPos, const Vector &vecDir, Vector *vecResult, Vector *vecNormal );

	CHandle<CSprite>	m_hGlowSprite;
	int					m_nGlowSpriteHandle;
	float				m_flFlinchTime;
	float				m_flNextIdleSoundTime;
	float				m_flNextSquealSoundTime;
	bool				m_bOutsidePVS;
	GrubState_e			m_State;

	COutputEvent		m_OnAgitated;
	COutputEvent		m_OnDeath;
	COutputEvent		m_OnDeathByPlayer;
};

#endif	//NPC_ANTLIONGRUB_H