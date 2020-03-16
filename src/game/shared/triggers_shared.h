//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TRIGGERS_SHARED_H
#define TRIGGERS_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// Spawnflags
enum
{
	SF_TRIGGER_ALLOW_CLIENTS				= 0x01,		// Players can fire this trigger
	SF_TRIGGER_ALLOW_NPCS					= 0x02,		// NPCS can fire this trigger
	SF_TRIGGER_ALLOW_PUSHABLES				= 0x04,		// Pushables can fire this trigger
	SF_TRIGGER_ALLOW_PHYSICS				= 0x08,		// Physics objects can fire this trigger
	SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS		= 0x10,		// *if* NPCs can fire this trigger, this flag means only player allies do so
	SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES		= 0x20,		// *if* Players can fire this trigger, this flag means only players inside vehicles can 
	SF_TRIGGER_ALLOW_ALL					= 0x40,		// Everything can fire this trigger EXCEPT DEBRIS!
	SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES	= 0x200,	// *if* Players can fire this trigger, this flag means only players outside vehicles can 
	SF_TRIG_PUSH_ONCE						= 0x80,		// trigger_push removes itself after firing once
	SF_TRIG_PUSH_AFFECT_PLAYER_ON_LADDER	= 0x100,	// if pushed object is player on a ladder, then this disengages them from the ladder (HL2only)
	SF_TRIG_TOUCH_DEBRIS 					= 0x400,	// Will touch physics debris objects
	SF_TRIGGER_ONLY_NPCS_IN_VEHICLES		= 0X800,	// *if* NPCs can fire this trigger, only NPCs in vehicles do so (respects player ally flag too)
	SF_TRIGGER_DISALLOW_BOTS				= 0x1000,   // Bots are not allowed to fire this trigger
	SF_TRIGGER_ALLOW_ITEMS					= 0x2000,	// MOVETYPE_FLYGRAVITY (Weapons, items, flares, etc.) can fire this trigger
};

// Spawnflags for CTriggerPlayerMovement
const int SF_TRIGGER_MOVE_AUTODISABLE				= 0x80;		// disable auto movement
const int SF_TRIGGER_AUTO_DUCK						= 0x800;	// Duck automatically
const int SF_TRIGGER_AUTO_WALK						= 0x1000;
const int SF_TRIGGER_DISABLE_JUMP					= 0x2000;

// not sure where to put.
enum togglemovetypes_t
{
	MOVE_TOGGLE_NONE = 0,
	MOVE_TOGGLE_LINEAR = 1,
	MOVE_TOGGLE_ANGULAR = 2,
};

#endif // TRIGGERS_SHARED_H
