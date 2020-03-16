#include "cbase.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#else
#include "tf_player.h"
#endif

#include "tf_gamerules.h"
#include "tf_wearable_demoshield.h"

extern ConVar tf_demoman_charge_drain_time;
extern ConVar tf_demoman_charge_regen_rate;

#ifdef GAME_DLL
extern ConVar tf_debug_damage;
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFWearableDemoShield, DT_TFWearableDemoShield );

BEGIN_NETWORK_TABLE( CTFWearableDemoShield, DT_TFWearableDemoShield )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_wearable_demoshield, CTFWearableDemoShield );
PRECACHE_REGISTER( tf_wearable_demoshield );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFWearableDemoShield::CTFWearableDemoShield()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFWearableDemoShield::~CTFWearableDemoShield()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWearableDemoShield::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "DemoCharge.HitWorld" );
	PrecacheScriptSound( "DemoCharge.HitFlesh" );
	PrecacheScriptSound( "DemoCharge.HitFleshRange" );
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWearableDemoShield::Equip( CBasePlayer *pPlayer )
{
	BaseClass::Equip( pPlayer );
	
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( pTFPlayer ) 
		pTFPlayer->m_Shared.SetDemoShieldEquipped( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWearableDemoShield::UnEquip( CBasePlayer *pPlayer )
{
	BaseClass::UnEquip( pPlayer );

	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( pTFPlayer ) 
		pTFPlayer->m_Shared.SetDemoShieldEquipped( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFWearableDemoShield::GetDamage( void )
{
	float flDamage = 50.0f;
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if (!pOwner) return flDamage;

	flDamage = ( 10.0f * Min( pOwner->m_Shared.GetDecapitationCount(), 5 ) ) + 50.0f;

	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pOwner, flDamage, charge_impact_damage );

	return flDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector CTFWearableDemoShield::GetDamageForce( void )
{
	Vector vecForce( 0.0f );
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if (!pOwner) return vecForce;

	vecForce = pOwner->GetAbsVelocity();
	Vector vecVel = pOwner->GetLocalVelocity();
	if (pOwner->VPhysicsGetObject())
	{
		pOwner->VPhysicsGetObject()->GetVelocity( &vecVel, nullptr );
		vecVel *= vecVel.Normalized();
	}

	vecForce *= vecVel;

	return vecForce;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWearableDemoShield::ShieldBash( CTFPlayer *pAttacker )
{
	if ( !pAttacker ) 
		return;

	if ( !m_bBashed )
	{
		m_bBashed = true;

		Vector vecDir, vecEnd;
		Vector vecStart = pAttacker->Weapon_ShootPosition();
		pAttacker->EyeVectors( &vecDir );
		vecEnd = vecStart + vecDir * 48.0;

		trace_t tr;
		CTraceFilterSimple filter( pAttacker, COLLISION_GROUP_NONE );
		UTIL_TraceHull( vecStart, vecEnd, Vector( 0, 0, 0 ), Vector( 24, 24, 24 ), MASK_SOLID_BRUSHONLY, &filter, &tr );

		if ( tf_debug_damage.GetBool() )
			NDebugOverlay::Line( vecStart, vecEnd, 0, 255, 0, true, 5.0f );

		Vector vecOrigin = pAttacker->WorldSpaceCenter() + pAttacker->BodyDirection2D() * 24;
		Vector mins = vecOrigin - Vector( 16, 16, 24 );
		Vector maxs = vecOrigin + Vector( 16, 16, 24 );

		CBaseEntity *pList[256];

		int count = UTIL_EntitiesInBox( pList, 256, mins, maxs, 0 );

		if ( tf_debug_damage.GetBool() )
			NDebugOverlay::Box( vecOrigin, -Vector( 16, 16, 24 ), Vector( 16, 16, 24 ), 0, 255, 0, 40, 5.0f );

		for ( int i = 0; i < count; i++ )
		{
			CBaseEntity *pEntity = pList[i];

			if ( pEntity == this || InSameTeam( pEntity ) || !FVisible( pEntity, MASK_SOLID ) )
				continue;

			//CTFPlayer *pVictim = ToTFPlayer( pEntity );
			//CAI_BaseNPC *pNPCVictim = dynamic_cast<CAI_BaseNPC *>( pEntity );
			//CBreakableProp *pProp = dynamic_cast< CBreakableProp * >( pEntity );

			Vector vecDamagePos = WorldSpaceCenter();
			vecDamagePos += ( pEntity->WorldSpaceCenter() - vecDamagePos ) * 0.75f;

			int iNoChargeImpactRange = CAttributeManager::AttribHookValue<int>( 0, "no_charge_impact_range", pAttacker );

			CTakeDamageInfo info;
			info.SetAttacker( pAttacker );
			info.SetInflictor( this );
			info.SetWeapon( this );
			info.SetDamage( GetDamage() );
			info.SetDamageForce( GetDamageForce() );
			info.SetDamagePosition( vecDamagePos );
			info.SetDamageCustom( TF_DMG_CUSTOM_CHARGE_IMPACT );
			info.SetDamageType( DMG_CLUB );

			if ( pEntity->IsPlayer() || pEntity->IsNPC() || pEntity->ClassMatches( "prop_physics" ) )
			{
				if ( ( iNoChargeImpactRange != 0 ) || pAttacker->m_Shared.GetShieldChargeMeter() < 40.0f )
				{
					pAttacker->EmitSound( "DemoCharge.HitFleshRange" );
					pEntity->TakeDamage( info );
					ApplyMultiDamage();
					pAttacker->m_Shared.CalcChargeCrit( true );
				}
				else
				{
					pAttacker->EmitSound( "DemoCharge.HitFlesh" );
				}
			}
		}

		if ( !tr.m_pEnt )
		{
			pAttacker->EmitSound( "DemoCharge.HitWorld" );
		}

		UTIL_ScreenShake( pAttacker->WorldSpaceCenter(), 25.0f, 150.0f, 1.0f, 750.0f, SHAKE_START );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFWearableDemoShield::DoSpecialAction( CTFPlayer *pUser )
{
	if ( !TFGameRules() || TFGameRules()->State_Get() != GR_STATE_PREROUND )
	{
		if ( !pUser->m_Shared.IsLoser() && !pUser->m_Shared.InCond( TF_COND_TAUNTING ) && !pUser->m_Shared.InCond( TF_COND_STUNNED ) && pUser->m_Shared.GetShieldChargeMeter() == 100.0f )
		{
			float flChargeDuration = tf_demoman_charge_drain_time.GetFloat();
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pUser, flChargeDuration, mod_charge_time );

			float flChargeRechargeRate = tf_demoman_charge_regen_rate.GetFloat();
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pUser, flChargeRechargeRate, charge_recharge_rate );

			pUser->m_Shared.SetShieldChargeDrainRate( flChargeDuration );
			pUser->m_Shared.SetShieldChargeRegenRate( flChargeRechargeRate );
			pUser->m_Shared.AddCond( TF_COND_SHIELD_CHARGE, flChargeDuration );

			m_bBashed = false;

			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFWearableDemoShield::EndSpecialAction( CTFPlayer *pUser )
{
	if (!CAttributeManager::AttribHookValue<int>( 0, "attack_not_cancel_charge", pUser ) && pUser->m_Shared.InCond( TF_COND_SHIELD_CHARGE ))
	{
		pUser->m_Shared.CalcChargeCrit( false );
		pUser->m_Shared.SetShieldChargeMeter( 0 );
		pUser->m_Shared.RemoveCond( TF_COND_SHIELD_CHARGE );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFWearableDemoShield *GetEquippedDemoShield( CTFPlayer *pPlayer )
{
	for ( int i=0; i<pPlayer->GetNumWearables(); ++i )
	{
		CTFWearableDemoShield *pShield = dynamic_cast<CTFWearableDemoShield *>( pPlayer->GetWearable( i ) );
		if ( pShield )
			return pShield;
	}

	return nullptr;
}