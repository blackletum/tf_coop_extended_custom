//=============================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef BASECOMBATCHARACTER_SHARED_H
#define BASECOMBATCHARACTER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

#ifdef CLIENT_DLL
class C_BaseCombatCharacter;
#else
class CBaseCombatCharacter;
#endif

#ifdef CLIENT_DLL
	EXTERN_RECV_TABLE( DT_BaseCombatCharacterShared );
// Server specific.
#else
	EXTERN_SEND_TABLE( DT_BaseCombatCharacterShared );
#endif

class CBaseCombatCharacterShared
{
public:

#ifdef CLIENT_DLL
	friend class C_BaseCombatCharacter;
	typedef C_BaseCombatCharacter OuterClass;
	DECLARE_PREDICTABLE();
#else
	friend class CBaseCombatCharacter;
	typedef CBaseCombatCharacter OuterClass;
#endif

	DECLARE_EMBEDDED_NETWORKVAR()
	DECLARE_CLASS_NOBASE( CBaseCombatCharacterShared );

	CBaseCombatCharacterShared();
	//~CBaseCombatCharacterShared();

	void	Init( OuterClass *pOuter );

private:

	OuterClass *m_pOuter;
};			   

#endif // AI_BASENPC_SHARED_H
