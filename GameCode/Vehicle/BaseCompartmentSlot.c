class BaseCompartmentSlot : ExtBaseCompartmentSlot
{
	[Attribute("0", desc: "Switching seats is allowed only between compartments with matching section")]
	protected int m_iCompartmentSection;

	[Attribute("0", desc: "Set whether compartment is water tight. People inside may drown if false")]
	protected bool m_bIsWaterTight;

	[Attribute(desc: "Contains Default Prefab of character to be spawned into compartment", params: "et")]
	protected ref SCR_DefaultOccupantData m_DefaultOccupantData;

	static const vector SPAWN_IN_VEHICLE_OFFSET = Vector(0, 250, 0); //~ Offset added when characters are spawned to add to vehicle. To make sure they are close and streamed but not on the vehicle as this would kill them

	private bool m_bCompartmentAccessible = true;

	//------------------------------------------------------------------------------------------------
	override void DebugDrawPosition()
	{
		Color c = Color.White;
		if (!m_bCompartmentAccessible)
			c = Color.Black;
		vector pos = GetPosition();
		if (pos != vector.Zero)
			Shape.CreateCylinder(c.PackToInt(), ShapeFlags.ONCE, GetPosition(), 0.05, 5);
	}

	//------------------------------------------------------------------------------------------------
	bool IsCompartmentAccessible()
	{
		return m_bCompartmentAccessible;
	}

	//------------------------------------------------------------------------------------------------
	void SetCompartmentAccessible(bool val)
	{
		m_bCompartmentAccessible = val;
	}

	//------------------------------------------------------------------------------------------------
	//! Switching seats is allowed only between compartments with matching area
	int GetCompartmentSection()
	{
		return m_iCompartmentSection;
	}

	/*!
	Get type of a compartment.
	\return Compartment type, or -1 if invalid
	*/
	ECompartmentType GetType()
	{
		switch (Type())
		{
			case CargoCompartmentSlot: return ECompartmentType.Cargo;
			case PilotCompartmentSlot: return ECompartmentType.Pilot;
			case TurretCompartmentSlot:	return ECompartmentType.Turret;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool GetIsWaterTight()
	{
		return m_bIsWaterTight;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get vehicle this slot belongs to.
	May differ from GetOwner() if the slot is placed in vehicle's child entity, e.g., truck's cargo frame.
	\return Vehicle entity
	*/
	IEntity GetVehicle()
	{
		IEntity owner = GetOwner();
		IEntity vehicle;
		while (owner)
		{
			if (owner.FindComponent(BaseCompartmentManagerComponent))
				vehicle = owner;

			owner = owner.GetParent();
		}
		return vehicle;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get vehicle this slot belongs to.
	May differ from GetOwner() if the slot is placed in vehicle's child entity, e.g., truck's cargo frame.
	\param[out] compartmentID Variable to be filled with ID of the compartment relative to the vehicle. May differ from GetCompartmentSlotID() which returns only ID within the entity the slot belong sto.
	\return Vehicle entity
	*/
	IEntity GetVehicle(out int compartmentID)
	{
		IEntity owner = GetOwner();
		IEntity vehicle;
		Managed component, componentCandidate;
		while (owner)
		{
			componentCandidate = owner.FindComponent(BaseCompartmentManagerComponent);
			if (componentCandidate)
			{
				vehicle = owner;
				component = componentCandidate;
			}

			owner = owner.GetParent();
		}
		if (vehicle)
		{
			BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(component);
			array<BaseCompartmentSlot> compartments = {};
			manager.GetCompartments(compartments);
			compartmentID = compartments.Find(this);
		}
		return vehicle;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get if compartment is occupied
	\return returns true if occupied else returns false
	*/
	bool IsOccupied()
	{
		return GetOccupant() != null;
	}

	//------------------------------------------------------------------------------------------------
	void DamageOccupant(float damage, EDamageType damageType, notnull Instigator instigator, bool gettingIn = false, bool gettingOut = false)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return;

		RplComponent rpl = character.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return;

		// Ignore characters that only began to get in the vehicle
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!gettingIn && access && access.IsGettingIn())
			return;

		if (!gettingOut && access && access.IsGettingOut())
			return;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (damageManager)
			damageManager.DamageRandomHitZones(damage, damageType, instigator);
	}

	//------------------------------------------------------------------------------------------------
	void KillOccupant(notnull Instigator instigator, bool eject = false, bool gettingIn = false, bool gettingOut = false)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!gettingIn && access && access.IsGettingIn())
			return;

		if (!gettingOut && access && access.IsGettingOut())
			return;

		if (eject && access)
		{
			access.EjectOutOfVehicle();
			controller.GetInputContext().SetVehicleCompartment(null);
			// because person needs to be out of vehicle for sure
			GetGame().GetCallqueue().CallLater(damageManager.Kill, 1, false, instigator);
		}
		else
		{
			damageManager.Kill(instigator);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get default occupent prefab data
	\return Default occupant data
	*/
	SCR_DefaultOccupantData GetDefaultOccupantData()
	{
		return m_DefaultOccupantData;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get default occupent prefab ResourceName
	\return Default occupant prefab
	*/
	ResourceName GetDefaultOccupantPrefab(bool checkIfValid = true)
	{
		if (!m_DefaultOccupantData || (checkIfValid && !m_DefaultOccupantData.IsValid()))
			return string.Empty;

		return m_DefaultOccupantData.GetDefaultOccupantPrefab();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Spawn default character within the compartment (Server only)
	Default characters are defined on the CompartmentSlot
	\param[inout] group if Left empty it will create a new group and place the character in it and return it. Else it will place the character in the given group
	\param groupPrefabGroup prefab, Generally want to keep it as default value as faction and usch is set automaticly
	\return Returns spawned character
	*/
	IEntity SpawnDefaultCharacterInCompartment(inout AIGroup group, ResourceName groupPrefab = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")
	{
		return SpawnCharacterInCompartment(GetDefaultOccupantPrefab(), group, groupPrefab);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Spawn character within the compartment (Server only)
	\param characterPrefab Prefab to spawn in compartment
	\param[inout] group if Left empty it will create a new group and place the character in it and return it. Else it will place the character in the given group
	\param createGroupForCharacter If a group should be created for the spawned character
	\return Returns spawned character
	*/
	IEntity SpawnCharacterInCompartment(ResourceName characterPrefab, inout AIGroup group, ResourceName groupPrefab = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")
	{
		if (characterPrefab.IsEmpty() || !IsCompartmentAccessible() || GetOccupant() != null)
			return null;

		//~ Spawn at vehicle position
		EntitySpawnParams params = new EntitySpawnParams();
		GetOwner().GetTransform(params.Transform);

		//~ Spawn characters above the vehicle to make sure physics do not interact with the character and they die on contact of the vehicle
		params.Transform[3] + SPAWN_IN_VEHICLE_OFFSET;

		IEntity spawnedCharacter = GetGame().SpawnEntityPrefab(Resource.Load(characterPrefab), params: params);
		if (!spawnedCharacter)
		{
			Print(string.Format("'BaseCompartmentSlot' Failed to spawn character in compartment. Check if ResourceName is correct! Path: '%1'", characterPrefab), LogLevel.ERROR);
			return null;
		}

		AIControlComponent agentControlComponent = AIControlComponent.Cast(spawnedCharacter.FindComponent(AIControlComponent));
		if (!agentControlComponent)
		{
			Print("'BaseCompartmentSlot' Could not get AIControlComponent from spawned character, so entity is deleted!", LogLevel.ERROR);
			delete spawnedCharacter;
			return null;
		}

		agentControlComponent.ActivateAI();

		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(spawnedCharacter.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
		{
			Print("'BaseCompartmentSlot' Could not get CompartmentAccessComponent from spawned character, so entity is deleted!", LogLevel.ERROR);
			delete spawnedCharacter;
			return null;
		}

		//~ Could not move in compartment so delete
		if (!compartmentAccess.MoveInVehicle(GetOwner(), this))
		{
			Print(string.Format("'BaseCompartmentSlot' Trying to spawn character in compartment but it could not be moved into it so character is deleted. Compartment type: %1", typename.EnumToString(ECompartmentType, SCR_CompartmentAccessComponent.GetCompartmentType(this))), LogLevel.WARNING);
			delete spawnedCharacter;
			return null;
		}

		//~ Create new group
		if (!group)
		{
			IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(groupPrefab));
			if (!groupEntity)
			{
				Print("'BaseCompartmentSlot' Could not create group for spawned character. Most likly incorrect group prefab given!", LogLevel.ERROR);
				return spawnedCharacter;
			}

			group = AIGroup.Cast(groupEntity);
			if (!group)
			{
				Print("'BaseCompartmentSlot' Could not create group for spawned character as spawned group prefab is missing AIGroup component!", LogLevel.ERROR);
				delete groupEntity;
				return spawnedCharacter;
			}

			//~ Set group's faction
			SCR_AIGroup groupScripted = SCR_AIGroup.Cast(group);
			if (groupScripted)
			{
				FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(spawnedCharacter.FindComponent(FactionAffiliationComponent));
				if (factionComponent)
				{
					Faction faction = factionComponent.GetAffiliatedFaction();
					if (faction)
						groupScripted.InitFactionKey(faction.GetFactionKey());
				}
			}
		}

		//~ Add the entity to the group
		group.AddAgent(agentControlComponent.GetControlAIAgent());

		return spawnedCharacter;
	}
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_sDefaultOccupantPrefab")]
class SCR_DefaultOccupantData
{
	[Attribute(desc: "Default Prefab of character to be spawned into compartment", params: "et")]
	protected ResourceName m_sDefaultOccupantPrefab;

	[Attribute("1", desc: "This entry will be considered empty and is ignored if flase")]
	protected bool m_bEnabled;

	ResourceName GetDefaultOccupantPrefab()
	{
		return m_sDefaultOccupantPrefab;
	}

	bool IsValid()
	{
		return m_bEnabled && !m_sDefaultOccupantPrefab.IsEmpty();
	}
}
