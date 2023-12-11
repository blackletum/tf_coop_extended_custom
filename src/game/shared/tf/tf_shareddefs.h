//====== Copyright © 1996-2006, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_SHAREDDEFS_H
#define TF_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "mp_shareddefs.h"

// Using MAP_DEBUG mode?
#ifdef MAP_DEBUG
	#define MDEBUG(x) x
#else
	#define MDEBUG(x)
#endif

//-----------------------------------------------------------------------------
// Teams.
//-----------------------------------------------------------------------------
enum
{
	TF_TEAM_RED = FIRST_GAME_TEAM,
	TF_TEAM_BLUE,
	TF_TEAM_GREEN,
	TF_TEAM_YELLOW,
	TF_TEAM_COUNT
};

#define TF_TEAM_AUTOASSIGN (TF_TEAM_COUNT + 1 )

#define TF_STORY_TEAM TF_TEAM_RED
#define TF_COMBINE_TEAM TF_TEAM_BLUE
#define TF_ZOMBIE_TEAM TF_TEAM_GREEN
#define TF_ANTLION_TEAM TF_TEAM_YELLOW

#define TF_TEAM_PVE_DEFENDERS TF_TEAM_RED
#define TF_TEAM_PVE_INVADERS TF_TEAM_GREEN

#define TF_TEAM_HALLOWEEN_BOSS TF_TEAM_YELLOW

extern const char *g_aTeamNames[TF_TEAM_COUNT];
extern const char *g_aTeamNamesShort[TF_TEAM_COUNT];
extern const char *g_aTeamParticleNames[TF_TEAM_COUNT];
extern color32 g_aTeamColors[TF_TEAM_COUNT];
extern color32 g_aTeamSkinColors[TF_TEAM_COUNT];

bool IsTeamName( const char *name );

const char *GetTeamParticleName( int iTeam, bool bHasFourTeam = false, const char **pNames = g_aTeamParticleNames );
const char *ConstructTeamParticle( const char *pszFormat, int iTeam, bool bHasFourTeam = false, const char **pNames = g_aTeamParticleNames );
void PrecacheTeamParticles( const char *pszFormat, bool bHasFourTeam = false, const char **pNames = g_aTeamParticleNames );

#define CONTENTS_REDTEAM	CONTENTS_TEAM1
#define CONTENTS_BLUETEAM	CONTENTS_TEAM2
#define CONTENTS_GREENTEAM	CONTENTS_UNUSED
#define CONTENTS_YELLOWTEAM	CONTENTS_UNUSED6
			
// Team roles
enum 
{
	TEAM_ROLE_NONE = 0,
	TEAM_ROLE_DEFENDERS,
	TEAM_ROLE_ATTACKERS,

	NUM_TEAM_ROLES,
};

//-----------------------------------------------------------------------------
// CVar replacements
//-----------------------------------------------------------------------------
#define TF_DAMAGE_CRIT_CHANCE				0.02f
#define TF_DAMAGE_CRIT_CHANCE_RAPID			0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID		2.0f
#define TF_DAMAGE_CRIT_CHANCE_MELEE			0.15f

#define LFE_DAMAGE_NPC_CRIT_DURATION_RAPID	1.5f

#define TF_DAMAGE_CRITMOD_MAXTIME			20
#define TF_DAMAGE_CRITMOD_MINTIME			2
#define TF_DAMAGE_CRITMOD_DAMAGE			800
#define TF_DAMAGE_CRITMOD_MAXMULT			6

#define TF_DAMAGE_CRIT_MULTIPLIER			3.0f
#define TF_DAMAGE_MINICRIT_MULTIPLIER		1.35f


//-----------------------------------------------------------------------------
// TF-specific viewport panels
//-----------------------------------------------------------------------------
#define PANEL_CLASS_BLUE		"class_blue"
#define PANEL_CLASS_RED			"class_red"
#define PANEL_MAPINFO			"mapinfo"
#define PANEL_ROUNDINFO			"roundinfo"
#define PANEL_ARENATEAMSELECT	"arenateamselect"
#define PANEL_COOPSCOREBOARD	"coopscoreboard"
#define PANEL_COOPTEAMSELECT	"coopteamselect"

// file we'll save our list of viewed intro movies in
#define MOVIES_FILE				"viewed.res"

//-----------------------------------------------------------------------------
// Used in calculating the health percentage of a player
//-----------------------------------------------------------------------------
#define TF_HEALTH_UNDEFINED		1

//-----------------------------------------------------------------------------
// Falling damage stuff.
//-----------------------------------------------------------------------------
#define TF_PLAYER_MAX_SAFE_FALL_SPEED	650		

//-----------------------------------------------------------------------------
// Used to mark a spy's disguise attribute (team or class) as "unused"
//-----------------------------------------------------------------------------
#define TF_SPY_UNDEFINED		TEAM_UNASSIGNED

#define COLOR_TF_BLUE	Color( 64, 64, 255, 255 )
#define COLOR_TF_RED	Color( 255, 64, 64, 255 )
#define COLOR_TF_GREEN	Color( 64, 255, 64, 255 )
#define COLOR_TF_YELLOW	Color( 255, 255, 64, 255 )
#define COLOR_TF_SPECTATOR Color( 245, 229, 196, 255 )

#define COLOR_EYEBALLBOSS_TEXT	Color( 134, 80, 172, 255 )
#define COLOR_MERASMUS_TEXT	Color( 112, 176, 74, 255 )

//-----------------------------------------------------------------------------
// Player Classes.
//-----------------------------------------------------------------------------
#define TF_CLASS_COUNT			( TF_CLASS_COUNT_ALL - 1 )

#define TF_FIRST_NORMAL_CLASS	( TF_CLASS_UNDEFINED + 1 )
#define TF_LAST_NORMAL_CLASS	( TF_CLASS_CIVILIAN - 1 )

#define TF_FIRST_NPC_CLASS	( TF_CLASS_CIVILIAN + 1 )
#define TF_LAST_NPC_CLASS	( TF_CLASS_COUNT_ALL - 1 )

#define	TF_CLASS_MENU_BUTTONS	( TF_CLASS_RANDOM + 1 )

enum ETFClass	// DO NOT ADD BEYOND 16 CLASSES
{
	TF_CLASS_UNDEFINED = 0,

	TF_CLASS_SCOUT,			// TF_FIRST_NORMAL_CLASS
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HEAVYWEAPONS,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,		// TF_LAST_NORMAL_CLASS
	
	// Add any new classes after Engineer.
	TF_CLASS_CIVILIAN,

	TF_CLASS_COMBINE,
	TF_CLASS_ZOMBIEFAST,
	TF_CLASS_WILDCARD,

	TF_CLASS_COUNT_ALL,

	TF_CLASS_RANDOM
};

extern const char *g_aPlayerClassNames[];				// localized class names
extern const char *g_aPlayerClassNames_NonLocalized[];	// non-localized class names

extern const char *g_aRawPlayerClassNamesShort[];	// raw short class mames
extern const char *g_aRawPlayerClassNames[];	// raw class names

extern const char *g_aPlayerClassEmblems[];
extern const char *g_aPlayerClassEmblemsDead[];

extern const char *g_aPlayerClassEmblemsAlt[];
extern const char *g_aPlayerClassEmblemsAltDead[];

bool IsPlayerClassName( const char *name );

int GetClassIndexFromString( const char *name, int maxClass );

//-----------------------------------------------------------------------------
// For entity_capture_flags to use when placed in the world
//-----------------------------------------------------------------------------
enum ETFFlagType
{
	TF_FLAGTYPE_CTF = 0,
	TF_FLAGTYPE_ATTACK_DEFEND,
	TF_FLAGTYPE_TERRITORY_CONTROL,
	TF_FLAGTYPE_INVADE,
	TF_FLAGTYPE_SPECIAL_DELIVERY,
	TF_FLAGTYPE_PLAYER_DESTRUCTION,
};

//-----------------------------------------------------------------------------
// For the game rules to determine which type of game we're playing
//-----------------------------------------------------------------------------
enum ETFGameType
{
	TF_GAMETYPE_UNDEFINED = 0,
	TF_GAMETYPE_CTF,
	TF_GAMETYPE_CP,
	TF_GAMETYPE_ESCORT,
	TF_GAMETYPE_ARENA,
	TF_GAMETYPE_MVM,
	TF_GAMETYPE_RD,
	TF_GAMETYPE_PASSTIME,
	TF_GAMETYPE_PD,
	TF_GAMETYPE_COOP,
	TF_GAMETYPE_VS,
	TF_GAMETYPE_BLUCOOP,
	TF_GAMETYPE_HORDE,
	TF_GAMETYPE_FREE,
	TF_GAMETYPE_INFECTION,
};
extern const char *g_aGameTypeNames[];	// localized gametype names

//-----------------------------------------------------------------------------
// Buildings.
//-----------------------------------------------------------------------------
enum
{
	TF_BUILDING_SENTRY				= (1<<0),
	TF_BUILDING_DISPENSER			= (1<<1),
	TF_BUILDING_TELEPORT			= (1<<2),
};

//-----------------------------------------------------------------------------
// Items.
//-----------------------------------------------------------------------------
enum
{
	TF_ITEM_UNDEFINED		= 0,
	TF_ITEM_CAPTURE_FLAG	= (1<<0),
	TF_ITEM_HEALTH_KIT		= (1<<1),
	TF_ITEM_ARMOR			= (1<<2),
	TF_ITEM_AMMO_PACK		= (1<<3),
	TF_ITEM_GRENADE_PACK	= (1<<4),
};

//-----------------------------------------------------------------------------
// Ammo.
//-----------------------------------------------------------------------------
enum
{
	TF_AMMO_DUMMY = 0,	// Dummy index to make the CAmmoDef indices correct for the other ammo types.
	TF_AMMO_PRIMARY,
	TF_AMMO_SECONDARY,
	TF_AMMO_METAL,
	TF_AMMO_GRENADES1,
	TF_AMMO_GRENADES2,
	TF_AMMO_GRENADES3,	// spellbook
	LFE_AMMO_GRENADES1,	// tfc grenades
	LFE_AMMO_GRENADES2,	// tfc grenades
	TF_AMMO_COUNT
};

enum EAmmoSource
{
	TF_AMMO_SOURCE_AMMOPACK = 0, // Default, used for ammopacks
	TF_AMMO_SOURCE_RESUPPLY,
	TF_AMMO_SOURCE_DISPENSER,
	TF_AMMO_SOURCE_COUNT
};

//-----------------------------------------------------------------------------
// Grenade Launcher mode (for pipebombs).
//-----------------------------------------------------------------------------
enum
{
	TF_GL_MODE_REGULAR = 0,
	TF_GL_MODE_REMOTE_DETONATE,
	TF_GL_MODE_FIZZLE,
};

//-----------------------------------------------------------------------------
// Weapon Types
//-----------------------------------------------------------------------------
enum
{
	TF_WPN_TYPE_PRIMARY = 0,
	TF_WPN_TYPE_SECONDARY,
	TF_WPN_TYPE_MELEE,
	TF_WPN_TYPE_GRENADE,
	TF_WPN_TYPE_BUILDING,
	TF_WPN_TYPE_PDA,
	TF_WPN_TYPE_ITEM1,
	TF_WPN_TYPE_ITEM2,
	TF_WPN_TYPE_HEAD,
	TF_WPN_TYPE_MISC,
	TF_WPN_TYPE_MELEE_ALLCLASS,
	TF_WPN_TYPE_SECONDARY2,
	TF_WPN_TYPE_PRIMARY2,
	TF_WPN_TYPE_ITEM3,
	TF_WPN_TYPE_ITEM4,
	LFE_WPN_TYPE_PHYSGUN,

	TF_WPN_TYPE_COUNT
};

extern const char *g_AnimSlots[];
extern const char *g_LoadoutSlots[];
extern const char *g_LoadoutTranslations[];

//-----------------------------------------------------------------------------
// Loadout slots
//-----------------------------------------------------------------------------
enum loadout_positions_t
{
	LOADOUT_POSITION_INVALID = -1,
	LOADOUT_POSITION_PRIMARY = 0,
	LOADOUT_POSITION_SECONDARY,
	LOADOUT_POSITION_MELEE,
	LOADOUT_POSITION_UTILITY,
	LOADOUT_POSITION_BUILDING,
	LOADOUT_POSITION_PDA1,
	LOADOUT_POSITION_PDA2,
	LOADOUT_POSITION_HAT,
	LOADOUT_POSITION_MISC,
	LOADOUT_POSITION_ACTION,
	LOADOUT_POSITION_MISC2,
	LOADOUT_POSITION_TAUNT,
	LOADOUT_POSITION_TAUNT2,
	LOADOUT_POSITION_TAUNT3,
	LOADOUT_POSITION_TAUNT4,
	LOADOUT_POSITION_TAUNT5,
	LOADOUT_POSITION_TAUNT6,
	LOADOUT_POSITION_TAUNT7,
	LOADOUT_POSITION_TAUNT8,

	LOADOUT_POSITION_BUFFER,
	LOADOUT_POSITION_COUNT
};

extern const char *g_aAmmoNames[];

//-----------------------------------------------------------------------------
// Weapons.
//-----------------------------------------------------------------------------
#define TF_PLAYER_WEAPON_COUNT		8
#define TF_PLAYER_GRENADE_COUNT		2
#define TF_PLAYER_BUILDABLE_COUNT	4

#define TF_WEAPON_PRIMARY_MODE		0
#define TF_WEAPON_SECONDARY_MODE	1

#define TF_WEAPON_GRENADE_FRICTION						0.6f
#define TF_WEAPON_GRENADE_GRAVITY						0.81f
#define TF_WEAPON_GRENADE_INITPRIME						0.8f
#define TF_WEAPON_GRENADE_CONCUSSION_TIME				15.0f
#define TF_WEAPON_GRENADE_MIRV_BOMB_COUNT				4
#define TF_WEAPON_GRENADE_CALTROP_TIME					8.0f

#define TF_WEAPON_PIPEBOMB_WORLD_COUNT					15
#define TF_WEAPON_PIPEBOMB_COUNT						8
#define TF_WEAPON_PIPEBOMB_INTERVAL						0.6f
#define TF_PIPEBOMB_MIN_CHARGE_VEL 900
#define TF_PIPEBOMB_MAX_CHARGE_VEL 2400
#define TF_PIPEBOMB_MAX_CHARGE_TIME 4.0f

#define TF_WEAPON_ROCKET_INTERVAL						0.8f

#define TF_WEAPON_FLAMETHROWER_INTERVAL					0.15f
#define TF_WEAPON_FLAMETHROWER_ROCKET_INTERVAL			0.8f

#define TF_WEAPON_ZOOM_FOV								20

#define TF_WEAPON_MAX_REVENGE							35

enum
{
	TF_WEAPON_NONE = 0,
	TF_WEAPON_BAT,
	TF_WEAPON_BAT_WOOD,
	TF_WEAPON_BOTTLE,
	TF_WEAPON_FIREAXE,
	TF_WEAPON_CLUB,
	TF_WEAPON_CROWBAR,
	TF_WEAPON_KNIFE,
	TF_WEAPON_FISTS,
	TF_WEAPON_SHOVEL,
	TF_WEAPON_WRENCH,
	TF_WEAPON_BONESAW,
	TF_WEAPON_SHOTGUN_PRIMARY,
	TF_WEAPON_SHOTGUN_SOLDIER,
	TF_WEAPON_SHOTGUN_HWG,
	TF_WEAPON_SHOTGUN_PYRO,
	TF_WEAPON_SCATTERGUN,
	TF_WEAPON_SNIPERRIFLE,
	TF_WEAPON_MINIGUN,
	TF_WEAPON_SMG,
	TF_WEAPON_SYRINGEGUN_MEDIC,
	TF_WEAPON_TRANQ,
	TF_WEAPON_ROCKETLAUNCHER,
	TF_WEAPON_GRENADELAUNCHER,
	TF_WEAPON_PIPEBOMBLAUNCHER,
	TF_WEAPON_FLAMETHROWER,
	TF_WEAPON_GRENADE_NORMAL,
	TF_WEAPON_GRENADE_CONCUSSION,
	TF_WEAPON_GRENADE_NAIL,
	TF_WEAPON_GRENADE_MIRV,
	TF_WEAPON_GRENADE_MIRV_DEMOMAN,
	TF_WEAPON_GRENADE_NAPALM,
	TF_WEAPON_GRENADE_GAS,
	TF_WEAPON_GRENADE_EMP,
	TF_WEAPON_GRENADE_CALTROP,
	TF_WEAPON_GRENADE_PIPEBOMB,
	TF_WEAPON_GRENADE_SMOKE_BOMB,
	TF_WEAPON_GRENADE_HEAL,
	TF_WEAPON_GRENADE_STUNBALL,
	TF_WEAPON_GRENADE_JAR,
	TF_WEAPON_GRENADE_JAR_MILK,
	TF_WEAPON_PISTOL,
	TF_WEAPON_PISTOL_SCOUT,
	TF_WEAPON_REVOLVER,
	TF_WEAPON_NAILGUN,
	TF_WEAPON_PDA,
	TF_WEAPON_PDA_ENGINEER_BUILD,
	TF_WEAPON_PDA_ENGINEER_DESTROY,
	TF_WEAPON_PDA_SPY,
	TF_WEAPON_BUILDER,
	TF_WEAPON_MEDIGUN,
	TF_WEAPON_GRENADE_MIRVBOMB,
	TF_WEAPON_FLAMETHROWER_ROCKET,
	TF_WEAPON_GRENADE_DEMOMAN,
	TF_WEAPON_SENTRY_BULLET,
	TF_WEAPON_SENTRY_ROCKET,
	TF_WEAPON_DISPENSER,
	TF_WEAPON_INVIS,
	TF_WEAPON_FLAREGUN,
	TF_WEAPON_LUNCHBOX,
	TF_WEAPON_JAR,
	TF_WEAPON_COMPOUND_BOW,
	TF_WEAPON_BUFF_ITEM,
	TF_WEAPON_PUMPKIN_BOMB,
	TF_WEAPON_SWORD,
	TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT,
	TF_WEAPON_LIFELINE,
	TF_WEAPON_LASER_POINTER,
	TF_WEAPON_DISPENSER_GUN,
	TF_WEAPON_SENTRY_REVENGE,
	TF_WEAPON_JAR_MILK,
	TF_WEAPON_HANDGUN_SCOUT_PRIMARY,
	TF_WEAPON_BAT_FISH,
	TF_WEAPON_CROSSBOW,
	TF_WEAPON_STICKBOMB,
	TF_WEAPON_HANDGUN_SCOUT_SECONDARY,
	TF_WEAPON_SODA_POPPER,
	TF_WEAPON_SNIPERRIFLE_DECAP,
	TF_WEAPON_RAYGUN,
	TF_WEAPON_PARTICLE_CANNON,
	TF_WEAPON_MECHANICAL_ARM,
	TF_WEAPON_DRG_POMSON,
	TF_WEAPON_BAT_GIFTWARP,
	TF_WEAPON_GRENADE_ORNAMENT_BALL,
	TF_WEAPON_FLAREGUN_REVENGE,
	TF_WEAPON_PEP_BRAWLER_BLASTER,
	TF_WEAPON_CLEAVER,
	TF_WEAPON_GRENADE_CLEAVER,
	TF_WEAPON_STICKY_BALL_LAUNCHER,
	TF_WEAPON_GRENADE_STICKY_BALL,
	TF_WEAPON_SHOTGUN_BUILDING_RESCUE,
	TF_WEAPON_CANNON,
	TF_WEAPON_THROWABLE,
	TF_WEAPON_GRENADE_THROWABLE,
	TF_WEAPON_PDA_SPY_BUILD,
	TF_WEAPON_GRENADE_WATERBALLOON,
	TF_WEAPON_HARVESTER_SAW,
	TF_WEAPON_SPELLBOOK,
	TF_WEAPON_SPELLBOOK_PROJECTILE,
	TF_WEAPON_SNIPERRIFLE_CLASSIC,
	TF_WEAPON_PARACHUTE,
	TF_WEAPON_GRAPPLINGHOOK,
	TF_WEAPON_PASSTIME_GUN,
	TF_WEAPON_CHARGED_SMG,
	TF_WEAPON_BREAKABLE_SIGN,
	TF_WEAPON_ROCKETPACK,
	TF_WEAPON_SLAP,
	TF_WEAPON_JAR_GAS,
	TF_WEAPON_GRENADE_JAR_GAS,
	TF_WEAPON_ROCKETLAUNCHER_FIREBALL, //TF_WEPON_FLAME_BALL, // valve why?
	TF_WEAPON_ROBOT_ARM,

	// ADD NEW WEAPONS AFTER THIS
	TF_WEAPON_TFC_CROWBAR,
	TF_WEAPON_TFC_KNIFE,
	TF_WEAPON_TFC_MEDIKIT,
	TF_WEAPON_TFC_MINIGUN,
	TF_WEAPON_TFC_NAILGUN,
	TF_WEAPON_TFC_SHOTGUN,
	TF_WEAPON_TFC_SPANNER,
	TF_WEAPON_TFC_SUPER_NAILGUN,
	TF_WEAPON_TFC_SUPER_SHOTGUN,
	TF_WEAPON_TFC_ROCKETLAUNCHER,
	TF_WEAPON_TFC_FLAMETHROWER,
	TF_WEAPON_TFC_GRENADELAUNCHER,
	TF_WEAPON_TFC_PIPEBOMBLAUNCHER,
	TF_WEAPON_TFC_SNIPERRIFLE,
	TF_WEAPON_TFC_AUTOMATICRIFLE,
	TF_WEAPON_TFC_INCENDIARYCANNON,
	TF_WEAPON_TFC_TRANQ,
	TF_WEAPON_TFC_RAILGUN,
	TF_WEAPON_TFC_UMBRELLA,
	TF_WEAPON_TFC_GRENADE_FLARE,

	TF_WEAPON_ZOMBIE_CLAW,
	TF_WEAPON_ANTLION_CLAW,

	TF_WEAPON_HL2_CROWBAR,
	TF_WEAPON_HL2_STUNSTICK,
	TF_WEAPON_HL2_PACKAGE,
	TF_WEAPON_HL2_HARPOON,
	TF_WEAPON_HL2_SMG1,
	TF_WEAPON_HL2_SMG2,
	TF_WEAPON_HL2_SHOTGUN,
	TF_WEAPON_HL2_ANNABELLE,
	TF_WEAPON_HL2_AR2,
	TF_WEAPON_HL2_RPG,
	TF_WEAPON_HL2_CROSSBOW,
	TF_WEAPON_HL2_FRAG,
	TF_WEAPON_HL2_BUGBAIT,
	TF_WEAPON_HL2_SLAM,
	TF_WEAPON_HL2_TRIPMINE,
	TF_WEAPON_HL2_PISTOL,
	TF_WEAPON_HL2_357,
	TF_WEAPON_HL2_ALYXGUN,

	TF_WEAPON_PORTALGUN,
	TF_WEAPON_PHYSGUN,
	TF_WEAPON_CHEATGUN,
	TF_WEAPON_PHYSCANNON,

	TF_WEAPON_COUNT
};

extern const char *g_aWeaponNames[];
extern int g_aWeaponDamageTypes[];
extern const Vector g_vecFixedWpnSpreadPellets[];

int GetWeaponId( const char *pszWeaponName );
#ifdef GAME_DLL
int GetWeaponFromDamage( const CTakeDamageInfo &info );
#endif
int GetBuildableId( const char *pszBuildableName );

const char *WeaponIdToAlias( int iWeapon );
const char *WeaponIdToClassname( int iWeapon );
const char *TranslateWeaponEntForClass( const char *pszName, int iClass );

bool WeaponID_IsSniperRifle( int iWeaponID );
bool WeaponID_IsSniperRifleOrBow( int iWeaponID );

enum
{
	TF_PROJECTILE_NONE,
	TF_PROJECTILE_BULLET,
	TF_PROJECTILE_ROCKET,
	TF_PROJECTILE_PIPEBOMB,
	TF_PROJECTILE_PIPEBOMB_REMOTE,
	TF_PROJECTILE_SYRINGE,
	TF_PROJECTILE_FLARE,
	TF_PROJECTILE_JAR,
	TF_PROJECTILE_ARROW,
	TF_PROJECTILE_FLAME_ROCKET,
	TF_PROJECTILE_JAR_MILK,
	TF_PROJECTILE_HEALING_BOLT,
	TF_PROJECTILE_ENERGY_BALL,
	TF_PROJECTILE_ENERGY_RING,
	TF_PROJECTILE_PIPEBOMB_REMOTE_PRACTICE,
	TF_PROJECTILE_CLEAVER,
	TF_PROJECTILE_STICKY_BALL,
	TF_PROJECTILE_CANNONBALL,
	TF_PROJECTILE_BUILDING_REPAIR_BOLT,
	TF_PROJECTILE_FESTITIVE_ARROW,
	TF_PROJECTILE_THROWABLE,
	TF_PROJECTILE_SPELLFIREBALL,
	TF_PROJECTILE_FESTITIVE_URINE,
	TF_PROJECTILE_FESTITIVE_HEALING_BOLT,
	TF_PROJECTILE_BREADMONSTER_JARATE,
	TF_PROJECTILE_BREADMONSTER_MADMILK,
	TF_PROJECTILE_GRAPPLINGHOOK,
	TF_PROJECTILE_SENTRY_ROCKET,
	TF_PROJECTILE_BREAD_MONSTER,
	TF_PROJECTILE_JAR_GAS,
	TF_PROJECTILE_BALLOFFIRE,
	// Add new projectiles here.
	LFE_HL2_PROJECTILE_FRAG,
	LFE_HL2_PROJECTILE_COMBINEBALL,
	LFE_HL2_PROJECTILE_AR2,
	LFE_HL2_PROJECTILE_SPIT,
	LFE_HL2_PROJECTILE_CROSSBOW_BOLT,
	LFE_HL2_PROJECTILE_RPG_MISSILE,
	LFE_HL1_PROJECTILE_RPG_ROCKET,
	LFE_HL1_PROJECTILE_HORNET,
	LFE_HL1_PROJECTILE_CROSSBOW_BOLT,
	LFE_HL1_PROJECTILE_GRENADE,
	LFE_TFC_PROJECTILE_NAIL,
	LFE_TFC_PROJECTILE_NAIL_SUPER,
	LFE_TFC_PROJECTILE_NAIL_TRANQ,
	LFE_TFC_PROJECTILE_NAIL_RAILGUN,
	LFE_TFC_PROJECTILE_NAIL_GRENADE,
	LFE_TFC_PROJECTILE_GRENADE,
	LFE_TFC_PROJECTILE_ROCKET,
	LFE_TFC_PROJECTILE_IC,
	TF_NUM_PROJECTILES
};

extern const char *g_szProjectileNames[];

//-----------------------------------------------------------------------------
// Attributes.
//-----------------------------------------------------------------------------
#define TF_PLAYER_VIEW_OFFSET	Vector( 0, 0, 64.0 ) //--> see GetViewVectors()

//-----------------------------------------------------------------------------
// TF Player Condition.
//-----------------------------------------------------------------------------

// Burning
#define TF_BURNING_FREQUENCY		0.5f
#define TF_BURNING_FLAME_LIFE		10.0
#define TF_BURNING_FLAME_LIFE_PYRO	0.25		// pyro only displays burning effect momentarily
#define TF_BURNING_DMG				3

// Bleeding
#define TF_BLEEDING_FREQUENCY		0.5f
#define TF_BLEEDING_DAMAGE			4

// disguising
#define TF_TIME_TO_CHANGE_DISGUISE 0.5
#define TF_TIME_TO_DISGUISE 2.0
#define TF_TIME_TO_SHOW_DISGUISED_FINISHED_EFFECT 5.0


#define SHOW_DISGUISE_EFFECT 1
#define TF_DISGUISE_TARGET_INDEX_NONE	( MAX_PLAYERS + 1 )
#define TF_PLAYER_INDEX_NONE			( MAX_PLAYERS + 1 )

#define PERMANENT_CONDITION		-1
#define PERMANENT_ATTRIBUTE		-1

// Most of these conds aren't actually implemented but putting them here for compatibility.
enum ETFCond
{
	TF_COND_AIMING = 0,		// Sniper aiming, Heavy minigun.
	TF_COND_ZOOMED,
	TF_COND_DISGUISING,
	TF_COND_DISGUISED,
	TF_COND_STEALTHED,
	TF_COND_INVULNERABLE,
	TF_COND_TELEPORTED,
	TF_COND_TAUNTING,
	TF_COND_INVULNERABLE_WEARINGOFF,
	TF_COND_STEALTHED_BLINK,
	TF_COND_SELECTED_TO_TELEPORT,
	TF_COND_CRITBOOSTED,
	TF_COND_TMPDAMAGEBONUS,
	TF_COND_FEIGN_DEATH,
	TF_COND_PHASE,
	TF_COND_STUNNED,
	TF_COND_OFFENSEBUFF,
	TF_COND_SHIELD_CHARGE,
	TF_COND_DEMO_BUFF,
	TF_COND_ENERGY_BUFF,
	TF_COND_RADIUSHEAL,
	TF_COND_HEALTH_BUFF,
	TF_COND_BURNING,
	TF_COND_HEALTH_OVERHEALED,
	TF_COND_URINE,
	TF_COND_BLEEDING,
	TF_COND_DEFENSEBUFF,
	TF_COND_MAD_MILK,
	TF_COND_MEGAHEAL,
	TF_COND_REGENONDAMAGEBUFF,
	TF_COND_MARKEDFORDEATH,
	TF_COND_NOHEALINGDAMAGEBUFF,
	TF_COND_SPEED_BOOST,
	TF_COND_CRITBOOSTED_PUMPKIN,
	TF_COND_CRITBOOSTED_USER_BUFF,
	TF_COND_CRITBOOSTED_DEMO_CHARGE,
	TF_COND_SODAPOPPER_HYPE,
	TF_COND_CRITBOOSTED_FIRST_BLOOD,
	TF_COND_CRITBOOSTED_BONUS_TIME,
	TF_COND_CRITBOOSTED_CTF_CAPTURE,
	TF_COND_CRITBOOSTED_ON_KILL,
	TF_COND_CANNOT_SWITCH_FROM_MELEE,
	TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
	TF_COND_REPROGRAMMED,
	TF_COND_CRITBOOSTED_RAGE_BUFF,
	TF_COND_DEFENSEBUFF_HIGH,
	TF_COND_SNIPERCHARGE_RAGE_BUFF,
	TF_COND_DISGUISE_WEARINGOFF,
	TF_COND_MARKEDFORDEATH_SILENT,
	TF_COND_DISGUISED_AS_DISPENSER,
	TF_COND_SAPPED,
	TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE,
	TF_COND_INVULNERABLE_USER_BUFF,
	TF_COND_HALLOWEEN_BOMB_HEAD,
	TF_COND_HALLOWEEN_THRILLER,
	TF_COND_RADIUSHEAL_ON_DAMAGE,
	TF_COND_CRITBOOSTED_CARD_EFFECT,
	TF_COND_INVULNERABLE_CARD_EFFECT,
	TF_COND_MEDIGUN_UBER_BULLET_RESIST,
	TF_COND_MEDIGUN_UBER_BLAST_RESIST,
	TF_COND_MEDIGUN_UBER_FIRE_RESIST,
	TF_COND_MEDIGUN_SMALL_BULLET_RESIST,
	TF_COND_MEDIGUN_SMALL_BLAST_RESIST,
	TF_COND_MEDIGUN_SMALL_FIRE_RESIST,
	TF_COND_STEALTHED_USER_BUFF,
	TF_COND_MEDIGUN_DEBUFF,
	TF_COND_STEALTHED_USER_BUFF_FADING,
	TF_COND_BULLET_IMMUNE,
	TF_COND_BLAST_IMMUNE,
	TF_COND_FIRE_IMMUNE,
	TF_COND_PREVENT_DEATH,
	TF_COND_MVM_BOT_STUN_RADIOWAVE,
	TF_COND_HALLOWEEN_SPEED_BOOST,
	TF_COND_HALLOWEEN_QUICK_HEAL,
	TF_COND_HALLOWEEN_GIANT,
	TF_COND_HALLOWEEN_TINY,
	TF_COND_HALLOWEEN_IN_HELL,
	TF_COND_HALLOWEEN_GHOST_MODE,
	TF_COND_MINICRITBOOSTED_ON_KILL,
	TF_COND_OBSCURED_SMOKE,
	TF_COND_PARACHUTE_ACTIVE,
	TF_COND_BLASTJUMPING,
	TF_COND_HALLOWEEN_KART,
	TF_COND_HALLOWEEN_KART_DASH,
	TF_COND_BALLOON_HEAD,
	TF_COND_MELEE_ONLY,
	TF_COND_SWIMMING_CURSE,
	TF_COND_FREEZE_INPUT,
	TF_COND_HALLOWEEN_KART_CAGE,
	TF_COND_DONOTUSE_0,
	TF_COND_RUNE_STRENGTH,
	TF_COND_RUNE_HASTE,
	TF_COND_RUNE_REGEN,
	TF_COND_RUNE_RESIST,
	TF_COND_RUNE_VAMPIRE,
	TF_COND_RUNE_WARLOCK,
	TF_COND_RUNE_PRECISION,
	TF_COND_RUNE_AGILITY,
	TF_COND_GRAPPLINGHOOK,
	TF_COND_GRAPPLINGHOOK_SAFEFALL,
	TF_COND_GRAPPLINGHOOK_LATCHED,
	TF_COND_GRAPPLINGHOOK_BLEEDING,
	TF_COND_AFTERBURN_IMMUNE,
	TF_COND_RUNE_KNOCKOUT,
	TF_COND_RUNE_IMBALANCE,
	TF_COND_CRITBOOSTED_RUNE_TEMP,
	TF_COND_PASSTIME_INTERCEPTION,
	TF_COND_SWIMMING_NO_EFFECTS,
	TF_COND_PURGATORY,
	TF_COND_RUNE_KING,
	TF_COND_RUNE_PLAGUE,
	TF_COND_RUNE_SUPERNOVA,
	TF_COND_PLAGUE,
	TF_COND_KING_BUFFED,
	TF_COND_TEAM_GLOWS,
	TF_COND_KNOCKED_INTO_AIR,
	TF_COND_COMPETITIVE_WINNER,
	TF_COND_COMPETITIVE_LOSER,
	TF_COND_HEALING_DEBUFF,
	TF_COND_PASSTIME_PENALTY_DEBUFF,
	TF_COND_GRAPPLED_TO_PLAYER,
	TF_COND_GRAPPLED_BY_PLAYER,
	TF_COND_PARACHUTE_DEPLOYED,
	TF_COND_GAS,
	TF_COND_BURNING_PYRO,
	TF_COND_ROCKETPACK,
	TF_COND_LOST_FOOTING,
	TF_COND_AIR_CURRENT,
	TF_COND_HALLOWEEN_HELL_HEAL,

	// Add New conds here
	TF_COND_SMOKE_BOMB,
	LFE_COND_FLASHLIGHT,
	LFE_COND_NOCLIP,
	LFE_COND_CUTSCENE,
	LFE_COND_POWERPLAY,
	LFE_COND_ZOMBIE_SPAWN,
	LFE_COND_ZOMBIE_LEAP,

	TF_COND_LAST
};

extern int condition_to_attribute_translation[];

bool ConditionExpiresFast( int nCond );

//-----------------------------------------------------------------------------
// Mediguns.
//-----------------------------------------------------------------------------
enum
{
	TF_MEDIGUN_STOCK = 0,
	TF_MEDIGUN_KRITZKRIEG,
	TF_MEDIGUN_QUICKFIX,
	TF_MEDIGUN_VACCINATOR,
	TF_MEDIGUN_COUNT
};

enum medigun_charge_types
{
	TF_CHARGE_NONE = -1,
	TF_CHARGE_INVULNERABLE = 0,
	TF_CHARGE_CRITBOOSTED,
	TF_CHARGE_MEGAHEAL,
	TF_CHARGE_BULLET_RESIST,
	TF_CHARGE_BLAST_RESIST,
	TF_CHARGE_FIRE_RESIST,

	TF_CHARGE_COUNT
};

enum medigun_resist_types_t
{
	TF_RESIST_NONE = -1,
	TF_RESIST_BULLET = 0,
	TF_RESIST_BLAST,
	TF_RESIST_FIRE,
	TF_RESIST_COUNT
};

typedef struct
{
	int condition_enable;
	int condition_disable;
	const char *sound_enable;
	const char *sound_disable;
} MedigunEffects_t;

extern MedigunEffects_t g_MedigunEffects[];

//-----------------------------------------------------------------------------
// TF Player State.
//-----------------------------------------------------------------------------
enum 
{
	TF_STATE_ACTIVE = 0,		// Happily running around in the game.
	TF_STATE_WELCOME,			// First entering the server (shows level intro screen).
	TF_STATE_OBSERVER,			// Game observer mode.
	TF_STATE_DYING,				// Player is dying.
	TF_STATE_COUNT
};

//-----------------------------------------------------------------------------
// TF FlagInfo State.
//-----------------------------------------------------------------------------
#define TF_FLAGINFO_NONE		0
#define TF_FLAGINFO_STOLEN		(1<<0)
#define TF_FLAGINFO_DROPPED		(1<<1)

enum ETFFlagEventTypes
{
	TF_FLAGEVENT_PICKUP = 1,
	TF_FLAGEVENT_CAPTURE,
	TF_FLAGEVENT_DEFEND,
	TF_FLAGEVENT_DROPPED,
	TF_FLAGEVENT_RETURNED
};

//-----------------------------------------------------------------------------
// Class data
//-----------------------------------------------------------------------------
#define TF_MEDIC_REGEN_TIME			1.0		// Number of seconds between each regen.
#define TF_MEDIC_REGEN_AMOUNT		3 		// Amount of health regenerated each regen.
#define TF_RUNE_REGEN_TIME			0.5		// faster than normal regen

//-----------------------------------------------------------------------------
// Assist-damage constants
//-----------------------------------------------------------------------------
#define TF_TIME_ASSIST_KILL				3.0f	// Time window for a recent damager to get credit for an assist for a kill
#define TF_TIME_ENV_DEATH_KILL_CREDIT	5.0f
#define TF_TIME_SUICIDE_KILL_CREDIT		10.0f	// Time window for a recent damager to get credit for a kill if target suicides

//-----------------------------------------------------------------------------
// Domination/nemesis constants
//-----------------------------------------------------------------------------
#define TF_KILLS_DOMINATION				4		// # of unanswered kills to dominate another player

//-----------------------------------------------------------------------------
// TF Hints
//-----------------------------------------------------------------------------
enum
{
	HINT_FRIEND_SEEN = 0,				// #Hint_spotted_a_friend
	HINT_ENEMY_SEEN,					// #Hint_spotted_an_enemy
	HINT_ENEMY_KILLED,					// #Hint_killing_enemies_is_good
	HINT_AMMO_EXHAUSTED,				// #Hint_out_of_ammo
	HINT_TURN_OFF_HINTS,				// #Hint_turn_off_hints
	HINT_PICKUP_AMMO,					// #Hint_pickup_ammo
	HINT_CANNOT_TELE_WITH_FLAG,			// #Hint_Cannot_Teleport_With_Flag
	HINT_CANNOT_CLOAK_WITH_FLAG,		// #Hint_Cannot_Cloak_With_Flag
	HINT_CANNOT_DISGUISE_WITH_FLAG,		// #Hint_Cannot_Disguise_With_Flag
	HINT_CANNOT_ATTACK_WHILE_CLOAKED,	// #Hint_Cannot_Attack_While_Cloaked
	HINT_CLASSMENU,						// #Hint_ClassMenu

	// Grenades
	HINT_GREN_CALTROPS,					// #Hint_gren_caltrops
	HINT_GREN_CONCUSSION,				// #Hint_gren_concussion
	HINT_GREN_EMP,						// #Hint_gren_emp
	HINT_GREN_GAS,						// #Hint_gren_gas
	HINT_GREN_MIRV,						// #Hint_gren_mirv
	HINT_GREN_NAIL,						// #Hint_gren_nail
	HINT_GREN_NAPALM,					// #Hint_gren_napalm
	HINT_GREN_NORMAL,					// #Hint_gren_normal

	// Weapon alt-fires
	HINT_ALTFIRE_SNIPERRIFLE,			// #Hint_altfire_sniperrifle
	HINT_ALTFIRE_FLAMETHROWER,			// #Hint_altfire_flamethrower
	HINT_ALTFIRE_GRENADELAUNCHER,		// #Hint_altfire_grenadelauncher
	HINT_ALTFIRE_PIPEBOMBLAUNCHER,		// #Hint_altfire_pipebomblauncher
	HINT_ALTFIRE_ROTATE_BUILDING,		// #Hint_altfire_rotate_building

	// Class specific
	// Soldier
	HINT_SOLDIER_RPG_RELOAD,			// #Hint_Soldier_rpg_reload

	// Engineer
	HINT_ENGINEER_USE_WRENCH_ONOWN,		// "#Hint_Engineer_use_wrench_onown",
	HINT_ENGINEER_USE_WRENCH_ONOTHER,	// "#Hint_Engineer_use_wrench_onother",
	HINT_ENGINEER_USE_WRENCH_FRIEND,	// "#Hint_Engineer_use_wrench_onfriend",
	HINT_ENGINEER_BUILD_SENTRYGUN,		// "#Hint_Engineer_build_sentrygun"
	HINT_ENGINEER_BUILD_DISPENSER,		// "#Hint_Engineer_build_dispenser"
	HINT_ENGINEER_BUILD_TELEPORTERS,	// "#Hint_Engineer_build_teleporters"
	HINT_ENGINEER_PICKUP_METAL,			// "#Hint_Engineer_pickup_metal"
	HINT_ENGINEER_REPAIR_OBJECT,		// "#Hint_Engineer_repair_object"
	HINT_ENGINEER_METAL_TO_UPGRADE,		// "#Hint_Engineer_metal_to_upgrade"
	HINT_ENGINEER_UPGRADE_SENTRYGUN,	// "#Hint_Engineer_upgrade_sentrygun"

	HINT_OBJECT_HAS_SAPPER,				// "#Hint_object_has_sapper"

	HINT_OBJECT_YOUR_OBJECT_SAPPED,		// "#Hint_object_your_object_sapped"
	HINT_OBJECT_ENEMY_USING_DISPENSER,	// "#Hint_enemy_using_dispenser"
	HINT_OBJECT_ENEMY_USING_TP_ENTRANCE,	// "#Hint_enemy_using_tp_entrance"
	HINT_OBJECT_ENEMY_USING_TP_EXIT,	// "#Hint_enemy_using_tp_exit"

	NUM_HINTS
};
extern const char *g_pszHintMessages[];



/*======================*/
//      Menu stuff      //
/*======================*/

#define MENU_DEFAULT				1
#define MENU_TEAM 					2
#define MENU_CLASS 					3
#define MENU_MAPBRIEFING			4
#define MENU_INTRO 					5
#define MENU_CLASSHELP				6
#define MENU_CLASSHELP2 			7
#define MENU_REPEATHELP 			8

#define MENU_SPECHELP				9


#define MENU_SPY					12
#define MENU_SPY_SKIN				13
#define MENU_SPY_COLOR				14
#define MENU_ENGINEER				15
#define MENU_ENGINEER_FIX_DISPENSER	16
#define MENU_ENGINEER_FIX_SENTRYGUN	17
#define MENU_ENGINEER_FIX_MORTAR	18
#define MENU_DISPENSER				19
#define MENU_CLASS_CHANGE			20
#define MENU_TEAM_CHANGE			21

#define MENU_REFRESH_RATE 			25

#define MENU_VOICETWEAK				50

// Additional classes
// NOTE: adding them onto the Class_T's in baseentity.h is cheesy, but so is
// having an #ifdef for each mod in baseentity.h.
/*#define CLASS_TFGOAL				((Class_T)NUM_AI_CLASSES)
#define CLASS_TFGOAL_TIMER			((Class_T)(NUM_AI_CLASSES+1))
#define CLASS_TFGOAL_ITEM			((Class_T)(NUM_AI_CLASSES+2))
#define CLASS_TFSPAWN				((Class_T)(NUM_AI_CLASSES+3))
#define CLASS_MACHINE				((Class_T)(NUM_AI_CLASSES+4))*/

// TeamFortress State Flags
#define TFSTATE_GRENPRIMED		0x000001 // Whether the player has a primed grenade
#define TFSTATE_RELOADING		0x000002 // Whether the player is reloading
#define TFSTATE_ALTKILL			0x000004 // #TRUE if killed with a weapon not in self.weapon: NOT USED ANYMORE
#define TFSTATE_RANDOMPC		0x000008 // Whether Playerclass is random, new one each respawn
#define TFSTATE_INFECTED		0x000010 // set when player is infected by the bioweapon
#define TFSTATE_INVINCIBLE		0x000020 // Player has permanent Invincibility (Usually by GoalItem)
#define TFSTATE_INVISIBLE		0x000040 // Player has permanent Invisibility (Usually by GoalItem)
#define TFSTATE_QUAD			0x000080 // Player has permanent Quad Damage (Usually by GoalItem)
#define TFSTATE_RADSUIT			0x000100 // Player has permanent Radsuit (Usually by GoalItem)
#define TFSTATE_BURNING			0x000200 // Is on fire
#define TFSTATE_GRENTHROWING	0x000400  // is throwing a grenade
#define TFSTATE_AIMING			0x000800  // is using the laser sight
#define TFSTATE_ZOOMOFF			0x001000  // doesn't want the FOV changed when zooming
#define TFSTATE_RESPAWN_READY	0x002000  // is waiting for respawn, and has pressed fire
#define TFSTATE_HALLUCINATING	0x004000  // set when player is hallucinating
#define TFSTATE_TRANQUILISED	0x008000  // set when player is tranquilised
#define TFSTATE_CANT_MOVE		0x010000  // player isn't allowed to move
#define TFSTATE_RESET_FLAMETIME 0x020000 // set when the player has to have his flames increased in health
#define TFSTATE_HIGHEST_VALUE	TFSTATE_RESET_FLAMETIME

// items
#define IT_SHOTGUN				(1<<0)
#define IT_SUPER_SHOTGUN		(1<<1) 
#define IT_NAILGUN				(1<<2) 
#define IT_SUPER_NAILGUN		(1<<3) 
#define IT_GRENADE_LAUNCHER		(1<<4) 
#define IT_ROCKET_LAUNCHER		(1<<5) 
#define IT_LIGHTNING			(1<<6) 
#define IT_EXTRA_WEAPON			(1<<7) 

#define IT_SHELLS				(1<<8) 
#define IT_BULLETS				(1<<9) 
#define IT_ROCKETS				(1<<10) 
#define IT_CELLS				(1<<11) 
#define IT_AXE					(1<<12) 

#define IT_ARMOR1				(1<<13) 
#define IT_ARMOR2				(1<<14) 
#define IT_ARMOR3				(1<<15) 
#define IT_SUPERHEALTH			(1<<16) 

#define IT_KEY1					(1<<17) 
#define IT_KEY2					(1<<18) 

#define IT_INVISIBILITY			(1<<19) 
#define IT_INVULNERABILITY		(1<<20) 
#define IT_SUIT					(1<<21)
#define IT_QUAD					(1<<22) 
#define IT_HOOK					(1<<23)

#define IT_KEY3					(1<<24)	// Stomp invisibility
#define IT_KEY4					(1<<25)	// Stomp invulnerability
#define IT_LAST_ITEM			IT_KEY4

/*==================================================*/
/* New Weapon Related Defines		                */
/*==================================================*/

// Medikit
#define WEAP_MEDIKIT_OVERHEAL 50 // Amount of superhealth over max_health the medikit will dispense
#define WEAP_MEDIKIT_HEAL	200  // Amount medikit heals per hit

//--------------
// TF Specific damage flags
//--------------
//#define DMG_UNUSED					(DMG_LASTGENERICFLAG<<2)
// We can't add anymore dmg flags, because we'd be over the 32 bit limit.
// So lets re-use some of the old dmg flags in TF
#define DMG_USE_HITLOCATIONS	(DMG_AIRBOAT)
#define DMG_HALF_FALLOFF		(DMG_RADIATION)
#define DMG_CRITICAL			(DMG_ACID)
#define DMG_MINICRITICAL		(DMG_NERVEGAS)
#define DMG_RADIUS_MAX			(DMG_ENERGYBEAM)
#define DMG_IGNITE				(DMG_PLASMA)
#define DMG_USEDISTANCEMOD		(DMG_SLOWBURN)		// NEED TO REMOVE CALTROPS
#define DMG_NOCLOSEDISTANCEMOD	(DMG_POISON)

#define DMG_CRIT			(DMG_CRITICAL|DMG_MINICRITICAL)

#define TF_DMG_SENTINEL_VALUE	0xFFFFFFFF

// This can only ever be used on a TakeHealth call, since it re-uses a dmg flag that means something else
#define DMG_IGNORE_MAXHEALTH	(DMG_BULLET)

//--------------
// HL2 SPECIFIC
//--------------
#define DMG_SNIPER			(DMG_LASTGENERICFLAG<<1)	// This is sniper damage
#define DMG_MISSILEDEFENSE	(DMG_LASTGENERICFLAG<<2)	// The only kind of damage missiles take. (special missile defense)

// Special Damage types
enum ETFDmgCustom
{
	TF_DMG_CUSTOM_NONE = 0,
	TF_DMG_CUSTOM_HEADSHOT,
	TF_DMG_CUSTOM_BACKSTAB,
	TF_DMG_CUSTOM_BURNING,
	TF_DMG_WRENCH_FIX,
	TF_DMG_CUSTOM_MINIGUN,
	TF_DMG_CUSTOM_SUICIDE,
	TF_DMG_CUSTOM_TAUNTATK_HADOUKEN, // Hadouken
	TF_DMG_CUSTOM_BURNING_FLARE,
	TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON, // POW HAHA.
	TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM, // Homerun
	TF_DMG_CUSTOM_PENETRATE_MY_TEAM,
	TF_DMG_CUSTOM_PENETRATE_ALL_PLAYERS,
	TF_DMG_CUSTOM_TAUNTATK_FENCING, // Knife Fencing
	TF_DMG_CUSTOM_PENETRATE_NONBURNING_TEAMMATE,
	TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB, // Stab Stab Stab
	TF_DMG_CUSTOM_TELEFRAG,
	TF_DMG_CUSTOM_BURNING_ARROW,
	TF_DMG_CUSTOM_FLYINGBURN,
	TF_DMG_CUSTOM_PUMPKIN_BOMB,
	TF_DMG_CUSTOM_DECAPITATION, // Sword
	TF_DMG_CUSTOM_TAUNTATK_GRENADE, // Kamikaze
	TF_DMG_CUSTOM_BASEBALL,
	TF_DMG_CUSTOM_CHARGE_IMPACT,
	TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING, // Sword Decapitation Swing
	TF_DMG_CUSTOM_AIR_STICKY_BURST,
	TF_DMG_CUSTOM_DEFENSIVE_STICKY,
	TF_DMG_CUSTOM_PICKAXE,
	TF_DMG_CUSTOM_ROCKET_DIRECTHIT, // Crits
	TF_DMG_CUSTOM_TAUNTATK_UBERSLICE, // I'm going to saw through your BONES
	TF_DMG_CUSTOM_PLAYER_SENTRY,
	TF_DMG_CUSTOM_STANDARD_STICKY,
	TF_DMG_CUSTOM_SHOTGUN_REVENGE_CRIT, // Frontier Justice
	TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH, // Dischord
	TF_DMG_CUSTOM_BLEEDING,
	TF_DMG_CUSTOM_GOLD_WRENCH, // GOLD ICON
	TF_DMG_CUSTOM_CARRIED_BUILDING,
	TF_DMG_CUSTOM_COMBO_PUNCH, // Gunslinger 3rd hit
	TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KILL, // Organ Grinder
	TF_DMG_CUSTOM_FISH_KILL, // FISH KILL!
	TF_DMG_CUSTOM_TRIGGER_HURT, // ???
	TF_DMG_CUSTOM_DECAPITATION_BOSS, // HHH's Axe
	TF_DMG_CUSTOM_STICKBOMB_EXPLOSION,
	TF_DMG_CUSTOM_AEGIS_ROUND, // ???
	TF_DMG_CUSTOM_FLARE_EXPLOSION, // Detonador
	TF_DMG_CUSTOM_BOOTS_STOMP,
	TF_DMG_CUSTOM_PLASMA, // Cow Mangler this should also dissolve ragdoll
	TF_DMG_CUSTOM_PLASMA_CHARGED, // Cow Mangler Secondary Attack
	TF_DMG_CUSTOM_PLASMA_GIB, // turn gib instead of dissolve
	TF_DMG_CUSTOM_PRACTICE_STICKY, // Sticky Jumper
	TF_DMG_CUSTOM_EYEBALL_ROCKET,
	TF_DMG_CUSTOM_HEADSHOT_DECAPITATION,
	TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON, // Pyro's Rainbow Armageddon
	TF_DMG_CUSTOM_FLARE_PELLET, // Scorch Shot
	TF_DMG_CUSTOM_CLEAVER,
	TF_DMG_CUSTOM_CLEAVER_CRIT,
	TF_DMG_CUSTOM_SAPPER_RECORDER_DEATH,
	TF_DMG_CUSTOM_MERASMUS_PLAYER_BOMB,
	TF_DMG_CUSTOM_MERASMUS_GRENADE,
	TF_DMG_CUSTOM_MERASMUS_ZAP,
	TF_DMG_CUSTOM_MERASMUS_DECAPITATION,
	TF_DMG_CUSTOM_CANNONBALL_PUSH, // Double DONK
	TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF, // Unused Shred Alert Damage
	TF_DMG_CUSTOM_THROWABLE, // Unused throwable bread from expiration date.
	TF_DMG_CUSTOM_THROWABLE_KILL, // but this one deflectable
	TF_DMG_CUSTOM_SPELL_TELEPORT, // TELEPORT STUCK
	TF_DMG_CUSTOM_SPELL_SKELETON,
	TF_DMG_CUSTOM_SPELL_MIRV, // ???
	TF_DMG_CUSTOM_SPELL_METEOR,
	TF_DMG_CUSTOM_SPELL_LIGHTNING,
	TF_DMG_CUSTOM_SPELL_FIREBALL,
	TF_DMG_CUSTOM_SPELL_MONOCULUS, // RednBlu Monoculus
	TF_DMG_CUSTOM_SPELL_BLASTJUMP, // i guess this is just for recent damager.
	TF_DMG_CUSTOM_SPELL_BATS,
	TF_DMG_CUSTOM_SPELL_TINY, // TINY STUCK
	TF_DMG_CUSTOM_KART,
	TF_DMG_CUSTOM_GIANT_HAMMER, // Necro Smasher
	TF_DMG_CUSTOM_RUNE_REFLECT, // Reflect Powerup
	TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE, // burning to death
	TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING, // hitting to death
	TF_DMG_CUSTOM_SLAP_KILL, // SLAP KILL!
	TF_DMG_CUSTOM_CROC, // Mercenary Park Crocodiles
	TF_DMG_CUSTOM_TAUNTATK_GASBLAST, // Fart of death
	TF_DMG_CUSTOM_AXTINGUISHER_BOOSTED, // smack to death
	// put new custom dmg here.
	LFE_DMG_CUSTOM_LEECHES, // ocean mann become skeleton mann
	LFE_DMG_CUSTOM_JEEP,
	LFE_DMG_CUSTOM_AIRBOAT,
	LFE_DMG_CUSTOM_JALOPY,
	LFE_DMG_CUSTOM_HUNTER_FLECHETTE,
	LFE_DMG_CUSTOM_HUNTER_FLECHETTE_EXPLODE,
	LFE_DMG_CUSTOM_ANTLION_WORKER_EXPLODE,
	LFE_DMG_CUSTOM_PHYSCANNON_MEGA,
	LFE_DMG_CUSTOM_PHYSCANNON_MEGA_TERTIARY,
};

// Crit types
enum ECritType
{
	kCritType_None,
	kCritType_MiniCrit,
	kCritType_Crit
};

#define TF_JUMP_ROCKET	( 1 << 0 )
#define TF_JUMP_STICKY	( 1 << 1 )
#define TF_JUMP_OTHER	( 1 << 2 )

enum
{
	TF_COLLISIONGROUP_GRENADES = LAST_SHARED_COLLISION_GROUP,
	TFCOLLISION_GROUP_OBJECT,
	TFCOLLISION_GROUP_OBJECT_SOLIDTOPLAYERMOVEMENT,
	TFCOLLISION_GROUP_COMBATOBJECT,
	TFCOLLISION_GROUP_ROCKETS,		// Solid to players, but not player movement. ensures touch calls are originating from rocket
	TFCOLLISION_GROUP_RESPAWNROOMS,
	TFCOLLISION_GROUP_PUMPKIN_BOMB,
	TFCOLLISION_GROUP_ARROWS,

	HL2COLLISION_GROUP_PLASMANODE,
	HL2COLLISION_GROUP_SPIT,
	HL2COLLISION_GROUP_HOMING_MISSILE,
	HL2COLLISION_GROUP_COMBINE_BALL,

	HL2COLLISION_GROUP_FIRST_NPC,
	HL2COLLISION_GROUP_HOUNDEYE,
	HL2COLLISION_GROUP_CROW,
	HL2COLLISION_GROUP_HEADCRAB,
	HL2COLLISION_GROUP_STRIDER,
	HL2COLLISION_GROUP_GUNSHIP,
	HL2COLLISION_GROUP_ANTLION,
	HL2COLLISION_GROUP_LAST_NPC,
	HL2COLLISION_GROUP_COMBINE_BALL_NPC,
};

//-----------------
// TF Objects Info
//-----------------

#define SENTRYGUN_UPGRADE_COST			130
#define SENTRYGUN_UPGRADE_METAL			200
#define SENTRYGUN_EYE_OFFSET_LEVEL_1	Vector( 0, 0, 32 )
#define SENTRYGUN_EYE_OFFSET_LEVEL_2	Vector( 0, 0, 40 )
#define SENTRYGUN_EYE_OFFSET_LEVEL_3	Vector( 0, 0, 46 )
#define SENTRYGUN_MAX_SHELLS_1			150
#define SENTRYGUN_MAX_SHELLS_2			200
#define SENTRYGUN_MAX_SHELLS_3			200
#define SENTRYGUN_MAX_ROCKETS			20
#define SENTRYGUN_BASE_RANGE			1100.0f

// Dispenser's maximum carrying capability
#define DISPENSER_MAX_METAL_AMMO		400
#define	MAX_DISPENSER_HEALING_TARGETS	32

//--------------------------------------------------------------------------
// OBJECTS
//--------------------------------------------------------------------------
enum
{
	OBJ_DISPENSER=0,
	OBJ_TELEPORTER,
	OBJ_SENTRYGUN,

	// Attachment Objects
	OBJ_ATTACHMENT_SAPPER,

	// If you add a new object, you need to add it to the g_ObjectInfos array 
	// in tf_shareddefs.cpp, and add it's data to the scripts/object.txt

	OBJ_LAST,
};

// Warning levels for buildings in the building hud, in priority order
typedef enum
{
	BUILDING_HUD_ALERT_NONE = 0,
	BUILDING_HUD_ALERT_LOW_AMMO,
	BUILDING_HUD_ALERT_LOW_HEALTH,
	BUILDING_HUD_ALERT_VERY_LOW_AMMO,
	BUILDING_HUD_ALERT_VERY_LOW_HEALTH,
	BUILDING_HUD_ALERT_SAPPER,	

	MAX_BUILDING_HUD_ALERT_LEVEL
} BuildingHudAlert_t;

typedef enum
{
	BUILDING_DAMAGE_LEVEL_NONE = 0,		// 100%
	BUILDING_DAMAGE_LEVEL_LIGHT,		// 75% - 99%
	BUILDING_DAMAGE_LEVEL_MEDIUM,		// 50% - 76%
	BUILDING_DAMAGE_LEVEL_HEAVY,		// 25% - 49%	
	BUILDING_DAMAGE_LEVEL_CRITICAL,		// 0% - 24%

	MAX_BUILDING_DAMAGE_LEVEL
} BuildingDamageLevel_t;

//--------------
// Scoring
//--------------

#define TF_SCORE_KILL							1
#define TF_SCORE_DEATH							0
#define TF_SCORE_CAPTURE						2
#define TF_SCORE_DEFEND							1
#define TF_SCORE_DESTROY_BUILDING				1
#define TF_SCORE_HEADSHOT						1
#define TF_SCORE_BACKSTAB						1
#define TF_SCORE_INVULN							1
#define TF_SCORE_REVENGE						1
#define TF_SCORE_KILL_ASSISTS_PER_POINT			2
#define TF_SCORE_TELEPORTS_PER_POINT			2	
#define TF_SCORE_HEAL_HEALTHUNITS_PER_POINT		600
#define TF_SCORE_BONUS_PER_POINT				1
#define TF_SCORE_REVIVE_PLAYER					2

//-------------------------
// Shared Teleporter State
//-------------------------
enum
{
	TELEPORTER_STATE_BUILDING = 0,				// Building, not active yet
	TELEPORTER_STATE_IDLE,						// Does not have a matching teleporter yet
	TELEPORTER_STATE_READY,						// Found match, charged and ready
	TELEPORTER_STATE_SENDING,					// Teleporting a player away
	TELEPORTER_STATE_RECEIVING,					
	TELEPORTER_STATE_RECEIVING_RELEASE,
	TELEPORTER_STATE_RECHARGING,				// Waiting for recharge
	TELEPORTER_STATE_UPGRADING
};

#define OBJECT_MODE_NONE			0
#define TELEPORTER_TYPE_ENTRANCE	0
#define TELEPORTER_TYPE_EXIT		1

#define TELEPORTER_RECHARGE_TIME				10		// seconds to recharge

extern float g_flTeleporterRechargeTimes[];
extern float g_flDispenserAmmoRates[];
extern float g_flDispenserHealRates[];

//-------------------------
// Shared Sentry State
//-------------------------
enum
{
	SENTRY_STATE_INACTIVE = 0,
	SENTRY_STATE_SEARCHING,
	SENTRY_STATE_ATTACKING,
	SENTRY_STATE_UPGRADING,
	SENTRY_STATE_WRANGLED,
	SENTRY_STATE_WRANGLED_RECOVERY,
	SENTRY_STATE_SAPPER_RECOVERY,

	SENTRY_NUM_STATES,
};

//--------------------------------------------------------------------------
// OBJECT FLAGS
//--------------------------------------------------------------------------
enum
{
	OF_ALLOW_REPEAT_PLACEMENT				= 0x01,
	OF_MUST_BE_BUILT_ON_ATTACHMENT			= 0x02,
	OF_IS_CART_OBJECT						= 0x04, //I'm not sure what the exact name is, but live tf2 uses it for the payload bomb dispenser object
	OF_CAN_BE_BUILT_ON_ATTACHMENT			= 0x08,

	OF_BIT_COUNT	= 5
};

//--------------------------------------------------------------------------
// Builder "weapon" states
//--------------------------------------------------------------------------
enum 
{
	BS_IDLE = 0,
	BS_SELECTING,
	BS_PLACING,
	BS_PLACING_INVALID
};


//--------------------------------------------------------------------------
// Builder object id...
//--------------------------------------------------------------------------
enum
{
	BUILDER_OBJECT_BITS = 8,
	BUILDER_INVALID_OBJECT = ((1 << BUILDER_OBJECT_BITS) - 1)
};

// Analyzer state
enum
{
	AS_INACTIVE = 0,
	AS_SUBVERTING,
	AS_ANALYZING
};

// Max number of objects a team can have
#define MAX_OBJECTS_PER_PLAYER	5
//#define MAX_OBJECTS_PER_TEAM	128

// sanity check that commands send via user command are somewhat valid
#define MAX_OBJECT_SCREEN_INPUT_DISTANCE	100

//--------------------------------------------------------------------------
// BUILDING
//--------------------------------------------------------------------------
// Build checks will return one of these for a player
enum
{
	CB_CAN_BUILD,			// Player is allowed to build this object
	CB_CANNOT_BUILD,		// Player is not allowed to build this object
	CB_LIMIT_REACHED,		// Player has reached the limit of the number of these objects allowed
	CB_NEED_RESOURCES,		// Player doesn't have enough resources to build this object
	CB_NEED_ADRENALIN,		// Commando doesn't have enough adrenalin to build a rally flag
	CB_UNKNOWN_OBJECT,		// Error message, tried to build unknown object
};

// Build animation events
#define TF_OBJ_ENABLEBODYGROUP			6000
#define TF_OBJ_DISABLEBODYGROUP			6001
#define TF_OBJ_ENABLEALLBODYGROUPS		6002
#define TF_OBJ_DISABLEALLBODYGROUPS		6003
#define TF_OBJ_PLAYBUILDSOUND			6004

#define TF_AE_CIGARETTE_THROW			7000

#define OBJECT_COST_MULTIPLIER_PER_OBJECT			3
#define OBJECT_UPGRADE_COST_MULTIPLIER_PER_LEVEL	3

//--------------------------------------------------------------------------
// Invasion Powerups
//--------------------------------------------------------------------------
enum
{
	POWERUP_BOOST,		// Medic, buff station
	POWERUP_EMP,		// Technician
	POWERUP_RUSH,		// Rally flag
	POWERUP_POWER,		// Object power
	MAX_POWERUPS
};

//--------------------------------------------------------------------------
// Stun
//--------------------------------------------------------------------------
#define TF_STUNFLAG_SLOWDOWN			(1<<0) // activates slowdown modifier
#define TF_STUNFLAG_BONKSTUCK			(1<<1) // bonk sound, stuck
#define TF_STUNFLAG_LIMITMOVEMENT		(1<<2) // disable forward/backward movement
#define TF_STUNFLAG_CHEERSOUND			(1<<3) // cheering sound
#define TF_STUNFLAG_NOSOUNDOREFFECT		(1<<4) // no sound or particle
#define TF_STUNFLAG_THIRDPERSON			(1<<5) // panic animation
#define TF_STUNFLAG_GHOSTEFFECT			(1<<6) // ghost particles
#define TF_STUNFLAG_BONKEFFECT			(1<<7) // sandman particles
#define TF_STUNFLAG_RESISTDAMAGE		(1<<8) // damage resist modifier
	
enum
{
	TF_STUNFLAGS_LOSERSTATE		= TF_STUNFLAG_THIRDPERSON | TF_STUNFLAG_SLOWDOWN | TF_STUNFLAG_NOSOUNDOREFFECT, // Currently unused
	TF_STUNFLAGS_GHOSTSCARE		= TF_STUNFLAG_THIRDPERSON | TF_STUNFLAG_GHOSTEFFECT, // Ghost stun
	TF_STUNFLAGS_SMALLBONK		= TF_STUNFLAG_THIRDPERSON | TF_STUNFLAG_SLOWDOWN | TF_STUNFLAG_BONKEFFECT, // Half stun
	TF_STUNFLAGS_NORMALBONK		= TF_STUNFLAG_BONKSTUCK, // Full stun
	TF_STUNFLAGS_BIGBONK		= TF_STUNFLAG_CHEERSOUND | TF_STUNFLAG_BONKSTUCK | TF_STUNFLAG_RESISTDAMAGE | TF_STUNFLAG_BONKEFFECT, // Moonshot
	TF_STUNFLAGS_COUNT // This doesn't really work with flags
};

enum
{
	STUN_PHASE_NONE,
	STUN_PHASE_LOOP,
	STUN_PHASE_END,
};

 //--------------------------------------------------------------------------
// Holiday
//--------------------------------------------------------------------------
enum EHoliday
{
	kHoliday_None = 1,
	kHoliday_TF2Birthday,
	kHoliday_Halloween,
	kHoliday_Christmas,
	kHoliday_CommunityUpdate,
	kHoliday_EOTL,
	kHoliday_ValentinesDay,
	kHoliday_MeetThePyro,
	kHoliday_FullMoon,
	kHoliday_HalloweenOrFullMoon,
	kHoliday_HalloweenOrFullMoonOrValentines,
	kHoliday_AprilFools,

	kHoliday_LFBirthday,
	kHoliday_NewYears,
	kHoliday_HLAlyx,

	kHolidayCount
};

//--------------------------------------------------------------------------
// Rage
//--------------------------------------------------------------------------
enum
{
	TF_BUFF_OFFENSE = 1,
	TF_BUFF_DEFENSE,
	TF_BUFF_REGENONDAMAGE,
	TF_BUFF_PARACHUTE,
	TF_BUFF_CRITBOOSTED,
	TF_BUFF_COUNT
};

#define	MAX_CABLE_CONNECTIONS 4

bool IsObjectAnUpgrade( int iObjectType );
bool IsObjectAVehicle( int iObjectType );
bool IsObjectADefensiveBuilding( int iObjectType );

class CHudTexture;

#define OBJECT_MAX_GIB_MODELS	9

class CObjectInfo
{
public:
	CObjectInfo( char *pObjectName );
	CObjectInfo( const CObjectInfo& obj ) {}
	~CObjectInfo();

	// This is initialized by the code and matched with a section in objects.txt
	char	*m_pObjectName;

	// This stuff all comes from objects.txt
	char	*m_pClassName;					// Code classname (in LINK_ENTITY_TO_CLASS).
	char	*m_pStatusName;					// Shows up when crosshairs are on the object.
	float	m_flBuildTime;
	int		m_nMaxObjects;					// Maximum number of objects per player
	int		m_Cost;							// Base object resource cost
	float	m_CostMultiplierPerInstance;	// Cost multiplier
	int		m_UpgradeCost;					// Base object resource cost for upgrading
	float	m_flUpgradeDuration;
	int		m_MaxUpgradeLevel;				// Max object upgrade level
	char	*m_pBuilderWeaponName;			// Names shown for each object onscreen when using the builder weapon
	char	*m_pBuilderPlacementString;		// String shown to player during placement of this object
	int		m_SelectionSlot;				// Weapon selection slots for objects
	int		m_SelectionPosition;			// Weapon selection positions for objects
	bool	m_bSolidToPlayerMovement;
	bool	m_bUseItemInfo;
	char    *m_pViewModel;					// View model to show in builder weapon for this object
	char    *m_pPlayerModel;				// World model to show attached to the player
	int		m_iDisplayPriority;				// Priority for ordering in the hud display ( higher is closer to top )
	bool	m_bVisibleInWeaponSelection;	// should show up and be selectable via the weapon selection?
	char	*m_pExplodeSound;				// gamesound to play when object explodes
	char	*m_pExplosionParticleEffect;	// particle effect to play when object explodes
	bool	m_bAutoSwitchTo;				// should we let players switch back to the builder weapon representing this?
	char	*m_pUpgradeSound;				// gamesound to play when upgrading
	int		m_BuildCount;					// ???
	bool	m_bRequiresOwnBuilder;			// ???

	CUtlVector< const char * > m_AltModes;

	// HUD weapon selection menu icon ( from hud_textures.txt )
	char	*m_pIconActive;
	char	*m_pIconInactive;
	char	*m_pIconMenu;

	// HUD building status icon
	char	*m_pHudStatusIcon;

	// gibs
	int		m_iMetalToDropInGibs;
};

// Loads the objects.txt script.
class IBaseFileSystem;
void LoadObjectInfos( IBaseFileSystem *pFileSystem );

// Get a CObjectInfo from a TFOBJ_ define.
const CObjectInfo* GetObjectInfo( int iObject );

// Object utility funcs
bool	ClassCanBuild( int iClass, int iObjectType );
int		CalculateObjectCost( int iObjectType, bool bMini = false /*, int iNumberOfObjects, int iTeam, bool bLast = false*/ );
int		CalculateObjectUpgrade( int iObjectType, int iObjectLevel );

// Shell ejections
enum
{
	EJECTBRASS_PISTOL,
	EJECTBRASS_MINIGUN,
};

// Win panel styles
enum
{
	WINPANEL_BASIC = 0,
};

#define TF_DEATH_ANIMATION_TIME			2.0

// Taunt attack types
enum taunts_t
{
	TAUNTATK_NONE = 0,
	TAUNTATK_PYRO_HADOUKEN,
	TAUNTATK_HEAVY_EAT, // Nom
	TAUNTATK_HEAVY_RADIAL_BUFF, // Unused
	TAUNTATK_SCOUT_DRINK,
	TAUNTATK_HEAVY_HIGH_NOON, // POW!
	TAUNTATK_SCOUT_GRAND_SLAM, // BONK!
	TAUNTATK_MEDIC_INHALE,
	TAUNTATK_SPY_FENCING_SLASH_A, // Just lay
	TAUNTATK_SPY_FENCING_SLASH_B, // Your weapon down
	TAUNTATK_SPY_FENCING_STAB, // And walk away.
	TAUNTATK_RPS_KILL,
	TAUNTATK_SNIPER_ARROW_STAB_IMPALE, // Stab stab
	TAUNTATK_SNIPER_ARROW_STAB_KILL, // STAB
	TAUNTATK_SOLDIER_GRENADE_KILL,
	TAUNTATK_DEMOMAN_BARBARIAN_SWING,
	TAUNTATK_MEDIC_UBERSLICE_IMPALE, // I'm going to saw
	TAUNTATK_MEDIC_UBERSLICE_KILL, // THROUGH YOUR BONES!
	TAUNTATK_FLIP_LAND_PARTICLE,
	TAUNTATK_RPS_PARTICLE,
	TAUNTATK_HIGHFIVE_PARTICLE,
	TAUNTATK_ENGINEER_GUITAR_SMASH,
	TAUNTATK_ENGINEER_ARM_IMPALE, // Grinder Start
	TAUNTATK_ENGINEER_ARM_KILL, // Grinder Kill
	TAUNTATK_ENGINEER_ARM_BLEND, // Grinder Stun and hurt
	TAUNTATK_SOLDIER_GRENADE_KILL_WORMSIGN,
	TAUNTATK_SHOW_ITEM,
	TAUNTATK_MEDIC_RELEASE_DOVES,
	TAUNTATK_PYRO_ARMAGEDDON,
	TAUNTATK_PYRO_SCORCHSHOT,
	TAUNTATK_ALLCLASS_GUITAR_RIFF,
	TAUNTATK_MEDIC_HEROIC_TAUNT,
	TAUNTATK_PYRO_GASBLAST,

	// LFE Taunt Attacks
	TAUNTATK_COMBINES_BASH,
	TAUNTATK_COMBINES_THROW_GRENADE,
	TAUNTATK_ANTLION_IMPALE,
	TAUNTATK_ANTLION_KILL,
	TAUNTATK_DEMOMAN_CABER,
	TAUNTATK_WATCHANDLEARN,
};

extern const char *g_aTauntsName[];
int GetTauntAttackByName( const char *pszTaunts );

typedef enum
{
	HUD_NOTIFY_YOUR_FLAG_TAKEN,
	HUD_NOTIFY_YOUR_FLAG_DROPPED,
	HUD_NOTIFY_YOUR_FLAG_RETURNED,
	HUD_NOTIFY_YOUR_FLAG_CAPTURED,

	HUD_NOTIFY_ENEMY_FLAG_TAKEN,
	HUD_NOTIFY_ENEMY_FLAG_DROPPED,
	HUD_NOTIFY_ENEMY_FLAG_RETURNED,
	HUD_NOTIFY_ENEMY_FLAG_CAPTURED,

	HUD_NOTIFY_TOUCHING_ENEMY_CTF_CAP,

	HUD_NOTIFY_NO_INVULN_WITH_FLAG,
	HUD_NOTIFY_NO_TELE_WITH_FLAG,

	HUD_NOTIFY_SPECIAL,

	HUD_NOTIFY_GOLDEN_WRENCH,

	HUD_NOTIFY_RD_ROBOT_ATTACKED,

	HUD_NOTIFY_HOW_TO_CONTROL_GHOST,
	HUD_NOTIFY_HOW_TO_CONTROL_KART,

	HUD_NOTIFY_PASSTIME_HOWTO,
	HUD_NOTIFY_PASSTIME_BALL_BASKET,
	HUD_NOTIFY_PASSTIME_BALL_ENDZONE,
	HUD_NOTIFY_PASSTIME_SCORE,
	HUD_NOTIFY_PASSTIME_FRIENDLY_SCORE,
	HUD_NOTIFY_PASSTIME_ENEMY_SCORE,
	HUD_NOTIFY_PASSTIME_NO_TELE,
	HUD_NOTIFY_PASSTIME_NO_CARRY,
	HUD_NOTIFY_PASSTIME_NO_INVULN,
	HUD_NOTIFY_PASSTIME_NO_DISGUISE,
	HUD_NOTIFY_PASSTIME_NO_CLOAK,
	HUD_NOTIFY_PASSTIME_NO_OOB,
	HUD_NOTIFY_PASSTIME_NO_HOLSTER,
	HUD_NOTIFY_PASSTIME_NO_TAUNT,

	NUM_STOCK_NOTIFICATIONS
} HudNotification_t;

class CTraceFilterIgnorePlayers : public CTraceFilterSimple
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS( CTraceFilterIgnorePlayers, CTraceFilterSimple );

	CTraceFilterIgnorePlayers( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
		return pEntity && !pEntity->IsPlayer();
	}
};

class CTraceFilterIgnoreTeammates : public CTraceFilterSimple
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS( CTraceFilterIgnoreTeammates, CTraceFilterSimple );

	CTraceFilterIgnoreTeammates( const IHandleEntity *passentity, int collisionGroup, int iIgnoreTeam )
		: CTraceFilterSimple( passentity, collisionGroup ), m_iIgnoreTeam( iIgnoreTeam )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );

		if ( pEntity->IsPlayer() && pEntity->GetTeamNumber() == m_iIgnoreTeam )
		{
			return false;
		}

		return true;
	}

	int m_iIgnoreTeam;
};

class CTraceFilterIgnoreTeammatesAndTeamObjects : public CTraceFilterSimple
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS( CTraceFilterIgnoreTeammatesAndTeamObjects, CTraceFilterSimple );

	CTraceFilterIgnoreTeammatesAndTeamObjects( const IHandleEntity *passentity, int collisionGroup, int teamNumber )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
		m_iTeamNumber = teamNumber;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );

		if ( pEntity && ( pEntity->GetTeamNumber() == m_iTeamNumber ) )
			return false;

		return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
	}

private:
	int m_iTeamNumber;
};

class CTraceFilterIgnoreFriendlyCombatItems : public CTraceFilterSimple
{
	DECLARE_CLASS_GAMEROOT( CTraceFilterIgnoreFriendlyCombatItems, CTraceFilterSimple );
public:
	CTraceFilterIgnoreFriendlyCombatItems( IHandleEntity const *ignore, int collissionGroup, int teamNumber )
		: CTraceFilterSimple( ignore, collissionGroup )
	{
		m_iTeamNumber = teamNumber;
		m_bSkipBaseTrace = false;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		if ( pEntity == nullptr )
			return false;

		if ( !pEntity->IsCombatItem() )
			return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );

		if ( pEntity->GetTeamNumber() == m_iTeamNumber )
			return false;

		if( !m_bSkipBaseTrace )
			return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );

		return true;
	}

	void AlwaysHitItems( void ) { m_bSkipBaseTrace = true; }

private:
	int m_iTeamNumber;
	bool m_bSkipBaseTrace;
};

// Unused death flags
#define TF_DEATH_FIRST_BLOOD	0x0010
#define TF_DEATH_FEIGN_DEATH	0x0020
#define TF_DEATH_GIB			0x0080
#define TF_DEATH_PURGATORY		0x0100
#define TF_DEATH_AUSTRALIUM		0x0400

#define HUD_ALERT_SCRAMBLE_TEAMS 0

// Third person camera settings
#define TF_CAMERA_DIST 64
#define TF_CAMERA_DIST_RIGHT 30
#define TF_CAMERA_DIST_UP 0

inline int GetEnemyTeam( CBaseEntity *ent )
{
	int myTeam = ent->GetTeamNumber();
	return ( myTeam == TF_TEAM_BLUE ? TF_TEAM_RED : ( myTeam == TF_TEAM_RED ? TF_TEAM_BLUE : TEAM_ANY ) );
}

bool IsSpaceToSpawnHere( const Vector& pos );

void BuildBigHeadTransformation( CBaseAnimating *pAnimating, CStudioHdr *pStudio, Vector *pos, Quaternion *q, matrix3x4_t const &cameraTransformation, int boneMask, class CBoneBitList &boneComputed, float flScale );

#define TF_RESIST_SHIELD_MODEL "models/effects/resist_shield/resist_shield.mdl"
#define TF_REVIVEMARKER_MODEL "models/props_mvm/mvm_revive_tombstone.mdl"

#define IN_TYPING ( 1 << 31 )

extern const char *g_pszBreadModels[];

extern const char *g_pszClassModels[];
extern const char *g_pszRobotClassModels[];
extern const char *g_pszRobotBossClassModels[];
extern const char *g_pszTFCClassModels[];
extern const char *s_pszRuneIcons[];

#define TF_BOT_SENTRYBUSTER_MODEL "models/bots/demo/bot_sentry_buster.mdl"

#define FOR_EACH_PLAYER( code ) for ( int it = 0; it <= gpGlobals->maxClients; ++it )  {\
	CTFPlayer *pPlayer = ToTFPlayer(UTIL_PlayerByIndex( it ));\
	if ( !pPlayer ) continue;\
	code\
}
 #define FOR_EACH_PLAYER_TEAM( code, team ) for ( int it = 0; it <= gpGlobals->maxClients; ++it )  {\
	CTFPlayer *pPlayer = ToTFPlayer(UTIL_PlayerByIndex( it ));\
	if ( !pPlayer ) continue;\
	if ( team != TEAM_ANY && pPlayer->GetTeamNumber() != team ) continue;\
	code\
}

extern const char *g_aTFCondNames[];
int GetTFCondId( const char *pszTFCondName );

enum RuneTypes_t
{
	TF_RUNE_NONE = -1,
	TF_RUNE_STRENGTH = 0,
	TF_RUNE_HASTE,
	TF_RUNE_REGEN,
	TF_RUNE_RESIST,
	TF_RUNE_VAMPIRE,
	TF_RUNE_WARLOCK,
	TF_RUNE_PRECISION,
	TF_RUNE_AGILITY,
	TF_RUNE_KNOCKOUT,
	TF_RUNE_KING,
	TF_RUNE_PLAGUE,
	TF_RUNE_SUPERNOVA,
	TF_RUNE_COUNT
};

//======================== Portal ========================//

#define PORTAL_HALF_WIDTH 32.0f
#define PORTAL_HALF_HEIGHT 54.0f
#define PORTAL_HALF_DEPTH 2.0f
#define PORTAL_BUMP_FORGIVENESS 2.0f

#define PORTAL_ANALOG_SUCCESS_NO_BUMP 1.0f
#define PORTAL_ANALOG_SUCCESS_BUMPED 0.3f
#define PORTAL_ANALOG_SUCCESS_CANT_FIT 0.1f
#define PORTAL_ANALOG_SUCCESS_CLEANSER 0.028f
#define PORTAL_ANALOG_SUCCESS_OVERLAP_LINKED 0.027f
#define PORTAL_ANALOG_SUCCESS_NEAR 0.0265f
#define PORTAL_ANALOG_SUCCESS_INVALID_VOLUME 0.026f
#define PORTAL_ANALOG_SUCCESS_INVALID_SURFACE 0.025f
#define PORTAL_ANALOG_SUCCESS_PASSTHROUGH_SURFACE 0.0f

#define MIN_FLING_SPEED 300

#define PORTAL_HIDE_PLAYER_RAGDOLL 1

enum PortalFizzleType_t
{
	PORTAL_FIZZLE_SUCCESS = 0,			// Placed fine (no fizzle)
	PORTAL_FIZZLE_CANT_FIT,
	PORTAL_FIZZLE_OVERLAPPED_LINKED,
	PORTAL_FIZZLE_BAD_VOLUME,
	PORTAL_FIZZLE_BAD_SURFACE,
	PORTAL_FIZZLE_KILLED,
	PORTAL_FIZZLE_CLEANSER,
	PORTAL_FIZZLE_CLOSE,
	PORTAL_FIZZLE_NEAR_BLUE,
	PORTAL_FIZZLE_NEAR_RED,
	PORTAL_FIZZLE_NONE,

	NUM_PORTAL_FIZZLE_TYPES
};


enum PortalPlacedByType
{
	PORTAL_PLACED_BY_FIXED = 0,
	PORTAL_PLACED_BY_PEDESTAL,
	PORTAL_PLACED_BY_PLAYER
};

enum PortalLevelStatType
{
	PORTAL_LEVEL_STAT_NUM_PORTALS = 0,
	PORTAL_LEVEL_STAT_NUM_STEPS,
	PORTAL_LEVEL_STAT_NUM_SECONDS,

	PORTAL_LEVEL_STAT_TOTAL
};

enum PortalChallengeType
{
	PORTAL_CHALLENGE_NONE = 0,
	PORTAL_CHALLENGE_PORTALS,
	PORTAL_CHALLENGE_STEPS,
	PORTAL_CHALLENGE_TIME,

	PORTAL_CHALLENGE_TOTAL
};

enum MedicCallerType
{
	TF_CALL_NONE = 0,
	TF_CALL_MEDIC = 1,		// E
	TF_CALL_BURNING,		// on fire
	TF_CALL_HEALTH,			// low health
	TF_CALL_BLEEDING,		// bleeding
	TF_CALL_AUTO,			// auto call
	TF_CALL_REVIVE_EASY,	// just died
	TF_CALL_REVIVE_MEDIUM,	// wait for med
	TF_CALL_REVIVE_HARD,	// bout to respawn

	TF_CALL_COUNT
};

enum TF_PingTypes_t
{
	TF_PING_NONE = 0,
	TF_PING_SPY = 1,
	TF_PING_SENTRY,
	TF_PING_DISPENSER,
	TF_PING_TELEPORTER,
	TF_PING_GO,
	TF_PING_HELP,

	TF_PING_COUNT
};

enum ProjectileType_t
{
	TF_PROJECTILETYPE_ARROW = 0,
	TF_PROJECTILETYPE_HEALING_BOLT = 1,
	TF_PROJECTILETYPE_BUILDING_BOLT,
	TF_PROJECTILETYPE_ARROW_FESTIVE,
	TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE,
	TF_PROJECTILETYPE_HOOK,

	TF_NUM_PROJECTILETYPES
};

extern char *g_ppszPortalPassThroughMaterials[];

const char *GetPowerupIconName( RuneTypes_t RuneType, int iTeam );

// Medikit
#define WEAP_MEDIKIT_OVERHEAL 50 // Amount of superhealth over max_health the medikit will dispense
#define WEAP_MEDIKIT_HEAL	200  // Amount medikit heals per hit

extern const char *g_pszBDayGibs[22];

extern const char *g_pszEconHolidayNames[kHolidayCount];

//-----------------------------------------------------------------------------
// Credits Files.
//-----------------------------------------------------------------------------
#define CREDITS_BASE_FILE "scripts/credits/credits.txt"
#define CREDITS_HL2_FILE "scripts/credits/credits_hl2.txt"
#define CREDITS_EP1_FILE "scripts/credits/credits_ep1.txt"
#define CREDITS_EP2_FILE "scripts/credits/credits_ep2.txt"
#define CREDITS_HLS_FILE "scripts/credits/credits_hls.txt"
#define CREDITS_PORTAL_FILE "scripts/credits/credits_portal.txt"

#endif // TF_SHAREDDEFS_H
