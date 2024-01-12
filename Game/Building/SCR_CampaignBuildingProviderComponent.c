[EntityEditorProps(category: "GameScripted/Building", description: "Component attached to a provider, responsible for basic provider behaviour.")]
class SCR_CampaignBuildingProviderComponentClass : SCR_MilitaryBaseLogicComponentClass
{

}

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingProviderComponent : SCR_MilitaryBaseLogicComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Can the building mode at this provider executed only via user action?")]
	protected bool m_bUserActionActivationOnly;

	[Attribute("0", UIWidgets.CheckBox, "Register at nearby base, if available.")]
	protected bool m_bRegisterAtBase;

	[Attribute("50", UIWidgets.EditBox, "Building radius")]
	protected float m_fBuildingRadius;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Minimal rank that allows player to use the provider to build structures.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;

	[Attribute("500", UIWidgets.EditBox, "Max. value for prop budget. if -1 is set, the budget is unlimited. Use with caution, might have a severe performance impact.")]
	protected int m_iMaxPropValue;

	[Attribute(desc: "Fill in the budgets to be used with this provider", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected ref array<EEditableEntityBudget> m_aBudgetsToEvaluate;

	[Attribute(desc: "Traits this provider will provide. Each trait represents a tab in building interface. The tabs have to be defined in building mode's SCR_ContentBrowserEditorComponent.", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aAvailableTraits;

	[RplProp()]
	protected int m_iCurrentPropValue;

	protected Physics m_ProviderPhysics;

	protected SCR_ResourceComponent m_ResourceComponent;

	protected ref array<int> m_aActiveUsersIDs = {};
	protected ref array<int> m_aAvailableUsersIDs = {};

	protected static ref ScriptInvokerVoid s_OnProviderCreated = new ScriptInvokerVoid();

	protected const int MOVING_CHECK_PERIOD = 1000;
	protected const int PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ = 1;

	//------------------------------------------------------------------------------------------------
	override void RegisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (!m_bRegisterAtBase)
			return;

		super.RegisterBase(base);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if the provider can be registered at base or not.
	bool CanRegisterAtMilitaryBase()
	{
		return m_bRegisterAtBase;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a military base component of the base this provider is registered to. If is registered to more then one, it returns the 1st one.
	SCR_MilitaryBaseComponent GetMilitaryBaseComponent()
	{
		array<SCR_MilitaryBaseComponent> bases = {};
		GetBases(bases);

		if (bases.IsEmpty())
			return null;

		return bases[0];
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetAccessRank()
	{
		return m_iRank;
	}

	//------------------------------------------------------------------------------------------------
	float GetBuildingRadius()
	{
		return m_fBuildingRadius;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxPropValue()
	{
		return m_iMaxPropValue;
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentPropValue()
	{
		return m_iCurrentPropValue;
	}

	//------------------------------------------------------------------------------------------------
	void AddPropValue(int value)
	{
		m_iCurrentPropValue += value;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	array<EEditableEntityLabel> GetAvailableTraits()
	{
		array<EEditableEntityLabel> availableTraits = {};
		availableTraits.Copy(m_aAvailableTraits);
		return availableTraits;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);

		super.OnPostInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (System.IsConsoleApp() || !GetGame().GetPlayerController())
			SetOnProviderFactionChangedEvent();
	}

	//------------------------------------------------------------------------------------------------
	//! Add an ID of a user on the list of those who are currently in use of provider
	void AddNewActiveUser(int userID)
	{
		m_aActiveUsersIDs.Insert(userID);
		SetOnPlayerDeathActiveUserEvent(userID);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove an ID of a user from the list of those who are currently in use of provider
	void RemoveActiveUser(int userID)
	{
		m_aActiveUsersIDs.RemoveItem(userID);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the array of users id and it's count.
	int GetActiveUsers(out notnull array<int> users)
	{
		users.Copy(m_aActiveUsersIDs);
		return users.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool ContainActiveUsers(int playerId)
	{
		return m_aActiveUsersIDs.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void AddNewAvailableUser(int userID)
	{
		m_aAvailableUsersIDs.Insert(userID);
		SetOnPlayerDeathAvailableUserEvent(userID);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveAvailableUser(int userID)
	{
		m_aAvailableUsersIDs.RemoveItem(userID);
	}

	//------------------------------------------------------------------------------------------------
	int GetAvailableUsers(out array<int> users)
	{
		users.Copy(m_aAvailableUsersIDs);
		return users.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool ContainAvailableUsers(int playerId)
	{
		return m_aAvailableUsersIDs.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Requesting a building mode. If trigger exist (was spawned with provider, because "Y" can be used to enter the mode, it will open the mode directly. If not, it 1st spawn the building area trigger.
	void RequestBuildingMode(int playerID, bool UserActionUsed)
	{
		RequestEnterBuildingMode(playerID, UserActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	void RequestEnterBuildingMode(int playerID, bool UserActionUsed)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		SetOnPlayerConsciousnessChanged(playerID);
		SetOnPlayerTeleported(playerID);
		editorManager.GetOnOpened().Insert(BuildingModeCreated);
		networkComponent.RequestEnterBuildingMode(GetOwner(), playerID, m_bUserActionActivationOnly, UserActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	void SetOnPlayerTeleported(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerTeleportedFeedbackComponent playerTeleportComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportComponent)
			return;

		playerTeleportComponent.GetOnPlayerTeleported().Insert(PlayerTeleported);
	}

	//------------------------------------------------------------------------------------------------
	void SetOnPlayerConsciousnessChanged(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged, false, true);
	}

	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		if (conscious)
			return;

		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	void PlayerTeleported(SCR_EditableCharacterComponent character, bool isLongFade, SCR_EPlayerTeleportedReason teleportReason)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());

	}

	//------------------------------------------------------------------------------------------------
	void RemoveOnConsciousnessChanged()
	{
		IEntity ent = SCR_PlayerController.GetLocalControlledEntity();
		if (!ent)
			return;

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(ent.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged, false);
	}

	//------------------------------------------------------------------------------------------------
	// Insert a method called when the provider faction is changed. For an example base is taken by an enemy.
	void SetOnProviderFactionChangedEvent()
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;

		factionComponent.GetOnFactionChanged().Insert(OnBaseOwnerChanged);
	}

	//------------------------------------------------------------------------------------------------
	void SetOnPlayerDeathActiveUserEvent(int userID)
	{
		SCR_CharacterControllerComponent comp = GetCharacterControllerComponent(userID);
		if (!comp)
			return;

		comp.GetOnPlayerDeathWithParam().Insert(OnActiveUserDeath);
	}

	//------------------------------------------------------------------------------------------------
	void SetOnPlayerDeathAvailableUserEvent(int userID)
	{
		SCR_CharacterControllerComponent comp = GetCharacterControllerComponent(userID);
		if (!comp)
			return;

		comp.GetOnPlayerDeathWithParam().Insert(OnAvailableUserDeath);
	}

	//------------------------------------------------------------------------------------------------
	void OnActiveUserDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity instigatorEntity, notnull Instigator instigator)
	{
		RemoveActiveUser(GetPlayerIdFromCharacterController(characterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	void OnAvailableUserDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity instigatorEntity, notnull Instigator instigator)
	{
		RemoveAvailableUser(GetPlayerIdFromCharacterController(characterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	void BuildingModeCreated()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnOpened().Remove(BuildingModeCreated);
		editorManager.GetOnClosed().Insert(OnModeClosed);
	}

	//------------------------------------------------------------------------------------------------
	void OnModeClosed()
	{
		RemoveOnModeClosed();
		RemoveOnConsciousnessChanged();
		RemoveOnPlayerTeleported();

		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerVoid GetOnProviderCreated()
	{
		return s_OnProviderCreated;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveOnModeClosed()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnClosed().Remove(OnModeClosed);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveOnPlayerTeleported()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerTeleportedFeedbackComponent playerTeleportComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportComponent)
			return;

		playerTeleportComponent.GetOnPlayerTeleported().Remove(PlayerTeleported);
	}

	//------------------------------------------------------------------------------------------------
	bool GetUserActionInitOnly()
	{
		return m_bUserActionActivationOnly;
	}

	//------------------------------------------------------------------------------------------------
	//! GetNetworkManager
	protected SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Get Local Editor manager
	protected SCR_EditorManagerEntity GetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager();
	}

	//------------------------------------------------------------------------------------------------
	//! Get Editor manager by player ID
	protected SCR_EditorManagerEntity GetEditorManagerByID(int playerId)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterControllerComponent GetCharacterControllerComponent(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return null;

		return SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlayerIdFromCharacterController(SCR_CharacterControllerComponent characterControllerComponent)
	{
		if (!characterControllerComponent)
			return -1;

		IEntity ent = characterControllerComponent.GetOwner();
		if (!ent)
			return -1;

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
	}

	//------------------------------------------------------------------------------------------------
	//! Does player faction match the provider faction
	// Method called by na user action to make sure it can be shown only to the players of the same faction as the base belong to.
	bool IsPlayerFactionSame(notnull IEntity player)
	{
		Faction playerFaction = GetEntityFaction(player);
		if (!playerFaction)
			return false;

		IEntity provider = GetOwner();
		if (!provider)
			return false;

		Faction owningFaction = GetEntityFaction(provider);
		if (!owningFaction)
			return false;

		return playerFaction == owningFaction;
	}

	//------------------------------------------------------------------------------------------------
	protected Faction GetEntityFaction(notnull IEntity ent)
	{
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));

		// Seacrch for the faction component on parent entities as not always is it on the same component as this one (vehicle for an example)
		while (!factionComp && ent)
		{
			ent = ent.GetParent();
			if (ent)
				factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		}

		if (!factionComp)
			return null;

		Faction faction = factionComp.GetAffiliatedFaction();
		if (!faction)
			faction = factionComp.GetDefaultAffiliatedFaction();

		return faction;
	}

	//------------------------------------------------------------------------------------------------
	//! Method triggered when owning faction of provider has changed.
	protected void OnBaseOwnerChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		// ToDo: In the future if the trigger activation will be enabled again, here I have to use a trigger range to find all entities which should be added and remove those on the list now.
		RemoveActiveUsers();
	}

	//------------------------------------------------------------------------------------------------
	//! Caches and returns the resource component.
	SCR_ResourceComponent GetResourceComponent()
	{
		if (!m_ResourceComponent)
			m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());
		
		return m_ResourceComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns provider supply component
	[Obsolete("SCR_CampaignBuildingProviderComponent.GetResourceComponent() should be used instead.")]
	SCR_CampaignSuppliesComponent GetSuppliesComponent()
	{
		return SCR_CampaignSuppliesComponent.Cast(GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true / false if this budget is suppose to be taken into account with this provider.
	bool IsBudgetToEvaluate(EEditableEntityBudget blockingBudget)
	{
		return m_aBudgetsToEvaluate.Contains(blockingBudget);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the provider was destroyed or deleted to remove a provider.
	void RemoveActiveUsers()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		bool isActiveUser;
		foreach (int playerId : m_aAvailableUsersIDs)
		{
			isActiveUser = false;

			SCR_EditorManagerEntity editorManager = GetEditorManagerByID(playerId);
			if (!editorManager)
				return;

			SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(editorManager.FindModeEntity(EEditorMode.BUILDING));
			if (!modeEntity)
				return;

			if (m_aActiveUsersIDs.Contains(playerId))
			{
				RemoveActiveUser(playerId);
				isActiveUser = true;
			}

			buildingManagerComponent.RemoveProvider(playerId, this, isActiveUser);
			RemoveAvailableUser(playerId);
		}

		RemoveCheckProviderMove();
	}

	//------------------------------------------------------------------------------------------------
	void RemoveCheckProviderMove()
	{
		if (IsProviderDynamic())
			GetGame().GetCallqueue().Remove(CheckProviderMove);
	}

	//------------------------------------------------------------------------------------------------
	void SetCheckProviderMove()
	{
		if (IsProviderDynamic())
			GetGame().GetCallqueue().CallLater(CheckProviderMove, MOVING_CHECK_PERIOD, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provider velocity in meters per second is faster then allowed. If so, terminate  a building mode.
	private void CheckProviderMove()
	{
		vector velocity = m_ProviderPhysics.GetVelocity();

		if ((velocity.LengthSq()) > PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ)
			RemoveActiveUsers();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provider is dynamic. If so, save his physic component for move check.
	private bool IsProviderDynamic()
	{
		IEntity mainParent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		if (!mainParent)
			return false;

		m_ProviderPhysics = mainParent.GetPhysics();

		return m_ProviderPhysics && m_ProviderPhysics.IsDynamic();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		s_OnProviderCreated.Invoke();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignBuildingProviderComponent()
	{
		if (!m_aAvailableUsersIDs.IsEmpty())
			RemoveActiveUsers();
	}
}
