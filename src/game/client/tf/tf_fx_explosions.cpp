//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific explosion effects
//
//=============================================================================//
#include "cbase.h"
#include "c_te_effect_dispatch.h"
#include "tempent.h"
#include "c_te_legacytempents.h"
#include "tf_shareddefs.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_parse.h"
#include "c_basetempentity.h"
#include "tier0/vprof.h"
#include "econ_item_system.h"
#include "tf_gamerules.h"
#include "c_tf_playerresource.h"
#include "tf_gamerules.h"

#include "ragdollexplosionenumerator.h"
#include "dlight.h"
#include "iefx.h"
extern ConVar lfe_muzzlelight; 

//--------------------------------------------------------------------------------------------------------------
extern CTFWeaponInfo *GetTFWeaponInfo( int iWeapon );
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFExplosionCallback( const Vector &vecOrigin, const Vector &vecNormal, int iWeaponID, ClientEntityHandle_t hEntity, int iItemID, int nSoundIndex, int nParticleSystemIndex )
{
	// Get the weapon information.
	CTFWeaponInfo *pWeaponInfo = NULL;
	switch ( iWeaponID )
	{
	case TF_WEAPON_GRENADE_PIPEBOMB:
	case TF_WEAPON_GRENADE_DEMOMAN:
		pWeaponInfo = GetTFWeaponInfo( TF_WEAPON_PIPEBOMBLAUNCHER );
		break;

	case TF_WEAPON_GRENADE_MIRVBOMB:
		//pWeaponInfo = GetTFWeaponInfo( TF_WEAPON_GRENADE_MIRV );
		pWeaponInfo = GetTFWeaponInfo( TF_WEAPON_PIPEBOMBLAUNCHER );
		break;

	case TF_WEAPON_SENTRY_ROCKET:
	case TF_WEAPON_PUMPKIN_BOMB:
		pWeaponInfo = GetTFWeaponInfo( TF_WEAPON_ROCKETLAUNCHER );
		break;

	case TF_WEAPON_GRENADE_NORMAL:
	case TF_WEAPON_GRENADE_CONCUSSION:
	case TF_WEAPON_GRENADE_NAIL:
	case TF_WEAPON_GRENADE_MIRV:
	case TF_WEAPON_GRENADE_MIRV_DEMOMAN:
	case TF_WEAPON_GRENADE_NAPALM:
	case TF_WEAPON_GRENADE_GAS:
	case TF_WEAPON_GRENADE_EMP:
	case TF_WEAPON_GRENADE_CALTROP:
	case TF_WEAPON_GRENADE_SMOKE_BOMB:
	case TF_WEAPON_GRENADE_HEAL:
		pWeaponInfo = GetTFWeaponInfo( TF_WEAPON_PIPEBOMBLAUNCHER );
		break;

	default:
		pWeaponInfo = GetTFWeaponInfo( iWeaponID );
		break;
	}

	bool bIsPlayer = false;
	if ( hEntity.Get() )
	{
		C_BaseEntity *pEntity = C_BaseEntity::Instance( hEntity );
		if ( pEntity && pEntity->IsPlayer() )
		{
			bIsPlayer = true;
		}
	}

	// Calculate the angles, given the normal.
	bool bIsWater = ( UTIL_PointContents( vecOrigin ) & CONTENTS_WATER );
	bool bInAir = false;
	QAngle angExplosion( 0.0f, 0.0f, 0.0f );

	// Cannot use zeros here because we are sending the normal at a smaller bit size.
	if ( fabs( vecNormal.x ) < 0.05f && fabs( vecNormal.y ) < 0.05f && fabs( vecNormal.z ) < 0.05f )
	{
		bInAir = true;
		angExplosion.Init();
	}
	else
	{
		VectorAngles( vecNormal, angExplosion );
		bInAir = false;
	}

	// Base explosion effect and sound.
	const char *pszFormat = (nParticleSystemIndex == -1) ? "ExplosionCore_wall" : GetParticleSystemNameFromIndex( nParticleSystemIndex );
	const char *pszSound = "BaseExplosionEffect.Sound";

	if ( TFGameRules()->IsTFCAllowed() )
		pszSound = "BaseExplosionEffect_TFC.Sound";

	if ( pWeaponInfo )
	{
		if( nParticleSystemIndex == -1 )
		{
			// Explosions.
			if ( bIsWater )
			{
				if ( Q_strlen( pWeaponInfo->m_szExplosionWaterEffect ) > 0 )
				{
					pszFormat = pWeaponInfo->m_szExplosionWaterEffect;
				}
			}
			else
			{
				if ( bIsPlayer || bInAir )
				{
					if ( Q_strlen( pWeaponInfo->m_szExplosionPlayerEffect ) > 0 )
					{
						pszFormat = pWeaponInfo->m_szExplosionPlayerEffect;
					}
				}
				else
				{
					if ( Q_strlen( pWeaponInfo->m_szExplosionEffect ) > 0 )
					{
						pszFormat = pWeaponInfo->m_szExplosionEffect;
					}
				}
			}
		}

		// Sound.
		if ( Q_strlen( pWeaponInfo->m_szExplosionSound ) > 0 )
		{
			pszSound = pWeaponInfo->m_szExplosionSound;
		}
	}

	// Allow schema to override explosion sound.
	if ( iItemID >= 0 )
	{
		CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( iItemID );
		if ( pItemDef && pItemDef->GetVisuals()->aWeaponSounds[nSoundIndex][0] != '\0' )
		{
			pszSound = pItemDef->GetVisuals()->aWeaponSounds[nSoundIndex];
		}
	}

	if ( iWeaponID == TF_WEAPON_PUMPKIN_BOMB )
		pszSound = "Halloween.PumpkinExplode";

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, pszSound, &vecOrigin );

	const char *pszEffect = pszFormat;

	if ( iWeaponID != TF_WEAPON_JAR && iWeaponID != TF_WEAPON_JAR_MILK )
	{
		if ( lfe_muzzlelight.GetBool() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC );
			dl->origin = vecOrigin;

			dl->color.r = 255;
			dl->color.g = 220;
			dl->color.b = 128;
			dl->radius	= 340.f;

			dl->decay	= 200;
			dl->die		= gpGlobals->curtime + 0.1f;
		}

		DispatchParticleEffect( pszEffect, vecOrigin, angExplosion );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_TETFExplosion : public C_BaseTempEntity
{
public:

	DECLARE_CLASS( C_TETFExplosion, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	C_TETFExplosion( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:

	Vector			m_vecOrigin;
	Vector			m_vecNormal;
	int				m_iWeaponID;
	int				m_iItemID;
	int				m_nSound;
	int				m_nParticleIndex;
	ClientEntityHandle_t m_hEntity;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TETFExplosion::C_TETFExplosion( void )
{
	m_vecOrigin.Init();
	m_vecNormal.Init();
	m_iWeaponID = TF_WEAPON_NONE;
	m_iItemID = -1;
	m_hEntity = INVALID_EHANDLE_INDEX;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TETFExplosion::PostDataUpdate( DataUpdateType_t updateType )
{
	VPROF( "C_TETFExplosion::PostDataUpdate" );

	CRagdollExplosionEnumerator	ragdollEnum( m_vecOrigin, 125, 300 );
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_RESPONSIVE_EDICTS, m_vecOrigin, 125, false, &ragdollEnum );

	TFExplosionCallback( m_vecOrigin, m_vecNormal, m_iWeaponID, m_hEntity, m_iItemID, m_nSound, m_nParticleIndex );
}

static void RecvProxy_ExplosionEntIndex( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	int nEntIndex = pData->m_Value.m_Int;
	((C_TETFExplosion*)pStruct)->m_hEntity = (nEntIndex < 0) ? INVALID_EHANDLE_INDEX : ClientEntityList().EntIndexToHandle( nEntIndex );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT( C_TETFExplosion, DT_TETFExplosion, CTETFExplosion )
	RecvPropFloat( RECVINFO( m_vecOrigin[0] ) ),
	RecvPropFloat( RECVINFO( m_vecOrigin[1] ) ),
	RecvPropFloat( RECVINFO( m_vecOrigin[2] ) ),
	RecvPropVector( RECVINFO( m_vecNormal ) ),
	RecvPropInt( RECVINFO( m_iWeaponID ) ),
	RecvPropInt( RECVINFO( m_iItemID ) ),
	RecvPropInt( RECVINFO( m_nSound ) ),
	RecvPropInt( RECVINFO( m_nParticleIndex ) ),
	RecvPropInt( "entindex", 0, SIZEOF_IGNORE, 0, RecvProxy_ExplosionEntIndex ),
END_RECV_TABLE()

