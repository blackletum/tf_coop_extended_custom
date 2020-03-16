//=============================================================================//
//
//
//
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "fx.h"
#include "decals.h"
#include "iefx.h"
#include "engine/IEngineSound.h"
#include "materialsystem/imaterialvar.h"
#include "IEffects.h"
#include "engine/IEngineTrace.h"
#include "vphysics/constraints.h"
#include "engine/ivmodelinfo.h"
#include "tempent.h"
#include "c_te_legacytempents.h"
#include "engine/ivdebugoverlay.h"
#include "c_te_effect_dispatch.h"
#include "c_tf_player.h"
#include "tf_projectile_arrow.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IPhysicsSurfaceProps *physprops;
IPhysicsObject *GetWorldPhysObject( void );

extern ITempEnts* tempents;

void CreateCrossbowBoltTF( const Vector &vecOrigin, const Vector &vecDirection, int iType, byte nSkin )
{
	model_t *pModel = (model_t *)engine->LoadModel( g_pszArrowModels[iType] );

	QAngle vAngles;

	VectorAngles( vecDirection, vAngles );

	C_LocalTempEntity *pTemp = tempents->SpawnTempModel( pModel, vecOrigin - vecDirection * 5.f, vAngles, vec3_origin, 15.0f, FTENT_NONE );
	if ( pTemp )
	{
		pTemp->SetModelScale( 1.0f );
		pTemp->m_nSkin = nSkin;
	}
}

void StickRagdollNowTF( const Vector &vecOrigin, const Vector &vecDirection, ClientEntityHandle_t const &pEntity, int iBone, int iPhysicsBone, int iOwner, int iHitGroup, int iVictim, int iProjType, byte nSkin )
{
	trace_t tr;
	Ray_t ray;
	ray.Init( vecOrigin, vecOrigin + vecDirection * 16.0f );
	UTIL_Portal_TraceRay( ray, MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &tr );

	if ( tr.surface.flags & SURF_SKY )
		return;

	C_BaseAnimating *pModel = dynamic_cast<C_BaseAnimating *>( ClientEntityList().GetBaseEntityFromHandle( pEntity ) );
	if ( pModel && pModel->m_pRagdoll )
	{
		IPhysicsObject *pPhysics = NULL;
		if ( iPhysicsBone >= 0 )
		{
			ragdoll_t *pRagdoll = pModel->m_pRagdoll->GetRagdoll();

			if ( iPhysicsBone < pRagdoll->listCount )
			{
				pPhysics = pRagdoll->list[iPhysicsBone].pObject;
			}
		}

		if ( GetWorldPhysObject() && pPhysics )
		{
			Vector vecPhyOrigin; QAngle vecPhyAngle;
			pPhysics->GetPosition( &vecPhyOrigin, &vecPhyAngle );

			vecPhyOrigin = vecOrigin + ( vecDirection * ( rand() / VALVE_RAND_MAX ) ) * 7.f + vecDirection * 7.f;
			pPhysics->SetPosition( vecPhyOrigin, vecPhyAngle, true );

			pPhysics->EnableMotion( false );
		}
	}

	UTIL_ImpactTrace( &tr, DMG_GENERIC );
	CreateCrossbowBoltTF( vecOrigin, vecDirection, iProjType, nSkin );

	// Notify achievements
	if ( iHitGroup == HITGROUP_HEAD )
	{
		C_TFPlayer *pPlayer = CTFPlayer::GetLocalTFPlayer();
		if ( pPlayer && pPlayer->entindex() == iOwner )
		{
			C_TFPlayer *pVictim = ToTFPlayer( UTIL_PlayerByIndex( iVictim ) );
			if ( pVictim && pVictim->IsPlayerClass( TF_CLASS_HEAVYWEAPONS ) )
			{
				IGameEvent *event = gameeventmanager->CreateEvent( "player_pinned" );
				if ( event )
					gameeventmanager->FireEventClientSide( event );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void StickyBoltCallbackTF( const CEffectData &data )
{
	StickRagdollNowTF( data.m_vOrigin, data.m_vNormal, data.m_hEntity, 0, data.m_nMaterial, data.m_nHitBox, data.m_nDamageType, data.m_nSurfaceProp, data.m_fFlags, data.m_nColor );
}

DECLARE_CLIENT_EFFECT( "TFBoltImpact", StickyBoltCallbackTF );