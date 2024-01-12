/** @ingroup Editable_Entities
*/

//------------------------------------------------------------------------------------------------
/*!
Component defining editable entity.
- Any entity with this component is exposed in the editor (SCR_EditorManagerCore).
- Editable entities are tracked locally in a list managed by SCR_EditableEntityCore.
- Editable entity, unless it has a flag EEditableEntityFlag.LOCAL, must also have a RplComponent attached.
- Despite the name, not every entity with this component is actually editable - those with EEditableEntityFlag.NON_INTERACTIVE are not.

## Implementation Notes
- Keep memory footprint of component instance (i.e., the size of variables) to minimum. The component can axist on thousands of entities per world.
- Do not reference editor (e.g., SCR_EditorManagerEntity, SCR_EditorModeEntity, or SCR_BaseEditorComponent) from here!
 + Editable entities are independent on the editor. Each player has their own editor, so there is no single editor to point to anyway.
*/
class SCR_EditableEntityComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.ComboBox, category: "Editable Entity", desc: "Decide whether the entity should be registered automatically to the list of editable entities.", enums: ParamEnumArray.FromEnum(EEditableEntityRegister))]
	protected EEditableEntityRegister m_bAutoRegister;

	[Attribute("", UIWidgets.Auto, category: "Visualization", desc: "Icon offset (m) from entity's origin.")]
	protected vector m_vIconPos;

	[Attribute("1", category: "Visualization", desc: "Set entity's visibility setting. Apart from this custom value, visibility of an entitymay be influenced by other factors like distance to the camera or active layer.")]
	protected bool m_bVisible;

	[Attribute(category: "Visualization", desc: "Mark entity as static. When static, entity's position is not calculated each frame. Instead, cached position is used.")]
	protected bool m_bStatic;

	[Attribute("0", category: "Visualization", desc: "Max distance in which the entity is drawn.\nWhen 0 or below, default for given type is used.")]
	protected float m_fMaxDrawDistance;

	[Attribute("1", UIWidgets.Flags, category: "Editable Entity", desc: "For editor users to edit or even see the entity, at least one of their editor keys must match entity's keys.\nFOr example, if the entity has KEY_1 and KEY_2, while the editor has KEY_2 and KEY_8, the entity will be available, since both have KEY_2.", enums: ParamEnumArray.FromEnum(EEditableEntityAccessKey))]
	protected EEditableEntityAccessKey m_AccessKey;

	[Attribute("", UIWidgets.Flags, category: "Editable Entity", desc: "Set unique flags.", enums: ParamEnumArray.FromEnum(EEditableEntityFlag))]
	protected EEditableEntityFlag m_Flags;

	protected SCR_EditableEntityComponent m_ParentEntity;
	protected GenericEntity m_Owner;
	protected EEditableEntityState m_EntityState;
	protected SCR_UIInfo m_UIInfoInstance;
	protected ref set<SCR_EditableEntityComponent> m_Entities;
	protected vector m_vStaticPos;
	protected int m_iIconBoneIndex = -1;

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity name from info component.
	When info is undefined, use entity's variable name, or, if that one is also undefined, its class name.
	\return Type
	*/
	string GetDisplayName()
	{
		string displayName;

		//--- Get GUI name
		SCR_UIInfo info = GetInfo();
		if (info)
			displayName = info.GetName();

		if (!displayName.IsEmpty() || !m_Owner)
			return displayName;

		//--- Get entity name
		displayName = m_Owner.GetName();
		if (!displayName.IsEmpty())
			return "[" + displayName + "]";

		//--- Get class name
		return "{" + m_Owner.Type().ToString() + "}";
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity prefab.
	\param shorten True to include only GUID, not file path
	\return Prefab path
	*/
	ResourceName GetPrefab(bool shorten = false)
	{
		if (!m_Owner)
			return ResourceName.Empty;

		EntityPrefabData prefabData = m_Owner.GetPrefabData();
		if (!prefabData)
			return ResourceName.Empty;

		ResourceName prefab = prefabData.GetPrefabName();
		if (!prefab)
			prefab = SCR_BaseContainerTools.GetPrefabResourceName(prefabData.GetPrefab()); //--- Modified instance, find prefab in ancestors

		if (shorten && prefab)
		{
			"{"; // fix indent
			int guidIndex = prefab.LastIndexOf("}");
			if (guidIndex >= 0)
				prefab = prefab.Substring(0, guidIndex + 1);
		}

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get prefab data of this editable entity.
	\param Owner entity of this component, used when m_Owner==null, after deletion
	\return Component prefab data
	*/
	SCR_EditableEntityComponentClass GetEditableEntityData(IEntity owner = null)
	{
		if (owner)
		{
			return SCR_EditableEntityComponentClass.Cast(GetComponentData(owner));
		}
		else if (m_Owner)
		{
			return SCR_EditableEntityComponentClass.Cast(GetComponentData(m_Owner));
		}
		else
		{
			return null;
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity type.
	**Avoid basing your functionality on specific entity types!**
	If everyone checked for specific types, adding a new type would mean all conditions have to be revised.
	Instead, base your system on a function inside this class. Such function can be overrided by inherited classes to give desired result.
	\param Owner entity of this component
	\return Type
	*/
	EEditableEntityType GetEntityType(IEntity owner = null)
	{
		SCR_EditableEntityComponentClass prefabData = GetEditableEntityData(owner);
		if (prefabData)
			return prefabData.GetEntityType();
		else
			return EEditableEntityType.GENERIC;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get entity interaction rules of this entity. If it doesn't contain any custom rules, those for its type will be used.
	\return Interaction rules
	*/
	SCR_EditableEntityInteraction GetEntityInteraction(IEntity owner = null)
	{
		SCR_EditableEntityComponentClass prefabData = GetEditableEntityData(owner);
		if (prefabData)
			return prefabData.GetEntityInteraction();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get information about the entity. When none exist, create a dummy one.
	\return Info class
	*/
	SCR_UIInfo GetInfo(IEntity owner = null)
	{
		//--- From instance
		if (m_UIInfoInstance)
			return m_UIInfoInstance;

		//--- From prefab
		SCR_EditableEntityComponentClass prefabData = GetEditableEntityData(owner);
		if (prefabData)
			return prefabData.GetInfo();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set information about the entity on entity instance, locally on this machine.
	This is a weak ref! The info needs to be held somewhere else, the entity will merely link to it.
	\param info Info class
	*/
	void SetInfoInstance(SCR_UIInfo info)
	{
		m_UIInfoInstance = info;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check component's replication. Show an error when it's not registered for replication (e.g., RplComponent is missing)
	\param[out] replicationID ID used by Replication.FindItem()
	\return True if replicated
	*/
	bool IsReplicated(out RplId replicationID = -1)
	{
		//--- Never considered replicated when flagged as LOCAL
		if (HasEntityFlag(EEditableEntityFlag.LOCAL))
			return false;

		replicationID = Replication.FindId(this);
		if (replicationID == -1)
		{
			//Print(string.Format("Replication ID not found for '%1'!", GetDisplayName()), LogLevel.ERROR);
			return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the entity can be serialized during session saving managed by SCR_EditableEntityStruct.
	\param[out] target Entity to which this entity is attached to outside of hierarchy structure, e.g., character in a vehicle or waypoint on a target
	\param[out] targetIndex Further specification of the target, e.g., crew position index in a vehicle
	\param[out] isDestroyed Variable to be set to true if the entity is destroyed
	\return True if it can be serialized
	*/
	bool Serialize(out SCR_EditableEntityComponent outTarget = null, out int outTargetIndex = -1, out EEditableEntitySaveFlag outSaveFlags = 0)
	{
		if (IsDestroyed())
			outSaveFlags |= EEditableEntitySaveFlag.DESTROYED;
		
		//--- Children are spawned by the link component, but they were not spawned yet (e.g., in building mode)
		if ((m_Flags & EEditableEntityFlag.LINKED_CHILDREN) && !SCR_EditorLinkComponent.IsSpawned(this))
			outSaveFlags |= EEditableEntitySaveFlag.NOT_SPAWNED;
		
		return !(m_Flags & EEditableEntityFlag.LOCAL) && !(m_Flags & EEditableEntityFlag.NON_SERIALIZABLE);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Deserialize the entity based on given params.
	\param target Entity to which this entity is attached to outside of hierarchy structure, e.g., character in a vehicle or waypoint on a target
	\param targetIndex Further specification of the target, e.g., crew position index in a vehicle
	*/
	void Deserialize(SCR_EditableEntityComponent target, int targetValue)
	{
		SetParentEntity(target);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsServer()
	{
		//--- Do we have server rights (LOCAL entities have always full rights)
		return Replication.IsServer() || Replication.Loadtime() || HasEntityFlag(EEditableEntityFlag.LOCAL);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanRpc()
	{
		//--- Check if replication is possible (e.g., not when the game is shutting down) or allowed (e.g., not when flagged as LOCAL)
		return Replication.IsRunning() && !HasEntityFlag(EEditableEntityFlag.LOCAL);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return Owner entity.
	*/
	GenericEntity GetOwnerScripted()
	{
		return m_Owner;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get position representing the entity.
	\param[out] pos Entity's position
	\return True if the entity has a position
	*/
	bool GetPos(out vector pos)
	{
		if (!m_Owner)
			return false;

		if (m_bStatic)
		{
			//--- Cached position
			pos = m_vStaticPos;
			return true;
		}
		else if (m_iIconBoneIndex == -1)
		{
			//--- Offset position
			pos = m_Owner.CoordToParent(m_vIconPos);
			return true;
		}
		else
		{
			//--- Bone position
			if (m_Owner.GetAnimation())
			{
				vector transform[4];
				m_Owner.GetAnimation().GetBoneMatrix(m_iIconBoneIndex, transform);
				pos = m_Owner.CoordToParent(transform[3] + m_vIconPos);
			}
			else
			{
				pos = m_Owner.CoordToParent(m_vIconPos);
			}
			return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get icon offset
	\param Offset
	*/
	vector GetIconPos()
	{
		return m_vIconPos;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get world transformation matrix of editable entity.
	\param[out] outTransform Transformation matrix
	*/
	bool GetTransform(out vector outTransform[4])
	{
		if (!m_Owner)
			return false;

		m_Owner.GetTransform(outTransform);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get local transformation matrix of editable entity.
	\param[out] outTransform Transformation matrix
	*/
	bool GetLocalTransform(out vector outTransform[4])
	{
		if (!m_Owner)
			return false;

		if (m_Owner.GetParent())
		{
			//--- Use engine hierarchy
			m_Owner.GetLocalTransform(outTransform);
		}
		else
		{
			//--- Calculate relative trandform towards editor hierarchy parent
			m_Owner.GetTransform(outTransform);
			if (m_ParentEntity)
			{
				vector parentTransform[4];
				if (m_ParentEntity.GetTransform(parentTransform))
					Math3D.MatrixInvMultiply4(parentTransform, outTransform, outTransform);
			}
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get player controlling this entity.
	\return Player ID
	*/
	int GetPlayerID()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity's faction.
	\return Faction
	*/
	Faction GetFaction()
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when GUI should refresh entity's GUI, i.e., update faction color and call events in GUI widgets.
	To be overriden by inherited classes.
	\return Script invoker
	*/
	ScriptInvoker GetOnUIRefresh()
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when GUI should reset widgets that are used for entity visualization.
	To be overriden by inherited classes.
	\return Script invoker
	*/
	ScriptInvoker GetOnUIReset()
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity's AI group.
	\return Editable entity of the group
	*/
	SCR_EditableEntityComponent GetAIGroup()
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity which represents this entity as AI
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetAIEntity()
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get vehicle the entity's in
	\return Editable entity of the vehicle
	*/
	SCR_EditableEntityComponent GetVehicle()
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get crew of vehicle or if in a vehicle get self.
	\param[out] crewCompartmentAccess an array of CompartmentAccessComponent of all crew memebers
	\param ignorePlayers will never return player CompartmentAccessComponent if true
	\return int count of crew members
	*/
	int GetCrew(out notnull array<CompartmentAccessComponent> crewCompartmentAccess, bool ignorePlayers = true)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity health
	\return Health value in range 0-1
	*/
	float GetHealth()
	{
		if (!m_Owner)
			return 0;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(m_Owner.FindComponent(DamageManagerComponent));
		if (damageManager)
			return damageManager.GetHealthScaled();
		else
			return 1;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return If destroying of entity is allowed. Does not check if entity is destroyed already. Use IsDestroyed() for this
	*/
	bool CanDestroy()
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
		return damageManager && damageManager.IsDamageHandlingEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check if the entity is destroyed.
	\return True when destroyed
	*/
	bool IsDestroyed()
	{
		if (!m_Owner)
			return true;
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(m_Owner.FindComponent(DamageManagerComponent));
		if (damageManager)
			return damageManager.GetState() == EDamageState.DESTROYED;
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set entity as static.
	When static, entity's position is not calculated each frame. Instead, cached position is used.
	Calling this function with isStatic=true refreshes cached position based on the current position.
	Use only for entities that are not expected to move!
	Certain entity types cannot be set as static, and this command will ignore them.
	Ignored types are GROUP, CHARACTER and VEHICLE.
	\param isStatic True to set as static
	*/
	void SetStatic(bool isStatic)
	{
		if (isStatic && m_Owner.GetFlags() & EntityFlags.ACTIVE)
		{
			Print(string.Format("Cannot mark '%1' as static, it's marked as EntityFlags.ACTIVE!", m_Owner), LogLevel.WARNING);
			return;
		}

		m_bStatic = isStatic;
		if (m_bStatic)
			UpdateStaticPos();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the entity is marked as static.
	\return True if static
	*/
	bool GetStatic()
	{
		return m_bStatic;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Update static cached position.
	Doesn't do anything when the entity is not marked as static.
	*/
	void UpdateStaticPos()
	{
		if (!m_bStatic)
			return;

		m_bStatic = false; //--- Set to false so GetPos() below returns actual position
		GetPos(m_vStaticPos);
		m_bStatic = true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Mark hierarchy in all parents of the entity as dirty, i.e., modified by user.
	Used for example to evaluate if the entity should be saved into a save file in its entirety.
	*/
	void SetHierarchyAsDirtyInParents()
	{
		if (!IsServer()) //--- Server-only flag
			return;

		SCR_EditableEntityComponent parent = GetParentEntity();
		while (parent)
		{
			parent.SetHierarchyAsDirty();
			parent = parent.GetParentEntity();
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Mark entity hierarchy as dirty, i.e., modified by user.
	Used for example to evaluate if the entity should be saved into a save file in its entirety.
	*/
	void SetHierarchyAsDirty()
	{
		m_Flags |= EEditableEntityFlag.DIRTY_HIERARCHY;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Update transformation of the entity and all its editor children and broadcast the changes to all clients.
	\param transform Target transformation
	*/
	void SetTransformWithChildren(vector transform[4])
	{
		if (!IsServer() || !m_Owner)
			return;

		SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParams(transform, verticalMode: EEditorTransformVertical.TERRAIN);
		SCR_RefPreviewEntity.SpawnAndApplyReference(this, params);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Update entity's transformation and broadcast the changes to all clients.
	\param transform Target transformation
	\param changedByUser True when the change was initiated by user
	*/
	void SetTransform(vector transform[4], bool changedByUser = false)
	{
		if (!IsServer() || !m_Owner)
			return;

		//Inform Scheduler
		RplComponent rpl = RplComponent.Cast(m_Owner.FindComponent(RplComponent));
		vector prevTransform1[4];

		m_Owner.GetWorldTransform(prevTransform1);

		//--- Entities modified by user are always to be serialized.
		if (changedByUser)
			SetHierarchyAsDirtyInParents();

		//--- Not ideal to hard-code speific classes, but we can no longer use BaseGameEntity as it's also used for interactive lights
		//BaseGameEntity baseGameEntity = BaseGameEntity.Cast(m_Owner);
		if(!System.IsCLIParam("clientVehicles"))
		{
			if (m_Owner.IsInherited(ChimeraCharacter))
			{		
				//--- Execute on owner, even server doesn't have the authority to do so
				Rpc(SetTransformOwner, transform);
			}
			else if (m_Owner.GetPhysics() && m_Owner.IsInherited(Vehicle))
			{
				// TODO: Managed by NwkVehicleMovementComponent if teleportation causes chaotic prediction
				Rpc(SetTransformOwner, transform);
				//--- Execute also on server if Vehicle
				SetTransformOwner(transform);
			}
			else
			{
				SetTransformBroadcast(transform);
				if (CanRpc()) Rpc(SetTransformBroadcast, transform);
			}
		}
		else
		{
			if (m_Owner.GetPhysics() && (m_Owner.IsInherited(ChimeraCharacter) || m_Owner.IsInherited(Vehicle)))
			{		
				//--- Execute on owner, even server doesn't have the authority to do so
				Rpc(SetTransformOwner, transform);
			}
			else
			{
				SetTransformBroadcast(transform);
				if (CanRpc()) Rpc(SetTransformBroadcast, transform);
			}
		}

		//Sends out on transform changed on server
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
		{
			vector prevTransform[4];
			m_Owner.GetWorldTransform(prevTransform);
			core.Event_OnEntityTransformChangedServer.Invoke(this, prevTransform);
		}

		rpl.ForceNodeMovement(prevTransform1[3]);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SetTransformOwner(vector transform[4])
	{
		BaseGameEntity baseGameEntity = BaseGameEntity.Cast(m_Owner);
		if (baseGameEntity)
		{
			baseGameEntity.Teleport(transform);
			//baseGameEntity.Update(); //--- Don't call Update, it would make characters fall through ground!

			Physics phys = baseGameEntity.GetPhysics();
			if (phys)
			{
				phys.SetVelocity(vector.Zero);
				phys.SetAngularVelocity(vector.Zero);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetTransformBroadcast(vector transform[4])
	{
		if (!m_Owner)
			return;

		float scale = m_Owner.GetScale();
		m_Owner.SetWorldTransform(transform);
		m_Owner.SetScale(scale);
		m_Owner.Update();
		UpdateStaticPos();

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityTransformChanged.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Kill/destroy this editable entity.
	\return True if destroyed
	*/
	bool Destroy()
	{
		if (IsServer())
		{			
			if (!IsDestroyed() && CanDestroy())
			{
				DamageManagerComponent damageManager = DamageManagerComponent.Cast(m_Owner.FindComponent(DamageManagerComponent));
				damageManager.SetHealthScaled(0);
				return true;
			}
		}
		return IsDestroyed();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Delete this editable entity.
	\param changedByUser True when the change was initiated by user
	\param updateNavmesh True to update navmesh after the entity is deleted (set to false when deleting children of already deleted entity)
	\return True if deleted
	*/
	bool Delete(bool changedByUser = false, bool updateNavmesh = true)
	{
		if (!IsServer())
			return false;

		//--- Skip if the entity is already being deleted
		if (!m_Owner || m_Owner.IsDeleted())
			return true;

		//--- Skip when deleting is disabled
		if (HasEntityFlag(EEditableEntityFlag.NON_DELETABLE))
			return false;

		//--- Update navmesh
		if (updateNavmesh)
		{
			SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
			if (aiWorld)
			{
				array<ref Tuple2<vector, vector>> areas = {}; // min, max
				array<bool> redoAreas = {};
				aiWorld.GetNavmeshRebuildAreas(GetOwner(), areas, redoAreas);
				GetGame().GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas, redoAreas); //--- Called *before* the entity is deleted with a delay, ensures the regeneration doesn't accidentaly get anything from the entity prior to full destruction
			}
		}

		//--- Mark parents as dirty
		if (changedByUser)
			SetHierarchyAsDirtyInParents();

		//--- Delete the entity
		RplComponent.DeleteRplEntity(m_Owner, false);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	RplId GetOwnerRplId()
	{
		RplComponent rpl = GetRplComponent();
		if (rpl)
			return rpl.Id();
		else
			return RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get squared maximum distance in which this entity is drawn in editor (e.g., with an icon).
	\return Squared distance in metres
	*/
	float GetMaxDrawDistanceSq()
	{
		return m_fMaxDrawDistance;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set maximum distance in which this entity is drawn in editor (e.g., with an icon).
	\param maxDrawDistance Distance in metres
	*/
	void SetMaxDrawDistance(float maxDrawDistance)
	{
		if (maxDrawDistance <= 0)
			return; //--- ToDo: Load from core?

		m_fMaxDrawDistance = maxDrawDistance * maxDrawDistance;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get editable entity component on given entity.
	\param entity
	\return Editable entity component (if the entity has one)
	*/
	static SCR_EditableEntityComponent GetEditableEntity(IEntity owner)
	{
		if (owner)
			return SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity budget costs.
	\param outBudgets Array to be filled with budget values
	\return True if the entity cost should be based on outBudgets array, return false to fallback on entityType cost, return true with an empty array to avoid fallback entityType cost
	*/
	bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets, IEntity owner = null)
	{
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(GetInfo(owner));
		if (editableEntityUIInfo)
		{
			editableEntityUIInfo.GetEntityBudgetCost(outBudgets);
		}
		return !outBudgets.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity budget costs including cost of children (for groups/compositions)
	\param outBudgets Array to be filled with budget values
	\return True if the entity cost should be based on outBudgets, return false to fallback on entityType cost, return true with an empty array to avoid fallback entityType cost
	*/
	bool GetEntityChildrenBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets, IEntity owner = null)
	{
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(GetInfo(owner));
		if (editableEntityUIInfo)
		{
			editableEntityUIInfo.GetEntityChildrenBudgetCost(outBudgets);
		}
		return !outBudgets.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Can entity be duplicated by editor and which recipients should be passed to the duplicated entity
	Overridden by other EditableEntityComponents
	\param outRecipients editableEntityComponents that will be passed to the duplicated entity (Groupcomponent / FactionComponent)
	\return True if entity can be duplicated
	*/
	bool CanDuplicate(out notnull set<SCR_EditableEntityComponent> outRecipients)
	{
		if (HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE))
			return false;

		SCR_FactionAffiliationComponent factionAffiliationComponent = SCR_FactionAffiliationComponent.Cast(m_Owner.FindComponent(SCR_FactionAffiliationComponent));
		if (factionAffiliationComponent)
		{
			Faction faction = factionAffiliationComponent.GetAffiliatedFaction();
			SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
			if (delegateFactionManager)
			{
				SCR_EditableFactionComponent factionDelegate = delegateFactionManager.GetFactionDelegate(faction);
				if (factionDelegate)
				{
					outRecipients.Insert(factionDelegate);
				}
			}
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Parent / child management

	/*! @name Hierarchy
	Functions to manage hierarchy of the entity.
	*/
	///@{

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the entity can be moved to intended parent.
	\param parentEntity New parent. Null when evaluating root.
	*/
	bool CanSetParent(SCR_EditableEntityComponent parentEntity)
	{
		SCR_EditableEntityInteraction interaction = GetEntityInteraction();
		if (!interaction)
			return false;

		//--- Assumed defaults. ToDo: Receive from client
		EEditableEntityInteractionFlag interactionFlags = EEditableEntityInteractionFlag.DELEGATE | EEditableEntityInteractionFlag.LAYER_EDITING;

		if (!IsDestroyed())
			interactionFlags |= EEditableEntityInteractionFlag.ALIVE;

		if (GetPlayerID() == 0)
			interactionFlags |= EEditableEntityInteractionFlag.NON_PLAYABLE;

		return interaction.CanSetParent(parentEntity, interactionFlags);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set parent of the entity.
	When placed inside a parent, the entity will inherit some of its settings, like access key or visibility settings.
	Changing it is allowed only on server.
	\param parentEntity New parent. When null, the entity will be moved to the root.
	\param changedByUser True when the change was initiated by user
	\return New parent (in case it changes inside)
	*/
	SCR_EditableEntityComponent SetParentEntity(SCR_EditableEntityComponent parentEntity, bool changedByUser = false)
	{
		//--- Not on server or not replicated, ignore
		if (!IsServer()/* || !IsReplicated()*/)
			return parentEntity;

		//--- No change, ignore
		if (parentEntity == m_ParentEntity)
			return parentEntity;

		if (parentEntity)
		{
			//--- Setting to itself, ignore and notify
			if (parentEntity == this)
			{
				Log("Cannot set parent to itself!", true, LogLevel.WARNING);
				return parentEntity;
			}

			//--- Setting to itself, ignore and notify
			if (parentEntity.IsChildOf(this))
			{
				Log(string.Format("Cannot set parent to its own child %1!", parentEntity), true, LogLevel.WARNING);
				return parentEntity;
			}
		}

		//--- Check if changing parent is allowed
		if (!CanSetParent(parentEntity))
		{
			//Log(string.Format("Cannot set parent to %1, interaction condition not met!", parentEntity), true, LogLevel.WARNING);
			return parentEntity; //--- ToDo: Notification for the user
		}

		//--- Set and broadcast
		SCR_EditableEntityComponent parentPrev = m_ParentEntity;
		SetParentEntityBroadcast(parentEntity, parentPrev, changedByUser);
		if (CanRpc())
			Rpc(SetParentEntityBroadcastReceive, Replication.FindId(parentEntity), Replication.FindId(parentPrev), changedByUser);

		return parentEntity;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Add the entity to its previous parent after it was unregistered using RemoveParentEntity()
	Allowed only on server.
	*/
	void RestoreParentEntity()
	{
		//--- Not on server or not replicated, ignore
		if (!IsServer()/* || !IsReplicated()*/)
			return;

		SetParentEntityBroadcast(m_ParentEntity, m_ParentEntity);
		if (CanRpc())
		{
			int parentEntityID = Replication.FindId(m_ParentEntity);
			Rpc(SetParentEntityBroadcastReceive, parentEntityID, parentEntityID, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Remove entity from its parent.
	This will unregister the entity from the system, making it not editable.
	Later, it can be enabled again by adding it to editable parent.
	*/
	void RemoveParentEntity()
	{
		//--- Not on server or not replicated, ignore
		if (!IsServer()/* || !IsReplicated()*/)
			return;

		if (IsRegistered())
		{
			OnRegistrationChanged(false);
			if (CanRpc())
				Rpc(OnRegistrationChanged, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get parent entity.
	\return Parent entity
	*/
	SCR_EditableEntityComponent GetParentEntity()
	{
		return m_ParentEntity;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get hierarchy of all parent entities, all the way to the root.
	\param[out] entities Array to be filled with parent entities
	*/
	void GetParentEntities(notnull array<SCR_EditableEntityComponent> entities)
	{
		entities.Clear();
		SCR_EditableEntityComponent parent = GetParentEntity();
		while (parent)
		{
			entities.Insert(parent);
			parent = parent.GetParentEntity();
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the entity is in hierarchy of given entity.
	\param entity Queried entity
	\return True if it's in entity's hierarchy.
	*/
	bool IsChildOf(SCR_EditableEntityComponent entity)
	{
		SCR_EditableEntityComponent parent = GetParentEntity();
		while (parent)
		{
			if (parent == entity)
				return true;

			parent = parent.GetParentEntity();
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get direct reference to entity's immediate children.
	Use when performance is important.
	*DO NOT MODIFY THE LIST!*
	\return Set of child entities, or null when the entity has no children
	*/
	set<SCR_EditableEntityComponent> GetChildrenRef()
	{
		return m_Entities;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get child entities.
	\param[out] entities Array to be filled with child entities
	\param onlyDirect When true, only the direct descendants are returned, otherwise all children, children of children etc. are returned.
	\param skipIgnored When true, entities flagged by IGNORE_LAYERS will not be included in the list
	*/
	void GetChildren(notnull set<SCR_EditableEntityComponent> entities, bool onlyDirect = false, bool skipIgnored = false)
	{
		if (!m_Entities)
			return;

		foreach (SCR_EditableEntityComponent entity : m_Entities)
		{
			if (!entity || (skipIgnored && entity.HasEntityFlag(EEditableEntityFlag.IGNORE_LAYERS)))
				continue;

			entities.Insert(entity);
			if (!onlyDirect)
				entity.GetChildren(entities, onlyDirect, skipIgnored);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get child entities with compatible key.
	\param[out] entities Array to be filled with child entities
	\param onlyDirect When true, only the direct descendants are returned, otherwise all children, children of children etc. are returned.
	\param accessKey Return only entities with at least one key matching this value
	*/
	void GetChildren(notnull set<SCR_EditableEntityComponent> entities, bool onlyDirect, EEditableEntityAccessKey accessKey)
	{
		if (!m_Entities)
			return;

		entities.Clear();
		foreach (SCR_EditableEntityComponent entity : m_Entities)
		{
			if (!entity)
				continue;

			if (entity.HasAccessSelf(accessKey))
			{
				entities.Insert(entity);
				if (!onlyDirect)
					entity.GetChildren(entities, onlyDirect, accessKey);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get number of child entities.
	\param onlyDirect When true, return only direct children, othwise count recursively in their children as well
	\param Number of child entities
	*/
	int GetChildrenCount(bool onlyDirect = false)
	{
		if (!m_Entities)
			return 0;

		if (onlyDirect)
		{
			return m_Entities.Count();
		}
		else
		{
			int count = m_Entities.Count();
			foreach (SCR_EditableEntityComponent child : m_Entities)
			{
				count += child.GetChildrenCount(onlyDirect);
			}
			return count;
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get child on given index.
	\param index Child index
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetChild(int index)
	{
		return m_Entities[index];
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the entity is also a layer, i.e., has some child entities.
	\return True when layer.
	*/
	bool IsLayer()
	{
		return m_Entities && !m_Entities.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Checks if can enter layer.
	\param layersManager give a layermanager ref, it will find the layer manager if left empty
	\param toExtreme When enable, it will not move just one layer up, but all the way to root. And the same when moving down
	\return false if no layermanager or if unable to enter layer
	*/
	bool CanEnterLayer(SCR_LayersEditorComponent layersManager = null, bool toExtreme = false)
	{
		if (!layersManager)
		{
			layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));

			if (!layersManager)
				return false;
		}

		return this != layersManager.GetCurrentLayer() //--- Entity is not the current layer
			&& HasEntityFlag(EEditableEntityFlag.LAYER) //--- Entity is layer
			&& (!toExtreme || GetParentEntity() != layersManager.GetCurrentLayer());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if the entity is registered.
	Registered entity is a child of either root in SCR_EditableEntityCore or one of already registered entities.
	\return True when registered
	*/
	bool IsRegistered()
	{
		return m_bAutoRegister == -1;
	}
	///@}

	//------------------------------------------------------------------------------------------------
	/*!
	Authority Only, forces entities such as Character and Group to place characters into a specific vehicle position
	\param forceVehicleCompartments compartment types to force for entity
	*/
	void ForceVehicleCompartments(notnull array<ECompartmentType> forceVehicleCompartments)
	{

	}

	//------------------------------------------------------------------------------------------------
	protected void OnParentEntityChanged(SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev, bool changedByUser)
	{
		m_ParentEntity = parentEntity;

		if (parentEntity != parentEntityPrev)
			RemoveFromParent(parentEntityPrev, changedByUser);

		AddToParent(parentEntity, changedByUser);

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnParentEntityChanged.Invoke(this, parentEntity, parentEntityPrev);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetParentEntityBroadcast(SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev, bool changedByUser = false, bool isAutoRegistration = false)
	{
		//if (!CanSetParent(parentEntity)) //--- Prevents players from being registered. ToDo: Fix
		//	return;

		//--- Not registered, do it first (when allowed) and exit. This function will be called again from Register()
		if (!IsRegistered())
		{
			//--- Only when auto-registration is set to ALWAYS, not NEVER
			if (!isAutoRegistration || m_bAutoRegister <= EEditableEntityRegister.ALWAYS)
			{
				m_ParentEntity = parentEntity;
				OnRegistrationChanged(true);
			}
			return;
		}

		//--- Modify entity's parent when...
		if (
			//--- ...parent is root and entity is registered
			(!parentEntity && IsRegistered())
			||
			//--- ...parent is an entity and it has the same registration as this entity (so that registration doesn't change)
			(parentEntity && parentEntity.IsRegistered() == IsRegistered())
		)
		{
			OnParentEntityChanged(parentEntity, parentEntityPrev, changedByUser);
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetParentEntityBroadcastReceive(RplId parentEntityID, RplId parentEntityPrevID, bool changedByUser)
	{
		SCR_EditableEntityComponent parentEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(parentEntityID));
		SCR_EditableEntityComponent parentEntityPrev = SCR_EditableEntityComponent.Cast(Replication.FindItem(parentEntityPrevID));

		//--- When parent entity was not streamed in yet, register this entity as an orphan that will be fully registered after the parent is initialized
		if (
			(parentEntityID.IsValid() && !parentEntity) //--- Parent expected, but not found
			|| (parentEntity && !parentEntity.IsRegistered()) //--- Parent found, but not registered yet (e.g., when Replication order is different)
		)
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			core.AddOrphan(parentEntityID, Replication.FindId(this));
		}
		else
		{
			SetParentEntityBroadcast(parentEntity, parentEntityPrev, changedByUser);
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnRegistrationChanged(bool toRegister)
	{
		set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>;
		GetChildren(children);

		if (toRegister)
		{
			Register();
			foreach (SCR_EditableEntityComponent child : children)
			{
				child.Register();
			}
		}
		else
		{
			Unregister();
			foreach (SCR_EditableEntityComponent child : children)
			{
				child.Unregister();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected EEditableEntityRegister GetAutoRegister()
	{
		return m_bAutoRegister;
	}

	//------------------------------------------------------------------------------------------------
	protected void Register()
	{
		if (GetGame().GetWorld() != m_Owner.GetWorld())
  			return;
		
		if (IsRegistered())
			return;

		m_bAutoRegister = -1; //--- Mark as registered locally
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.RegisterEntity(this);
		SetParentEntityBroadcast(m_ParentEntity, m_ParentEntity);

		//--- Restore orphaned entites belonging to this parent
		array<SCR_EditableEntityComponent> orphans = {};
		for (int i = 0, count = core.RemoveOrphans(Replication.FindId(this), orphans); i < count; i++)
		{
			orphans[i].SetParentEntityBroadcast(this, orphans[i].GetParentEntity());
			Print(string.Format("Editor parent %1 restored for orphan %2", this, orphans[i]), LogLevel.VERBOSE);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Unregister(IEntity owner = null)
	{
		if (!IsRegistered())
			return;

		m_bAutoRegister = EEditableEntityRegister.ALWAYS; //--- Mark as unregistered locally

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.UnRegisterEntity(this, owner);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCreatedServer(notnull SCR_PlacingEditorComponent placedEditorComponent)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void AddToParent(SCR_EditableEntityComponent parentEntity, bool changedByUser)
	{
		if (parentEntity)
		{
			parentEntity.AddChild(this);

			//--- Mark a dirty
			if (changedByUser)
				SetHierarchyAsDirtyInParents();
		}
		else
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			if (core)
				core.AddToRoot(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveFromParent(SCR_EditableEntityComponent parentEntity, bool changedByUser)
	{
		if (parentEntity)
		{
			//--- Mark a dirty
			if (changedByUser)
				SetHierarchyAsDirtyInParents();

			parentEntity.RemoveChild(this);
		}
		else
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			if (core)
				core.RemoveFromRoot(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddChild(SCR_EditableEntityComponent entity)
	{
		if (entity.GetParentEntity() != this)
			return;

		//--- Add to editor hierarchy
		if (!m_Entities)
			m_Entities = new set<SCR_EditableEntityComponent>();
		m_Entities.Insert(entity);

		//--- Add to game hierarchy as well
		if (entity.HasEntityFlag(EEditableEntityFlag.GAME_HIERARCHY))
			UpdateGameHierarchy(m_Owner, entity.GetOwner(), true);

		OnChildEntityChanged(entity, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveChild(SCR_EditableEntityComponent entity)
	{
		if (entity.GetParentEntity() == this || !m_Entities)
			return;

		//--- Not a child of this entity, ignore
		int index = m_Entities.Find(entity);
		if (index == -1)
			return;

		//--- Remove from editor hierarchy
		m_Entities.Remove(index);
		if (m_Entities.IsEmpty())
			m_Entities = null;

		//--- Remove from game hierarchy
		if (entity.HasEntityFlag(EEditableEntityFlag.GAME_HIERARCHY))
			UpdateGameHierarchy(m_Owner, entity.m_Owner, false); //--- Don't use entity.GetOwner(), would cause a crash when closing the game

		OnChildEntityChanged(entity, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateGameHierarchy(IEntity parent, IEntity child, bool toAdd)
	{
		if (!parent || !child)
			return;

		if (parent && parent.IsDeleted())
			return;
		if (child && child.IsDeleted())
			return;

		//--- Modify the hierarchy (on clients as well; ToDo: Rely on AutoHierarchy in RplComponent)
		if (toAdd)
		{
			parent.AddChild(child, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM);
		}
		else
		{
			vector transform[4];
			child.GetWorldTransform(transform);
			parent.RemoveChild(child);
			child.SetWorldTransform(transform);
		}
	}

	//------------------------------------------------------------------------------------------------
	//--- To be overloaded by inherited classes
	protected void OnChildEntityChanged(SCR_EditableEntityComponent child, bool isAdded);

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- State (local)
	/*!
	Check if given entity state is active.
	\param state
	\return True if the state is active
	*/
	bool HasEntityState(EEditableEntityState state)
	{
		return (m_EntityState & state) == state;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check bit array with all currently active states
	\return Bit array
	*/
	EEditableEntityState GetEntityStates()
	{
		return m_EntityState;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set value of an entity state. Multiple states can exist at the same time (e.g., HOVER and SELECTED).

	<b>State has only informational value!</b> For example setting it to SELECTED will not actually select the entity.
	\param state Target state
	\param toSet True to activate the state, false to deactivate
	\param reset True to reset the currentt values before setting the new one
	*/
	void SetEntityState(EEditableEntityState state, bool toSet)
	{
		if (toSet)
		{
			if (m_EntityState & state)
				return;

			m_EntityState = m_EntityState | state;
		}
		else
		{
			if (!(m_EntityState & state))
				return;

			m_EntityState = m_EntityState &~ state;
		}

		if (HasEntityFlag(EEditableEntityFlag.VIRTUAL))
			SetEntityStateInChildren(m_Owner, state, toSet);

		//--- Global event call disabled, takes too much performance when editor mode is being initialized
		//SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		//if (core) core.Event_OnEntityStateChanged.Invoke(this, state, toSet);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Reset all entity states.
	*/
	void ResetEntityStates()
	{
		EEditableEntityState state = m_EntityState;
		m_EntityState = 0;

		if (HasEntityFlag(EEditableEntityFlag.VIRTUAL))
			SetEntityStateInChildren(m_Owner, state, false);
		/*
		//--- Deconstruct state flags and call handlers on each individual flag
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		int state = 1;
		while (m_EntityState > 0 && state < int.MAX)
		{
			if (m_EntityState & state)
			{
				SetEntityStateInChildren(m_Owner, state, false);
				//if (core) core.Event_OnEntityStateChanged.Invoke(this, state, false);
				m_EntityState -= state;
			}
			state *= 2;
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if a flag is active.
	\param flag
	\return True if the flag is active
	*/
	bool HasEntityFlag(EEditableEntityFlag flag)
	{
		return (m_Flags & flag) == flag;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity flags
	\return Entity flags
	*/
	EEditableEntityFlag GetEntityFlags()
	{
		return m_Flags;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set entity flag.
	\param flag Flag type
	\return toSet True to set the flag
	*/
	void SetEntityFlag(EEditableEntityFlag flag, bool toSet)
	{
		if (toSet)
		{
			if (m_Flags & flag)
				return;
			m_Flags = m_Flags | flag;
		}
		else
		{
			if (!(m_Flags & flag))
				return;
			m_Flags = m_Flags &~ flag;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEntityStateInChildren(IEntity owner, EEditableEntityState state, bool toSet, out array<Managed> components = null)
	{
		if (!owner)
			return;
		
		//~ Get editable entity
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		
		//~ Stop on editable entity that is not owner and does not have the VIRTUAL flag
		if (owner != m_Owner && editableEntity && !SCR_Enum.HasFlag(editableEntity.GetEntityFlags(), EEditableEntityFlag.VIRTUAL))
			return;
		
		//--- Call event on all SCR_EditableEntityBaseChildComponent on this entity
		if (!components)
			components = {};

		int componentsCount = owner.FindComponents(SCR_EditableEntityBaseChildComponent, components);
		for (int i = 0; i < componentsCount; i++)
		{
			SCR_EditableEntityBaseChildComponent component = SCR_EditableEntityBaseChildComponent.Cast(components[i]);
			if (component && component.CanApply(state))
				component.EOnStateChanged(GetEntityStates(), state, toSet);
		}

		//--- Go deeper
		IEntity child = owner.GetChildren();
		while (child)
		{
			SetEntityStateInChildren(child, state, toSet, components);
			child = child.GetSibling();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Visibility (local)
	protected void OnVisibilityChanged()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		core.Event_OnEntityVisibilityChanged.Invoke(this);
	}

	/*! @name Visibility
	Functions to manage visibility of the entity.
	*/
	///@{

	//------------------------------------------------------------------------------------------------
	/*!
	Set entity visibility. When set to false, the entity will not be shown to the user.
	Visibility is local to editor user.
	\param show True to show, false to hide
	*/
	void SetVisible(bool show)
	{
		//SetEntityState(EEditableEntityState.VISIBLE, show, !show);
		m_bVisible = show;
		OnVisibilityChanged();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check visibility setting of the entity.
	\return True if the entity is set as visible
	*/
	bool GetVisibleSelf()
	{
		//return HasEntityState(EEditableEntityState.VISIBLE);
		return m_bVisible;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check visibility setting of the entity in hierarchy (e.g., if an entity is set as visible, but its parent is not, false will be returned).
	\return True if the entity is visible in hierarchy
	*/
	bool GetVisibleInHierarchy()
	{
		if (m_ParentEntity)
			return m_ParentEntity.GetVisibleInHierarchy();
		else
			return GetVisibleSelf();
	}
	///@}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Access key
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnAccessKeyChanged(EEditableEntityAccessKey accessKey)
	{
		m_AccessKey = accessKey;

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;

		core.Event_OnEntityAccessKeyChanged.Invoke(this);

		//--- Execute handler also on all children
		SCR_AccessKeysEditorComponent accessKeysComponent = SCR_AccessKeysEditorComponent.Cast(SCR_AccessKeysEditorComponent.GetInstance(SCR_AccessKeysEditorComponent));
		EEditableEntityAccessKey editorAccessKey = -1;
		if (accessKeysComponent)
			editorAccessKey = accessKeysComponent.GetAccessKey();

		set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>;
		GetChildren(children, false, editorAccessKey);
		foreach (SCR_EditableEntityComponent child : children)
		{
			core.Event_OnEntityAccessKeyChanged.Invoke(child);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*! @name Access Key
	Functions to manage access key.
	- Access key is shared over network and define who has rights to edit the entity.
	- Key is a flag composed of multiple EEditableEntityAccessKey values.
	- When at least one of the values is compatible with editor key values, the entity will become available.
	*/
	///@{
	/*!
	Add access key.
	Available only on server!
	\param accessKey Key to be added
	*/
	void AddAccessKey(EEditableEntityAccessKey accessKey)
	{
		if (!IsServer()/* || !IsReplicated()*/ || HasAccessSelf(accessKey))
			return;

		m_AccessKey = m_AccessKey | accessKey;
		OnAccessKeyChanged(m_AccessKey);
		if (CanRpc())
			Rpc(OnAccessKeyChanged, m_AccessKey);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Remove access key.
	Available only on server!
	\param accessKey Key to be removed
	*/
	void RemoveAccessKey(EEditableEntityAccessKey accessKey)
	{
		if (!IsServer()/* || !IsReplicated()*/ || !HasAccessSelf(accessKey))
			return;
		m_AccessKey = m_AccessKey &~ accessKey;
		OnAccessKeyChanged(m_AccessKey);
		if (CanRpc())
			Rpc(OnAccessKeyChanged, m_AccessKey);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Clear all access keys.
	Available only on server!
	*/
	void ClearAccessKeys()
	{
		if (!IsServer()/* || !IsReplicated()*/)
			return;

		m_AccessKey = 0;
		OnAccessKeyChanged(m_AccessKey);
		if (CanRpc())
			Rpc(OnAccessKeyChanged, m_AccessKey);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity's access key.
	\return Access key
	*/
	EEditableEntityAccessKey GetAccessKey()
	{
		return m_AccessKey;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check access key of the entity.
	\param accessKey Access key which be checked in entity's access key
	\return True if given access key and entity's access key is compatible
	*/
	bool HasAccessSelf(EEditableEntityAccessKey accessKey)
	{
		return m_AccessKey & accessKey;// || accessKey == int.MAX; //--- Enable this to let admin see also entities without any key
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check acces keys of the entity in hierarchy (e.g., if an entity is compatible, but its parent is not, false will be returned).
	\param accessKey Access key which be checked in entity's access key
	\return True if given access key and entity's access key is compatible
	*/
	bool HasAccessInHierarchy(EEditableEntityAccessKey accessKey)
	{
		if (m_ParentEntity)
			return m_ParentEntity.HasAccessInHierarchy(accessKey);
		else
			return HasAccessSelf(accessKey);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check access key of the entity compared to those defined in SCR_AccessKeysEditorComponent.
	\return True if given access key and entity's access key are compatible
	*/
	bool HasAccessSelf()
	{
		SCR_AccessKeysEditorComponent accessKeysComponent = SCR_AccessKeysEditorComponent.Cast(SCR_AccessKeysEditorComponent.GetInstance(SCR_AccessKeysEditorComponent));
		if (!accessKeysComponent)
			return true;

		return HasAccessSelf(accessKeysComponent.GetAccessKey());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check access key of the entity in hierarchy (e.g., if an entity is compatible, but its parent is not, false will be returned).
	Compared with the key defined in SCR_AccessKeysEditorComponent.
	\return True if given access key and entity's access key are compatible
	*/
	bool HasAccessInHierarchy()
	{
		SCR_AccessKeysEditorComponent accessKeysComponent = SCR_AccessKeysEditorComponent.Cast(SCR_AccessKeysEditorComponent.GetInstance(SCR_AccessKeysEditorComponent));
		if (!accessKeysComponent)
			return true;

		return HasAccessInHierarchy(accessKeysComponent.GetAccessKey());
	}
	///@}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Support Functions

	//------------------------------------------------------------------------------------------------
	//--- Will send event for GUI to refresh instantly, to be called from inherited classes in case initialization takes a bit longer.
	protected void Refresh()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityRefreshed.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected RplComponent GetRplComponent()
	{
		return RplComponent.Cast(GetOwner().FindComponent(RplComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected bool ValidateType()
	{
		switch (GetEntityType())
		{
			case EEditableEntityType.CHARACTER:
				return m_Owner.IsInherited(ChimeraCharacter) || m_Owner.FindComponent(SCR_EditablePlayerDelegateComponent) != null;

			//--- No longer valid, tripods are considered vehicles even when not inheriting from class Vehicle
			//case EEditableEntityType.VEHICLE:
			//	return m_Owner.IsInherited(Vehicle);

			case EEditableEntityType.GROUP:
				return m_Owner.IsInherited(AIGroup);

			case EEditableEntityType.WAYPOINT:
				return m_Owner.IsInherited(AIWaypoint);

			case EEditableEntityType.COMMENT:
				return this.IsInherited(SCR_EditableCommentComponent);

			case EEditableEntityType.ITEM:
				return m_Owner.FindComponent(InventoryItemComponent) != null;
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Log
	string GetLogText(string prefix = "")
	{
		string displayName = GetDisplayName();
		string prefabName;
		EntityPrefabData prefabData = m_Owner.GetPrefabData();
		if (prefabData)
			prefabName = FilePath.StripPath(prefabData.GetPrefabName());

		string space = " ";
		for (int i = 0; i < 50 - prefix.Length() - displayName.Length(); i++)
		{
			space += " ";
		}

		return string.Format("%1%2 | entity: %3, prefab: '%4', pos: %5, flags: %6", displayName, space, m_Owner, prefabName, m_Owner.GetOrigin(), SCR_Enum.FlagsToString(EEditableEntityFlag, m_Flags));
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Print out entity information.
	\param prefix Text added before the printed text
	\param onlyDirect When true, only the direct descendants are logged, otherwise all children, children of children etc. are logged.
	\param logLevel Log level type
	*/
	void Log(string prefix = "", bool onlyDirect = false, LogLevel logLevel = LogLevel.DEBUG)
	{
		if (!m_Owner)
		{
			Print(string.Format(prefix + " - Error: Null m_Owner in %1", this), LogLevel.WARNING);
			return;
		}

		if (m_Entities && !m_Entities.IsEmpty())
		{
			Print(prefix + "+ " + GetLogText(prefix), logLevel);
			if (onlyDirect)
				return;

			foreach (SCR_EditableEntityComponent entity : m_Entities)
			{
				if (entity)
					entity.Log(prefix + " ");
				else
					Log(string.Format(prefix + " - Error: Null child in %1!", this), true, LogLevel.WARNING)
			}
		}
		else
		{
			Print(prefix + "- " + GetLogText(prefix), logLevel);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Print out the entity and all its descendants which are compatible with given key.
	\param prefix Text added before the printed text
	\param accessKey Access key which be checked in entity's access key
	*/
	void Log(string prefix, EEditableEntityAccessKey accessKey)
	{
		if (!m_Owner)
			return;

		if (m_Entities && !m_Entities.IsEmpty())
		{
			Print(prefix + "+ " + GetLogText(prefix), LogLevel.DEBUG);
			foreach (SCR_EditableEntityComponent entity : m_Entities)
			{
				if (entity.HasAccessSelf(m_AccessKey))
					entity.Log(prefix + " ", accessKey);
			}
		}
		else
		{
			Print(prefix + "- " + GetLogText(prefix), LogLevel.DEBUG);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Print out entity's access key
	*/
	void LogAccessKey()
	{
		string output = "";
		typename enumType = EEditableEntityAccessKey;
		int enumCount = enumType.GetVariableCount();
		for (int i = 0; i < enumCount; i++)
		{
			int val;
			if (enumType.GetVariableType(i) == int && enumType.GetVariableValue(null, i, val))
			{
				if (HasAccessSelf(val))
				{
					if (!output.IsEmpty())
						output += ", ";

					output += enumType.GetVariableName(i);
				}
			}
		}
		Print(string.Format("%1 access key: %2", GetDisplayName(), output), LogLevel.DEBUG);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- External events

	//------------------------------------------------------------------------------------------------
	/*!
	Event called on server when the entity is placed in the editor.
	\param[out] parent Editable entity in which the new one is being created (rewrite the variable the change the parent)
	\param recipient Entity that receives this editable entity (e.g., a group receiving a waypoint)
	\param isQueue True if the entity was placed in a queue (i.e., placing remains active)
	\param flags Placing flags enabled by user
	\return Editable entity which is added to editor hieraechy (can be overloaded, e.g., to provide group after spawning a character)
	*/
	SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue)
	{
		return this;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Event called on server when the session is being loaded by SCR_EditableEntityStruct
	\param parent Editable entity in which the new one is being created
	*/
	void EOnEditorSessionLoad(SCR_EditableEntityComponent parent)
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(GetOwnerScripted());
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions

	//------------------------------------------------------------------------------------------------
	//--- JIP on server
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.Write(m_AccessKey, 32);

		RplId parentID = Replication.FindId(m_ParentEntity);
		writer.WriteRplId(parentID);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//--- JIP on client
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.Read(m_AccessKey, 32);

		RplId parentID;
		reader.ReadRplId(parentID);

		SetParentEntityBroadcastReceive(parentID, parentID, false);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		//--- Marked as -1 in constructor when edit mode is active
		if (m_EntityState == -1)
			return;

		//--- Delete entities in editor hierarchy
		if (IsServer() && m_Entities)
		{
			for (int i = m_Entities.Count() - 1; i >= 0; i--)
			{
				SCR_EditableEntityComponent entity = m_Entities[i];
				if (!entity.HasEntityFlag(EEditableEntityFlag.GAME_HIERARCHY))
					m_Entities[i].Delete(false);
			}
		}

		//--- Unregister
		SCR_EditableEntityComponent parentEntity = GetParentEntity();
		m_ParentEntity = null; //--- Clear the variable, otherise condition in RemoveChild() would ignore this
		RemoveFromParent(parentEntity, false);
		Unregister(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!m_Owner)
			return;

		//--- Determine entity type
#ifdef WORKBENCH
		if (!ValidateType())
			Log(string.Format("Wrong type %1 used!", typename.EnumToString(EEditableEntityType, GetEntityType())), true, LogLevel.WARNING);
#endif

		//--- Update static pos
		SetStatic(m_bStatic);

		//--- Exit in edit mode, can wreck havoc when WB thumbnails of editable entities are loaded while the game is running
		if (SCR_Global.IsEditMode(owner))
			return;

		//--- Check for presence of RplComponent. Entity cannot be editable without it.
		RplComponent rpl = RplComponent.Cast(m_Owner.FindComponent(RplComponent));
		if (HasEntityFlag(EEditableEntityFlag.LOCAL))
		{
			if (rpl)
			{
				Print(string.Format("Editable entity @\"%1\" is flagged as LOCAL, but contains RplComponent!", GetPrefab()), LogLevel.ERROR);
				m_Owner = null;
				return;
			}
		}
		else
		{
			if (!rpl)
			{
				Print(string.Format("Editable entity @\"%1\" is missing RplComponent! Maybe try regenerating it again.", GetPrefab()), LogLevel.ERROR);
				m_Owner = null;
				return;
			}
		}

		//--- Make sure distance value is squared
		if (m_fMaxDrawDistance != 0)
			m_fMaxDrawDistance *= m_fMaxDrawDistance;

		//--- Register to the system
		if (IsServer())
		{
			SetParentEntityBroadcast(m_ParentEntity, m_ParentEntity, isAutoRegistration: true);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_EditableEntityComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_DISABLE))
			return;

		m_Owner = GenericEntity.Cast(ent);

		//--- Ignore when not in run-time
		if (!GetGame() || SCR_Global.IsEditMode(ent))
		{
			m_EntityState = -1; //--- Tell destructor and OnDelete to terminate
			return;
		}

		//--- Get bone index on which the icon will be rendered
		SCR_EditableEntityComponentClass prefabData = GetEditableEntityData();
		if (prefabData)
		{
			string boneName = prefabData.GetIconBoneName();
			if (!boneName.IsEmpty())
				m_iIconBoneIndex = m_Owner.GetAnimation().GetBoneIndex(boneName);
		}

		//--- Get parent entity
		if (parent)
		{
			GenericEntity genericParent = GenericEntity.Cast(parent);
			SCR_EditableEntityComponent parentEntity;
			if (genericParent)
			{
				parentEntity = SCR_EditableEntityComponent.Cast(genericParent.FindComponent(SCR_EditableEntityComponent));
				if (parentEntity)
				{
					//--- Cannot have worse auto-registration settings than parent (to make sure child entities are registered)
					if (m_bAutoRegister != EEditableEntityRegister.NEVER)
						m_bAutoRegister = Math.Min(m_bAutoRegister, parentEntity.GetAutoRegister());

					//--- Cannot be marked as registered yet (registered value is -1, see Register())
					m_bAutoRegister = Math.Max(m_bAutoRegister, EEditableEntityRegister.ALWAYS);
				}
			}
			m_ParentEntity = parentEntity;
		}

		//--- When spawned dynamically, change auto-registration WHEN_SPAWNED to ALWAYS
		if (m_bAutoRegister == EEditableEntityRegister.WHEN_SPAWNED && !m_Owner.IsLoaded())
			m_bAutoRegister = EEditableEntityRegister.ALWAYS;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_EditableEntityComponent()
	{

	}
};
