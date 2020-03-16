//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Logic.
//
//=============================================================================//

#include "cbase.h"
#include "tf_passtime_logic.h"
#include "tf_gamerules.h"

#include "tier0/memdbgon.h"


IMPLEMENT_SERVERCLASS_ST( CTFPasstimeLogic, DT_TFPasstimeLogic )
END_SEND_TABLE()

BEGIN_DATADESC( CTFPasstimeLogic )
	DEFINE_INPUTFUNC( FIELD_VOID, "JumpPadUsed", InputJumpPadUsed ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SpeedBoostUsed", InputSpeedBoostUsed ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RoomTriggerOnTouch", InputRoomTriggerOnTouch ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetSection", InputSetSection ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SpawnBall", InputSpawnBall ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TimeUp", InputTimeUp ),
	DEFINE_INPUTFUNC( FIELD_VOID, "statica", statica ),
	DEFINE_INPUTFUNC( FIELD_VOID, "staticb", staticb ),
	DEFINE_INPUTFUNC( FIELD_VOID, "staticc", staticc ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( passtime_logic, CTFPasstimeLogic );

CTFPasstimeLogic::CTFPasstimeLogic()
{
}

CTFPasstimeLogic::~CTFPasstimeLogic()
{
}

void CTFPasstimeLogic::Precache( void )
{
	PrecacheScriptSound( "Passtime.Crowd.Boo" );
	PrecacheScriptSound( "Passtime.Crowd.Cheer" );
	PrecacheScriptSound( "Passtime.Crowd.React.Neg" );
	PrecacheScriptSound( "Passtime.Crowd.React.Pos" );
	PrecacheScriptSound( "Passtime.Merasmus.Laugh" );
}

void CTFPasstimeLogic::InputJumpPadUsed( inputdata_t &inputdata )
{
}

void CTFPasstimeLogic::InputSpeedBoostUsed( inputdata_t &inputdata )
{
}

void CTFPasstimeLogic::InputSetSection( inputdata_t &inputdata )
{
}

void CTFPasstimeLogic::InputSpawnBall( inputdata_t &inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose: merasmus room
//-----------------------------------------------------------------------------
void CTFPasstimeLogic::InputRoomTriggerOnTouch( inputdata_t &inputdata )
{
}

void CTFPasstimeLogic::InputTimeUp( inputdata_t &inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose: tv turn on
//-----------------------------------------------------------------------------
void CTFPasstimeLogic::statica( inputdata_t &inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose: passtime ball screen
//-----------------------------------------------------------------------------
void CTFPasstimeLogic::staticb( inputdata_t &inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose: ACHIEVEMENT UNLOCKED
//-----------------------------------------------------------------------------
void CTFPasstimeLogic::staticc( inputdata_t &inputdata )
{
}
