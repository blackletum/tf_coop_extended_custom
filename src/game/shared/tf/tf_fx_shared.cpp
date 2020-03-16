//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
//  
//
//=============================================================================
#include "cbase.h"
#include "tf_fx_shared.h"
#include "tf_weaponbase.h"
#include "takedamageinfo.h"
#include "basehlcombatweapon_shared.h"

// Client specific.
#ifdef CLIENT_DLL
#include "fx_impact.h"
#include "c_te_effect_dispatch.h"
#include "c_ai_basenpc.h"
// Server specific.
#else
#include "tf_fx.h"
#include "ilagcompensationmanager.h"
#include "te_effect_dispatch.h"
#include "ai_basenpc.h"
#endif

ConVar tf_use_fixed_weaponspreads( "tf_use_fixed_weaponspreads", "0", FCVAR_NOTIFY|FCVAR_REPLICATED, "If set to 1, weapons that fire multiple pellets per shot will use a non-random pellet distribution." );

ConVar lfe_hl2_weapon_use_fixed_weaponspreads( "lfe_hl2_weapon_use_fixed_weaponspreads", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "If set to 1, weapons that fire multiple pellets per shot will use a non-random pellet distribution." );

// Client specific.
#ifdef CLIENT_DLL

class CGroupedSound
{
public:
	string_t	m_SoundName;
	Vector		m_vecPos;
};

CUtlVector<CGroupedSound> g_aGroupedSounds;

//-----------------------------------------------------------------------------
// Purpose: Called by the ImpactSound function.
//-----------------------------------------------------------------------------
void ImpactSoundGroup( const char *pSoundName, const Vector &vecEndPos )
{
	int iSound = 0;

	// Don't play the sound if it's too close to another impact sound.
	for ( iSound = 0; iSound < g_aGroupedSounds.Count(); ++iSound )
	{
		CGroupedSound *pSound = &g_aGroupedSounds[iSound];
		if ( pSound )
		{
			if ( vecEndPos.DistToSqr( pSound->m_vecPos ) < ( 300.0f * 300.0f ) )
			{
				if ( Q_stricmp( pSound->m_SoundName, pSoundName ) == 0 )
					return;
			}
		}
	}

	// Ok, play the sound and add it to the list.
	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, NULL, pSoundName, &vecEndPos );

	iSound = g_aGroupedSounds.AddToTail();
	g_aGroupedSounds[iSound].m_SoundName = pSoundName;
	g_aGroupedSounds[iSound].m_vecPos = vecEndPos;
}

//-----------------------------------------------------------------------------
// Purpose: This is a cheap ripoff from CBaseCombatWeapon::WeaponSound().
//-----------------------------------------------------------------------------
void FX_WeaponSound( int iPlayer, WeaponSound_t soundType, const Vector &vecOrigin, CTFWeaponInfo *pWeaponInfo )
{
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *pShootSound = pWeaponInfo->aShootSounds[soundType]; 
	if ( !pShootSound || !pShootSound[0] )
		return;

	CBroadcastRecipientFilter filter; 
	if ( !te->CanPredict() )
		return;

	CBaseEntity::EmitSound( filter, iPlayer, pShootSound, &vecOrigin ); 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StartGroupingSounds()
{
	Assert( g_aGroupedSounds.Count() == 0 );
	SetImpactSoundRoute( ImpactSoundGroup );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void EndGroupingSounds()
{
	g_aGroupedSounds.Purge();
	SetImpactSoundRoute( NULL );
}

// Server specific.
#else

// Server doesn't play sounds.
void FX_WeaponSound ( int iPlayer, WeaponSound_t soundType, const Vector &vecOrigin, CTFWeaponInfo *pWeaponInfo ) {}
void StartGroupingSounds() {}
void EndGroupingSounds() {}

#endif

void FX_TFTracer( const char *pszTracerEffectName, const Vector &vecStart, const Vector &vecEnd, int iEntIndex, bool bWhiz )
{
	int iParticleIndex = GetParticleSystemIndex( pszTracerEffectName );

	CEffectData data;
	data.m_vStart = vecStart;
	data.m_vOrigin = vecEnd;
#ifdef CLIENT_DLL
	data.m_hEntity = ClientEntityList().EntIndexToHandle( iEntIndex );
#else
	data.m_nEntIndex = iEntIndex;
#endif
	data.m_nHitBox = iParticleIndex;

	if ( bWhiz )
	{
		data.m_fFlags |= TRACER_FLAG_WHIZ;
	}

	DispatchEffect( "TFParticleTracer", data );
}

//-----------------------------------------------------------------------------
// Purpose: This runs on both the client and the server.  On the server, it 
// only does the damage calculations.  On the client, it does all the effects.
//-----------------------------------------------------------------------------
void FX_FireBullets( int iPlayer, const Vector &vecOrigin, const QAngle &vecAngles,
					 int iWeapon, int iMode, int iSeed, float flSpread, float flDamage /* = -1.0f */, bool bCritical /* = false*/ )
{
	// Get the weapon information.
	CTFWeaponInfo *pWeaponInfo = GetTFWeaponInfo( iWeapon );
	if( !pWeaponInfo )
		return;

	bool bDoEffects = false;

#ifdef CLIENT_DLL
	C_TFPlayer *pPlayer = ToTFPlayer( ClientEntityList().GetBaseEntity( iPlayer ) );
#else
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( iPlayer ) );
#endif
	if ( !pPlayer )
		return;

// Client specific.
#ifdef CLIENT_DLL
	bDoEffects = true;

	// The minigun has custom sound & animation code to deal with its windup/down.
	if ( iWeapon != TF_WEAPON_MINIGUN )
	{
		// Fire the animation event.
		if ( !pPlayer->IsDormant() )
		{
			if ( iMode == TF_WEAPON_PRIMARY_MODE )
			{
				pPlayer->m_PlayerAnimState->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
			}
			else
			{
				pPlayer->m_PlayerAnimState->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_SECONDARY );
			}
		}

		//FX_WeaponSound( pPlayer->entindex(), SINGLE, vecOrigin, pWeaponInfo );
	}

// Server specific.
#else
	// If this is server code, send the effect over to client as temp entity and 
	// dispatch one message for all the bullet impacts and sounds.
	TE_FireBullets( pPlayer->entindex(), vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, bCritical );

	// Let the player remember the usercmd he fired a weapon on. Assists in making decisions about lag compensation.
	pPlayer->NoteWeaponFired();

#endif

	// Fire bullets, calculate impacts & effects.
	StartGroupingSounds();

#if !defined (CLIENT_DLL)
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, LAG_COMPENSATE_BOUNDS );
#endif

	// Get the shooting angles.
	Vector vecShootForward, vecShootRight, vecShootUp;
	AngleVectors( vecAngles, &vecShootForward, &vecShootRight, &vecShootUp );

	// Initialize the static firing information.
	FireBulletsInfo_t fireInfo;
	fireInfo.m_vecSrc = vecOrigin;
	if ( flDamage < 0.0f )
	{
		fireInfo.m_flDamage = pWeaponInfo->GetWeaponData( iMode ).m_nDamage;
	}
	else
	{
		fireInfo.m_flDamage = static_cast<int>(flDamage);
	}
	fireInfo.m_flDistance = pWeaponInfo->GetWeaponData( iMode ).m_flRange;
	fireInfo.m_iShots = 1;
	fireInfo.m_vecSpread.Init( flSpread, flSpread, 0.0f );
	fireInfo.m_iAmmoType = pWeaponInfo->iAmmoType;

	// Setup the bullet damage type & roll for crit.
	int	nDamageType	= DMG_GENERIC;
	int nCustomDamageType = TF_DMG_CUSTOM_NONE;
	bool bBuckshot = false;
	CTFWeaponBase *pWeapon = pPlayer->GetActiveTFWeapon();
	if ( pWeapon )
	{
		nDamageType	= pWeapon->GetDamageType();
		if ( pWeapon->IsCurrentAttackACrit() || bCritical )
		{
			nDamageType |= DMG_CRITICAL;
		}
		else if ( pWeapon->IsCurrentAttackAMiniCrit() )
		{
			nDamageType |= DMG_MINICRITICAL;
		}

		nCustomDamageType = pWeapon->GetCustomDamageType();
		bBuckshot = ( nDamageType & DMG_BUCKSHOT ) != 0;
	}

	if ( iWeapon != TF_WEAPON_MINIGUN )
	{
		fireInfo.m_iTracerFreq = 2;
	}

	// Reset multi-damage structures.
	ClearMultiDamage();

	int nBulletsPerShot = pWeaponInfo->GetWeaponData( iMode ).m_nBulletsPerShot;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nBulletsPerShot, mult_bullets_per_shot );
	
	int nFixedSpread = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nFixedSpread, fixed_shot_pattern );

	// Only shotguns should get fixed spread pattern.
	bool bFixedSpread = false;

	if ( bBuckshot && nBulletsPerShot > 1 )
	{
		bFixedSpread = tf_use_fixed_weaponspreads.GetBool();
		if ( nFixedSpread )
			bFixedSpread = true;
	}

	for ( int iBullet = 0; iBullet < nBulletsPerShot; ++iBullet )
	{
		// Initialize random system with this seed.
		RandomSeed( iSeed );	

		float x = 0.0f;
		float y = 0.0f;

		// Determine if the first bullet should be perfectly accurate.
		bool bPerfectAccuracy = false;

		if ( pWeapon && iBullet == 0 )
		{
			float flFireInterval = gpGlobals->curtime - pWeapon->GetLastFireTime();
			if ( nBulletsPerShot == 1 )
				bPerfectAccuracy = flFireInterval > 1.25f;
			else
				bPerfectAccuracy = flFireInterval > 0.25f;
		}

		// See if we're using pre-determined spread pattern.
		if ( bFixedSpread )
		{
			int iIndex = iBullet;
			while ( iIndex > 9 )
			{
				iIndex -= 10;
			}

			x = 0.5f * g_vecFixedWpnSpreadPellets[iIndex].x;
			y = 0.5f * g_vecFixedWpnSpreadPellets[iIndex].y;
		}

		// Apply random spread if none of the above conditions are true.
		if ( !bPerfectAccuracy && !bFixedSpread )
		{
			// Get circular gaussian spread.
			x = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
			y = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
		}

		// Initialize the varialbe firing information.
		fireInfo.m_vecDirShooting = vecShootForward + ( x *  flSpread * vecShootRight ) + ( y * flSpread * vecShootUp );
		fireInfo.m_vecDirShooting.NormalizeInPlace();

		// Fire a bullet.
		pPlayer->FireBullet( fireInfo, bDoEffects, nDamageType, nCustomDamageType );

		// Use new seed for next bullet.
		++iSeed; 
	}

	// Apply damage if any.
	ApplyMultiDamage();

#if !defined (CLIENT_DLL)
	lagcompensation->FinishLagCompensation( pPlayer );
#endif

	EndGroupingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: This runs on both the client and the server.  On the server, it 
// only does the damage calculations.  On the client, it does all the effects.
//-----------------------------------------------------------------------------
void FX_NPCFireBullets( int iNPC, const Vector &vecOrigin, const Vector &vecAngles,
					 int iWeapon, int iMode, int iSeed, float flSpread, int iShot, float flDamage /* = -1.0f */, bool bCritical /* = false*/ )
{
	bool bDoEffects = false;

#ifdef CLIENT_DLL
	C_AI_BaseNPC *pNPC = dynamic_cast<C_AI_BaseNPC *>( ClientEntityList().GetBaseEntity( iNPC ) );
#else
	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( UTIL_EntityByIndex( iNPC ) );
#endif
	if ( !pNPC )
		return;

// Client specific.
#ifdef CLIENT_DLL
	bDoEffects = true;

	//FX_WeaponSound( pNPC->entindex(), SINGLE, vecOrigin, pWeaponInfo );
// Server specific.
#else
	QAngle angShootDir;
	VectorAngles( vecAngles, angShootDir );

	// If this is server code, send the effect over to client as temp entity and 
	// dispatch one message for all the bullet impacts and sounds.
	TE_FireBullets( pNPC->entindex(), vecOrigin, angShootDir, iWeapon, iMode, iSeed, flSpread, bCritical );
#endif

	CBaseHLCombatWeapon *pWeapon = dynamic_cast<CBaseHLCombatWeapon *>( pNPC->GetActiveWeapon() );

	// Fire bullets, calculate impacts & effects.
	StartGroupingSounds();

	// Initialize the static firing information.
	FireBulletsInfo_t fireInfo;
	fireInfo.m_vecSrc = vecOrigin;
	if ( flDamage < 0.0f )
		fireInfo.m_flDamage = 3;
	else
		fireInfo.m_flDamage = static_cast<int>(flDamage);

	fireInfo.m_flDistance = MAX_TRACE_LENGTH;
	fireInfo.m_iShots = iShot;
	//fireInfo.m_vecSpread.Init( flSpread, flSpread, 0.0f );
	fireInfo.m_vecSpread = vec3_origin;
	fireInfo.m_iAmmoType = pWeapon->m_iPrimaryAmmoType;

	// Setup the bullet damage type & roll for crit.
	int	nDamageType	= DMG_GENERIC;
	int nCustomDamageType = TF_DMG_CUSTOM_NONE;
	bool bBuckshot = false;
	if ( pWeapon )
	{
		nDamageType	= pWeapon->GetDamageType();
		if ( pWeapon->IsCurrentAttackACrit() || bCritical )
		{
			nDamageType |= DMG_CRITICAL;
		}
		else if ( pWeapon->IsCurrentAttackAMiniCrit() )
		{
			nDamageType |= DMG_MINICRITICAL;
		}

		nCustomDamageType = pWeapon->GetCustomDamageType();
		bBuckshot = ( nDamageType & DMG_BUCKSHOT ) != 0;
	}

	fireInfo.m_iTracerFreq = 2;

	// Reset multi-damage structures.
	ClearMultiDamage();

	int nBulletsPerShot = 1;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nBulletsPerShot, mult_bullets_per_shot );
	
	int nFixedSpread = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nFixedSpread, fixed_shot_pattern );

	// Only shotguns should get fixed spread pattern.
	bool bFixedSpread = false;

	if ( bBuckshot && nBulletsPerShot > 1 )
	{
		bFixedSpread = lfe_hl2_weapon_use_fixed_weaponspreads.GetBool();
		if ( nFixedSpread )
			bFixedSpread = true;
	}

	for ( int iBullet = 0; iBullet < nBulletsPerShot; ++iBullet )
	{
		// Initialize random system with this seed.
		RandomSeed( iSeed );	

		float x = 0.0f;
		float y = 0.0f;

		// Determine if the first bullet should be perfectly accurate.
		bool bPerfectAccuracy = false;

		if ( pWeapon && iBullet == 0 )
		{
			float flFireInterval = gpGlobals->curtime - pWeapon->GetLastFireTime();
			if ( nBulletsPerShot == 1 )
				bPerfectAccuracy = flFireInterval > 1.25f;
			else
				bPerfectAccuracy = flFireInterval > 0.25f;
		}

		// See if we're using pre-determined spread pattern.
		if ( bFixedSpread )
		{
			int iIndex = iBullet;
			while ( iIndex > 9 )
			{
				iIndex -= 10;
			}

			x = 0.5f * g_vecFixedWpnSpreadPellets[iIndex].x;
			y = 0.5f * g_vecFixedWpnSpreadPellets[iIndex].y;
		}

		// Apply random spread if none of the above conditions are true.
		if ( !bPerfectAccuracy && !bFixedSpread )
		{
			// Get circular gaussian spread.
			x = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
			y = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
		}

		// Initialize the varialbe firing information.
		fireInfo.m_vecDirShooting = vecAngles;

		// Fire a bullet.
		pNPC->FireBullet( fireInfo, bDoEffects, nDamageType, nCustomDamageType );

		// Use new seed for next bullet.
		++iSeed; 
	}

	// Apply damage if any.
	ApplyMultiDamage();

	EndGroupingSounds();
}
