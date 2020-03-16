//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Ball Controller.
//
//=============================================================================//

#include "cbase.h"
#include "passtime_ballcontroller.h"
#include "tf_passtime_ball.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_AUTO_LIST( IPasstimeBallControllerAutoList );

//CPasstimeBallController g_PasstimeBallController;

CPasstimeBallController::CPasstimeBallController( int index )
{

}

CPasstimeBallController::~CPasstimeBallController()
{

}

void CPasstimeBallController::ApplyTo( CPasstimeBall *pBall )
{

}

void CPasstimeBallController::BallCollision( CPasstimeBall *pBall, int index, gamevcollisionevent_t *pEvent )
{

}

void CPasstimeBallController::OnBallCollision( CPasstimeBall *pBall, int index, gamevcollisionevent_t *pEvent )
{

}

void CPasstimeBallController::BallDamaged( CPasstimeBall *pBall )
{
}

void CPasstimeBallController::OnBallDamaged( CPasstimeBall *pBall )
{
}

void CPasstimeBallController::BallPickedUp( CPasstimeBall *pBall, CTFPlayer *pPicker )
{
}

void CPasstimeBallController::OnBallPickedUp( CPasstimeBall *pBall, CTFPlayer *pPicker )
{
}

void CPasstimeBallController::BallSpawned( CPasstimeBall *pBall )
{
}

void CPasstimeBallController::OnBallSpawned( CPasstimeBall *pBall )
{
}