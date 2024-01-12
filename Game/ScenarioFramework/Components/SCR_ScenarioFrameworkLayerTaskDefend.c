#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerTaskDefendClass : SCR_ScenarioFrameworkLayerTaskClass
{
	// prefab properties here
};

class SCR_ScenarioFrameworkLayerTaskDefend : SCR_ScenarioFrameworkLayerTask
{
	[Attribute(desc: "Will use trigger that is named for Defend params calculations", category: "Task")]
	protected string 											m_sTriggerName;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox, desc: "Text that will be displayed above the countdown number", category: "Defend params")]
	protected LocalizedString 											m_sCountdownTitleText;

	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "For how long you have to Defend the objective of the task. Value -1 is for indefinitely.", params: "-1 86400 1", category: "Defend params")]
	protected float 											m_fDefendTime;

	[Attribute(defvalue: "1", desc: "When enabled, it will display the text and how much time remains for the Task Defend", category: "Defend params")]
	protected bool 												m_bDisplayCountdownHUD;

	[Attribute(defvalue: "{47864BB47AB0B1F4}UI/layouts/HUD/CampaignMP/CampaignMainHUD.layout", category: "Defend params")]
	protected ResourceName 										m_sCountdownHUD;

	[Attribute(category: "Defend params")]
	protected ref array<ref SCR_ScenarioFrameworkTaskDefendFactionSettings> 		m_aFactionSettings;

	[Attribute(uiwidget: UIWidgets.Slider, desc: "When compared to the number of attackers, minimum of how much of the characters present in the task area must be from defending side to successfully complete the objective on evaluation", params: "0 1 0.01", precision: 2, category: "Defend params")]
	protected float												m_fMinDefenderPercentageRatio;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Layer containing attacker forces. Can be more layers, but these layers must include only AI units/groups and nothing else.", category: "Defend params")]
	protected ref array<string>											m_aAttackerLayerNames;

	[Attribute(desc: "When enabled, it will can finish the task earlier than the countdown when all attackers are eliminated", category: "Defend params")]
	protected bool 												m_bEarlierEvaluation;

	[Attribute(desc: "When enabled, evaluation will be delayed and defenders will need to eliminate all attackers in order for the task to be succesfully completed", category: "Defend params")]
	protected bool 												m_bDelayedEvaluation;

	[Attribute(desc: "When enabled, it will display the text to inform players that they have to eliminate all attacker units", category: "Defend params")]
	protected bool 												m_bDisplayDelayedEvaluationText;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox, desc: "Text that will be displayed to inform players that they have to eliminate all attacker units", category: "Defend params")]
	protected LocalizedString 											m_sDelayedEvaluationText;

	protected SCR_CharacterTriggerEntity m_CharacterTriggerEntity;

	protected float m_fTempCountdown = m_fDefendTime;

	protected string m_sFormattedCountdownTitle = string.Format(WidgetManager.Translate("<color rgba=\"226, 168, 80, 255\">%1</color>", m_sCountdownTitleText));
	protected string m_sFormattedDelayedEvaluationText = string.Format(WidgetManager.Translate("<color rgba=\"226, 168, 80, 255\">%1</color>", m_sDelayedEvaluationText));

	protected ref array<SCR_ScenarioFrameworkLayerBase> m_aAttackerLayer = {};

	protected float m_fTempTimeSlice;
	protected bool m_bTaskEvaluated;
	protected bool m_bEvaluationSet;
	protected WorldTimestamp m_fEvaluateTimeStart;
	protected WorldTimestamp m_fEvaluateTimeEnd;

	protected Widget m_wRoot;
	protected Widget m_wInfoOverlay;
	protected Widget m_wCountdownOverlay;
	protected RichTextWidget m_wCountdown;
	protected RichTextWidget m_wFlavour;

	//------------------------------------------------------------------------------------------------
	SCR_CharacterTriggerEntity GetCharacterTriggerEntity()
	{
		return m_CharacterTriggerEntity;
	}

	//------------------------------------------------------------------------------------------------
	void FindCharacterTriggerEntity()
	{
		IEntity foundEntity = GetGame().GetWorld().FindEntityByName(m_sTriggerName);
		if (!foundEntity)
			return;

		SCR_ScenarioFrameworkSlotTrigger slotTrigger = SCR_ScenarioFrameworkSlotTrigger.Cast(foundEntity.FindComponent(SCR_ScenarioFrameworkSlotTrigger));
		if (!slotTrigger)
			return;

		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(slotTrigger.GetSpawnedEntity());
		if (!trigger)
			return;

		m_CharacterTriggerEntity = trigger;
	}

	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDefendSupportEntity))
		{
			Print("ScenarioFramework: Task Defend support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskDefendSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDefendSupportEntity));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerTaskDefend(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.DEFEND;
	}

	//------------------------------------------------------------------------------------------------
	protected void CountAttackerDefenderNumbers(out int defenderCount, out int attackerCount)
	{
		for (int i = 0, count = m_aFactionSettings.Count(); i < count; i++)
		{
			SCR_ScenarioFrameworkTaskDefendDefendingFaction defender = SCR_ScenarioFrameworkTaskDefendDefendingFaction.Cast(m_aFactionSettings[i]);
			if (defender)
			{
				if (defender.GetCountOnlyPlayers())
					defenderCount += m_CharacterTriggerEntity.GetPlayersCountByFactionInsideTrigger(defender.GetFaction());
				else
					defenderCount += m_CharacterTriggerEntity.GetCharacterCountByFactionInsideTrigger(defender.GetFaction());
			}

			SCR_ScenarioFrameworkTaskDefendAttackingFaction attacker = SCR_ScenarioFrameworkTaskDefendAttackingFaction.Cast(m_aFactionSettings[i]);
			if (attacker)
			{
				if (attacker.GetCountOnlyPlayers())
					attackerCount += m_CharacterTriggerEntity.GetPlayersCountByFactionInsideTrigger(attacker.GetFaction());
				else
					attackerCount += m_CharacterTriggerEntity.GetCharacterCountByFactionInsideTrigger(attacker.GetFaction());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void EvaluateStatus()
	{
		m_bTaskEvaluated = true;

		if (!m_Task)
		{
			SCR_ScenarioFrameworkTask taskToFind;
			array<SCR_BaseTask> activeTasks = {};
			GetTaskManager().GetTasks(activeTasks);

			foreach (SCR_BaseTask task : activeTasks)
			{
				taskToFind = SCR_ScenarioFrameworkTask.Cast(task);
				if (!taskToFind)
					continue;

				if (taskToFind.GetTaskLayer() == this)
				{
					m_Task = taskToFind;
					break;
				}
			}
		}

		if (!m_Task)
		{
			RemovePeriodicUpdates();
			return;
		}

		if (!m_CharacterTriggerEntity && !m_sTriggerName.IsEmpty())
			FindCharacterTriggerEntity();

		if (m_CharacterTriggerEntity)
		{
			int defenderCount = 0;
			int attackerCount = 0;
			CountAttackerDefenderNumbers(defenderCount, attackerCount);

			if (m_fMinDefenderPercentageRatio == 0 || attackerCount == 0)
			{
				m_SupportEntity.FinishTask(m_Task);
				RemovePeriodicUpdates();
				return;
			}

			float defenderRatioEval = defenderCount / attackerCount;
			if (defenderRatioEval < m_fMinDefenderPercentageRatio)
			{
				m_SupportEntity.FailTask(m_Task);
				RemovePeriodicUpdates();
				return;
			}
		}

		m_SupportEntity.FinishTask(m_Task);
		RemovePeriodicUpdates();
	}

	//------------------------------------------------------------------------------------------------
	void SetupAttackerLayer()
	{
		SCR_ScenarioFrameworkLayerBase attackerLayer;
		IEntity attackerLayerEntity;
		foreach (string layerName : m_aAttackerLayerNames)
		{
			attackerLayerEntity = GetGame().GetWorld().FindEntityByName(layerName);
			if (!attackerLayerEntity)
				continue;

			attackerLayer = SCR_ScenarioFrameworkLayerBase.Cast(attackerLayerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!attackerLayer)
				continue;

			m_aAttackerLayer.Insert(attackerLayer);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetupEvaluation()
	{
		if (m_fDefendTime > 0)
		{
			ChimeraWorld world = GetOwner().GetWorld();
			m_fEvaluateTimeStart = world.GetServerTimestamp();
			m_fEvaluateTimeEnd = m_fEvaluateTimeStart.PlusSeconds(m_fDefendTime);
			m_bEvaluationSet = true;
		}

		if (m_bEarlierEvaluation || m_bDelayedEvaluation)
			SetupAttackerLayer();

		//If trigger is already set via the Trigger Name attribute, we don't need to search for it
		if (m_CharacterTriggerEntity)
			return;

		for (int i = 0, count = m_aChildren.Count(); i < count; i++)
		{
			SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(m_aChildren[i]);
			if (!layerBase)
				continue;

			array<IEntity> spawnedEntities = layerBase.GetSpawnedEntities();
			for (int j = 0, countJ = spawnedEntities.Count(); j < countJ; j++)
			{
				SCR_CharacterTriggerEntity charTriggr = SCR_CharacterTriggerEntity.Cast(spawnedEntities[j]);
				if (charTriggr)
					m_CharacterTriggerEntity = charTriggr;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicDespawn()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bInitiated)
			return;
		
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
			return;

		super.Init(area, activation);
	}
	
	//------------------------------------------------------------------------------------------------
	override void AfterAllChildrenSpawned()
	{
		m_bInitiated = true;
		
		ActivateLogic();
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}
		
		foreach(SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.Init(GetOwner());
		}

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);

		if (m_fRepeatedSpawnTimer >= 0)
			RepeatedSpawn();
		
		if (!m_sTriggerName.IsEmpty())
			FindCharacterTriggerEntity();

		InitHUD();
		SetupEvaluation();
		OnPostInit(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (m_bInitiated)
		{
			SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	void InitHUD()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;
		
		m_wRoot = hudManager.CreateLayout(m_sCountdownHUD, EHudLayers.MEDIUM, 0);
		if (!m_wRoot)
			return;
		
		m_wInfoOverlay = m_wRoot.FindAnyWidget("Info");
		m_wCountdownOverlay = m_wRoot.FindAnyWidget("Countdown");
		ImageWidget leftFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideBlue"));
		ImageWidget rightFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideRed"));
		RichTextWidget leftScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreBlue"));
		RichTextWidget rightScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreRed"));
		RichTextWidget winScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("TargetScore"));
		m_wCountdown = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CountdownWin"));
		m_wFlavour = RichTextWidget.Cast(m_wRoot.FindAnyWidget("FlavourText"));
		ImageWidget winScoreSideLeft = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveLeft"));
		ImageWidget winScoreSideRight = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveRight"));

		m_wInfoOverlay.SetVisible(false);
		leftFlag.SetVisible(false);
		rightFlag.SetVisible(false);
		leftScore.SetVisible(false);
		rightScore.SetVisible(false);
		winScore.SetVisible(false);
		winScoreSideLeft.SetVisible(false);
		winScoreSideRight.SetVisible(false);

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);

		if (!m_bDisplayCountdownHUD)
			m_wRoot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateHUD()
	{
		m_fTempTimeSlice = 0;
		m_fTempCountdown--;
		
		int taskID = -1;
		if (m_Task)
			taskID = m_Task.GetTaskID();
		
		Rpc(RpcDo_UpdateHUD, m_fTempCountdown, taskID);
		
		if (!m_wRoot)
			return;

		if (m_fTempCountdown < 0 || !m_Task || !m_bDisplayCountdownHUD)
		{
			if (m_bDelayedEvaluation && m_bDisplayDelayedEvaluationText && !m_bTaskEvaluated)
			{
				m_wFlavour.SetText(m_sFormattedDelayedEvaluationText);
				m_wRoot.SetVisible(true);
				m_wCountdownOverlay.SetVisible(false);
				return;
			}

			m_wRoot.SetVisible(false);
			return;
		}

		if (m_Task.GetTaskState() == SCR_TaskState.CANCELLED || m_Task.GetTaskState() == SCR_TaskState.FINISHED)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);
		m_wRoot.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdateHUD(float countdown, int taskID)
	{
		m_fTempCountdown = countdown;
		
		if (!m_wRoot)
		{
			InitHUD();
			if (!m_wRoot)
				return;
		}
		
		if (!m_Task)
		{
			SCR_BaseTaskManager taskManager = GetTaskManager();
			if (taskManager)
				m_Task = SCR_ScenarioFrameworkTask.Cast(taskManager.GetTask(taskID));
		}

		if (m_fTempCountdown < 0 || !m_Task || !m_bDisplayCountdownHUD)
		{
			if (m_bDelayedEvaluation && m_bDisplayDelayedEvaluationText && !m_bTaskEvaluated)
			{
				m_wFlavour.SetText(m_sFormattedDelayedEvaluationText);
				m_wRoot.SetVisible(true);
				m_wCountdownOverlay.SetVisible(false);
				return;
			}

			m_wRoot.SetVisible(false);
			return;
		}

		if (m_Task.GetTaskState() == SCR_TaskState.CANCELLED || m_Task.GetTaskState() == SCR_TaskState.FINISHED)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);
		m_wRoot.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	void CheckAttackerLayers()
	{
		m_fTempTimeSlice = 0;
		foreach (SCR_ScenarioFrameworkLayerBase attackerLayer : m_aAttackerLayer)
		{
			if (attackerLayer.GetEnableRepeatedSpawn() && attackerLayer.GetRepeatedSpawnNumber() > 1)
				return;

			array<IEntity> spawnedEntities = attackerLayer.GetSpawnedEntities();
			SCR_ChimeraCharacter character;
			DamageManagerComponent damageManager;
			foreach (IEntity entity : spawnedEntities)
			{
				character = SCR_ChimeraCharacter.Cast(entity);
				if (!character)
				{
					SCR_AIGroup group = SCR_AIGroup.Cast(entity);
					if (group)
						return;
					else
						continue;
				}

				damageManager = character.GetDamageManager();
				if (!damageManager.IsDestroyed())
					return;
			}
		}

		EvaluateStatus();
	}

	//------------------------------------------------------------------------------------------------
	void RemovePeriodicUpdates()
	{
		if (!m_bShowDebugShapesDuringRuntime)
			GetOwner().ClearFlags(EntityFlags.ACTIVE);

		UpdateHUD();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		m_fTempTimeSlice += timeSlice;
		if (m_fTempTimeSlice >= 1 && m_fTempCountdown >= 0)
		{
			UpdateHUD();
			if (m_bEarlierEvaluation)
				CheckAttackerLayers();
		}
		else if (m_bEarlierEvaluation && m_fTempTimeSlice >= 5)
		{
			CheckAttackerLayers();
		}

		ChimeraWorld world = owner.GetWorld();
		if (!m_bTaskEvaluated && m_bEvaluationSet && world.GetServerTimestamp().GreaterEqual(m_fEvaluateTimeEnd))
		{
			if (m_bDelayedEvaluation && m_fTempTimeSlice >= 5)
				CheckAttackerLayers();

			if (!m_bDelayedEvaluation)
				EvaluateStatus();
		}
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkTaskDefendFactionSettings
{
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Faction Name", category: "")]
	protected FactionKey				m_sFactionKey;

	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "When disabled, all units from this faction will be counted with for other Task Defend conditions", category: "")]
	protected bool				m_bCountOnlyPlayers;

	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			return factionManager.GetFactionByKey(m_sFactionKey);

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void SetFactionKey(FactionKey factionKey)
	{
		m_sFactionKey = factionKey;
	}

	//------------------------------------------------------------------------------------------------
	bool GetCountOnlyPlayers()
	{
		return m_bCountOnlyPlayers;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskDefendDefendingFactionTitle: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Defending faction";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskDefendAttackingFactionTitle: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Attacking faction";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ScenarioFrameworkTaskDefendDefendingFactionTitle()]
class SCR_ScenarioFrameworkTaskDefendDefendingFaction : SCR_ScenarioFrameworkTaskDefendFactionSettings
{
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ScenarioFrameworkTaskDefendAttackingFactionTitle()]
class SCR_ScenarioFrameworkTaskDefendAttackingFaction : SCR_ScenarioFrameworkTaskDefendFactionSettings
{
};
