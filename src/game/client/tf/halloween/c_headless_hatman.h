#ifndef C_HEADLESS_HATMAN_H
#define C_HEADLESS_HATMAN_H
#ifdef _WIN32
#pragma once
#endif

#include "c_ai_basenpc.h"

class C_HeadlessHatman : public C_AI_BaseNPC
{
	DECLARE_CLASS( C_HeadlessHatman, C_AI_BaseNPC )
public:
	C_HeadlessHatman();
	virtual ~C_HeadlessHatman() { };
	
	DECLARE_CLIENTCLASS();
	
	virtual void Spawn( void );
	virtual void ClientThink( void );
	virtual void FireEvent(const Vector& origin, const QAngle& angle, int event, const char* options);

private:
	CNewParticleEffect *m_pGlow;
	CNewParticleEffect *m_pLeftEye;
	CNewParticleEffect *m_pRightEye;

	C_HeadlessHatman( const C_HeadlessHatman& );
};

#endif