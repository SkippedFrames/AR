
//------------------------------------------------------------------------------------------------
class SCR_ContainerActionTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_ScenarioFrameworkAction", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}

	protected void SplitStringByUpperCase(string input, out string output)
	{
		output = "";
		bool wasPreviousUpperCase;
		int asciiChar;
		for (int i, count = input.Length(); i < count; i++)
		{
			asciiChar = input.ToAscii(i);
			bool isLowerCase = (asciiChar > 96 && asciiChar < 123);
			if (i > 0 && !wasPreviousUpperCase && !isLowerCase)
			{
				output += " " + asciiChar.AsciiToString();
				wasPreviousUpperCase = true;
			}
			else
			{
				 if (isLowerCase)
					wasPreviousUpperCase = false;
				output += asciiChar.AsciiToString();
			}
		}
	}
}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//TODO: make this a generic action which can be used anywhere anytime (i.e. on task finished, etc)
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Graph, params: "-1 10000 1", desc: "How many times this action can be performed if this gets triggered? Value -1 for infinity")];
	protected int					m_iMaxNumberOfActivations;

	protected IEntity				m_Entity;
	protected int					m_iNumberOfActivations;

	//------------------------------------------------------------------------------------------------
	void Init(IEntity entity)
	{
		if (!SCR_BaseTriggerEntity.Cast(entity))
		{
			OnActivate(entity);
			m_Entity = entity;
			return;
		}

		m_Entity = entity;
		ScriptInvoker pOnActivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnActivate();
		if (pOnActivateInvoker)
			pOnActivateInvoker.Insert(OnActivate);

		ScriptInvoker pOnDeactivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnDeactivate();
		if (pOnDeactivateInvoker)
			pOnDeactivateInvoker.Insert(OnActivate);		//registering OnDeactivate to OnActivate - we need both changes
	}

	//------------------------------------------------------------------------------------------------
	bool CanActivate()
	{
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
			return false;

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate(IEntity object)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType)
	{
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;

		foreach (string sObjectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!object)
			{
				PrintFormat("ScenarioFramework: Can't spawn object set in slot %1. Slot doesn't exist", sObjectName, LogLevel.ERROR);
				continue;
			}
			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				PrintFormat("ScenarioFramework: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", sObjectName, LogLevel.ERROR);
				continue;
			}
			layer.Init(null, eActivationType);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionIncrementCounter : SCR_ScenarioFrameworkActionBase
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Counter to increment")];
	protected string				m_sCounterName;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity entity)
	{
		if (!m_sCounterName)
			return;

		super.Init(entity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sCounterName);
		if (!entity)
			return;

		SCR_ScenarioFrameworkLogic counter = SCR_ScenarioFrameworkLogic.Cast(entity.FindComponent(SCR_ScenarioFrameworkLogic));
		if (counter)
		{
			counter.OnInput(1, object);
			return;
		}

		SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(entity);
		if (logicCounter)
			logicCounter.OnInput(1, object);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnObjects : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "These objects will spawn once the trigger becomes active.")];
	protected ref array<string> 	m_aNameOfObjectsToSpawnOnActivation;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SpawnObjects(m_aNameOfObjectsToSpawnOnActivation, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityPosition : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the entity to be teleported")];
	protected ref SCR_ScenarioFrameworkGet m_EntityGetter;

	[Attribute(defvalue: "0 0 0", desc: "Position that the entity will be teleported to")];
	protected vector 	m_vDestination;

	[Attribute(desc: "Name of the entity that above selected entity will be teleported to (optional)")];
	protected ref SCR_ScenarioFrameworkGet m_DestinationEntityGetter;

	[Attribute(defvalue: "0 0 0", desc: "Position that will be used in relation to the entity for the position to teleport to (optional)")];
	protected vector 	m_vDestinationEntityRelativePosition;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_EntityGetter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		if (!m_DestinationEntityGetter)
		{
			entity.SetOrigin(m_vDestination);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> destinationEntityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_DestinationEntityGetter.Get());
		if (!entityWrapper)
			return;

		IEntity destinationEntity = IEntity.Cast(destinationEntityWrapper.GetValue());
		if (!destinationEntity)
			return;

		entity.SetOrigin(destinationEntity.GetOrigin() + m_vDestinationEntityRelativePosition);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionDeleteEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be deleted")];
	protected ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(entity);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionKillEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be killed")];
	protected ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		SCR_DamageManagerComponent damageMananager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (damageMananager)
			damageMananager.Kill(Instigator.CreateInstigator(object));
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionEndMission : SCR_ScenarioFrameworkActionBase
{
	[Attribute(UIWidgets.CheckBox, desc: "If true, it will override any previously set game over type with selected one down bellow")];
	protected bool		m_bOverrideGameOverType;

	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))];
	protected EGameOverTypes			m_eOverriddenGameOverType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		if (m_bOverrideGameOverType)
			manager.SetMissionEndScreen(m_eOverriddenGameOverType);

		manager.Finish();
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionWaitAndExecute : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "How long to wait before executing action")];
	protected int							m_iDelayInSeconds;

	[Attribute(defvalue: "1", desc: "Which actions will be executed once set time passes", UIWidgets.Auto)];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	void ExecuteActions(IEntity object)
	{
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(object);
		}

	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		//Used to delay the call as it is the feature of this action
		GetGame().GetCallqueue().CallLater(ExecuteActions, m_iDelayInSeconds * 1000, false, object);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCompareCounterAndExecute : SCR_ScenarioFrameworkActionBase
{

	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator			m_eComparisonOperator;

	[Attribute(desc: "Value")];
	protected int							m_iValue;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Counter to increment")];
	protected string						m_sCounterName;

	[Attribute(defvalue: "1", desc: "What to do once counter is reached", UIWidgets.Auto, category: "OnActivate")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sCounterName);
		if (!entity)
			return;

		int counterValue;
		string className = entity.ClassName();
		if (className == "SCR_ScenarioFrameworkLogicCounter")
		{
			SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(entity);
			counterValue = logicCounter.m_iCnt;
		}

		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (counterValue < m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (counterValue <= m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (counterValue == m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 		&& (counterValue >= m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 			&& (counterValue > m_iValue))
		)
		{
			foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
			{
				actions.OnActivate(object);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetMissionEndScreen : SCR_ScenarioFrameworkActionBase
{

	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))];
	protected EGameOverTypes			m_eGameOverType;

	[Attribute()]
	LocalizedString m_sSubtitle;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(gamemode.FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		manager.SetMissionEndScreen(m_eGameOverType);

		SCR_GameOverScreenManagerComponent gameOverScreenMgr = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverScreenMgr)
			return;

		SCR_GameOverScreenConfig m_GameOverScreenConfig = gameOverScreenMgr.GetGameOverConfig();
		if (!m_GameOverScreenConfig)
			return;

		SCR_BaseGameOverScreenInfo targetScreenInfo;
		m_GameOverScreenConfig.GetGameOverScreenInfo(m_eGameOverType, targetScreenInfo);
		if (!targetScreenInfo)
			return;

		SCR_BaseGameOverScreenInfoOptional optionalParams = targetScreenInfo.GetOptionalParams();
		if (!optionalParams)
			return;

		optionalParams.m_sSubtitle = m_sSubtitle;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetBriefingEntryText : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	protected FactionKey m_sFactionKey;

	[Attribute()]
	protected string m_sCustomEntryName;

	[Attribute()]
	protected string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{

			if (journalEntry.GetCustomEntryName() != m_sCustomEntryName)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		if (targetJournalEntry)
			targetJournalEntry.SetEntryText(WidgetManager.Translate(m_sTargetText));
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAppendBriefingEntryText : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	protected FactionKey m_sFactionKey;

	[Attribute()]
	protected string m_sCustomEntryName;

	[Attribute()]
	protected string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{

			if (journalEntry.GetCustomEntryName() != m_sCustomEntryName)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		string finalText = targetJournalEntry.GetEntryText() + "<br/>" + "<br/>" + m_sTargetText;
		if (targetJournalEntry)
			targetJournalEntry.SetEntryText(finalText);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAppendBriefingEntryTextBasedOnTask : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	protected FactionKey m_sFactionKey;

	[Attribute()]
	protected string m_sCustomEntryName;

	[Attribute(desc: "From which task to fetch text")];
	protected ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;
		
		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{

			if (journalEntry.GetCustomEntryName() != m_sCustomEntryName)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		if (!targetJournalEntry)
			return;
		
		string finalText = targetJournalEntry.GetEntryText() + "<br/><br/>" + task.GetTaskExecutionBriefing();
		respawnBriefing.RewriteEntryMain(m_sFactionKey, m_sCustomEntryName, finalText);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetBriefingEntryTextBasedOnGeneratedTasks : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	protected FactionKey m_sFactionKey;

	[Attribute()]
	protected string m_sCustomEntryName;

	[Attribute(desc: "Text that you want to use. Leave empty if you want to utilize the one set in config.")]
	protected string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{

			if (journalEntry.GetCustomEntryName() != m_sCustomEntryName)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetTasks(tasks);

		string tasksToShow;
		foreach (SCR_BaseTask task : tasks)
		{
			tasksToShow = tasksToShow + "<br/>" + string.Format(WidgetManager.Translate(task.GetTitle()));
		}
		
		if (!targetJournalEntry)
			return;

		if (m_sTargetText.IsEmpty())
			m_sTargetText = targetJournalEntry.GetEntryText();

		string finalText = string.Format(WidgetManager.Translate(m_sTargetText), tasksToShow);
		respawnBriefing.RewriteEntryMain(m_sFactionKey, m_sCustomEntryName, finalText);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetExecutionEntryTextBasedOnGeneratedTasks : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	protected FactionKey m_sFactionKey;

	[Attribute()]
	protected string m_sCustomEntryName;

	[Attribute(desc: "Text that you want to use. Leave empty if you want to utilize the one set in config.")]
	protected string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{

			if (journalEntry.GetCustomEntryName() != m_sCustomEntryName)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetTasks(tasks);
		
		array<SCR_ScenarioFrameworkTask> frameworkTasks = {};
		foreach (SCR_BaseTask task : tasks)
		{
			if (SCR_ScenarioFrameworkTask.Cast(task))
				frameworkTasks.Insert(SCR_ScenarioFrameworkTask.Cast(task));
		}

		string tasksToShow;
		foreach (SCR_ScenarioFrameworkTask frameworkTask : frameworkTasks)
		{
			tasksToShow = tasksToShow + "<br/><br/>" + frameworkTask.GetTaskExecutionBriefing();
		}

		if (m_sTargetText.IsEmpty())
			m_sTargetText = targetJournalEntry.GetEntryText();
		
		string finalText = string.Format(WidgetManager.Translate(m_sTargetText), tasksToShow);
		respawnBriefing.RewriteEntryMain(m_sFactionKey, m_sCustomEntryName, finalText);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowHint : SCR_ScenarioFrameworkActionBase
{
	[Attribute()];
	protected string		m_sTitle;

	[Attribute()];
	protected string		m_sText;


	[Attribute(defvalue: "15")];
	protected int			m_iTimeout;


	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_HintUIInfo info = SCR_HintUIInfo.CreateInfo(WidgetManager.Translate(m_sText), WidgetManager.Translate(m_sTitle), m_iTimeout, 0, 0, true);
		if (info)
			SCR_HintManagerComponent.ShowHint(info);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnClosestObjectFromList : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "The closest one from the list will be spawned")];
	protected ref array<string> 	m_aListOfObjects;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print("ScenarioFramework: The object the distance is calculated from is missing!", LogLevel.ERROR);
			return;
		}
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		IEntity entityInList;
		SCR_ScenarioFrameworkLayerBase selectedLayer;
		if (!entityFrom)
		{
			Print("ScenarioFramework: Action: Getter returned null object. Random object spawned instead.", LogLevel.WARNING);
			array<string> aRandomObjectToSpawn = {};
			aRandomObjectToSpawn.Insert(m_aListOfObjects[m_aListOfObjects.GetRandomIndex()]);

			entityInList = GetGame().GetWorld().FindEntityByName(aRandomObjectToSpawn[0]);
			if (!entityInList)
			{
				PrintFormat("ScenarioFramework: Object %1 doesn't exist", aRandomObjectToSpawn[0], LogLevel.ERROR);
				return;
			}

			SpawnObjects(aRandomObjectToSpawn, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
			return;
		}

		IEntity closestEntity;
		float fDistance = float.MAX;
		foreach (string sObjectName : m_aListOfObjects)
		{
			entityInList = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!entityInList)
			{
				PrintFormat("ScenarioFramework: Object %1 doesn't exist", sObjectName, LogLevel.ERROR);
				continue;
			}

			float fActualDistance = Math.AbsFloat(vector.Distance(entityFrom.GetOrigin(), entityInList.GetOrigin()));

			if (fActualDistance < fDistance)
			{
				closestEntity = entityInList;
				fDistance = fActualDistance;
			}
		}

		if (!closestEntity)
			return;

		selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(closestEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));

		if (selectedLayer)
		{
			selectedLayer.GetParentArea().SetAreaSelected(true);
			SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(selectedLayer);
			if (layerTask)
				selectedLayer.GetParentArea().SetLayerTask(layerTask);

			selectedLayer.Init(null, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
		}
		else
		{
			PrintFormat("ScenarioFramework: Can't spawn slot %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", closestEntity.GetName(), LogLevel.ERROR);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAddItemToInventory : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Slot which contains target entity")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	protected ref array<ref SCR_ScenarioFrameworkPrefabFilterCountNoInheritance> m_aPrefabFilter

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		entity.GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
		Math3D.AnglesToMatrix(angles, spawnParams.Transform);
		
		foreach (SCR_ScenarioFrameworkPrefabFilterCountNoInheritance prefabFilter : m_aPrefabFilter)
		{
			for (int i = 0; i < prefabFilter.m_iPrefabCount; i++)
			{
				Resource resource = Resource.Load(prefabFilter.m_sPrefabName);
				if (!resource)
					continue;
				
				IEntity item = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
				if (!item)
					continue;
				
				SCR_InvCallBackCheck pInvCallback = new SCR_InvCallBackCheck();
				pInvCallback.m_sNameMaster = entity.GetName();
				pInvCallback.m_sNameItem = item.GetName();
				
				inventoryComponent.TryInsertItem(item, EStoragePurpose.PURPOSE_ANY, pInvCallback)
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRemoveItemFromInventory : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Slot which contains target entity")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	protected ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		//Due to how invokers are setup, deletion is sometimes triggered before said item is actually in said inventory
		GetGame().GetCallqueue().CallLater(OnActivateCalledLater, 1000, false, object);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActivateCalledLater(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;
		
		array<IEntity> items = {};
		inventoryComponent.GetItems(items);
		if (items.IsEmpty())
			return;
		
		array<IEntity> itemsToRemove = {};
		foreach (SCR_ScenarioFrameworkPrefabFilterCount prefabFilter : m_aPrefabFilter)
		{
			BaseContainer prefabContainer = SCR_BaseContainerTools.GetBaseContainer(prefabFilter.m_sSpecificPrefabName);
			if (!prefabContainer)
				continue;
			
			int count;
			int targetCount = prefabFilter.m_iPrefabCount;
			
			bool includeInheritance = prefabFilter.m_bIncludeChildren;
			
			foreach (IEntity item : items)
			{
				if (!item)
					continue;
				
				EntityPrefabData prefabData = item.GetPrefabData();
				if (!prefabData)
					continue;
				
				BaseContainer container = prefabData.GetPrefab();
				if (!container)
					continue;
			
				if (!includeInheritance)
				{
					if (container != prefabContainer)
						continue;
					
					itemsToRemove.Insert(item);
					count++;
					if (count == targetCount)
						break;
				}
				else
				{
					while (container)
					{
						if (container == prefabContainer)
						{
							itemsToRemove.Insert(item);
							count++;
							break;
						}
			
						container = container.GetAncestor();
					}
				}
				
				if (count == targetCount)
					break;
			}
			
			for (int i = itemsToRemove.Count() - 1; i >= 0; i--)
			{
				inventoryComponent.TryDeleteItem(itemsToRemove[i]);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCountInventoryItemsAndExecuteAction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Slot which contains target entity")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	protected ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;
	
	[Attribute(UIWidgets.Auto, desc: "If conditions from Prefab Filter are true, it will execute these actions")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsToExecute;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;
		
		array<IEntity> items = {};
		inventoryComponent.GetItems(items);
		if (items.IsEmpty())
			return;
		
		bool countCondition;
		foreach (SCR_ScenarioFrameworkPrefabFilterCount prefabFilter : m_aPrefabFilter)
		{
			BaseContainer prefabContainer = SCR_BaseContainerTools.GetBaseContainer(prefabFilter.m_sSpecificPrefabName);
			if (!prefabContainer)
				continue;
			
			int count;
			int targetCount = prefabFilter.m_iPrefabCount;
			bool includeInheritance = prefabFilter.m_bIncludeChildren;
			countCondition = false;
			
			foreach (IEntity item : items)
			{
				if (!item)
					continue;
				
				EntityPrefabData prefabData = item.GetPrefabData();
				if (!prefabData)
					continue;
				
				BaseContainer container = prefabData.GetPrefab();
				if (!container)
					continue;
			
				if (!includeInheritance)
				{
					if (container != prefabContainer)
						continue;
					
					count++;
					if (count == targetCount)
					{
						countCondition = true;
						break;
					}
				}
				else
				{
					while (container)
					{
						if (container == prefabContainer)
						{
							count++;
							break;
						}
			
						container = container.GetAncestor();
					}
				}
				
				if (count == targetCount)
				{
					countCondition = true;
					break;
				}
			}
			
			//If just one prefab filter is false, we don't count any further and return
			if (!countCondition)
				return;
		}
		
		if (countCondition)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsToExecute)
			{
				action.OnActivate(object);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLockOrUnlockAllTargetVehiclesInTrigger : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Slot which spawns the trigger")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "true", desc: "If set to true, it will lock all vehicles, if set to false it will unlock all vehicles")];
	protected bool m_bLock;
	//------------------------------------------------------------------------------------------------
	bool CanActivateTriggerVariant(IEntity object, out SCR_CharacterTriggerEntity trigger)
	{
		trigger = SCR_CharacterTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void LockOrUnlockAllVehicles(SCR_CharacterTriggerEntity trigger = null)
	{
		if (!trigger)
			return;

		array<IEntity> entitesInside = {};
		trigger.GetEntitiesInside(entitesInside);
		if (entitesInside.IsEmpty())
			return;

		foreach (IEntity entity : entitesInside)
		{
			if (!Vehicle.Cast(entity))
				continue;

			SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(entity.FindComponent(SCR_VehicleSpawnProtectionComponent));
			if (m_bLock)
			{
				spawnProtectionComponent.SetProtectOnlyDriverSeat(false);
				spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_BuildBlocked-UC");
				spawnProtectionComponent.SetVehicleOwner(-2);
			}
			else
			{
				spawnProtectionComponent.SetProtectOnlyDriverSeat(true);
				spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_CannotEnterVehicle-UC");
				spawnProtectionComponent.ReleaseProtection();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		SCR_CharacterTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			LockOrUnlockAllVehicles(trigger);
			return;
		}

		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return;

		trigger = SCR_CharacterTriggerEntity.Cast(entityFrom);
		if (trigger)
		{
			LockOrUnlockAllVehicles(trigger);
			return;
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLockOrUnlockVehicle : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Slot which spawns the vehicle")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "true", desc: "If set to true, it will lock the vehicle, if set to false it will unlock the vehicle")];
	protected bool m_bLock;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		if (!Vehicle.Cast(entity))
			return;

		SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(entity.FindComponent(SCR_VehicleSpawnProtectionComponent));
		if (!spawnProtectionComponent)
			return;

		if (m_bLock)
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(false);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_BuildBlocked-UC");
			spawnProtectionComponent.SetVehicleOwner(-2);
		}
		else
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(true);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_CannotEnterVehicle-UC");
			spawnProtectionComponent.ReleaseProtection();
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionFailTaskIfVehiclesInTriggerDestroyed : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Trigger Getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Target Layer Task. If this is attached to the Layer Task you intend to work with, you can leave it empty")];
	protected string m_sTargetLayerTask;
	
	[Attribute(defvalue: "false", desc: "If set to true, it will fail the task only when player caused the destruction")];
	protected bool m_bCausedByPlayer;

	bool m_bAlreadyDestroyed;
	SCR_ScenarioFrameworkTask m_Task;
	ref array<IEntity> m_aTargetEntities = {};

	//------------------------------------------------------------------------------------------------
	bool CanActivateTriggerVariant(IEntity object, out SCR_CharacterTriggerEntity trigger)
	{
		trigger = SCR_CharacterTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void AddListener(IEntity object = null, SCR_CharacterTriggerEntity trigger = null)
	{
		if (!object)
			return;

		if (!trigger)
			return;

		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layerTask)
			return;

		m_Task = layerTask.GetTask();
		if (!m_Task)
			return;

		array<IEntity> entitesInside = {};
		trigger.GetEntitiesInside(entitesInside);
		if (entitesInside.IsEmpty())
			return;

		foreach (IEntity entity : entitesInside)
		{
			if (!Vehicle.Cast(entity))
				continue;

			ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(entity.FindComponent(ScriptedDamageManagerComponent));
			if (objectDmgManager)
			{
				m_aTargetEntities.Insert(entity);
				objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || m_bAlreadyDestroyed)
			return;
		
		if (m_bCausedByPlayer)
		{
			bool destroyedByPlayer;
			foreach (IEntity entity : m_aTargetEntities)
			{
				ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(entity.FindComponent(ScriptedDamageManagerComponent));
				if (!objectDmgManager)
					continue;
				
				if (objectDmgManager.GetState() == EDamageState.DESTROYED)
				{
				
					Instigator instigator = objectDmgManager.GetInstigator();
					if (!instigator)
						continue;
					
					InstigatorType instigatorType = instigator.GetInstigatorType();
					if (instigatorType && instigatorType == InstigatorType.INSTIGATOR_PLAYER)
					{
						destroyedByPlayer = true;
						break;
					}
				}
			}
		
			if (!destroyedByPlayer)
				return;
		}

		SCR_BaseTaskSupportEntity supportEntity = SCR_BaseTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity));
		if (!supportEntity)
			return;

		m_bAlreadyDestroyed = true;
		if (m_Task)
			supportEntity.FailTask(m_Task);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_sTargetLayerTask.IsEmpty())
		{
			IEntity entity = GetGame().GetWorld().FindEntityByName(m_sTargetLayerTask);
			if (entity)
				object = entity;
		}

		SCR_CharacterTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			AddListener(object, trigger);
			return;
		}

		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return;

		trigger = SCR_CharacterTriggerEntity.Cast(entityFrom);
		if (trigger)
			AddListener(object, trigger);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTriggerActivationPresence : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger")]
	protected TA_EActivationPresence	m_eActivationPresence;

	//------------------------------------------------------------------------------------------------
	bool CanActivateTriggerVariant(IEntity object, out SCR_CharacterTriggerEntity trigger)
	{
		trigger = SCR_CharacterTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		SCR_CharacterTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			trigger.SetActivationPresence(m_eActivationPresence);
			return;
		}

		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(layer);
		if (area)
		{
			trigger = SCR_CharacterTriggerEntity.Cast(area.GetTrigger());
		}
		else
		{
			IEntity entity = layer.GetSpawnedEntity();
			if (!BaseGameTriggerEntity.Cast(entity))
			{
				Print("ScenarioFramework: SlotTrigger - The selected prefab is not trigger!", LogLevel.ERROR);
				return;
			}
			trigger = SCR_CharacterTriggerEntity.Cast(entity);
		}

		trigger.SetActivationPresence(m_eActivationPresence);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeLayerActivationType : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType), category: "Activation")]
	protected SCR_ScenarioFrameworkEActivationType m_eActivationType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;

		layer.SetActivationType(m_eActivationType);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPrepareAreaFromDynamicDespawn : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "If set to false, area will be despawned")];
	protected bool m_bStaySpawned;

	[Attribute(defvalue: "750", desc: "How close at least one observer camera must be in order to trigger dynamic spawn/despawn")];
	protected int m_iDynamicDespawnRange;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return;

		area.SetDynamicDespawnRange(m_iDynamicDespawnRange);
		manager.PrepareAreaSpecificDynamicDespawn(area, m_bStaySpawned);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRemoveAreaFromDynamicDespawn : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "If set to false, area will be despawned")];
	protected bool m_bStaySpawned;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return;

		manager.RemoveAreaSpecificDynamicDespawn(area, m_bStaySpawned);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlaySound : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Sound to play.")]
	protected string 			m_sSound;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		GetGame().GetCallqueue().CallLater(manager.PlaySoundOnEntity, 2000, false, null, m_sSound);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlaySoundOnEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play the sound on")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Sound to play.")]
	protected string 			m_sSound;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		GetGame().GetCallqueue().CallLater(manager.PlaySoundOnEntity, 2000, false, IEntity.Cast(entityWrapper.GetValue()), m_sSound);
	}
}

//------------------------------------------------------------------------------------------------
//! Changes Task State (For example you can use it to finish or fail task)
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskState : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute("1", UIWidgets.ComboBox, "Task state", "", ParamEnumArray.FromEnum(SCR_TaskState))];
	protected SCR_TaskState m_eTaskState;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
			return;

		SCR_ScenarioFrameworkTaskSupportEntity supportEntity = task.GetSupportEntity();
		if (!supportEntity)
			return;
		
		//Task system is a mess, so this is why we have to tackle it this abysmal way:
		if (m_eTaskState == SCR_TaskState.FINISHED)
			supportEntity.FinishTask(task);
		else if (m_eTaskState == SCR_TaskState.REMOVED)
			supportEntity.FailTask(task);
		else if (m_eTaskState == SCR_TaskState.CANCELLED)
			supportEntity.CancelTask(task.GetTaskID());
		else
			task.SetState(m_eTaskState);
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionResetCounter : SCR_ScenarioFrameworkActionBase
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!object)
			return;

		SCR_ScenarioFrameworkLogicCounter counter = SCR_ScenarioFrameworkLogicCounter.Cast(object.FindComponent(SCR_ScenarioFrameworkLogicCounter));
		if (counter)
			counter.Reset();
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionExecuteFunction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Object the method will be called")];
	protected ref SCR_ScenarioFrameworkGet		m_ObjectToCallTheMethodFrom;

	[Attribute(desc: "Method to call")];
	protected string			m_sMethodToCall;

	[Attribute(desc: "Parameter1 to pass (string only)")];
	protected string		m_sParameter;

	[Attribute(desc: "Parameter2 to pass (string only)")];
	protected string		m_sParameter2;

	[Attribute(desc: "Parameter3 to pass (string only)")];
	protected string		m_sParameter3;

	[Attribute(desc: "Parameter4 to pass (string only)")];
	protected string		m_sParameter4;

	[Attribute(desc: "Parameter5 to pass (string only)")];
	protected string		m_sParameter5;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_ObjectToCallTheMethodFrom.Get());
		if (!entityWrapper)
			return;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkArea));
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerBase));
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (layer)
			GetGame().GetCallqueue().CallByName(layer, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else if (layerBase)
			GetGame().GetCallqueue().CallByName(layerBase, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else if (area)
			GetGame().GetCallqueue().CallByName(area, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else
			GetGame().GetCallqueue().CallByName(entityWrapper.GetValue(), m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
	}
}

