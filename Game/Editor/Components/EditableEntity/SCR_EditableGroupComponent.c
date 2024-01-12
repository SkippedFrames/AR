[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableGroupComponentClass: SCR_EditableEntityComponentClass
{
	static override bool GetEntitySourceBudgetCost(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetValues)
	{
		// Avoid fallback entityType cost
		return true;
	}
};

/** @ingroup Editable_Entities
*/

/*!
Special configuration for editable group.
*/
class SCR_EditableGroupComponent : SCR_EditableEntityComponent
{	
	protected SCR_AIGroup m_Group;

	[RplProp(onRplName: "OnLeaderIdChanged")]
	protected RplId m_LeaderId;
	protected SCR_EditableEntityComponent m_Leader;
	protected ref SCR_EditableGroupUIInfo m_GroupInfo;
	protected ref ScriptInvoker Event_OnUIRefresh = new ref ScriptInvoker;
	
	//~ Authority only, Forces spawned characters to be added to a specific vehicle position and will delete it if failed
	protected ref array<ECompartmentType> m_aForceSpawnVehicleCompartments;
	
	protected SCR_PlacingEditorComponent m_PlacedEditorComponent;

	/*
	protected void OnEmpty()
	{
	}
	*/
	protected void OnAgentAdded(AIAgent child)
	{
		if (!child)
			return;
		
		//--- Add newly joined soldier to group's layer
		SCR_EditableEntityComponent editableChild = SCR_EditableEntityComponent.GetEditableEntity(child.GetControlledEntity());
		if (editableChild)
			editableChild.SetParentEntity(this);
	}
	protected void OnAgentRemoved(SCR_AIGroup group, AIAgent child)
	{
		if (!child)
			return;
		
		//--- Remove the soldier who left from group's layer (only if the soldier is still this group's child)
		SCR_EditableEntityComponent editableChild = SCR_EditableEntityComponent.GetEditableEntity(child.GetControlledEntity());
		if (editableChild && editableChild.GetParentEntity() == this)
			editableChild.SetParentEntity(null);
	}
	protected void OnLeaderChanged(AIAgent currentLeader, AIAgent prevLeader)
	{
		if (!currentLeader)
			return;
		
		//--- Update pointer to the leader on all clients
		SCR_EditableEntityComponent editableLeader = SCR_EditableEntityComponent.GetEditableEntity(currentLeader.GetControlledEntity());
		if (editableLeader)
		{
			m_Leader = editableLeader;
			m_LeaderId = Replication.FindId(editableLeader);
			Replication.BumpMe();
			Refresh(); //--- Make the icon appear instantly
		}
	}
	protected void OnCurrentWaypointChanged(AIWaypoint currentWP, AIWaypoint prevWP)
	{
		ReindexWaypoints();
	}
	protected void OnWaypointCompleted(AIWaypoint wp)
	{
		ReindexWaypoints();
	}
	protected void OnWaypointAdded(AIWaypoint wp)
	{
		SCR_EditableWaypointComponent waypoint = SCR_EditableWaypointComponent.Cast(SCR_EditableEntityComponent.GetEditableEntity(wp));
		if (waypoint)
		{
			waypoint.SetParentEntity(this);
		}
		ReindexWaypoints();
	}
	protected void OnWaypointRemoved(AIWaypoint wp)
	{
		//--- Delete waypoints which were not assigned to another group
		SCR_EditableWaypointComponent waypoint = SCR_EditableWaypointComponent.Cast(SCR_EditableEntityComponent.GetEditableEntity(wp));
		if (waypoint && waypoint.GetParentEntity() == this)
		{
			waypoint.Delete();
		}
		ReindexWaypoints();
	}
	
	protected void ReindexWaypoints()
	{
		int index = 1;
		SCR_EditableWaypointComponent waypoint, currentWaypoint;
		array<SCR_EditableWaypointComponent> waypoints = new array<SCR_EditableWaypointComponent>;
		
		bool hasNonEditable, hasNull;
		AIWaypoint aiWaypoint;
		AIWaypoint currentAiWaypoint = m_Group.GetCurrentWaypoint();
		array<AIWaypoint> aiWaypoints = {};
		for (int i = 0, count = m_Group.GetWaypoints(aiWaypoints); i < count; i++)
		{
			aiWaypoint = aiWaypoints[i];
			waypoint = SCR_EditableWaypointComponent.Cast(SCR_EditableEntityComponent.GetEditableEntity(aiWaypoint));
			if (waypoint)
			{
				waypoints.Insert(waypoint);
				if (aiWaypoint == currentAiWaypoint)
					currentWaypoint = waypoint;
			}
			else if (aiWaypoint)
			{
				hasNonEditable = true;
			}
			else
			{
				hasNull = true;
			}
		}
		
		//--- Show warning when the group mixes editable and non-editable waypoints. The latter will not be visible in the editor, potentially causing confusion.
		if (hasNull)
		{
			Log("Group contains null waypoints!", true, LogLevel.WARNING);
		}
		else if (!waypoints.IsEmpty() && hasNonEditable)
		{
			Log("Group has a mix of editable and non-editable waypoints. Please use only one of those!", true, LogLevel.WARNING);
		}
		
		SCR_EditableWaypointComponent prevWaypoint;
		for (int i = 0, count = waypoints.Count(); i < count; i++)
		{
			waypoint = waypoints[i];
			waypoint.SetWaypointIndex(i + 1, waypoint == currentWaypoint, prevWaypoint);
			prevWaypoint = waypoint;
		}
	}	
	
	protected void OnLeaderIdChanged()
	{
		//--- Retrieve new leader from RplID
		SetLeader(SCR_EditableEntityComponent.Cast(Replication.FindItem(m_LeaderId)));
	}
	protected void SetLeader(SCR_EditableEntityComponent leader)
	{
		if (leader)
		{
			m_Leader = leader;
			Refresh();
		}
	}
	
	protected void OnFactionChanged(Faction faction)
	{
		Event_OnUIRefresh.Invoke();
		
		//Call on faction changed of group children
		if (m_Entities)
		{
			foreach (SCR_EditableEntityComponent entity: m_Entities)
			{
				SCR_EditableCharacterComponent editableCharacter = SCR_EditableCharacterComponent.Cast(entity);
				if (editableCharacter)
					editableCharacter.OnFactionChanged();
			}
		}
	}
	protected void OnIdentityChange(SCR_MilitarySymbol symbol, LocalizedString name)
	{
		//--- Create UI info for group instance, so we can change its symbol
		if (!m_GroupInfo)
		{
			m_GroupInfo = new SCR_EditableGroupUIInfo();
			m_GroupInfo.CopyFrom(GetInfo());
			SetInfoInstance(m_GroupInfo);
		}
		
		m_GroupInfo.SetInstance(symbol, name);
		
		Event_OnUIRefresh.Invoke();
	}
	/*!
	Get ai group component, server only
	\return m_Group
	*/
	SCR_AIGroup GetAIGroupComponent()
	{
		return m_Group;
	}
	
	override ScriptInvoker GetOnUIRefresh()
	{
		return Event_OnUIRefresh;
	}
	
	override Faction GetFaction()
	{
		Faction faction;
		
		if (m_Leader)
			faction = m_Leader.GetFaction();
		
		if (!faction)
			faction = m_Group.GetFaction();
		
		return faction;
	}
	override SCR_EditableEntityComponent GetAIGroup()
	{
		return this;
	}
	override SCR_EditableEntityComponent GetAIEntity()
	{
		return m_Leader;
	}	
	/*!
	Get number of group members.
	\return Number of group members
	*/
	int GetSize()
	{
		int size = 0;
		if (m_Entities)
		{
			foreach (SCR_EditableEntityComponent child: m_Entities)
			{
				if (child.GetEntityType() == EEditableEntityType.CHARACTER)
					size++;
			}
		}
		return size;
	}
	/*!
	Get count of all waypoints of this group
	\return Number of waypoints
	*/
	int GetWaypointCount()
	{
		int count = 0;
		/*set <SCR_EditableEntityComponent> groupChildren = new set <SCR_EditableEntityComponent>;
		GetChildren(groupChildren, true);*/
		
		if (m_Entities)
		{
			foreach (SCR_EditableEntityComponent child: m_Entities)
			{
				if (child.GetEntityType() == EEditableEntityType.WAYPOINT)
					count++;
			}
		}
		
		return count;
	}
	/*!
	Remove all waypoints from the group.
	*/
	void ClearWaypoints()
	{
		if (!m_Group)
			return;
		
		array<AIWaypoint> aiWaypoints = new array<AIWaypoint>;
		for (int i = 0, count = m_Group.GetWaypoints(aiWaypoints); i < count; i++)
		{
			SCR_EditableWaypointComponent waypoint = SCR_EditableWaypointComponent.Cast(SCR_EditableEntityComponent.GetEditableEntity(aiWaypoints[i]));
			if (waypoint)
			{
				waypoint.Delete();
			}
			else
			{
				//--- Unassign non-editable waypoint, but don't delete it (multiple groupls may be assigned to it)
				m_Group.RemoveWaypoint(aiWaypoints[i]);
			}
		}
	}
	
	override bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets, IEntity owner = null)
	{
		// Return empty cost array, avoid fallback entityType cost
		return true;
	}
	
	override bool CanDuplicate(out notnull set<SCR_EditableEntityComponent> outRecipients)
	{
		return false;
	}
	
	override SCR_EditableEntityComponent GetChild(int index)
	{
		if (!m_Leader)
			return m_Entities[index]; //--- No leader, use default method
		if (index == 0)
			return m_Leader; //--- Return leader as the first item (needed for session saving - SCR_EditableEntityStruct)
		else if (index <= m_Entities.Find(m_Leader))
			return m_Entities[index - 1]; //--- Before leader, push by one
		else
			return m_Entities[index]; //--- After leader, use default method
	}
	
	override bool GetPos(out vector pos)
	{
		if (!m_Leader)
			return false;
		
		if (!m_Leader.GetPos(pos))
			return false;
		
		pos += GetIconPos();
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanDestroy()
	{
		set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>();
		GetChildren(children, true);
		
		foreach (SCR_EditableEntityComponent child : children)
		{
			if (child.CanDestroy())
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsDestroyed()
	{
		set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>();
		GetChildren(children, true);
		
		foreach (SCR_EditableEntityComponent child : children)
		{
			if (!child.IsDestroyed())
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Destroy()
	{
		if (!IsServer())
			return false;
		
		set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>();
		GetChildren(children, true);
		
		bool isDestroyed = true;
		
		foreach (SCR_EditableEntityComponent child : children)
		{
			isDestroyed &= child.Destroy();
		}
		
		return isDestroyed;
	}
	/*
	override bool CanSetParent(SCR_EditableEntityComponent parentEntity)
	{
		if (!parentEntity)
			return true;
		
		EEditableEntityType type = parentEntity.GetEntityType();
		return super.CanSetParent(parentEntity) || type == EEditableEntityType.CHARACTER || type == EEditableEntityType.GROUP || type == EEditableEntityType.VEHICLE;             
	}
	*/
	
	//~ Authority Only. When spawned will force characters of group into vehicle position
	override void ForceVehicleCompartments(notnull array<ECompartmentType> forceVehicleCompartments)
	{
		if (forceVehicleCompartments.IsEmpty())
			return;
		
		m_aForceSpawnVehicleCompartments = {};
		
		foreach (ECompartmentType compartment: forceVehicleCompartments)
			m_aForceSpawnVehicleCompartments.Insert(compartment);
	}
	
	override void OnParentEntityChanged(SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev, bool changedByUser)
	{
		EEditableEntityType parentType;
		if (parentEntity) parentType = parentEntity.GetEntityType();
		
		switch (parentType)
		{
			case EEditableEntityType.GROUP:
			case EEditableEntityType.CHARACTER:
			case EEditableEntityType.VEHICLE:
			{
				if (!IsServer()) 
					break;
				
				set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>;
				GetChildren(children, true);
				if (children.IsEmpty() && !parentEntityPrev)
				{
					//--- Group is still empty after placing. Wait a bit before attempting to move soldiers under another parent (to prevent endless loop, do it only once by overriding parentEntityPrev)
					GetGame().GetCallqueue().CallLater(OnParentEntityChanged, 1, false, parentEntity, parentEntity, changedByUser);
				}
				else
				{
					foreach (SCR_EditableEntityComponent child: children)
					{
						if (child.GetEntityType() == EEditableEntityType.CHARACTER)
						{
							//~ Force children in vehicle position if any are assigned
							if (m_aForceSpawnVehicleCompartments)
								child.ForceVehicleCompartments(m_aForceSpawnVehicleCompartments);
							
							child.SetParentEntity(parentEntity);
						}
					}
					
					//~ Clear force positions
					if (m_aForceSpawnVehicleCompartments)
						m_aForceSpawnVehicleCompartments = null;
				}
				break;
			}
			default:
			{
				super.OnParentEntityChanged(parentEntity, parentEntityPrev, changedByUser);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnCreatedServer(notnull SCR_PlacingEditorComponent placedEditorComponent)
	{
		super.OnCreatedServer(placedEditorComponent);

		m_PlacedEditorComponent = placedEditorComponent;
		if (m_Group == null)
			return;

		int numberOfMembersToSpawn = m_Group.GetNumberOfMembersToSpawn();
		if (numberOfMembersToSpawn < 1)
			return;

		m_Group.GetOnAllDelayedEntitySpawned().Insert(OnAllMembersSpawned);
		m_PlacedEditorComponent.SetPlacingBlocked(true);

		SCR_BudgetEditorComponent budgetComponent = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent));
		if (!budgetComponent)
			return;

		SCR_EditableEntityCoreBudgetSetting aiBudget = budgetComponent.GetBudgetSetting(EEditableEntityBudget.AI);
		if (aiBudget)
			aiBudget.ReserveBudget(numberOfMembersToSpawn + 1);
	}

	//------------------------------------------------------------------------------------------------
	void OnAllMembersSpawned(SCR_AIGroup group)
	{
		if (!m_PlacedEditorComponent || !m_Group)
			return;

		m_Group.GetOnAllDelayedEntitySpawned().Remove(OnAllMembersSpawned);

		m_PlacedEditorComponent.SetPlacingBlocked(false);

		SCR_BudgetEditorComponent budgetComponent = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent));
		if (!budgetComponent)
			return;

		SCR_EditableEntityCoreBudgetSetting aiBudget = budgetComponent.GetBudgetSetting(EEditableEntityBudget.AI);
		if (aiBudget)
			aiBudget.UnreserveBudget(m_Group.GetNumberOfMembersToSpawn() + 1);
	}

	override void OnChildEntityChanged(SCR_EditableEntityComponent child, bool isAdded)
	{
		if (!IsServer())
		{
			if (isAdded && Replication.FindId(child) == m_LeaderId)
				SetLeader(child);
			
			return;
		}
		
		switch (child.GetEntityType())
		{
			case EEditableEntityType.CHARACTER:
			{
				SCR_EditableCharacterComponent childCharacter = SCR_EditableCharacterComponent.Cast(child);
				if (!childCharacter)
					return;
				
				AIAgent childAgent = childCharacter.GetAgent();
				if (!childAgent)
					return;
				
				if (isAdded)
				{
					m_Group.AddAgent(childAgent);
					
					//--- Make sure newly joined member is on the same faction
					Faction groupFaction = GetFaction();
					if (groupFaction && groupFaction != childCharacter.GetFaction())
					{
						FactionAffiliationComponent charactedFactionComponent = FactionAffiliationComponent.Cast(childCharacter.GetOwner().FindComponent(FactionAffiliationComponent));
						if (charactedFactionComponent)
							charactedFactionComponent.SetAffiliatedFaction(groupFaction);
						
						//--- ToDo: Notify player that this is sus!
					}
				}
				else
				{
					m_Group.RemoveAgent(childAgent);
				}
				
				break;
			}
			case EEditableEntityType.WAYPOINT:
			{
				AIWaypoint waypoint = AIWaypoint.Cast(child.GetOwner());
				if (!waypoint)
					return;
				
				if (isAdded)
				{
					//--- Check for duplicates, one waypoint can be added multiple times
					array<AIWaypoint> waypoints = {};
					m_Group.GetWaypoints(waypoints);
					if (waypoints.Find(waypoint) == -1)
						m_Group.AddWaypoint(waypoint);
				}
				else
				{
					m_Group.RemoveWaypoint(waypoint);
				}
				
				break;
			}
		}
	}
	override void OnPostInit(IEntity owner)
	{
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_DISABLE))
			return;
		
		super.OnPostInit(owner);
		
		if (m_Group)
		{
			//--- Track group's events (no need to remove them, the group gets detsroyed when the component does)
			if (IsServer())
			{
				//m_Group.GetOnEmpty().Insert(OnEmpty);
				m_Group.GetOnAgentAdded().Insert(OnAgentAdded);
				m_Group.GetOnAgentRemoved().Insert(OnAgentRemoved);
				m_Group.GetOnLeaderChanged().Insert(OnLeaderChanged);
				m_Group.GetOnCurrentWaypointChanged().Insert(OnCurrentWaypointChanged);
				m_Group.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
				m_Group.GetOnWaypointAdded().Insert(OnWaypointAdded);
				m_Group.GetOnWaypointRemoved().Insert(OnWaypointRemoved);
			}
			
			//On faction changed
			m_Group.GetOnFactionChanged().Insert(OnFactionChanged);
			
			SCR_GroupIdentityComponent groupIdentity = SCR_GroupIdentityComponent.Cast(owner.FindComponent(SCR_GroupIdentityComponent));
			if (groupIdentity)
				groupIdentity.GetOnIdentityChange().Insert(OnIdentityChange);
		}
	}
	void SCR_EditableGroupComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Group = SCR_AIGroup.Cast(ent);
		
		//--- Check if attached to correct entity type. Needs to be SCR_AIGroup, not just AIGroup, because it requires certain events.
		if (!m_Group)
			Print("SCR_EditableGroupComponent must be on SCR_AIGroup!", LogLevel.ERROR);
		
		//--- Check if entity type was configured correctly. Only in Workbench, no need to read sources in run-time.
		if (SCR_Global.IsEditMode(ent) && SCR_EditableGroupComponentClass.GetEntityType(src) != EEditableEntityType.GROUP)
			Print("SCR_EditableGroupComponent entity type must be set to GROUP!", LogLevel.ERROR);
	}
};
