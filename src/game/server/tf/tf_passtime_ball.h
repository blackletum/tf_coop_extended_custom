//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Ball.
//
//=============================================================================//
#ifndef TF_PASSTIME_BALL_H
#define TF_PASSTIME_BALL_H

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"
#include "tf_player.h"

//=============================================================================
//
// CTF Passtime Ball class.
//

class CPasstimeBall : public CBaseAnimating
{
public:
	DECLARE_CLASS( CPasstimeBall, CBaseAnimating );
	DECLARE_SERVERCLASS();

	CPasstimeBall();
	~CPasstimeBall();

	virtual void			Spawn();
	virtual void			Precache();

	void					DefaultThink( void );

	void					CreateModelCollider( void );
	void					CreateSphereCollider( void );

	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const;
	virtual void			VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	void					OnCollision( void );
	virtual bool			ShouldCollide( int collisionGroup, int contentsMask ) const;

	static CPasstimeBall	*Create( const Vector &vecOrigin, const QAngle &vecAngles );

	void					ResetTrail();

	bool					BIgnorePlayer( CTFPlayer *pTarget );

	void					TouchPlayer( CTFPlayer *pPlayer );

	void					SetStateCarried( CTFPlayer *pCarrier );
	void					SetStateFree( void );

	CBaseEntity				*GetThrower( void ) const { return GetOwnerEntity(); }
	CBaseEntity				*GetCarrier( void ) const { return GetOwnerEntity(); }

	void					MoveTo( const Vector &vecOrigin, const Vector &vecVelocity );

	virtual bool			IsDeflectable() { return true; }
	virtual void			Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir );

	virtual void			ChangeTeam( int iTeamNum );

	virtual int				OnTakeDamage( const CTakeDamageInfo &info );

	virtual int				UpdateTransmitState( void );

	/*bool		BOutOfPlay( void ) const
	CPasstimeBall::BlockDamage(CTFPlayer*, Vector const&)
	CPasstimeBall::BlockReflect(CTFPlayer*, Vector const&, Vector const&)
	float		GetAirtimeDistance() const
	float		GetAirtimeSec() const
	float		GetCarryDuration() const
	CPasstimeBall::GetCollisionCount() const
	CBaseEntity	*GetHomingTarget() const
	CBaseEntity	*GetPrevCarrier() const
	void		MoveToSpawner(Vector const&)
	void		OnBecomeNotCarried()
	void		SetHomingTarget( CTFPlayer *pTarget )
	void		SetStateOutOfPlay( void )*/

private:
	EHANDLE	m_hHomingTarget;
	EHANDLE	m_hCarrier;
	EHANDLE	m_hPrevCarrier;

	EHANDLE m_hSpriteTrail;

	int		m_iCollisionCount;

	CPasstimeBall( const CPasstimeBall & );

	DECLARE_DATADESC();
};

#endif // TF_PASSTIME_BALL_H
