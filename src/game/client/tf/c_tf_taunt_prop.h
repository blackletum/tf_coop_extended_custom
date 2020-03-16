//====== Copyright © 1996-2019, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef C_TF_TAUNT_PROP_H
#define C_TF_TAUNT_PROP_H
#ifdef _WIN32
#pragma once
#endif

#include "c_basecombatcharacter.h"

#define CTFTauntProp C_TFTauntProp

//-----------------------------------------------------------------------------
// Purpose: Rocket projectile.
//-----------------------------------------------------------------------------
class C_TFTauntProp : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_TFTauntProp, C_BaseCombatCharacter );
public:
	DECLARE_NETWORKCLASS();

	C_TFTauntProp();
	~C_TFTauntProp();

	//virtual void	UpdateOnRemove( void );

	virtual bool	StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, C_BaseEntity *pTarget );
	//virtual	bool	ClearSceneEvent( CSceneEventInfo *info, bool fastKill, bool canceled );
};

#endif // C_TF_TAUNT_PROP_H
