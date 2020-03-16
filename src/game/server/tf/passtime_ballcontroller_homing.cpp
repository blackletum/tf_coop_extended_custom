//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Ball Controller Homing.
//
//=============================================================================//

#include "cbase.h"
#include "passtime_ballcontroller.h"
#include "tf_passtime_ball.h"
#include "tf_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
/*class CPasstimeBallControllerHoming : public CPasstimeBallController
{
public:
	friend class CPasstimeBall;

	DECLARE_CLASS( CPasstimeBallControllerHoming, CPasstimeBallController );

	CPasstimeBallControllerHoming();
	~CPasstimeBallControllerHoming();

	virtual void	ApplyTo( CPasstimeBall *pBall );
	virtual void	OnBallCollision( CPasstimeBall *pBall, int index, gamevcollisionevent_t *pEvent );
	virtual void	OnBallDamaged( CPasstimeBall *pBall );
	virtual void	OnBallPickedUp( CPasstimeBall *pBall, CTFPlayer *pPicker );
	virtual void	OnBallSpawned( CPasstimeBall *pBall );
	//virtual void	OnDisabled( void );
	//bool	IsActive() const
	//void			SetTargetSpeed( float flSpeed );
	void			StartHoming( CPasstimeBall *pBall, CTFPlayer *pPrevOwner, bool bSomething );
};


CPasstimeBallControllerHoming::CPasstimeBallControllerHoming()
{
}

CPasstimeBallControllerHoming::~CPasstimeBallControllerHoming()
{
}

void CPasstimeBallControllerHoming::ApplyTo( CPasstimeBall *pBall )
{

}

void CPasstimeBallControllerHoming::OnBallCollision( CPasstimeBall *pBall, int index, gamevcollisionevent_t *pEvent )
{

}

void CPasstimeBallControllerHoming::OnBallDamaged( CPasstimeBall *pBall )
{
}

void CPasstimeBallControllerHoming::OnBallPickedUp( CPasstimeBall *pBall, CTFPlayer *pPicker )
{
}

void CPasstimeBallControllerHoming::OnBallSpawned( CPasstimeBall *pBall )
{
}

void CPasstimeBallControllerHoming::StartHoming( CPasstimeBall *pBall, CTFPlayer *pPrevOwner, bool bSomething )
{
	if ( !pBall )
		return;

	if ( !pPrevOwner )
		return;

	CBaseEntity *pTarget = pPrevOwner->m_Shared.GetPasstimePassTarget();
	if ( pTarget == pPrevOwner )
		return;

	if ( !pTarget->IsAlive() )
		return;

	Vector vecTarget = pTarget->WorldSpaceCenter();

	Vector vecDir = vecTarget - pBall->GetAbsOrigin();
	VectorNormalize( vecDir );

	float flSpeed = pBall->GetAbsVelocity().Length();
	QAngle angForward;
	VectorAngles( vecDir, angForward );
	pBall->SetAbsAngles( angForward );
	pBall->SetAbsVelocity( vecDir * flSpeed );
}*/