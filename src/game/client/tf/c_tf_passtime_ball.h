//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Ball.
//
//=============================================================================//
#ifndef C_TF_PASSTIME_BALL_H
#define C_TF_PASSTIME_BALL_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseanimating.h"

class C_PasstimeBall : public C_BaseAnimating
{
	DECLARE_CLASS( C_PasstimeBall, C_BaseAnimating );
public:
	DECLARE_CLIENTCLASS();

	C_PasstimeBall();
	~C_PasstimeBall();

	virtual int		DrawModel( int flags );
	virtual void	OnDataChanged( DataUpdateType_t updateType );

	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const;
	virtual bool	ShouldCollide( int collisionGroup, int contentsMask ) const;
/*
C_PasstimeBall::AddDecal(Vector const&, Vector const&, Vector const&, int, int, bool, CGameTrace&, int)
C_PasstimeBall::GetCarrier()
C_PasstimeBall::GetPrevCarrier()
*/

};

#endif // C_TF_PASSTIME_BALL_H