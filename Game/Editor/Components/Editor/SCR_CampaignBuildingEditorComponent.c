// Attached on EditorModeBuilding prefab
[ComponentEditorProps(category: "GameScripted/Editor", description: "Main conflict component for handling building editor mode", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingEditorComponentClass : SCR_BaseEditorComponentClass
{
}

//------------------------------------------------------------------------------------------------

class SCR_CampaignBuildingEditorComponent : SCR_BaseEditorComponent
{
	protected ref array<SCR_CampaignBuildingProviderComponent> m_aProvidersComponents = {};
	protected ref array<RplId> m_aProvidersRplIds = {};
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	protected SCR_CampaignBuildingProviderComponent m_ForcedProviderComponent;

	protected ref ScriptInvoker m_OnProviderChanged;

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnProviderChanged()
	{
		if (!m_OnProviderChanged)
			m_OnProviderChanged = new ScriptInvoker();

		return m_OnProviderChanged;
	}

	//------------------------------------------------------------------------------------------------
	void AddProviderEntityEditorComponent(SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (!providerComponent || m_aProvidersComponents.Contains(providerComponent))
			return;

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (base)
			m_aProvidersComponents.InsertAt(providerComponent, 0);
		else
			m_aProvidersComponents.Insert(providerComponent);

		GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveProviderEntityEditorComponent(SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (m_ForcedProviderComponent == providerComponent)
			SetForcedProvider(null);

		m_aProvidersComponents.RemoveItemOrdered(providerComponent);

		if (!m_aProvidersComponents.IsEmpty())
			GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);
	}

	//------------------------------------------------------------------------------------------------
	//! Return provider component of current provider.
	SCR_CampaignBuildingProviderComponent GetProviderComponent()
	{
		if (m_ForcedProviderComponent)
			return m_ForcedProviderComponent;

		if (m_aProvidersComponents.IsEmpty())
			return null;

		return m_aProvidersComponents[0];
	}

	//------------------------------------------------------------------------------------------------
	// Used when player initiate a building mode via user action - forced provider is an entity owning the user action
	void SetForcedProvider(SCR_CampaignBuildingProviderComponent forcedProviderComponent = null)
	{
		m_ForcedProviderComponent = forcedProviderComponent;
	}

	//------------------------------------------------------------------------------------------------
	int GetProviderEntitiesCount()
	{
		return m_aProvidersComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsActiveProvider(notnull SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (providerComponent == m_ForcedProviderComponent)
			return true;

		// there exist a forced provider and it's not a tested entity, then it can't be an active provider. ToDo: Zamyslet se a zeptat
		if (m_ForcedProviderComponent)
			return false;

		if (!m_aProvidersComponents.IsEmpty() && m_aProvidersComponents[0] == providerComponent)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool GetProviderResourceComponent(out SCR_ResourceComponent resourceComponent)
	{
		if (m_aProvidersComponents.IsEmpty())
			return false;

		resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetProviderEntity());
		
		if (resourceComponent)
			return true;

		IEntity parent = GetProviderEntity();
		
		while (parent)
		{
			resourceComponent = SCR_ResourceComponent.FindResourceComponent(parent);
			
			if (resourceComponent)
				return true;

			parent = parent.GetParent();
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("SCR_CampaignBuildingEditorComponent.GetProviderResourceComponent() should be used instead.")]
	bool GetProviderSuppliesComponent(out SCR_CampaignSuppliesComponent suppliesComponent)
	{
		if (m_aProvidersComponents.IsEmpty())
			return false;

		suppliesComponent = SCR_CampaignSuppliesComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignSuppliesComponent));
		if (suppliesComponent)
			return true;

		IEntity parent = GetProviderEntity();
		while (parent)
		{
			suppliesComponent = SCR_CampaignSuppliesComponent.Cast(parent.FindComponent(SCR_CampaignSuppliesComponent));
			if (suppliesComponent)
				return true;

			parent = parent.GetParent();
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	SCR_FactionAffiliationComponent GetProviderFactionComponent()
	{
		if (m_aProvidersComponents.IsEmpty())
			return null;

		SCR_FactionAffiliationComponent factionComp = SCR_FactionAffiliationComponent.Cast(GetProviderEntity().FindComponent(SCR_FactionAffiliationComponent));
		if (factionComp)
			return factionComp;

		IEntity parent = GetProviderEntity();
		while (parent)
		{
			factionComp = SCR_FactionAffiliationComponent.Cast(parent.FindComponent(SCR_FactionAffiliationComponent));
			if (factionComp)
				return factionComp;

			parent = parent.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_FreeRoamBuildingClientTriggerEntity GetTrigger()
	{
		if (m_aProvidersComponents.IsEmpty())
			return null;

		//ToDo: Is it still needed?
		// clear possible null, better solution - some invoker when the value in array change to null?
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
		{
			RemoveProviderEntityEditorComponent(providerComponent);
			return null;
		}

		IEntity child = GetProviderEntity().GetChildren();
		while (child)
		{
			SCR_FreeRoamBuildingClientTriggerEntity trg = SCR_FreeRoamBuildingClientTriggerEntity.Cast(child);
			if (trg)
				return trg;

			child = child.GetSibling();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//~  This function will add/remove the faction label of the saved editor state. Thus displaying the correct faction entities in the menu
	protected void AddRemoveFactionLabel(SCR_Faction faction, bool addLabel)
	{
		if (!faction)
			return;

		m_ContentBrowserManager.AddRemoveLabelOfPersistentBrowserState(faction.GetFactionLabel(), addLabel);
	}

	//------------------------------------------------------------------------------------------------
	ScriptedGameTriggerEntity SpawnClientTrigger()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return null;

		IEntity provider = GetProviderEntity();
		if (!provider)
			return null;

		Resource resource = Resource.Load(buildingManagerComponent.GetClientTriggerResourceName());
		if (!resource || !resource.IsValid())
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Parent = provider;

		return ScriptedGameTriggerEntity.Cast(GetGame().SpawnEntityPrefab(resource, provider.GetWorld(), params));
	}

	//------------------------------------------------------------------------------------------------
	//! Make the area around where is possible to build composition visible for player
	override protected void EOnEditorActivate()
	{
		SCR_CampaignBuildingProviderComponent providerComponenet = GetProviderComponent();
		if (!providerComponenet)
			return;

		if (!System.IsConsoleApp() && GetGame().GetPlayerController())
		{
			ScriptedGameTriggerEntity trigger = SpawnClientTrigger();

			if (trigger)
			{
				trigger.SetSphereRadius(providerComponenet.GetBuildingRadius());

				SCR_CampaignBuildingAreaMeshComponent areaMeshComponent = SCR_CampaignBuildingAreaMeshComponent.Cast(trigger.FindComponent(SCR_CampaignBuildingAreaMeshComponent));
				if (areaMeshComponent && areaMeshComponent.ShouldEnableFrameUpdateDuringEditor())
				{
					areaMeshComponent.ActivateEveryFrame();
					areaMeshComponent.GenerateAreaMesh();
				}

				trigger.SetFlags(EntityFlags.VISIBLE, false);

			}
		}

		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));

		FactionAffiliationComponent factionComponent = GetProviderFactionComponent();
		if (factionComponent)
		{
			// here we have to check both Affiliated faction and Default affiliated faction in case of vehicles. It's because vehicles can't have set a affiliated faction if no one is sitting inside (to prevent AI to shoot at empty vehicles)
			Faction buildingFaction = factionComponent.GetAffiliatedFaction();

			if (!buildingFaction)
				buildingFaction = factionComponent.GetDefaultAffiliatedFaction();

			if (buildingFaction)
				AddRemoveFactionLabel(SCR_Faction.Cast(buildingFaction), true);
		}

		array<SCR_EditorContentBrowserSaveStateDataUI> contentBrowserStates = {};
		int tabsCount = m_ContentBrowserManager.GetContentBrowserTabStates(contentBrowserStates);

		for (int i = 0; i < tabsCount; i++)
		{
			if (!contentBrowserStates[i])
				continue;
			
			//~ Todo: Fix first tab being broken
			//~ Hotfix for first tab being broken
			if (i == 0 || !CanBeShown(contentBrowserStates[i]))
				m_ContentBrowserManager.SetStateTabVisible(i, false);
		}

		ToggleBuildingTool(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the given tab can be shown
	bool CanBeShown(notnull SCR_EditorContentBrowserSaveStateDataUI tab)
	{
		if (!TabContainLabel(tab))
			return false;

		return tab.CanBeShown();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the given tabUI contains any label set on provider.
	bool TabContainLabel(SCR_EditorContentBrowserSaveStateDataUI tab)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return false;

		array<EEditableEntityLabel> labels = providerComponent.GetAvailableTraits();

		foreach (EEditableEntityLabel label : labels)
		{
			if (tab.ContainsLabel(label))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if the provider is a base
	bool IsProviderBase()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return false;

		return providerComponent.GetMilitaryBaseComponent();
	}

	//------------------------------------------------------------------------------------------------
	void ToggleBuildingTool(bool mode)
	{
		IEntity player = EntityUtils.GetPlayer();
		if (!player)
			return;

		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(player);
		if (!gadgetManager)
			return;
		
		SCR_GadgetComponent gadgetComponent = gadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return;
		
		if (gadgetComponent.GetType() == EGadgetType.BUILDING_TOOL)
			gadgetManager.ToggleHeldGadget(mode);
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetUserRank()
	{
		int playerId = SCR_PlayerController.GetLocalPlayerId();

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return SCR_ECharacterRank.INVALID;

		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		SCR_MilitaryBaseComponent base;

		if (GetProviderEntity())
		{
			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignBuildingProviderComponent));
			if (!providerComponent)
				return;

			base = providerComponent.GetMilitaryBaseComponent();
		}

		if (base)
			return;

		GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), GetProviderComponent().GetBuildingRadius(), AssociateCompositionsToProvider, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorCloseServer()
	{
		ToggleBuildingTool(true);

		if (!GetProviderEntity())
			return;

		// In case the provider of the building was the base, don't remove it's stamp from component. So the composition can't be build from another providers
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (base)
			return;

		GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), GetProviderComponent().GetBuildingRadius(), UnassignCompositionProvider, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	bool AssociateCompositionsToProvider(IEntity ent)
	{
		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent || editableComponent.GetParentEntity())
			return true;

		SCR_CampaignBuildingCompositionComponent comp = SCR_CampaignBuildingCompositionComponent.Cast(ent.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!comp)
			return true;

		// compositioni has no owner, set one.
		if (comp.GetProviderEntity() == null)
			comp.SetProviderEntityServer(GetProviderEntity());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool UnassignCompositionProvider(IEntity ent)
	{
		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent || editableComponent.GetParentEntity())
			return true;

		SCR_CampaignBuildingCompositionComponent comp = SCR_CampaignBuildingCompositionComponent.Cast(ent.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!comp)
			return true;

		if (comp.GetProviderEntity() == GetProviderEntity())
			comp.SetProviderEntityServer(null);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Hide the area of building and remove the active faction
	override protected void EOnEditorDeactivate()
	{
		SCR_FreeRoamBuildingClientTriggerEntity trigger = GetTrigger();
		if (trigger)
		{
			trigger.ClearFlags(EntityFlags.VISIBLE, false);

			SCR_CampaignBuildingAreaMeshComponent areaMeshComponent = SCR_CampaignBuildingAreaMeshComponent.Cast(trigger.FindComponent(SCR_CampaignBuildingAreaMeshComponent));
			if (areaMeshComponent && areaMeshComponent.ShouldEnableFrameUpdateDuringEditor())
			{
				areaMeshComponent.DeactivateEveryFrame();
			}
		}

		FactionAffiliationComponent factionComponent = GetProviderFactionComponent();
		if (factionComponent)
		{
			// here we have to check both Affiliated faction and Default affiliated faction in case of vehicles. It's because vehicles can't have set a affiliated faction if no one is sitting inside (to prevent AI to shoot at empty vehicles)
			Faction buildingFaction = factionComponent.GetAffiliatedFaction();

			if (!buildingFaction)
				buildingFaction = factionComponent.GetDefaultAffiliatedFaction();

			if (buildingFaction)
				AddRemoveFactionLabel(SCR_CampaignFaction.Cast(buildingFaction), false);
		}
		
		ToggleBuildingTool(true);
	}

	//------------------------------------------------------------------------------------------------
	// Return the entity which belongs to currently active provider componenet.
	IEntity GetProviderEntity()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return null;

		return providerComponent.GetOwner();
	}

	//------------------------------------------------------------------------------------------------
	//! Set a provider, loaded from RPL ID
	void SetProviderFromRplID()
	{
		int count = m_aProvidersRplIds.Count();
		for (int i = 0; i < count; i++)
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(m_aProvidersRplIds[i]));
			if (!rplComp)
				continue;

			IEntity provider = rplComp.GetEntity();
			if (!provider)
				continue;

			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (!providerComponent)
				continue;

			AddProviderEntityEditorComponent(providerComponent);
			m_aProvidersRplIds.RemoveOrdered(i);
			count--;
		}

		if (m_aProvidersRplIds.IsEmpty())
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Remove(SetProviderFromRplID);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		RplId entityRplID;
		int count = m_aProvidersComponents.Count();

		writer.WriteInt(count);
		for (int i = 0; i < count; i++)
		{
			IEntity provider = m_aProvidersComponents[i].GetOwner();
			if (!provider)
				continue;

			RplComponent rplCmp = RplComponent.Cast(provider.FindComponent(RplComponent));
			entityRplID = rplCmp.Id();
			writer.WriteRplId(entityRplID);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RplLoad(ScriptBitReader reader)
	{
		RplId entityRplID;
		IEntity ent;
		int count;
		reader.ReadInt(count);

		for (int i = 0; i < count; i++)
		{
			reader.ReadRplId(entityRplID);
			if (!entityRplID.IsValid())
				continue;

			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(entityRplID));
			if (!rplComp)
			{
				m_aProvidersRplIds.Insert(entityRplID);
				continue;
			}

			ent = IEntity.Cast(rplComp.GetEntity());
			if (!ent)
				continue;

			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(ent.FindComponent(SCR_CampaignBuildingProviderComponent));
			AddProviderEntityEditorComponent(providerComponent);
		}

		if (!m_aProvidersRplIds.IsEmpty())
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Insert(SetProviderFromRplID);

		if (!m_aProvidersComponents.IsEmpty())
			GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);

		return true;
	}
}
