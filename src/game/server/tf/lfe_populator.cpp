//============== Copyright LFE-TEAM Not All rights reserved. =================//
//
// Purpose: The system for handling npc population in horde.
//
//=============================================================================//

#include "cbase.h"
#include "lfe_populator.h"
#include "lfe_population_manager.h"
#include "igamesystem.h"
#include "engine/IEngineSound.h"
#include "soundenvelope.h"
#include "utldict.h"
#include "ai_basenpc.h"
#include "tf_gamerules.h"
#include "nav_mesh/tf_nav_mesh.h"
#include "nav_mesh/tf_nav_area.h"
#include "tf_team.h"
#include "ai_navigator.h"
#include "ai_network.h"
#include "ai_node.h"
#include "eventqueue.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar tf_mvm_miniboss_scale("tf_mvm_miniboss_scale", "1.75", FCVAR_CHEAT | FCVAR_REPLICATED, "Full body scale for minibosses." );

extern ConVar lfe_horde_debug;

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
IPopulationSpawner::IPopulationSpawner( IPopulator *populator )
	: m_Populator( populator )
{
}

IPopulationSpawner::~IPopulationSpawner()
{
}

bool IPopulationSpawner::IsWhereRequired()
{
	return true;
}

bool IPopulationSpawner::IsVarious()
{
	return false;
}

string_t IPopulationSpawner::GetClassName( int index )
{
	return NULL_STRING;
}

int IPopulationSpawner::GetHealth( int index )
{
	return 0;
}

bool IPopulationSpawner::IsMiniBoss( int index )
{
	return false;
}

/*bool IPopulationSpawner::HasAttribute( CTFBot::AttributeType attr, int index )
{
	return false;
}*/

IPopulationSpawner *IPopulationSpawner::ParseSpawner( IPopulator *populator, KeyValues *kv )
{
	const char *name = kv->GetName();
	if ( strlen(name) <= 0 )
		return nullptr;

	IPopulationSpawner* spawner;
	if ( V_stricmp( name, "TFNPC" ) == 0 )
	{
		spawner = new CTFNPCSpawner( populator );
		if ( spawner->Parse( kv ) )
		{
			return spawner;
			delete spawner;
		}
		else
		{
			Warning("Warning reading TFNPC spawner definition\n");
		}
	}
	
	return nullptr;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CTFNPCSpawner::CTFNPCSpawner( IPopulator *populator ) : IPopulationSpawner( populator )
{
	m_strClassName  = NULL_STRING;
	m_iHealth       = -1;
	m_flScale       = -1.0f;

	//m_DefaultAttrs.Reset();
}

CTFNPCSpawner::~CTFNPCSpawner()
{
}

bool CTFNPCSpawner::Parse( KeyValues *kv )
{
	m_strClassName  = NULL_STRING;
	m_iHealth       = -1;
	m_flScale       = -1.0f;
	
	//m_DefaultAttrs.Reset();
	
	//m_ECAttrs.RemoveAll();

	/*KeyValues *kv_tref = kv->FindKey( "Template" );
	if ( kv_tref != nullptr )
	{
		const char *tname = kv_tref->GetString();
		
		KeyValues *kv_timpl = m_Populator->m_PopMgr->m_kvTemplates->FindKey( tname );
		if ( kv_timpl != nullptr )
		{
			if ( !this->Parse( kv_timpl ) )
				return false;
		}
		else
		{
			Warning( "Unknown Template '%s' in TFNPCSpawner definition\n", tname );
		}
	}*/
	
	FOR_EACH_SUBKEY( kv, subkey )
	{
		const char *name = subkey->GetName();
		if ( strlen( name ) <= 0 )
			continue;

		/*if ( V_stricmp( name, "Template" ) == 0 )
			continue;*/

		if ( V_stricmp( name, "Classname" ) == 0)  
		{
			m_strClassName = AllocPooledString( subkey->GetString() );
		}
		else if ( V_stricmp( name, "Health" ) == 0 )
		{
			m_iHealth = subkey->GetInt();
		}
		else if ( V_stricmp( name, "Scale" ) == 0 )
		{
			m_flScale = subkey->GetFloat();
		}
		else if ( V_stricmp( name, "Name" ) == 0 )
		{
			m_strName = subkey->GetString();
		}
		else if ( V_stricmp( name, "TeleportWhere" ) == 0 ) 
		{
			m_TeleportWhere.CopyAndAddToTail( subkey->GetString() );
		}
		/*else
		{
			if ( V_stricmp( name, "EventChangeAttributes" ) == 0 ) 
			{
				if ( !ParseEventChangeAttributes( subkey ) ) 
				{
					Warning( "TFNPCSpawner: Failed to parse EventChangeAttributes\n" );
					return false;
				}
			}
			else
			{
				if ( !ParseDynamicAttributes( m_DefaultAttrs, subkey ) )
				{
					Warning( "TFNPCSpawner: Unknown field '%s'\n", name );
					return false;
				}
			}
		}*/
	}

	return true;
}

int CTFNPCSpawner::Spawn( const Vector& where, CUtlVector<CHandle<CBaseEntity>> *ents )
{
	VPROF_BUDGET( "CTFNPCSpawner::Spawn", VPROF_BUDGETGROUP_NPCS );

	/*CTFNavArea *area = static_cast<CTFNavArea *>( TheNavMesh->GetNavArea(where, 120.0f) );
	if ( ( area->m_nAttributes & NO_SPAWNING ) != 0 )
	{
		if ( lfe_horde_debug.GetBool() )
		{
			DevMsg( "CTFNPCSpawner: %3.2f: *** Tried to spawn in a NO_SPAWNING area at (%f, %f, %f)\n", gpGlobals->curtime, where.x, where.y, where.z );
		}

		return 0;
	}*/

	if ( TFGameRules() != nullptr && TFGameRules()->IsHordeMode() && TFGameRules()->State_Get() != GR_STATE_RND_RUNNING ) 
		return 0;

	Vector where_modified = where;

	bool is_space = false;
	for ( float dz = 0.0f; dz < 18.0f; dz += 4.0f )
	{
		where_modified.z = where.z + 18.0f;

		if ( IsSpaceToSpawnHere( where_modified ) )
		{
			is_space = true;
			break;
		}
	}

	if ( !is_space )
	{
		if (lfe_horde_debug.GetBool()) {
			DevMsg("CTFNPCSpawner: %3.2f: *** No space to spawn at (%f, %f, %f)\n",
				gpGlobals->curtime, where.x, where.y, where.z);
		}
		
		return 0;
	}

	CAI_BaseNPC *bot = dynamic_cast<CAI_BaseNPC *>( CreateEntityByName( STRING( GetClassName( 0 ) ) ) );
	if ( bot )
	{
		bot->SetLocalizeName( m_strName.Get() );

		bot->ChangeTeam( ( TFGameRules()->IsHordeMode() ? TF_TEAM_GREEN : TF_TEAM_BLUE ) );

		/*bot->m_ECAttrs.SetSize(0);
		FOR_EACH_VEC( m_ECAttrs, i ) 
		{
			bot->AddEventChangeAttributes( m_ECAttrs[i] );
		}*/

		/*if ( g_LFEPopManager.IsInEndlessWaves() )
			g_LFEPopManager.EndlessSetAttributesForBot( bot );*/
		
		/*FOR_EACH_VEC( m_TeleportWhere, i )
		{
			bot->m_TeleportWhere.CopyAndAddToTail(m_TeleportWhere[i] );
		}*/
		
		/*if ( HasAttribute( CTFBot::AttributeType::MINIBOSS, -1 ) )
			bot->m_bIsMiniBoss = true;

		if ( HasAttribute( CTFBot::AttributeType::USEBOSSHEALTHBAR, -1 ) )
			bot->m_bUseBossHealthBor = true;

		if ( HasAttribute( CTFBot::AttributeType::BULLETIMMUNE, -1 ) )
			bot->AddCond( TF_COND_BULLET_IMMUNE );

		if ( HasAttribute( CTFBot::AttributeType::BLASTIMMUNE, -1 ) )
			bot->AddCond( TF_COND_BLAST_IMMUNE );

		if ( HasAttribute( CTFBot::AttributeType::FIREIMMUNE, -1 ) )
			bot->AddCond( TF_COND_FIRE_IMMUNE );*/

		float scale = m_flScale;
		if ( scale < 0.0f )
			scale = 1.0f;

		bot->SetModelScale( scale );

		float health = (float)m_iHealth;
		if ( health < 0.0f )
			health = (float)bot->GetMaxHealth();

		//health *= g_LFEPopManager.GetHealthMultiplier( false );
		bot->SetHealth( (int)health );

		/*CCaptureFlag *flag = bot->GetFlagToFetch();
		if (flag != nullptr)
		{
			bot->SetFlagTarget(flag);
		}*/

		if ( ents != nullptr )
			ents->AddToTail( bot );

		DispatchSpawn( bot );

		if ( TFGameRules()->IsHordeMode() && bot->IsMiniBoss() )  
			TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_GIANT_CALLOUT, TF_TEAM_RED );
		
		if ( lfe_horde_debug.GetBool() )
		{
			DevMsg( "%3.2f: Spawned TFNPC '%s'\n", gpGlobals->curtime, m_strName.Get() );
		}
		
		return 1;
	}

	return 0;
}

string_t CTFNPCSpawner::GetClassName( int index )
{
	if ( !m_strClassName )
	{
		return AllocPooledString( "npc_zombie" );
	}
	else 
	{
		return m_strClassName;
	}
}

int CTFNPCSpawner::GetHealth( int index )
{
	return m_iHealth;
}

bool CTFNPCSpawner::IsMiniBoss( int index )
{
	return false; //(( m_DefaultAttrs.m_nBotAttrs & CTFBot::AttributeType::MINIBOSS ) != 0 );
}

/*bool CTFNPCSpawner::HasAttribute( CTFBot::AttributeType attr, int index )
{
	return ( ( m_DefaultAttrs.m_nBotAttrs & attr ) != 0 );
}

bool CTFNPCSpawner::HasEventChangeAttributes( const char *name ) const
{
	FOR_EACH_VEC( m_ECAttrs, i )
	{
		const CTFBot::EventChangeAttributes_t& ecattr = m_ECAttrs[i];
		
		const char *ecname = ecattr.m_strName.Get();
		if ( name == ecname || V_stricmp(name, ecname) == 0 )
			return true;
	}
	
	return false;
}

bool CTFNPCSpawner::ParseEventChangeAttributes( KeyValues *kv )
{
	if ( kv == nullptr )
		return true;

	FOR_EACH_SUBKEY(kv, subkey)
	{
		const char *name = subkey->GetName();

		m_ECAttrs.AddToTail();
		CTFBot::EventChangeAttributes_t& ecattr = m_ECAttrs.Tail();
		ecattr.m_strName = name;
		
		FOR_EACH_SUBKEY( subkey, subsubkey )
		{
			if ( !ParseDynamicAttributes( ecattr, subsubkey ) )
			{
				Warning( "TFBotSpawner EventChangeAttributes: Failed to parse event '%s' with unknown attribute '%s'\n", subkey->GetName(), subsubkey->GetName() );
				return false;
			}
		}

		if ( V_stricmp( name, "default" ) == 0 )
		{
			// use the default copy constructor
			this->m_DefaultAttrs = ecattr;
		}
	}

	return true;
}*/

/*bool ParseDynamicAttributes( CTFBot::EventChangeAttributes_t& ecattr, KeyValues *kv )
{
	const char *name = kv->GetName();

	if ( V_stricmp( name, "Skill" ) == 0 )
	{
		const char *val = kv->GetString();

		if ( V_stricmp( val, "Easy" ) == 0 )
			ecattr.m_iSkill = CTFBot::DifficultyType::EASY;
		else if ( V_stricmp( val, "Normal" ) == 0 )
			ecattr.m_iSkill = CTFBot::DifficultyType::NORMAL;
		else if ( V_stricmp( val, "Hard" ) == 0 )
			ecattr.m_iSkill = CTFBot::DifficultyType::HARD;
		else if ( V_stricmp( val, "Expert" ) == 0 )
			ecattr.m_iSkill = CTFBot::DifficultyType::EXPERT;
		else
		{
			Warning( "TFNPCSpawner: Invalid skill '%s'\n", val );
			return false;
		}

		return true;
	}
	
	if ( V_stricmp( name, "BehaviorModifiers" ) == 0 )
	{
		const char *val = kv->GetString();

		if ( V_stricmp( val, "Mobber" ) == 0 || V_stricmp( val, "Push" ) == 0 ) 
		{
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::AGGRESSIVE;
		}
		else
		{
			Warning( "TFBotSpawner: invalid behavior modifier '%s'\n", val );
			return false;
		}

		return true;
	}

	if ( V_stricmp( name, "MaxVisionRange" ) == 0 )
	{
		ecattr.m_flVisionRange = kv->GetFloat();
		return true;
	}

	if ( V_stricmp( name, "Item" ) == 0 )
	{
		ecattr.m_ItemNames.CopyAndAddToTail( kv->GetString() );
		return true;
	}

	if ( V_stricmp( name, "Tag" ) == 0 )
	{
		ecattr.m_Tags.CopyAndAddToTail( kv->GetString() );
		return true;
	}

	if ( V_stricmp( name, "Attributes" ) == 0 )
	{
		const char *val = kv->GetString();

		if (V_stricmp(val, "RemoveOnDeath") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::REMOVEONDEATH;
		else if (V_stricmp(val, "Aggressive") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::AGGRESSIVE;
		else if (V_stricmp(val, "SuppressFire") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::SUPPRESSFIRE;
		else if (V_stricmp(val, "DisableDodge") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::DISABLEDODGE;
		else if (V_stricmp(val, "BecomeSpectatorOnDeath") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::BECOMESPECTATORONDEATH;
		else if (V_stricmp(val, "RetainBuildings") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::RETAINBUILDINGS;
		else if (V_stricmp(val, "SpawnWithFullCharge") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::SPAWNWITHFULLCHARGE;
		else if (V_stricmp(val, "AlwaysCrit") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::ALWAYSCRIT;
		else if (V_stricmp(val, "IgnoreEnemies") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::IGNOREENEMIES;
		else if (V_stricmp(val, "HoldFireUntilFullReload") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::HOLDFIREUNTILFULLRELOAD;
		else if (V_stricmp(val, "AlwaysFireWeapon") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::ALWAYSFIREWEAPON;
		else if (V_stricmp(val, "TeleportToHint") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::TELEPORTTOHINT;
		else if (V_stricmp(val, "MiniBoss") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::MINIBOSS;
		else if (V_stricmp(val, "UseBossHealthBar") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::USEBOSSHEALTHBAR;
		else if (V_stricmp(val, "IgnoreFlag") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::IGNOREFLAG;
		else if (V_stricmp(val, "AirChargeOnly") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::AIRCHARGEONLY;
		else if (V_stricmp(val, "VaccinatorBullets") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::VACCINATORBULLETS;
		else if (V_stricmp(val, "VaccinatorBlast") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::VACCINATORBLAST;
		else if (V_stricmp(val, "VaccinatorFire") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::VACCINATORFIRE;
		else if (V_stricmp(val, "BulletImmune") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::BULLETIMMUNE;
		else if (V_stricmp(val, "BlastImmune") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::BLASTIMMUNE;
		else if (V_stricmp(val, "FireImmune") == 0)
			ecattr.m_nBotAttrs |= CTFBot::AttributeType::FIREIMMUNE;
		else
			Warning( "TFNPCSpawner: Invalid attribute '%s'\n", val );
			return false;
		}
		
		return true;
	}
	
	if ( V_stricmp( name, "CharacterAttributes" ) == 0 )
	{
		CUtlVector<static_attrib_t> attrs;

		FOR_EACH_SUBKEY(kv, subkey)
		{
			static_attrib_t attr;
			CUtlVector<CUtlString> errors;

			if ( attr.BInitFromKV_SingleLine( "CharacterAttributes", subkey, &errors, true ) )
			{
				attrs.AddToHead( attr );
			}
			else
			{
				FOR_EACH_VEC( errors, i )
				{
					Warning( "TFBotSpawner: attribute error: '%s'\n", errors[i].Get() );
				}
			}
		}

		FOR_EACH_VEC( attrs, i )
		{
			static_attrib_t& attr_new = attrs[i];

			bool found_old = false;
			FOR_EACH_VEC( ecattr.m_CharAttrs, j )
			{
				static_attrib_t& attr_old = ecattr.m_CharAttrs[j];

				if ( attr_new.m_iAttrIndex == attr_old.m_iAttrIndex )
				{
					attr_old.m_Value = attr_new.m_Value;

					found_old = true;
					break;
				}
			}

			if ( !found_old )
				ecattr.m_CharAttrs.AddToHead(attr_new);
		}

		return true;
	}

	if ( V_stricmp( name, "ItemAttributes" ) == 0)
	{
		const char *item_name = nullptr;
		CUtlVector<static_attrib_t> attrs;

		FOR_EACH_SUBKEY( kv, subkey )
		{
			if ( V_stricmp(subkey->GetName(), "ItemName") == 0 )
			{
				if ( item_name == nullptr )
					item_name = subkey->GetString();
				else
					Warning( "TFNPCSpawner: \"ItemName\" field specified multiple times ('%s' / '%s').\n", item_name, subkey->GetString() );
				}
			}
			else
			{
				static_attrib_t attr;
				CUtlVector<CUtlString> errors;
				
				if ( attr.BInitFromKV_SingleLine( "ItemAttributes", subkey, &errors, true ) )
				{
					attrs.AddToHead(attr);
				}
				else
				{
					FOR_EACH_VEC( errors, i )
					{
						Warning( "TFBotSpawner: attribute error: '%s'\n", errors[i].Get() );
					}
				}
			}
		}

		if ( item_name == nullptr )
		{
			Warning( "TFNPCSpawner: need to specify ItemName in ItemAttributes.\n" );
		}
		else 
		{
			FOR_EACH_VEC( attrs, i )
			{
				static_attrib_t& attr_new = attrs[i];

				// check if we already have an attr list for this item
				bool found_item_old = false;
				FOR_EACH_VEC( ecattr.m_ItemAttrs, j )
				{
					CTFBot::EventChangeAttributes_t::item_attributes_t& item_old = ecattr.m_ItemAttrs[j];

					if ( V_stricmp( item_old.strItemName, item_name ) == 0 )
					{
						// check if we already have an attr list entry for this
						// attribute on this item
						bool found_attr_old = false;
						FOR_EACH_VEC( item_old.m_Attrs, k )
						{
							static_attrib_t& attr_old = item_old.m_Attrs[k];

							if ( attr_new.m_iAttrIndex == attr_old.m_iAttrIndex )
							{
								attr_old.m_Value = attr_new.m_Value;

								found_attr_old = true;
								break;
							}
						}

						if ( !found_attr_old )
						{
							item_old.m_Attrs.AddToHead( attr_new );
						}

						found_item_old = true;
						break;
					}
				}

				if ( !found_item_old )
				{
					CTFBot::EventChangeAttributes_t::item_attributes_t item_new;
					item_new.m_strItemName = item_name;
					item_new.m_Attrs.AddToHead( attr_new );

					ecattr.m_ItemAttrs.AddToTail( item_new );
				}
			}
		}

		return true;
	}

	return false;
}*/