enum EEditableEntityLabel
{
	NONE = 0,
	
	ENTITYTYPE_OBJECT = 1,
	ENTITYTYPE_CHARACTER = 2,
	ENTITYTYPE_VEHICLE = 3,
	ENTITYTYPE_GROUP = 4,
	ENTITYTYPE_COMPOSITION = 5,
	ENTITYTYPE_ITEM = 6,
	ENTITYTYPE_SYSTEM = 7,
	
	FACTION_USSR = 10,
	FACTION_US = 11,
	FACTION_FIA = 12,
	FACTION_NONE = 13, //FACTION_ANY
	
	TRAIT_ARMOR = 20,
	TRAIT_ARMED = 21,
	TRAIT_UNARMED = 22,
	TRAIT_REFUELING = 23,
	TRAIT_MEDICAL = 24,
	TRAIT_ARMORPIERCING = 25,
	TRAIT_AMPHIBIOUS = 26,
	TRAIT_REARMING = 27,
	TRAIT_FORTIFICATION = 28,
	TRAIT_SERVICE = 29,
	TRAIT_SPAWNPOINT = 30,
	TRAIT_DESTRUCTABLE = 31,
	TRAIT_ILLUMINATION = 32,
	TRAIT_OPTICS = 33,
	TRAIT_SUPPRESSIVE = 34,
	TRAIT_RADIO = 35,
	TRAIT_PASSENGERS_SMALL = 36,
	TRAIT_PASSENGERS_LARGE = 37,
	TRAIT_EXPLOSIVE = 38,
	TRAIT_REPAIRING = 39,	
	
	ROLE_ANTITANK = 40,
	ROLE_GRENADIER = 41,
	ROLE_LEADER = 42,
	ROLE_MACHINEGUNNER = 43,
	ROLE_MEDIC = 44,
	ROLE_RADIOOPERATOR = 45,
	ROLE_RIFLEMAN = 46,
	ROLE_SHARPSHOOTER = 47,
	ROLE_AMMOBEARER = 48,
	ROLE_SCOUT = 49,
	
	VEHICLE_CAR = 50,
	VEHICLE_HELICOPTER = 51,
	VEHICLE_AIRPLANE = 52,
	VEHICLE_APC = 53,
	VEHICLE_TRUCK = 54,
	VEHICLE_TURRET = 55,
	
	GROUPTYPE_INFANTRY = 60,
	GROUPTYPE_MOTORIZED = 61,
	GROUPTYPE_AIRMOBILE = 62,
	
	GROUPSIZE_SMALL = 70,
	GROUPSIZE_MEDIUM = 71,
	GROUPSIZE_LARGE = 72,
	
	ITEMTYPE_MAGAZINE = 80,
	ITEMTYPE_THROWABLE = 81,
	ITEMTYPE_ITEM = 82,
	ITEMTYPE_WEAPON = 83,
	ITEMTYPE_CLOTHING = 84,
	
	SLOT_FLAT_SMALL = 90,
	SLOT_FLAT_MEDIUM = 91,
	SLOT_FLAT_LARGE = 92,
	SLOT_ROAD_SMALL = 93,
	SLOT_ROAD_MEDIUM = 94,
	SLOT_ROAD_LARGE = 95,
	
	THEME_AGRICULTURE = 100,
	THEME_AIRPORT = 101,
	THEME_COMMERCIAL = 102,
	THEME_CULTURAL = 103,
	THEME_FOREST = 104,
	THEME_HOUSES = 105,
	THEME_INDUSTRIAL = 106,
	THEME_INFRASTRUCTURE = 107,
	THEME_MILITARY = 108,
	THEME_RECREATION = 109,
	THEME_RUINS = 110,
	THEME_SERVICES = 111,
	THEME_SIGNS = 112,
	THEME_WALLS = 113,
	
	SIZE_XS = 120,
	SIZE_S = 121,
	SIZE_M = 122,
	SIZE_L = 123,
	SIZE_XL = 124,
	
	GAMELOGIC_RALLYPOINT = 130,
	
	//~ Traits continuation
	TRAIT_SUPPLYSTORAGE_SMALL = 200,
	TRAIT_SUPPLYSTORAGE_LARGE = 210,
	TRAIT_SUPPLYSTORAGE_FULL = 220,
	TRAIT_BASE = 230,
	TRAIT_MANAGEMENT_BASE = 240,
	TRAIT_MANAGEMENT_SQUAD = 250,
	TRAIT_MANAGEMENT_VEHICLE = 260,
	TRAIT_HELI_CREW = 270,
	TRAIT_ARSENAL = 280,
	
	SERVICE_ANTENNA = 901,
	SERVICE_ARMORY = 902,
	SERVICE_FUEL = 903,
	SERVICE_HQ = 904,
	SERVICE_LIVING_AREA = 905, 
	SERVICE_SUPPLY_STORAGE = 906,
	SERVICE_VEHICLE_DEPOT_LIGHT = 907,
	SERVICE_VEHICLE_DEPOT_HEAVY = 908,
	SERVICE_FIELD_HOSPITAL = 909,
	SERVICE_HELIPAD = 910,
	SERVICE_REPAIR = 911,
	SERVICE_PLAYER_HUB = 912,
	
	//~ Content Traits
	//CONTENT_BASEGAME = 1000,
	CONTENT_MODDED = 1001,
};

enum EEditableEntityLabelGroup
{
	NONE = 0,
	ENTITYTYPE = 1,
	FACTION = 2,
	TRAIT = 3,
	ROLE = 4,
	VEHICLE = 5,
	GROUPTYPE = 6,
	GROUPSIZE = 7,
	ITEMTYPE = 8,
	SLOT = 9,
	//10
	THEME = 11,
	SIZE = 12,
	CONTENT = 13,
};