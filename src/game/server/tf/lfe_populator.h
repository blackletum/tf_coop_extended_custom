//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: The system for handling npc population in horde.
//
//=============================================================================//
#ifndef LFE_POPULATOR_H
#define LFE_POPULATOR_H
#ifdef _WIN32
#pragma once
#endif

#include "filesystem.h"
#include <KeyValues.h>
#include "gamestats.h"
#include "fmtstr.h"

class IPopulator;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

class IPopulationSpawner
{
public:
	IPopulationSpawner( IPopulator *populator );
	virtual ~IPopulationSpawner();
	
	virtual bool Parse( KeyValues *kv ) = 0;
	virtual int Spawn( const Vector& where, CUtlVector<CHandle<CBaseEntity>> *ents ) = 0;
	virtual bool IsWhereRequired();
	virtual bool IsVarious();
	virtual string_t GetClassName( int index );
	virtual int GetHealth( int index );
	virtual bool IsMiniBoss( int index );
	//virtual bool HasAttribute(CTFBot::AttributeType attr, int index);
	//virtual bool HasEventChangeAttributes(const char *name) const = 0;

	static IPopulationSpawner *ParseSpawner( IPopulator *populator, KeyValues *kv );

protected:
	IPopulator *m_Populator;
};

class CTFNPCSpawner : public IPopulationSpawner
{
public:
	CTFNPCSpawner( IPopulator *populator );
	virtual ~CTFNPCSpawner();

	virtual bool Parse( KeyValues *kv ) override;
	virtual int Spawn( const Vector& where, CUtlVector<CHandle<CBaseEntity>> *ents ) override;
	virtual string_t GetClassName( int index ) override;
	virtual int GetHealth( int index ) override;
	virtual bool IsMiniBoss( int index ) override;
	//virtual bool HasAttribute( CTFBot::AttributeType attr, int index ) override;
	//virtual bool HasEventChangeAttributes( const char *name ) const override;

private:
	//bool ParseEventChangeAttributes( KeyValues *kv );

	int m_iClass;
	string_t m_strClassName;
	int m_iHealth;
	float m_flScale;
	CUtlString m_strName;
	CUtlStringList m_TeleportWhere;
	//CTFBot::EventChangeAttributes_t m_DefaultAttrs;
	//CUtlVector<CTFBot::EventChangeAttributes_t> m_ECAttrs;
};

//bool ParseDynamicAttributes(CTFBot::EventChangeAttributes_t& ecattr, KeyValues *kv);

#endif // LFE_POPULATOR_H
