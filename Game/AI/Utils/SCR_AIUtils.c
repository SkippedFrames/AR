// gets a string from value given in port
// basically workaround that we can't cast void from GetVariableIn into type of that variable
//------------------------------------------------------------------------------------------------
static string SCR_AIGetStringFromPort(Node node, string port)
{
	const string UNASSIGNED_VARIABLE = "Unassigned";
	if ( node.GetVariableType(true, port) == string )
	{
		string value;
		if(!node.GetVariableIn(port, value))
			value = UNASSIGNED_VARIABLE;
		return value;		
	}
	else if ( node.GetVariableType(true, port) == int )
	{
		int value;
		if(!node.GetVariableIn(port, value))
			return UNASSIGNED_VARIABLE;
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port) == float )
	{
		float value;
		if(!node.GetVariableIn(port, value))
			return UNASSIGNED_VARIABLE;
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port) == bool )
	{
		bool value;
		if(!node.GetVariableIn(port, value))
			return UNASSIGNED_VARIABLE;
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port) == vector )
	{
		vector value;
		if(!node.GetVariableIn(port, value))
			return UNASSIGNED_VARIABLE;
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port).IsInherited(Managed) )
	{
		Managed value;
		if(!node.GetVariableIn(port, value))
			return UNASSIGNED_VARIABLE;
		if (value)
			return value.ToString();
		else 
			return "NULL";
	}
	return "";
};

// use this for generating random numbers for AI, no need to create another instance
//------------------------------------------------------------------------------------------------------------------------------------------------------------
static ref RandomGenerator s_AIRandomGenerator = new RandomGenerator;

// use this for interpretting relevant character stance based on threat level
//------------------------------------------------------------------------------------------------
static ECharacterStance GetStanceFromThreat(EAIThreatState threatState)
{
	switch (threatState)
	{
		case EAIThreatState.THREATENED: return ECharacterStance.PRONE;
		case EAIThreatState.ALERTED: return ECharacterStance.CROUCH;
		case EAIThreatState.VIGILANT: return ECharacterStance.STAND;
		case EAIThreatState.SAFE: return ECharacterStance.STAND;
		default: return ECharacterStance.STAND;
	}
	return ECharacterStance.STAND;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Compartments on vehicles
class SCR_AICompartmentHandling
{
	//--------------------------------------------------------------------------------------------
	static ECompartmentType CompartmentClassToType(typename type)
	{
		switch (type)
		{
			case PilotCompartmentSlot:	return ECompartmentType.Pilot;
			case CargoCompartmentSlot: 	return ECompartmentType.Cargo;
			case TurretCompartmentSlot:	return ECompartmentType.Turret;
		}
		return -1;
	}
	
	//--------------------------------------------------------------------------------------------
	static bool FindAvailableCompartmentInVehicles (array<IEntity> vehicles, ECompartmentType roleInVehicle, out BaseCompartmentSlot compartmentOut, out IEntity vehicleOut)
	{
		foreach (IEntity vehicle: vehicles)
		{
			BaseCompartmentManagerComponent compartmentMan = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
			if (!Vehicle.Cast(vehicle) || !compartmentMan)
				return false;
			ref array<BaseCompartmentSlot> compartments = {};
			compartmentMan.GetCompartments(compartments);
			foreach (BaseCompartmentSlot compartment: compartments)
			{
				if (SCR_AICompartmentHandling.CompartmentClassToType(compartment.Type()) == roleInVehicle)
				{
					if (!compartment.GetOccupant() && compartment.IsCompartmentAccessible())
					{
						compartmentOut = compartment;
						vehicleOut = vehicle;
						return true;
					}
				}
			}
		}
		return false;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Weapon Handling
class SCR_AIWeaponHandling
{
	//--------------------------------------------------------------------------------------------
	static int GetCurrentMuzzleId(BaseWeaponManagerComponent weapMgr)
	{
		BaseWeaponComponent weaponComp = weapMgr.GetCurrentWeapon();
		
		if (!weaponComp)
			return -1;
				
		// Find muzzle ID
		array<BaseMuzzleComponent> muzzles = {};
		weaponComp.GetMuzzlesList(muzzles);
		BaseMuzzleComponent currentMuzzle = weaponComp.GetCurrentMuzzle();
		
		if (!currentMuzzle || muzzles.IsEmpty())
			return -1;
		
		int currentMuzzleId = muzzles.Find(currentMuzzle);
		return currentMuzzleId;
	}
	
	//--------------------------------------------------------------------------------------------
	static BaseMagazineComponent GetCurrentMagazineComponent(BaseWeaponManagerComponent weapMgr)
	{
		BaseWeaponComponent weaponComp = weapMgr.GetCurrentWeapon();
		
		if (!weaponComp)
			return null;
		
		BaseMuzzleComponent	currentMuzzle = weaponComp.GetCurrentMuzzle();
		if (!currentMuzzle)
			return null;
		
		return currentMuzzle.GetMagazine();
	}
	
	//--------------------------------------------------------------------------------------------
	//! Although trivial, there are several ways to get current weapon, thus let's keep this function
	static BaseWeaponComponent GetCurrentWeaponComponent(BaseWeaponManagerComponent weapMgr)
	{
		return weapMgr.GetCurrentWeapon();
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMuzzleSwitch(CharacterControllerComponent controller, int newMuzzleId)
	{
		controller.SetMuzzle(newMuzzleId);
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMagazineSwitchCharacter(CharacterControllerComponent controller, BaseMagazineComponent newMagazineComp)
	{
		controller.ReloadWeaponWith(newMagazineComp.GetOwner());
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMagazineSwitchTurret(TurretControllerComponent controller, BaseMagazineComponent newMagazineComp)
	{
		controller.DoReloadWeaponWith(newMagazineComp.GetOwner());
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartWeaponSwitchCharacter(CharacterControllerComponent controller, BaseWeaponComponent newWeaponComp)
	{
		controller.TryEquipRightHandItem(newWeaponComp.GetOwner(), EEquipItemType.EEquipTypeWeapon, false);
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartWeaponSwitchTurret(TurretControllerComponent controller, BaseWeaponComponent newWeaponComp, IEntity turretOperator)
	{
		// Find weapon slot which has this weapon
		IEntity newWeaponEntity = newWeaponComp.GetOwner();
		BaseWeaponManagerComponent weaponMgr = controller.GetWeaponManager();
		if (!weaponMgr)
			return;
		
		WeaponSlotComponent newWeaponSlot;
		array<WeaponSlotComponent> slots = {};
		weaponMgr.GetWeaponsSlots(slots);
		foreach (WeaponSlotComponent slot : slots)
		{
			if (slot.GetWeaponEntity() == newWeaponEntity)
				newWeaponSlot = slot;
		}
		
		if (!newWeaponSlot)
		{
			// There is no slot which has this weapon
			return;
		}
		
		controller.SelectWeapon(turretOperator, newWeaponSlot);
	}
}

class SCR_AIDamageHandling
{
	static bool IsAlive(IEntity entity)
	{
		if (!entity)
			return false;
		
		DamageManagerComponent damageManager;
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character)
			damageManager = character.GetDamageManager();
		else
			damageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		
		if (!damageManager)
			return true;
		
		return damageManager.GetState() != EDamageState.DESTROYED;
	}
	
	// This method abstracts away the internals of damage system for usage in various AI behaviors.
	static bool IsCharacterWounded(IEntity entity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return false;
		
		DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return false;
		
		return damageManager.IsDamagedOverTime(EDamageType.BLEEDING);
	}
}
