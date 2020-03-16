//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Logic.
//
//=============================================================================//
#ifndef TF_PASSTIME_LOGIC_H
#define TF_PASSTIME_LOGIC_H

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

//=============================================================================
//
// CTF Powerup class.
//

class CTFPasstimeLogic : public CPointEntity
{
public:
	DECLARE_CLASS( CTFPasstimeLogic, CPointEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CTFPasstimeLogic();
	~CTFPasstimeLogic();

	virtual void	Precache();

	void			InputJumpPadUsed( inputdata_t &inputdata );
	void			InputRoomTriggerOnTouch( inputdata_t &inputdata );
	void			InputSetSection( inputdata_t &inputdata );
	void			InputSpawnBall( inputdata_t &inputdata );
	void			InputSpeedBoostUsed( inputdata_t &inputdata );
	void			InputTimeUp( inputdata_t &inputdata );

	void			statica( inputdata_t &inputdata );
	void			staticb( inputdata_t &inputdata );
	void			staticc( inputdata_t &inputdata );

/*
void	AddBallPower(int)
bool	BCanPlayerPickUpBall(CTFPlayer*, HudNotification_t*) const
void	BallHistSampleThink()
void	BallPower_PackHealThink()
void	BallPower_PackThink()
void	BallPower_PowerThink()
CalcProgressFrac() const
EjectBall(CTFPlayer*, CTFPlayer*)
EndRoundExpiredTimer()
FireGameEvent(IGameEvent*)
CBaseEntity *GetBall() const
float	GetLastPassTime(CTFPlayer*)
float	GetPackSpeed(CTFPlayer*) const
LaunchBall(CTFPlayer*, Vector const&, Vector const&)
MoveBallToSpawner()
NetworkVar_m_trackPoints::Set(int, Vector const&)
OnBallCarrierDamaged(CTFPlayer*, CTFPlayer*, CTakeDamageInfo const&)
OnBallCarrierMeleeHit(CTFPlayer*, CTFPlayer*)
OnBallCollision(CPasstimeBall*, int, gamevcollisionevent_t*)
OnBallGet()
void	OnEnterGoal(CPasstimeBall*, CFuncPasstimeGoal*)
void	OnEnterGoal(CTFPlayer*, CFuncPasstimeGoal*)
void	OnExitGoal(CPasstimeBall*, CFuncPasstimeGoal*)
void	OnPlayerTouchBall(CTFPlayer*, CPasstimeBall*)
void	OnStayInGoal(CTFPlayer*, CFuncPasstimeGoal*)
void	OneSecStatsUpdateThink()
void	ParseSetSection(char const*, SetSectionParams&) const
void	PostSpawn()
ReplicatePackMemberBits()
void	RespawnBall()
void	Score(CTFPlayer*, int, int, bool)
SecretRoom_Solve()
SecretRoom_Spawn()
SecretRoom_UpdateTv(int)
void	SetLastPassTime(CTFPlayer*)
ShouldEndOvertime() const
virtual void	Spawn()
void	SpawnBallAtRandomSpawnerThink()
void	SpawnBallAtSpawner(CPasstimeBallSpawn*)
void	StealBall(CTFPlayer*, CTFPlayer*)
void	ThinkExpiredTimer()
CTFPasstimeLogic::UpdateTransmitState()

EHANDLE m_hBall;
m_trackPoints;
int		m_iNumSections;
int		m_iCurrentSection;
float	m_flMaxPassRange;
int		m_iBallPower;
float	m_flPackSpeed;
bool	m_bPlayerIsPackMember;
*/
};

#endif // TF_PASSTIME_LOGIC_H


