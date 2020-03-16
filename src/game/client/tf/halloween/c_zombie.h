#ifndef C_TF_ZOMBIE_H
#define C_TF_ZOMBIE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_ai_basenpc.h"

class C_TFZombie : public C_AI_BaseNPC
{
	DECLARE_CLASS( C_TFZombie, C_AI_BaseNPC )
public:
	C_TFZombie();
	virtual ~C_TFZombie();
	
	DECLARE_CLIENTCLASS();
	
	virtual void Spawn( void );
	virtual void ClientThink( void );
	virtual void FireEvent(const Vector& origin, const QAngle& angle, int event, const char* options);

	virtual void BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed );

	virtual bool ShouldCollide( int contentsMask, int collisionGroup ) const override;

	int m_nSkeletonType;
	int m_nZombieClass;

	C_BaseAnimating	*m_pCosmetic;

private:
	C_TFZombie( const C_TFZombie& );
};

#endif