//========= Copyright © Stacker/Nullen, All rights reserved. ============//
//
// Purpose: Human type soldier
//			TODO: 
//				Make them go back to following the player after combat if player made him follow before initiating combat
//				Give citizen medics the ability to heal them
//				Fix bug where npc_maker'd conscripts appear as ERROR signs if sk_conscript_model is set to "Random",
//				Current workaround: Just use template npc_conscripts and npc_template_makers
//				Implement gender-specific voice lines/sentences
//
//=============================================================================//
#ifndef	NPC_CONSCRIPT_H
#define	NPC_CONSCRIPT_H
#pragma once

#include "npc_talker.h"
#include "ai_basenpc.h"
#include "ai_basehumanoid.h"
#include "ai_behavior.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_functank.h"
#include "ai_behavior_rappel.h"
#include "ai_behavior_actbusy.h"
#include "ai_sentence.h"
#include "ai_baseactor.h"

//=========================================================
//	>> CNPC_Conscript
//=========================================================
class CNPC_Conscript : public CNPCSimpleTalker
{
	DECLARE_CLASS( CNPC_Conscript, CNPCSimpleTalker );

public:
	void			Spawn( void );
	void			Precache( void );
	float			MaxYawSpeed( void );
	int				GetSoundInterests( void );
	void			ConscriptFirePistol( void );
	void			AlertSound( void );
	Class_T			Classify ( void );
	void			HandleAnimEvent( animevent_t *pEvent );
	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void			RunTask( const Task_t *pTask );
	int				ObjectCaps( void ) { return UsableNPCObjectCaps( BaseClass::ObjectCaps() ); }
	int				OnTakeDamage_Alive( const CTakeDamageInfo &info );
	Vector			BodyTarget( const Vector &posSrc, bool bNoisy = true );

	Activity		GetFollowActivity( float flDistance ) { return ACT_RUN; }

	void			DeclineFollowing( void );

	Activity		NPC_TranslateActivity( Activity eNewActivity );
	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );

	EHANDLE m_hPhysicsEnt;

	// Override these to set behavior
	virtual int		TranslateSchedule( int scheduleType );
	virtual int		SelectSchedule( void );

	void			DeathSound( void );
	void			PainSound( void );
	
	void			TalkInit( void );

	bool			CreateBehaviors();

	void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );

	bool			m_fGunDrawn;
	float			m_painTime;
	float			m_checkAttackTime;
	float			m_nextLineFireTime;
	bool			m_lastAttackCheck;
	bool			m_bInBarnacleMouth;
	bool			m_bIsFemale;
	int				m_iPersonality;

	CAI_ActBusyBehavior			m_ActBusyBehavior;

	bool FindNearestPhysicsObject( int iMaxMass );
	virtual bool CanSwatPhysicsObjects( void ) { return true; }
	float DistToPhysicsEnt( void );
	int GetSwatActivity( void );

	//=========================================================
	// Conscript Tasks
	//=========================================================
	enum 
	{
		TASK_CONSCRIPT_CROUCH = BaseClass::NEXT_TASK,
	};

	//=========================================================
	// Conscript schedules
	//=========================================================
	enum
	{
		SCHED_CONSCRIPT_FOLLOW = BaseClass::NEXT_SCHEDULE,
		SCHED_CONSCRIPT_DRAW,
		SCHED_CONSCRIPT_FACE_TARGET,
		SCHED_CONSCRIPT_STAND,
		SCHED_CONSCRIPT_AIM,
		SCHED_CONSCRIPT_FIRE_RPG,
		SCHED_CONSCRIPT_ESTABLISH_RPG_LINE_OF_FIRE,
		SCHED_CONSCRIPT_SUPPRESSINGFIRE,
		SCHED_CONSCRIPT_BARNACLE_HIT,
		SCHED_CONSCRIPT_BARNACLE_PULL,
		SCHED_CONSCRIPT_BARNACLE_CHOMP,
		SCHED_CONSCRIPT_BARNACLE_CHEW,
		SCHED_CONSCRIPT_CAUTIOUS_TAKECOVER,
	};


public:
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};

#endif	//NPC_CONSCRIPT_H