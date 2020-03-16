//====== Copyright © 1996-2019, Valve Corporation, All rights reserved. =======//
//
// Purpose: Gun Mettle dropped weapon
//
//=============================================================================//

#include "cbase.h"
#include "tf_dropped_weapon.h"
#include "tf_gamerules.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( TFDroppedWeapon, DT_TFDroppedWeapon )

BEGIN_NETWORK_TABLE( CTFDroppedWeapon, DT_TFDroppedWeapon )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iAmmo ) ),
	RecvPropInt( RECVINFO( m_iMaxAmmo ) ),
#else
	SendPropInt( SENDINFO( m_iAmmo ), 10, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iMaxAmmo ), 10, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_dropped_weapon, CTFDroppedWeapon );

IMPLEMENT_AUTO_LIST( IDroppedWeaponAutoList );

CTFDroppedWeapon::CTFDroppedWeapon()
{
#ifdef CLIENT_DLL
	m_pGlowEffect = NULL;
	m_bShouldGlow = false;
#endif
	m_iMaxAmmo = 0;
}

CTFDroppedWeapon::~CTFDroppedWeapon()
{
#ifdef CLIENT_DLL
	if ( m_pPilotLightSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pPilotLightSound );
		m_pPilotLightSound = NULL;
	}

	delete m_pGlowEffect;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function 
//-----------------------------------------------------------------------------
void CTFDroppedWeapon::Spawn( void )
{
	SetModel( STRING( GetModelName() ) );

	BaseClass::Spawn();

	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolid( SOLID_BBOX );
	//SetBlocksLOS( false );

	VPhysicsInitNormal( SOLID_VPHYSICS, FSOLID_TRIGGER, false );

	SetCollisionGroup( COLLISION_GROUP_WEAPON );

	if ( VPhysicsGetObject() )
	{
		// All weapons must have same weight.
		VPhysicsGetObject()->SetMass( 25.0f );
	}

#ifdef GAME_DLL
	AddSpawnFlags( SF_NORESPAWN );
	m_flCreationTime = gpGlobals->curtime;

	// Remove 30s after spawning
	m_flRemoveTime = gpGlobals->curtime + 30.0f;
	SetThink( &CTFDroppedWeapon::RemovalThink );
	SetNextThink( gpGlobals->curtime );
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Start thinking
//-----------------------------------------------------------------------------
void C_TFDroppedWeapon::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		float flChangeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );
		Vector vecCurOrigin = GetLocalOrigin();

		// Now stick our initial velocity into the interpolation history 
		CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
		interpolator.ClearHistory();
		interpolator.AddToHead( flChangeTime - 0.15f, &vecCurOrigin, false );

		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

void C_TFDroppedWeapon::ClientThink()
{
	bool bShouldGlow = false;

	CEconItemDefinition *pStatic = m_Item.GetStaticData();

	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		if ( pStatic )
		{
			if ( pStatic->used_by_classes & ( 1 << pPlayer->GetPlayerClass()->GetClassIndex() ) )
				bShouldGlow = true;
			else
				bShouldGlow = false;
		}
		else
		{
			bShouldGlow = true;
		}
	}

	if ( m_bShouldGlow != bShouldGlow )
	{
		m_bShouldGlow = bShouldGlow;
		UpdateGlowEffect();
	}

	if ( pStatic && FStrEq( pStatic->item_class, "tf_weapon_flamethrower" ) )
	{
		if ( !m_pPilotLightSound )
		{
			// Create the looping pilot light sound
			const char *pilotlightsound = "Weapon_FlameThrower.PilotLoop";
			CLocalPlayerFilter filter;

			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			m_pPilotLightSound = controller.SoundCreate( filter, entindex(), pilotlightsound );

			controller.Play( m_pPilotLightSound, 1.0, 100 );
		}
	}
	else
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pPilotLightSound );
		m_pPilotLightSound = NULL;
	}

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

void C_TFDroppedWeapon::UpdateGlowEffect()
{
	if ( !m_pGlowEffect )
	{
		float flRed = RemapValClamped( m_iAmmo, m_iMaxAmmo / 2, m_iMaxAmmo, 0.75f, 0.15f );
		float flGreen = RemapValClamped( m_iAmmo, 0, m_iMaxAmmo / 2, 0.15f, 0.75f );
		float flBlue = 0.15f;

		if ( m_Item.GetStaticData() )
		{
			if ( ( m_Item.GetStaticData()->anim_slot == TF_WPN_TYPE_MELEE ) || ( m_Item.GetStaticData()->anim_slot == TF_WPN_TYPE_MELEE_ALLCLASS ) )
			{
				flRed = 0.0f;
				flGreen = 1.0f;
				flBlue = 0.0f;
			}
		}

		Vector vecColor = Vector( flRed, flGreen, flBlue );

		m_pGlowEffect = new CGlowObject( this, vecColor, 0.8f, false, true );
	}

	if ( m_bShouldGlow )
		m_pGlowEffect->SetAlpha( 0.8f );
	else
		m_pGlowEffect->SetAlpha( 0.0f );
}

const char *C_TFDroppedWeapon::GetWeaponName( void )
{
	return STRING( m_Item.GetStaticData()->item_name );
}

#else
void CTFDroppedWeapon::Precache( void )
{
	PrecacheScriptSound( "Player.PickupWeapon" );
}

void CTFDroppedWeapon::RemovalThink( void )
{
	if ( gpGlobals->curtime >= m_flRemoveTime )
		UTIL_Remove( this );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

CTFDroppedWeapon *CTFDroppedWeapon::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CTFWeaponBase *pWeapon )
{
	CTFDroppedWeapon *pDroppedWeapon = static_cast<CTFDroppedWeapon *>( CBaseAnimating::CreateNoSpawn( "tf_dropped_weapon", vecOrigin, vecAngles, pOwner ) );
	if ( pDroppedWeapon )
	{
		pDroppedWeapon->SetModel( pWeapon->GetWorldModel() );
		pDroppedWeapon->SetItem( pWeapon->GetItem() );
		pDroppedWeapon->m_nSkin = pWeapon->m_nSkin;

		DispatchSpawn( pDroppedWeapon );
	}

	return pDroppedWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDroppedWeapon::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	bool bSuccess = false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pActivator );
	if ( !pTFPlayer )
		return;

	int iSlot = m_Item.GetStaticData()->GetLoadoutSlot( pTFPlayer->GetDesiredPlayerClassIndex() );
	CTFWeaponBase *pWeapon = (CTFWeaponBase *)pTFPlayer->GetEntityForLoadoutSlot( iSlot );

	if ( pTFPlayer->CanPickupDroppedWeapon( this ) && gpGlobals->curtime - m_flCreationTime > 1.0f )
	{
		// Don't remove weapon while a player is standing over it.
		SetThink( NULL );

		CEconItemDefinition *pStatic = m_Item.GetStaticData();
		if ( pStatic )
		{
			if ( pStatic->used_by_classes & ( 1 << pTFPlayer->GetDesiredPlayerClassIndex() ) )
			{
				if ( pWeapon )
				{
					if ( pTFPlayer->ItemsMatch( pWeapon->GetItem(), &m_Item, pWeapon ) )
					{
						// Give however many ammo we have.
						if ( pTFPlayer->GiveAmmo( m_iAmmo, pWeapon->GetPrimaryAmmoType(), true, TF_AMMO_SOURCE_AMMOPACK ) )
							bSuccess = true;
					}
					else
					{
						// Drop a usable weapon.
						pTFPlayer->DropWeapon( pWeapon );

						pWeapon->UnEquip( pTFPlayer );
						pWeapon = NULL;
					}

					pTFPlayer->m_Shared.SetDesiredWeaponIndex( m_Item.GetItemDefIndex() );
				}

				if ( !pWeapon )
				{
					const char *pszWeaponName = m_Item.GetEntityName();
					CTFWeaponBase *pNewWeapon = (CTFWeaponBase *)pTFPlayer->GiveNamedItem( pszWeaponName, 0, &m_Item );
					if ( pNewWeapon )
					{
						pTFPlayer->SetAmmoCount( m_iAmmo, pNewWeapon->GetPrimaryAmmoType() );
						pNewWeapon->GiveTo( pTFPlayer );

						// If this is the same guy who dropped it restore old clip size to avoid exploiting swapping
						// weapons for faster reload.
						if ( pTFPlayer == GetOwnerEntity() )
						{
							pNewWeapon->m_iClip1 = m_iClip;
						}

						pTFPlayer->m_Shared.SetDesiredWeaponIndex( -1 );
						bSuccess = true;
					}
				}

				if ( bSuccess )
				{
					pTFPlayer->PickupObject( this, false );

					CSingleUserRecipientFilter user( pTFPlayer );
					user.MakeReliable();

					UserMessageBegin( user, "ItemPickup" );
						WRITE_STRING( GetClassname() );
					MessageEnd();

					pTFPlayer->EmitSound( "Player.PickupWeapon" );
					pTFPlayer->SpeakConceptIfAllowed( MP_CONCEPT_MVM_LOOT_COMMON );
					UTIL_Remove( this );
				}
			}
		}
	}
}

void CTFDroppedWeapon::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );

	CTFPlayer *pTFPlayer = dynamic_cast<CTFPlayer*>( pOther );
	if ( pTFPlayer )
	{
		pTFPlayer->m_Shared.SetDesiredWeaponIndex( -1 );
		SetThink( &CTFDroppedWeapon::RemovalThink );
		// Don't remove weapon immediately after player stopped touching it.
		SetNextThink( gpGlobals->curtime + 3.5f );
	}
}
#endif