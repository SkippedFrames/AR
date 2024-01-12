[EntityEditorProps(category: "GameScripted/Commanding", description: "Commanding manager, attach to game mode entity!.")]
class SCR_CommandingManagerComponentClass : SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CommandingManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")]
	protected ResourceName m_sAIGroupPrefab;
	
	[Attribute("{54764D4E706F348B}Configs/Commanding/Commands.conf")]
	protected ResourceName m_sCommandsConfigPath;
	
	[Attribute(defvalue: "8", UIWidgets.EditBox, desc: "How many AI soldiers can be recruited into single player group")]
	protected int m_iMaxAIPerGroup;
	
	protected SCR_PlayerCommandsConfig m_CommandsConfig;
	
	protected static SCR_CommandingManagerComponent s_Instance;
	
	protected ref array<ref SCR_BaseGroupCommand> m_aCommands;
	protected ref map<string, ref SCR_BaseGroupCommand> m_mNameCommand = new map<string, ref SCR_BaseGroupCommand>();

	protected static const int NORMAL_PRIORITY = 50;
	
	//------------------------------------------------------------------------
	static SCR_CommandingManagerComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetGroupPrefab()
	{
		return m_sAIGroupPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CommandingManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		InitiateCommandMaps();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Insert(ResetSlaveGroupWaypoints);
	}
	
	//------------------------------------------------------------------------------------------------
	void InitiateCommandMaps()
	{
		SCR_PlayerCommandsConfig commandsConfig = GetCommandsConfig();
		if (!commandsConfig)
			return;
		
		m_aCommands = commandsConfig.GetCommands();
		
		foreach (int i, SCR_BaseGroupCommand command : m_aCommands)
			m_mNameCommand.Insert(command.GetCommandName(), command);
			
	}
	
	//------------------------------------------------------------------------------------------------
	//! gets called when player respawns so the newly recruited AIs dont follow the old waypoints from orders
	void ResetSlaveGroupWaypoints(int playerId, IEntity player)
	{	
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;
		
		//do nothing if player is not a leader of group
		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(pc.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComponent || !groupComponent.IsPlayerLeaderOwnGroup())
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup masterGroup = groupsManager.GetPlayerGroup(playerId);
		if (!masterGroup)
			return;
		
		//no need to do this for empty group because there are no AIs to mess things up
		SCR_AIGroup slaveGroup = masterGroup.GetSlave();
		if (!slaveGroup || slaveGroup.GetAgentsCount() <= 0)
			return;
		
		//reset the waypoints for slave group, so they go to their default behavior
		array<AIWaypoint> currentWaypoints = {};
		slaveGroup.GetWaypoints(currentWaypoints);
		foreach (AIWaypoint currentwp : currentWaypoints)
		{
			slaveGroup.RemoveWaypoint(currentwp);
		}
		
		AIFormationComponent slaveGroupFormationComp = slaveGroup.GetFormationComponent();
		if (slaveGroupFormationComp)
			slaveGroupFormationComp.SetFormationDisplacement(0);
		
		//remove master-slave connection from slave
		slaveGroup.SetMaster(null);
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		//prepare new slave group for player
		Resource groupPrefabRes = Resource.Load(commandingManager.GetGroupPrefab());
		if (!groupPrefabRes.IsValid())
			return;
		
		IEntity groupEntity = GetGame().SpawnEntityPrefab(groupPrefabRes);
		
		if (!groupEntity)
			return;
		
		SCR_AIGroup newSlaveGroup = SCR_AIGroup.Cast(groupEntity);
		if (!newSlaveGroup)
			return;
		
		RplComponent RplComp = RplComponent.Cast(newSlaveGroup.FindComponent(RplComponent));
		if (!RplComp)
			return;
		
		RplId slaveGroupRplID = RplComp.Id();
		
		RplComp = RplComponent.Cast(masterGroup.FindComponent(RplComponent));
		if (!RplComp)
			return;
		
		groupsManager.RequestSetGroupSlave(RplComp.Id(), slaveGroupRplID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! called on server
	void RequestCommandExecution(int commandIndex, RplId cursorTargetID, RplId groupRplID, vector targetPosition, int playerID)
	{
		//check if the passed arguments are valid, if yes, send a callback RPC to commanders playercontroller so he can make a gesture.	
		RPC_DoExecuteCommand(commandIndex, cursorTargetID, groupRplID, targetPosition, playerID);
		Rpc(RPC_DoExecuteCommand, commandIndex, cursorTargetID, groupRplID, targetPosition, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoExecuteCommand(int commandIndex, RplId cursorTargetID, RplId groupRplID, vector targetPosition, int playerID)
	{
		RplComponent rplComp;
		IEntity cursorTarget, group;
		rplComp = RplComponent.Cast(Replication.FindItem(cursorTargetID));
		if (rplComp)
			cursorTarget = rplComp.GetEntity();
		
		rplComp = RplComponent.Cast(Replication.FindItem(groupRplID));
		if (rplComp)
			group = rplComp.GetEntity();
		
		rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComp)
		{
			SCR_BaseGroupCommand command = FindCommand(FindCommandNameFromIndex(commandIndex));
			if (command.Execute(cursorTarget, group, targetPosition, playerID, rplComp.IsProxy()))
			{
				PlayCommanderSound(playerID, commandIndex);
				if (!rplComp.IsMaster())
					return;
				
				PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerID);
				if (!controller)
					return;
				
				SCR_PlayerControllerCommandingComponent commandingComp = SCR_PlayerControllerCommandingComponent.Cast(controller.FindComponent(SCR_PlayerControllerCommandingComponent));
				if (!commandingComp)
					return;
				
				commandingComp.CommandExecutedCallback(commandIndex);			
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayCommanderSound(int playerID, int commandIndex)
	{
		SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		if (!playerCharacter)
			return;
		
		SignalsManagerComponent signalManager = SignalsManagerComponent.Cast(playerCharacter.FindComponent(SignalsManagerComponent));
		if (!signalManager)
			return;

		SCR_CommunicationSoundComponent soundComponent = SCR_CommunicationSoundComponent.Cast(playerCharacter.FindComponent(SCR_CommunicationSoundComponent));
		if (!soundComponent)
			return;
		
		int signalSoldierCalled = signalManager.FindSignal("SoldierCalled");
		
		string soundEventName = GetCommandSoundEventName(commandIndex);
		if (soundEventName.IsEmpty())
			return;
		
		signalManager.SetSignalValue(signalSoldierCalled, 1000);
		soundComponent.SoundEventPriority(soundEventName, NORMAL_PRIORITY);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseGroupCommand FindCommand(string commandName)
	{
		return m_mNameCommand.Get(commandName);
	}
	
	//------------------------------------------------------------------------------------------------
	int FindCommandIndex(string commandName)
	{
		SCR_BaseGroupCommand command = FindCommand(commandName);
		if (!command)
			return -1;
		
		return m_aCommands.Find(command);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCommandSoundEventName(int commandIndex)
	{
		SCR_BaseGroupCommand command = m_aCommands.Get(commandIndex);
		if (!command)
			return string.Empty;
		
		return command.GetSoundEventName();
	}
	
	//------------------------------------------------------------------------------------------------
	string FindCommandNameFromIndex(int commandIndex)
	{
		SCR_BaseGroupCommand command = m_aCommands.Get(commandIndex);
		if (!command)
			return string.Empty;
		
		return command.GetCommandName();
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanShowCommand(string commandName)
	{
		SCR_BaseGroupCommand command = FindCommand(commandName);
		if (!command)
			return false;
		
		return command.CanBeShown();
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanShowOnMap(string commandName)
	{
		SCR_BaseGroupCommand command = FindCommand(commandName);
		if (!command)
			return false;
		
		return command.CanShowOnMap();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_PlayerCommandsConfig GetCommandsConfig()
	{
		if (m_CommandsConfig)
			return m_CommandsConfig;
		
		Resource holder = BaseContainerTools.LoadContainer(m_sCommandsConfigPath);
		if (!holder)
			return null;
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return null;
		
		SCR_PlayerCommandsConfig commandsConfig = SCR_PlayerCommandsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!commandsConfig)
			return null;
		
		m_CommandsConfig = commandsConfig;
		return m_CommandsConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMaxAIPerGroup()
	{
		return m_iMaxAIPerGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMaxAIPerGroup(int max)
	{
		m_iMaxAIPerGroup = max;
	}
}
