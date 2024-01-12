#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
class SCR_DestructionBaseComponentClass: ScriptedDamageManagerComponentClass
{
	[Attribute("100", UIWidgets.Slider, "Base health value of the object. Damage received above this value results in destruction (overrides HPMax in hit zone)", "0.01 100000 0.01", category: "Destruction Setup")]
	float m_fBaseHealth;
	[Attribute("50", UIWidgets.Slider, "Relative contact force to damage multiplier", "0.01 50000 0.01", category: "Destruction Setup")]
	float m_fForceToDamageScale;
	[Attribute("0.05", UIWidgets.Slider, "Contact momentum to damage multiplier", "0.01 10 0.01", category: "Destruction Setup")]
	float m_fMomentumToDamageScale;
	[Attribute("0.5", UIWidgets.Slider, "Minimum relative contact force (impulse / mass) threshold below which contacts are ignored", "0 50000 0.01", category: "Destruction Setup")]
	float m_fRelativeContactForceThresholdMinimum;
	[Attribute("3000", UIWidgets.Slider, "Maximum damage threshold above which the object is completely destroyed and no effects are played (eg: nuclear bomb damage)", "0.01 50000 0.01", category: "Destruction Setup")]
	float m_fDamageThresholdMaximum;
	[Attribute("0", desc: "Should children destructible objects also receive the damage dealt to this one?", category: "Destruction Setup")]
	bool m_bPassDamageToChildren;
	[Attribute("0", desc: "Should the parent of this object also be destroyed when this object gets destroyed?", category: "Destruction Setup")]
	bool m_bDestroyParentWhenDestroyed;
	
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the object", category: "Destruction FX")]
	ref array<ref SCR_BaseSpawnable> m_DestroySpawnObjects;
};

//------------------------------------------------------------------------------------------------
//! Base destruction component, destruction types extend from this
class SCR_DestructionBaseComponent : ScriptedDamageManagerComponent
{
#ifdef ENABLE_BASE_DESTRUCTION
	static protected ref ScriptInvoker s_OnDestructibleDestroyed = new ScriptInvoker();
	protected static bool s_bReadingInit = false; //Used to determine whether we are in gameplay or synchronization state
	
#ifdef WORKBENCH
	protected static bool s_bPrintMissingComponent;
	protected static bool s_bPrintMissingPlayerController;
	protected static bool s_bPrintInitializationFailed;
#endif
	
	private static int s_iFirstFreeDestructionBaseData = -1;
	private static ref array<ref SCR_DestructionBaseData> s_aDestructionBaseData = {};
	
	private int m_iDestructionBaseDataIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnDestructibleDestroyedInvoker() { return s_OnDestructibleDestroyed; }
	
	//------------------------------------------------------------------------------------------------
	static bool GetReadingInit(bool readingInit)
	{
		return s_bReadingInit;
	}
	
	//------------------------------------------------------------------------------------------------
	static void SetReadingInit(bool readingInit)
	{
		s_bReadingInit = readingInit;
	}
	
	//------------------------------------------------------------------------------------------------
	protected notnull SCR_DestructionBaseData GetDestructionBaseData()
	{
		if (m_iDestructionBaseDataIndex == -1)
			m_iDestructionBaseDataIndex = AllocateDestructionBaseData();
		
		return s_aDestructionBaseData[m_iDestructionBaseDataIndex];
	}
	
	//------------------------------------------------------------------------------------------------
	private int AllocateDestructionBaseData()
	{
		if (s_iFirstFreeDestructionBaseData == -1)
			return s_aDestructionBaseData.Insert(new SCR_DestructionBaseData());
		else
		{
			int returnIndex = s_iFirstFreeDestructionBaseData;
			SCR_DestructionBaseData data = s_aDestructionBaseData[returnIndex];
			s_iFirstFreeDestructionBaseData = data.m_iNextFreeIndex;
			data.m_iNextFreeIndex = -1;
			return returnIndex;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void FreeDestructionBaseData(int index)
	{
		s_aDestructionBaseData[index].Reset();
		s_aDestructionBaseData[index].m_iNextFreeIndex = s_iFirstFreeDestructionBaseData;
		s_iFirstFreeDestructionBaseData = index;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the on damage script invoker if one exists, otherwise creates one and returns it
	ScriptInvoker GetOnDamageInvoker()
	{
		return GetDestructionBaseData().GetOnDamage();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DestructionHitInfo GetDestructionHitInfo(bool createNew = false)
	{
		if (m_iDestructionBaseDataIndex == -1 && !createNew)
			return null;
		
		return GetDestructionBaseData().GetHitInfo(createNew);
	}
	
	//------------------------------------------------------------------------------------------------
	bool ShouldDestroyParent()
	{
		SCR_DestructionBaseComponentClass prefabData = SCR_DestructionBaseComponentClass.Cast(GetComponentData(GetOwner()));
		return prefabData && prefabData.m_bDestroyParentWhenDestroyed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the object should disable physics on destruction (before being deleted or changed)
	bool GetDisablePhysicsOnDestroy()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the object is undamaged
	bool GetUndamaged()
	{
		if (m_iDestructionBaseDataIndex == -1)
			return GetHealth() == 1;
		
		return !GetDestructionBaseData().GetHitInfo(false) && GetHealth() == 1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the object is destroyed
	bool GetDestroyed()
	{
		if (m_iDestructionBaseDataIndex == -1)
			return GetHealth() <= 0;
		
		return GetDestructionBaseData().GetHitInfo(false) || GetHealth() <= 0;
	}
	
	//------------------------------------------------------------------------------------------------
	// Spawns destroy objects of children as well as own
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void RPC_DoSpawnAllDestroyEffects()
	{
		SpawnDestroyObjects(new SCR_HitInfo());
		
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			SCR_DestructionBaseComponent destructible = SCR_DestructionBaseComponent.Cast(child.FindComponent(SCR_DestructionBaseComponent));
			if (destructible)
				destructible.SpawnDestroyObjects(new SCR_HitInfo());
			
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteParentWithEffects()
	{
		if (GetDestructionBaseData().GetDestructionQueued())
			return; // Destruction queued already, don't do this.
		
		RPC_DoSpawnAllDestroyEffects();
		Rpc(RPC_DoSpawnAllDestroyEffects);
		
		DeleteDestructible();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used in cases where we don't care about effects or anything, e. g. building destruction
	void DeleteDestructible()
	{
		if (ShouldDestroyParent())
		{
			IEntity parent = GetOwner().GetParent();
			if (parent)
			{
				SCR_DestructionBaseComponent destructible = SCR_DestructionBaseComponent.Cast(parent.FindComponent(SCR_DestructionBaseComponent));
				if (destructible)
				{
					destructible.DeleteParentWithEffects();
					return;
				}
			}
		}
		
		GetDestructionBaseData().SetDestructionQueued(true);
		RegenerateNavmeshDelayed();
		RplComponent.DeleteRplEntity(GetOwner(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the base health value of the hit zone (does not trigger destruction)
	void SetHitZoneHealth(float baseHealth, bool clearDamage = true)
	{
		HitZone hitZone = GetDefaultHitZone();
		if (!hitZone)
			return;
		
		hitZone.SetMaxHealth(baseHealth, ESetMaxHealthFlags.FULLHEAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets damage of the hit zone (does not trigger destruction)
	void SetHitZoneDamage(float damage)
	{
		HitZone hitZone = GetDefaultHitZone();
		if (!hitZone)
			return;
		
		hitZone.SetHealth(hitZone.GetMaxHealth() - damage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates and fills destruction hit info for the object
	void CreateDestructionHitInfo(bool totalDestruction, float lastHealth, float hitDamage, EDamageType damageType, vector hitPosition, vector hitDirection, vector hitNormal)
	{
		SCR_DestructionHitInfo hitInfo = GetDestructionBaseData().GetHitInfo();
		
		hitInfo.m_TotalDestruction = totalDestruction;
		hitInfo.m_LastHealth = lastHealth;
		hitInfo.m_HitDamage = hitDamage;
		hitInfo.m_DamageType = damageType;
		hitInfo.m_HitPosition = hitPosition;
		hitInfo.m_HitDirection = hitDirection;
		hitInfo.m_HitNormal = hitNormal;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the object receives damage, return false if damage should be ignored
	bool GetCanBeDamaged()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the object should be destroyed and various effects or other things need to be performed (actual destruction handled HandleDestruction())
	void QueueDestroy()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns objects that are meant to be created when the object is destroyed (particles, debris, etc)
	void SpawnDestroyObjects(SCR_HitInfo hitInfo)
	{
		Physics ownerPhysics = GetOwner().GetPhysics();
		
		array<ref SCR_BaseSpawnable> destroySpawnObjects = SCR_DestructionBaseComponentClass.Cast(GetComponentData(GetOwner())).m_DestroySpawnObjects;
		int numSpawnOnDestroy = destroySpawnObjects.Count();
		for (int i = 0; i < numSpawnOnDestroy; i++)
		{
			SCR_BaseSpawnable spawnObject = destroySpawnObjects[i];
			if (m_iDestructionBaseDataIndex != -1)
				spawnObject.Spawn(GetOwner(), ownerPhysics, GetDestructionBaseData().GetHitInfo(false));
			else
				spawnObject.Spawn(GetOwner(), ownerPhysics, null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RegenerateNavmeshDelayed()
	{
		if (Replication.IsClient())
			return;
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		
		if (!aiWorld)
			return;
		
		array<ref Tuple2<vector, vector>> areas = {};
		array<bool> redoRoads = {};
		aiWorld.GetNavmeshRebuildAreas(GetOwner(), areas, redoRoads); // Get area with current phase
		GetGame().GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas, redoRoads); // Rebuild later with new phase/when this object is destroyed
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle destruction
	void HandleDestruction()
	{
		if (GetOwner().IsDeleted())
			return;
		
		SpawnDestroyObjects(null);
		PlaySound();
		DeleteDestructible();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize destruction
	void InitDestruction()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Receive encoded hit data from server
	void NetReceiveHitData(int hitIndex, EDamageType damageType, float damage, vector hitPosition, vector hitDirection)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when Item is initialized from replication stream. Carries the data from Master.
	void NetReadInit(ScriptBitReader reader)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when Item is getting replicated from Master to Slave connection.
	void NetWriteInit(ScriptBitWriter writer)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays sounds upon destruction
	void PlaySound()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the model of the object
	void SetModel(ResourceName model)
	{
		Resource resource = Resource.Load(model);
		VObject asset;
		if (resource)
		{
			BaseResourceObject resourceObject = resource.GetResource();
			if (resourceObject)
				asset = resourceObject.ToVObject();
			else
				asset = null;
		}
		else
			asset = null;
		GetOwner().SetObject(asset, "");
		GetOwner().Update();
		
		Physics phys = GetOwner().GetPhysics();
		if (!phys)
			return;
		
		// Update physics geometries after mesh change
		if (!phys.UpdateGeometries())
			phys.Destroy();// No geoms found, destroy physics
	}
	
	//------------------------------------------------------------------------------------------------
	void ReplicateDestructibleState(bool forceUpdate = false)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Queue destruction on clients
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RPC_QueueDestroy(bool totalDestruction, float lastHealth, float hitDamage, EDamageType damageType, vector hitPosition, vector hitDirection, vector hitNormal)
	{
		SetHitZoneDamage(GetMaxHealth());
		CreateDestructionHitInfo(totalDestruction, lastHealth, hitDamage, damageType, hitPosition, hitDirection, hitNormal);
		QueueDestroy();
		
		HandleDestruction();
	}
	
	//------------------------------------------------------------------------------------------------
	protected RplComponent FindParentRplComponent()
	{
		IEntity parent = GetOwner().GetParent();
		RplComponent currentRplComponent;
		while (parent)
		{
			currentRplComponent = RplComponent.Cast(parent.FindComponent(RplComponent));
			parent = parent.GetParent();
		}
		
		return currentRplComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PassDamageToChildren(EDamageType type, float damage, inout vector outMat[3], IEntity damageSource, notnull Instigator damageSourceParent, float speed, int colliderID)
	{
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			SCR_DestructionBaseComponent destructionComponent = SCR_DestructionBaseComponent.Cast(child.FindComponent(SCR_DestructionBaseComponent));
			if (!destructionComponent)
			{
				child = child.GetSibling();
				continue;
			}
			
			IEntity currentChild = child;
			child = child.GetSibling();
			destructionComponent.HandleDamage(type, damage, outMat, currentChild, null, damageSourceParent, null, colliderID, -1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if this entity is locally owned
	bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return (rplComponent && rplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateResponseIndex()
	{
		Physics physics = GetOwner().GetPhysics();
		int responseIndex = physics.GetResponseIndex();
		if (responseIndex <= MIN_DESTRUCTION_RESPONSE_INDEX)
			return; // Cannot go lower
		
		float healthPercentage = GetHealth() / GetMaxHealth();
		int minMaxDiff = MAX_DESTRUCTION_RESPONSE_INDEX - MIN_DESTRUCTION_RESPONSE_INDEX + 1;
		
		responseIndex = Math.ClampInt(MIN_DESTRUCTION_RESPONSE_INDEX - 1 + Math.Ceil(minMaxDiff * healthPercentage), MIN_DESTRUCTION_RESPONSE_INDEX, responseIndex);
		physics.SetResponseIndex(responseIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Contact
	override bool OnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (GetDestroyed())
			return false;
		
		if (!GetCanBeDamaged())
			return false;
		
		if (IsProxy())
			return false;
		if (other && other.IsInherited(SCR_DebrisSmallEntity)) // Ignore impacts from debris
 			return false;

		SCR_DestructionBaseComponentClass componentData = SCR_DestructionBaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return false;
		
		// Get the physics of the dynamic object (if owner is static, then we use the other object instead)
		Physics physics = contact.Physics1;
		int responseIndex = physics.GetResponseIndex();
		float ownerMass = physics.GetMass();
		float otherMass;
		int ownerReponseIndex = physics.GetResponseIndex();
		int otherResponseIndex;
		if (!physics.IsDynamic())
		{
			physics = contact.Physics2;
			if (!physics)
				return false; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects
			
			otherMass = physics.GetMass();
			otherResponseIndex = physics.GetResponseIndex();
		}
		else
		{
			Physics otherPhysics = other.GetPhysics();
			if (!otherPhysics)
				return false; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects
			
			otherMass = otherPhysics.GetMass();
			otherResponseIndex = otherPhysics.GetResponseIndex();
		}
		
		float momentum = CalculateMomentum(contact, ownerMass, otherMass);
		
		// Now get the relative force, which is the impulse divided by the mass of the dynamic object
		float relativeForce = 0;
		if (physics.IsDynamic())
		{
			relativeForce = contact.Impulse / physics.GetMass();
			//We divide the relative force by 20, because for some reason, objects with this response index receive ~20x bigger impulse
			//TODO: investigate @matousvoj1
			if (responseIndex >= MIN_DESTRUCTION_RESPONSE_INDEX && responseIndex <= MAX_DESTRUCTION_RESPONSE_INDEX)
				relativeForce *= 0.05;
		}
		
		if (relativeForce < componentData.m_fRelativeContactForceThresholdMinimum) // Below minimum threshold, ignore
			return false;
		
		vector outMat[3];
		vector relVel = contact.VelocityBefore2 - contact.VelocityBefore1;
		outMat[0] = contact.Position; // Hit position
		outMat[1] = relVel.Normalized(); // Hit direction
		outMat[2] = contact.Normal; // Hit normal
		
		//If vehicle response index is the same or higher -> automatically destroy
		if (otherResponseIndex - MIN_MOMENTUM_RESPONSE_INDEX >= ownerReponseIndex - MIN_DESTRUCTION_RESPONSE_INDEX)
		{
			HandleDamage(EDamageType.COLLISION, GetMaxHealth(), outMat, GetOwner(), null,Instigator.CreateInstigator(other), null, -1, -1); //Immediately destroy, otherwise the other object goes right through
		}
		else
		{	
			//Vehicle response index is smaller -> just deal damage
			//float damage = relativeForce * componentData.m_fForceToDamageScale;
			float damage = momentum * componentData.m_fMomentumToDamageScale;
			
			// Send damage to damage handling
			HandleDamage(EDamageType.COLLISION, damage, outMat, GetOwner(), null,Instigator.CreateInstigator(other), null, -1, -1);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Damage
	override void OnDamage(
				  EDamageType type,
				  float damage,
				  HitZone pHitZone,
				  notnull Instigator instigator, 
				  inout vector hitTransform[3], 
				  float speed,
				  int colliderID, 
				  int nodeID)
	{
		super.OnDamage(type, damage, pHitZone, instigator, hitTransform, speed, colliderID, nodeID);
		
		SCR_DestructionBaseComponentClass componentData = SCR_DestructionBaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;
		
		if (componentData.m_bPassDamageToChildren)
			PassDamageToChildren(type, damage, hitTransform, null, instigator, speed, colliderID);
		
		if (m_iDestructionBaseDataIndex != -1)
		{
			ScriptInvoker onDamage = GetDestructionBaseData().GetOnDamage(false);
			if (onDamage)
				onDamage.Invoke();
		}
		
		if (IsProxy())
			return;
		
		if (!GetDestroyed())
		{
			GetDestructionBaseData().SetPreviousHealth(GetHealth());
			UpdateResponseIndex();
			return;
		}
		
		Physics physics = GetOwner().GetPhysics();
		if (physics)
		{
			if (physics.IsDynamic() && !physics.IsActive()) // Wake the object up if sleeping
				physics.ApplyImpulse(vector.Up * physics.GetMass() * 0.001); // Applying impulse instead of SetActive, as we don't want to override ActiveState.ALWAYS_ACTIVE if set
			
			// Should disable physics on destroy
			if (GetDisablePhysicsOnDestroy())
				physics.SetInteractionLayer(EPhysicsLayerDefs.VehicleCast);
		}
		
		float previousHealth;
		if (m_iDestructionBaseDataIndex != -1)
			previousHealth = GetDestructionBaseData().GetPreviousHealth();
		
		CreateDestructionHitInfo(damage >= componentData.m_fDamageThresholdMaximum, previousHealth, damage, type, hitTransform[0], hitTransform[1], hitTransform[2]);
		
		QueueDestroy();
		
		SCR_DestructionHitInfo destructionHitInfo = GetDestructionHitInfo();
		if (!destructionHitInfo)
			return;
		
		Rpc(RPC_QueueDestroy, destructionHitInfo.m_TotalDestruction, destructionHitInfo.m_LastHealth, destructionHitInfo.m_HitDamage, destructionHitInfo.m_DamageType, destructionHitInfo.m_HitPosition, destructionHitInfo.m_HitDirection, destructionHitInfo.m_HitNormal);
		
		if (!IsDestructionQueued())
		{
			GetGame().GetCallqueue().CallLater(HandleDestruction); // Replaces OnFrame call, prevents crash when damage is dealt async/from physics step
			SCR_DestructionBaseData data = GetDestructionBaseData();
			data.SetDestructionQueued(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDestructionQueued()
	{
		SCR_DestructionBaseData data = GetDestructionBaseData();
		return data.GetDestructionQueued();
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTotalDestructionThreshold()
	{
		SCR_DestructionBaseComponentClass componentData = SCR_DestructionBaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return 3000;
		
		return componentData.m_fDamageThresholdMaximum;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.CONTACT);
		
		if (!GetDefaultHitZone())
			return;
		
		SCR_DestructionBaseComponentClass componentData = SCR_DestructionBaseComponentClass.Cast(GetComponentData(owner));
		if (!componentData)
		{
			Print("Component data is null!", LogLevel.ERROR);
			return;
		}
		
		SetHitZoneHealth(componentData.m_fBaseHealth);
		InitDestruction();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_DestructionBaseComponent()
	{
#ifdef WORKBENCH
		s_bPrintMissingComponent = false;
		s_bPrintMissingPlayerController = false;
		s_bPrintInitializationFailed = false;
#endif
		
		if (m_iDestructionBaseDataIndex != -1)
			FreeDestructionBaseData(m_iDestructionBaseDataIndex);
	}
#endif
};

//------------------------------------------------------------------------------------------------
//! Class to temporarily store information about the last hit that dealt damage
class SCR_HitInfo
{
	float m_LastHealth;
	float m_HitDamage;
	EDamageType m_DamageType;
	vector m_HitPosition;
	vector m_HitDirection;
	vector m_HitNormal;
};

//------------------------------------------------------------------------------------------------
//! Class to temporarily store information about the last hit that caused destruction
class SCR_DestructionHitInfo : SCR_HitInfo
{
	bool m_TotalDestruction;
	
	//------------------------------------------------------------------------------------------------
	static SCR_DestructionHitInfo FromHitInfo(SCR_HitInfo hitInfo, bool totalDestruction)
	{
		SCR_DestructionHitInfo destHitInfo = new SCR_DestructionHitInfo;
		destHitInfo.m_TotalDestruction = totalDestruction;
		destHitInfo.m_LastHealth = hitInfo.m_LastHealth;
		destHitInfo.m_HitDamage = hitInfo.m_HitDamage;
		destHitInfo.m_DamageType = hitInfo.m_DamageType;
		destHitInfo.m_HitPosition = hitInfo.m_HitPosition;
		destHitInfo.m_HitDirection = hitInfo.m_HitDirection;
		destHitInfo.m_HitNormal = hitInfo.m_HitNormal;
		return destHitInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DestructionHitInfo FromDestructionData(SCR_DestructionData destructionData)
	{
		SCR_DestructionHitInfo destructionHitInfo = new SCR_DestructionHitInfo();
		destructionHitInfo.m_TotalDestruction = destructionData.m_bTotalDestruction;
		destructionHitInfo.m_HitDamage = destructionData.m_fHitDamage;
		destructionHitInfo.m_DamageType = destructionData.m_eDamageType;
		destructionHitInfo.m_HitPosition = destructionData.m_vHitPosition;
		destructionHitInfo.m_HitDirection = destructionData.m_vHitDirection;
		destructionHitInfo.m_HitNormal = destructionData.m_vHitNormal;
		return destructionHitInfo;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_Spawnable_SmallDebrisTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Small Debris";
		return true;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_Spawnable_PrefabTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Prefab";
		return true;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_Spawnable_ParticleTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Particle Effect";
		return true;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BaseSpawnable
{
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Positional offset (in local space to the destructible)")]
	protected vector m_vOffsetPosition;
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Yaw, pitch & roll offset (in local space to the destructible)")]
	protected vector m_vOffsetRotation;
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (source.GetResourceName().Contains("BrickWall_01/BrickWall_01_white_2m.et"))
			Print("BROKEN");
		
		// Set all variables of the spawn object
		api.SetVariableValue(source, path, "m_vOffsetPosition", string.Format("%1 %2 %3", m_vOffsetPosition[0], m_vOffsetPosition[1], m_vOffsetPosition[2]));
		api.SetVariableValue(source, path, "m_vOffsetRotation", string.Format("%1 %2 %3", m_vOffsetRotation[0], m_vOffsetRotation[1], m_vOffsetRotation[2]));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	bool CompareAttributes(SCR_BaseSpawnable other)
	{
		if (other.m_vOffsetPosition != m_vOffsetPosition)
			return false;
		
		if (other.m_vOffsetRotation != m_vOffsetRotation)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool AlreadyExists(WorldEditorAPI api, IEntitySource source, int index)
	{
		array<ref BaseDestructionPhase> phases = {};
		source.Get("DamagePhases", phases);
		
		if (phases && phases.IsIndexValid(index))
		{
			SCR_BaseDestructionPhase phase = SCR_BaseDestructionPhase.Cast(phases[index]);
			for (int i = phase.m_aPhaseDestroySpawnObjects.Count() - 1; i >= 0; i--)
			{
				if (phase.m_aPhaseDestroySpawnObjects[i].Type() == Type())
				{
					if (CompareAttributes(phase.m_aPhaseDestroySpawnObjects[i]))
						return true;
				}
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_BaseSpawnable", index);
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void CopyToSource(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index, string currentObjectName)
	{
		if (!CreateObject(api, source, path, index))
			return;
		
		// Change the path to the current spawn object
		int last = path.Insert(new ContainerIdPathEntry(currentObjectName, index));
		
		SetVariables(api, source, path, index);
		
		path.Remove(last);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Calculates the spawn tranformation matrix for the object
	void GetSpawnTransform(IEntity owner, out vector outMat[4], bool localCoords = false)
	{
		if (localCoords)
		{
			Math3D.AnglesToMatrix(m_vOffsetRotation, outMat);
			// TODO: Remove hotfix for sleeping/static object
			if (m_vOffsetPosition == vector.Zero)
				outMat[3] = vector.Up * 0.001;
			else
				outMat[3] = m_vOffsetPosition;
		}
		else
		{
			vector localMat[4], parentMat[4];
			owner.GetWorldTransform(parentMat);
			Math3D.AnglesToMatrix(m_vOffsetRotation, localMat);
			localMat[3] = m_vOffsetPosition;
			
			Math3D.MatrixMultiply4(parentMat, localMat, outMat);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	IEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		return null;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_Spawnable_SmallDebrisTitle()]
class SCR_DebrisSpawnable : SCR_BaseSpawnable
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Debris model prefabs to spawn (spawns ALL of them)", "et xob")]
	ref array<ResourceName> m_ModelPrefabs;
	[Attribute("10", UIWidgets.Slider, "Mass of the debris", "0.01 1000 0.01")]
	float m_fMass;
	[Attribute("5", UIWidgets.Slider, "Minimum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMin;
	[Attribute("10", UIWidgets.Slider, "Maximum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMax;
	[Attribute("200", UIWidgets.Slider, "Maximum distance from camera above which the debris is not spawned (in m)", "0 3600 0.5")]
	float m_fDistanceMax;
	[Attribute("0", UIWidgets.Slider, "Higher priority overrides lower priority if at or over debris limit", "0 100 1")]
	int m_fPriority;
	[Attribute("0.1", UIWidgets.Slider, "Damage received to physics impulse (speed / mass) multiplier", "0 10000 0.01")]
	float m_fDamageToImpulse;
	[Attribute("2", UIWidgets.Slider, "Damage to speed multiplier, used when objects get too much damage to impulse", "0 10000 0.01")]
	float m_fMaxDamageToSpeedMultiplier;
	[Attribute("0.5", UIWidgets.Slider, "Random linear velocity multiplier (m/s)", "0 200 0.1")]
	float m_fRandomVelocityLinear;
	[Attribute("180", UIWidgets.Slider, "Random angular velocity multiplier (deg/s)", "0 3600 0.1")]
	float m_fRandomVelocityAngular;
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for debris sound", "", ParamEnumArray.FromEnum(SCR_EMaterialSoundTypeDebris))]
	SCR_EMaterialSoundTypeDebris m_eMaterialSoundType;
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	override bool CompareAttributes(SCR_BaseSpawnable other)
	{
		SCR_DebrisSpawnable otherDebris = SCR_DebrisSpawnable.Cast(other);
		
		if (!super.CompareAttributes(other))
			return false;
		
		if (otherDebris.m_ModelPrefabs != m_ModelPrefabs)
			return false;
		
		if (otherDebris.m_fLifetimeMin != m_fLifetimeMin)
			return false;
		
		if (otherDebris.m_fLifetimeMax != m_fLifetimeMax)
			return false;
		
		if (otherDebris.m_fDistanceMax != m_fDistanceMax)
			return false;
		
		if (otherDebris.m_fPriority != m_fPriority)
			return false;
		
		if (otherDebris.m_fDamageToImpulse != m_fDamageToImpulse)
			return false;
		
		if (otherDebris.m_fMaxDamageToSpeedMultiplier != m_fMaxDamageToSpeedMultiplier)
			return false;
		
		if (otherDebris.m_fRandomVelocityLinear != m_fRandomVelocityLinear)
			return false;
		
		if (otherDebris.m_fRandomVelocityAngular != m_fRandomVelocityAngular)
			return false;
		
		if (otherDebris.m_eMaterialSoundType != m_eMaterialSoundType)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		super.SetVariables(api, source, path, index);
		
		string prefabsArray = "";
		// Set all variables of the spawn object
		for (int i = 0, count = m_ModelPrefabs.Count(); i < count; i++)
		{
			prefabsArray += m_ModelPrefabs[i];
			
			if (i != count - 1) // Not last item
				prefabsArray += ",";
		}
		
		api.SetVariableValue(source, path, "m_ModelPrefabs", prefabsArray);
		
		api.SetVariableValue(source, path, "m_fMass", m_fMass.ToString());
		api.SetVariableValue(source, path, "m_fLifetimeMin", m_fLifetimeMin.ToString());
		api.SetVariableValue(source, path, "m_fLifetimeMax", m_fLifetimeMax.ToString());
		api.SetVariableValue(source, path, "m_fDistanceMax", m_fDistanceMax.ToString());
		api.SetVariableValue(source, path, "m_fPriority", m_fPriority.ToString());
		api.SetVariableValue(source, path, "m_fDamageToImpulse", m_fDamageToImpulse.ToString());
		api.SetVariableValue(source, path, "m_fMaxDamageToSpeedMultiplier", m_fMaxDamageToSpeedMultiplier.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityLinear", m_fRandomVelocityLinear.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityAngular", m_fRandomVelocityAngular.ToString());
		api.SetVariableValue(source, path, "m_eMaterialSoundType", m_eMaterialSoundType.ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_DebrisSpawnable", index);
			return true;
		}
		
		return false;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override IEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (!hitInfo)
			return null;
		
		int numModelPrefabs = 0;
		if (m_ModelPrefabs)
			numModelPrefabs = m_ModelPrefabs.Count();
		
		for (int i = 0; i < numModelPrefabs; i++)
		{
			ResourceName prefabPath = m_ModelPrefabs[i];
			
			ResourceName modelPath;
			string remap;
			SCR_Global.GetModelAndRemapFromResource(prefabPath, modelPath, remap);
			
			if (modelPath == ResourceName.Empty)
				continue;
			
			vector spawnMat[4];
			GetSpawnTransform(owner, spawnMat);
			
			SCR_DestructionBaseComponent destructionComponent = SCR_DestructionBaseComponent.Cast(owner.FindComponent(SCR_DestructionBaseComponent));
			
			float dmgSpeed = Math.Clamp(hitInfo.m_HitDamage * m_fDamageToImpulse / m_fMass, 0, m_fMaxDamageToSpeedMultiplier);
			
			vector linearVelocity = hitInfo.m_HitDirection * Math.RandomFloat(0, 1);
			linearVelocity += Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * m_fRandomVelocityLinear;
			linearVelocity *= dmgSpeed;
			vector angularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(0.25, 4) * m_fRandomVelocityAngular;
			angularVelocity *= dmgSpeed;
			
			if (parentPhysics)
			{
				linearVelocity += parentPhysics.GetVelocity();
				angularVelocity += parentPhysics.GetAngularVelocity();
			}
#ifdef ENABLE_BASE_DESTRUCTION
			SCR_DebrisSmallEntity.SpawnDebris(owner.GetWorld(), spawnMat, modelPath, m_fMass, Math.RandomFloat(m_fLifetimeMin, m_fLifetimeMax), m_fDistanceMax, m_fPriority, linearVelocity, angularVelocity, remap, false, m_eMaterialSoundType);
#endif
		}
		
		return null;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_Spawnable_PrefabTitle()]
class SCR_PrefabSpawnable : SCR_BaseSpawnable
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Prefabs to spawn (spawns ALL of them)", "et")]
	ref array<ResourceName> m_Prefabs;
	[Attribute("0.1", UIWidgets.Slider, "Damage received to physics impulse (speed / mass) multiplier", "0 10000 0.01")]
	float m_fDamageToImpulse;
	[Attribute("0.25", UIWidgets.Slider, "Random linear velocity multiplier (m/s)", "0 200 0.1")]
	float m_fRandomVelocityLinear;
	[Attribute("45", UIWidgets.Slider, "Random angular velocity multiplier (deg/s)", "0 3600 0.1")]
	float m_fRandomVelocityAngular;
	[Attribute("0", UIWidgets.CheckBox, "Whether the spawned prefabs should be set as children (sets auto-transform)")]
	bool m_bSpawnAsChildren;
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	override bool CompareAttributes(SCR_BaseSpawnable other)
	{
		SCR_PrefabSpawnable otherPrefab = SCR_PrefabSpawnable.Cast(other);
		
		if (!super.CompareAttributes(other))
			return false;
		
		int count = m_Prefabs.Count();
		if (otherPrefab.m_Prefabs.Count() != count)
			return false;
		
		for (int i = count - 1; i >= 0; i--)
		{
			if (otherPrefab.m_Prefabs[i] != m_Prefabs[i])
				return false;
		}
		
		if (otherPrefab.m_fDamageToImpulse != m_fDamageToImpulse)
			return false;
		
		if (otherPrefab.m_fRandomVelocityLinear != m_fRandomVelocityLinear)
			return false;
		
		if (otherPrefab.m_fRandomVelocityAngular != m_fRandomVelocityAngular)
			return false;
		
		if (otherPrefab.m_bSpawnAsChildren != m_bSpawnAsChildren)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		super.SetVariables(api, source, path, index);
		
		string prefabsArray = "";
		// Set all variables of the spawn object
		for (int i = 0, count = m_Prefabs.Count(); i < count; i++)
		{
			prefabsArray += m_Prefabs[i];
			
			if (i != count - 1) // Not last item
				prefabsArray += ",";
		}
		
		api.SetVariableValue(source, path, "m_Prefabs", prefabsArray);
		
		api.SetVariableValue(source, path, "m_fDamageToImpulse", m_fDamageToImpulse.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityLinear", m_fRandomVelocityLinear.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityAngular", m_fRandomVelocityAngular.ToString());
		api.SetVariableValue(source, path, "m_bSpawnAsChildren", m_bSpawnAsChildren.ToString(true));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_PrefabSpawnable", index);
			return true;
		}
		
		return false;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override IEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (!hitInfo)
			return null;
		
		int numModelPrefabs = 0;
		if (m_Prefabs)
			numModelPrefabs = m_Prefabs.Count();
		
		for (int i = 0; i < numModelPrefabs; i++)
		{
			ResourceName prefabPath = m_Prefabs[i];
			
			bool isPrefab;
			if (SCR_Global.GetResourceContainsComponent(prefabPath, "RplComponent", isPrefab) && RplSession.Mode() == RplMode.Client)
				continue;
			
			if (!isPrefab)
				continue;
			
			vector spawnMat[4];
			GetSpawnTransform(owner, spawnMat, m_bSpawnAsChildren);
			
			EntitySpawnParams prefabSpawnParams = EntitySpawnParams();
			prefabSpawnParams.Transform = spawnMat;
			IEntity spawnedPrefab = GetGame().SpawnEntityPrefab(Resource.Load(prefabPath), null, prefabSpawnParams);
			if (!spawnedPrefab)
				continue;
			
			if (m_bSpawnAsChildren)
			{
				owner.AddChild(spawnedPrefab, -1, EAddChildFlags.AUTO_TRANSFORM);
				continue;
			}
			
			Physics prefabPhysics = spawnedPrefab.GetPhysics();
			if (!prefabPhysics || !prefabPhysics.IsDynamic())
				continue;
			
			float dmgSpeed = hitInfo.m_HitDamage * m_fDamageToImpulse / prefabPhysics.GetMass();
			
			vector linearVelocity = hitInfo.m_HitDirection * Math.RandomFloat(0, 1);
			linearVelocity += Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * m_fRandomVelocityLinear;
			linearVelocity *= dmgSpeed;
			vector angularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(0.25, 4) * m_fRandomVelocityAngular;
			angularVelocity *= dmgSpeed;
			
			if (parentPhysics)
			{
				linearVelocity += parentPhysics.GetVelocity();
				angularVelocity += parentPhysics.GetAngularVelocity();
			}
			
			prefabPhysics.SetVelocity(linearVelocity);
			prefabPhysics.SetAngularVelocity(angularVelocity * Math.DEG2RAD);
		}
		
		return null; // We spawned multiple entities
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_Spawnable_ParticleTitle()]
class SCR_ParticleSpawnable : SCR_BaseSpawnable
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourceNamePicker, "Particle effect to spawn", "ptc")]
	ResourceName m_Particle;
	[Attribute("1", UIWidgets.CheckBox, "If true, the particle effect will play at the object's bounding box instead of at its origin")]
	bool m_bAtCenter;
	
	[Attribute("1", desc: "If true, the particle effect will play rotated in the hit direction.")]
	bool m_bDirectional;
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	override bool CompareAttributes(SCR_BaseSpawnable other)
	{
		SCR_ParticleSpawnable otherParticle = SCR_ParticleSpawnable.Cast(other);
		
		if (!super.CompareAttributes(other))
			return false;
		
		if (otherParticle.m_Particle != m_Particle)
			return false;
		
		if (otherParticle.m_bAtCenter != m_bAtCenter)
			return false;
		
		if (otherParticle.m_bDirectional != m_bDirectional)
			return false;
		
		return true;
	}
	//------------------------------------------------------------------------------------------------
	override void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		super.SetVariables(api, source, path, index);
		
		// Set all variables of the spawn object
		api.SetVariableValue(source, path, "m_Particle", m_Particle);
		api.SetVariableValue(source, path, "m_bAtCenter", m_bAtCenter.ToString(true));
		api.SetVariableValue(source, path, "m_bDirectional", m_bDirectional.ToString(true));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_ParticleSpawnable", index);
			return true;
		}
		
		return false;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	ParticleEffectEntity SpawnAsChild(IEntity owner, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (m_Particle == ResourceName.Empty)
			return null;
		
		vector spawnMat[4];
		GetSpawnTransform(owner, spawnMat);
		
		if (m_bAtCenter)
		{
			vector mins, maxs;
			owner.GetBounds(mins, maxs);
			vector center = (maxs - mins) * 0.5 + mins;
			spawnMat[3] = center.Multiply4(spawnMat);
		}
		
		vector position = spawnMat[3];
		if (snapToTerrain)
		{
			position[1] = SCR_TerrainHelper.GetTerrainY(position, owner.GetWorld());
			spawnMat[3] = position;
		}
		
		if (m_bDirectional)
		{
			vector newRight, newUp, newForward;
			
			newUp = -hitInfo.m_HitDirection;
			newRight = newUp * spawnMat[2];
			newForward = newUp * newRight;
			
			spawnMat[0] = newRight;
			spawnMat[1] = newUp;
			spawnMat[2] = newForward;
		}
		
		return SCR_DestructionCommon.PlayParticleEffect_Child(m_Particle, hitInfo.m_DamageType, owner, spawnMat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override IEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (!hitInfo || m_Particle == ResourceName.Empty)
			return null;
		
		vector spawnMat[4];
		GetSpawnTransform(owner, spawnMat);
		
		if (m_bAtCenter)
		{
			vector mins, maxs;
			owner.GetBounds(mins, maxs);
			vector center = (maxs - mins) * 0.5 + mins;
			spawnMat[3] = center.Multiply4(spawnMat);
		}
		
		vector position = spawnMat[3];
		if (snapToTerrain)
		{
			position[1] = SCR_TerrainHelper.GetTerrainY(position, owner.GetWorld());
			spawnMat[3] = position;
		}
		
		if (m_bDirectional)
		{
			vector newRight, newUp, newForward;
			
			newUp = -hitInfo.m_HitDirection;
			newRight = newUp * spawnMat[2];
			newForward = newUp * newRight;
			
			spawnMat[0] = newRight;
			spawnMat[1] = newUp;
			spawnMat[2] = newForward;
		}
		
		return SCR_DestructionCommon.PlayParticleEffect_Transform(m_Particle, hitInfo.m_DamageType, spawnMat);
	}
};
