//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Ball Controller.
//
//=============================================================================//
#ifndef TF_PASSTIME_BALLCONTROLLER_H
#define TF_PASSTIME_BALLCONTROLLER_H
#ifdef _WIN32
#pragma once
#endif

#include "filesystem.h"
#include <KeyValues.h>
#include "gamestats.h"


class CPasstimeBall;

DECLARE_AUTO_LIST( IPasstimeBallControllerAutoList )
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CPasstimeBallController : public IPasstimeBallControllerAutoList
{
public:
	friend class CPasstimeBall;

	DECLARE_CLASS_NOBASE( CPasstimeBallController );

	CPasstimeBallController( int index );
	~CPasstimeBallController();

	virtual void	ApplyTo( CPasstimeBall *pBall );
	void			BallCollision( CPasstimeBall *pBall , int index, gamevcollisionevent_t *pEvent );
	void			BallDamaged( CPasstimeBall *pBall );
	void			BallPickedUp( CPasstimeBall *pBall, CTFPlayer *pPicker );
	void			BallSpawned( CPasstimeBall *pBall );

	virtual void	OnBallCollision( CPasstimeBall *pBall, int index, gamevcollisionevent_t *pEvent );
	virtual void	OnBallDamaged( CPasstimeBall *pBall );
	virtual void	OnBallPickedUp( CPasstimeBall *pBall, CTFPlayer *pPicker );
	virtual void	OnBallSpawned( CPasstimeBall *pBall );
	/*void			DisableOn( CPasstimeBall const* );
	virtual void	OnDisabled( void );
	virtual void	OnEnabled( void );
	void	SetIsEnabled( bool bState );*/
};

extern CPasstimeBallController g_PasstimeBallController;

#endif // TF_PASSTIME_BALLCONTROLLER_H
