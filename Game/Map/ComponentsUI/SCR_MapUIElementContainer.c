class SCR_MapUIElementContainer : SCR_MapUIBaseComponent
{
	[Attribute("UIIconsContainer")];
	protected string m_sIconsContainer;
	protected Widget m_wIconsContainer;

	[Attribute("{E78DE3FD19654C1B}UI/layouts/Campaign/SpawnPointElement.layout", params: "layout")]
	protected ResourceName m_sSpawnPointElement;

	[Attribute("{EAB5D9841F081D07}UI/layouts/Campaign/TaskElementNew.layout", params: "layout")]
	protected ResourceName m_sTaskElement;

	[Attribute("{C013EB43E812F9C1}UI/layouts/Menus/DeployMenu/WarningHintTemp.layout")]
	protected ResourceName m_sWarningWidget;
	protected Widget m_wWarningWidget;

	[Attribute("0")]
	protected bool m_bShowSpawnPointsHint;

	[Attribute("1")]
	protected bool m_bShowSpawnPoints;

	[Attribute("1")]
	protected bool m_bShowTasks;

	protected bool m_bIsEditor; // Map opened in editor with gamemaster rights (!limited)
	protected bool m_bIsDeployMap;

	protected ref map<Widget, SCR_MapUIElement> m_mIcons = new map<Widget, SCR_MapUIElement>();

	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;
	protected SCR_PlayerControllerGroupComponent m_PlyGroupComp;
	protected SCR_BaseGameMode m_GameMode;

	protected ref ScriptInvoker<SCR_MapUIElement> m_OnElementSelected;
	protected ref ScriptInvoker<RplId> m_OnSpawnPointSelected;

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_bIsEditor = SCR_EditorManagerEntity.IsOpenedInstance(false);
	}

	protected void AddSpawnPoint(SCR_SpawnPoint spawnPoint)
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (!spawnPoint.IsSpawnPointVisibleForPlayer(SCR_PlayerController.GetLocalPlayerId()))
			return;

		if (playerFaction && playerFaction.GetFactionKey() == spawnPoint.GetFactionKey())
		{
			ShowSpawnPoint(spawnPoint);
			UpdateIcons();
		}
	}

	protected void RemoveSpawnPoint(SCR_SpawnPoint spawnPoint)
	{
		RplId pointId = spawnPoint.GetRplId();
		HideSpawnPoint(pointId);
		UpdateIcons();
	}
	
	protected void OnSpawnPointFactionChange(SCR_SpawnPoint spawnPoint)
	{
		if (spawnPoint.GetFactionKey() == m_PlyFactionAffilComp.GetAffiliatedFaction().GetFactionKey())
			AddSpawnPoint(spawnPoint);
		else
			RemoveSpawnPoint(spawnPoint);
	}

	protected void HideSpawnPoint(RplId spawnPointId)
	{
		foreach (Widget w, SCR_MapUIElement icon : m_mIcons)
		{
			if (icon && icon.GetSpawnPointId() == spawnPointId)
			{
				m_mIcons.Remove(w);
				w.RemoveFromHierarchy();
			}
		}
	}

	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (response)
		{
			RemoveAllIcons();
			
			if (m_bShowSpawnPoints)
				InitSpawnPoints();

			if (m_bShowTasks)
				InitTaskMarkers();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (m_PlyFactionAffilComp)
				m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);	

			m_PlyGroupComp = SCR_PlayerControllerGroupComponent.Cast(pc.FindComponent(SCR_PlayerControllerGroupComponent));
			if (m_PlyGroupComp)
				m_PlyGroupComp.GetOnGroupChanged().Insert(OnPlayerGroupChanged);		
		}
		
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionChange);
		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(AddSpawnPoint);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(RemoveSpawnPoint);

		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(OnTaskAdded);

		m_bIsDeployMap = (config.MapEntityMode == EMapEntityMode.SPAWNSCREEN);

		m_wIconsContainer = m_RootWidget.FindAnyWidget(m_sIconsContainer);
		m_wIconsContainer.SetVisible(true);

		Widget child = m_wIconsContainer.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			delete child;
			child = sibling;
		}

		if (m_bShowSpawnPoints)
			InitSpawnPoints();

		if (m_bShowTasks)
			InitTaskMarkers();
		
		m_MapEntity.GetOnMapPan().Insert(OnMapPan);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		if (m_PlyFactionAffilComp)
			m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Remove(OnPlayerFactionResponse);

		if (m_PlyGroupComp)
			m_PlyGroupComp.GetOnGroupChanged().Remove(OnPlayerGroupChanged);
		
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnPointFactionChange);
		SCR_SpawnPoint.Event_SpawnPointAdded.Remove(AddSpawnPoint);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(RemoveSpawnPoint);

		SCR_BaseTaskManager.s_OnTaskUpdate.Remove(OnTaskAdded);

		m_MapEntity.GetOnMapPan().Remove(OnMapPan);

		foreach(Widget w, SCR_MapUIElement e : m_mIcons)
		{
			if (!w)
				continue;
			w.RemoveFromHierarchy();
		}
		
		m_mIcons.Clear();
	}

	//------------------------------------------------------------------------------------------------
	void OnMapPan(float panX, float panY, bool adjustedPan)
	{
		 UpdateIcons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateIcons()
	{
		foreach (Widget widget, SCR_MapUIElement icon : m_mIcons)
		{
			if (!icon || !widget)
				continue;

			vector pos = icon.GetPos();
			float x, y;
			m_MapEntity.WorldToScreen(pos[0], pos[2], x, y, true);

			x = GetGame().GetWorkspace().DPIUnscale(x);
			y = GetGame().GetWorkspace().DPIUnscale(y);

			if (m_bShowSpawnPointsHint)
			{
				if (!m_wWarningWidget)
					m_wWarningWidget = GetGame().GetWorkspace().CreateWidgets(m_sWarningWidget, m_wIconsContainer);

				// just a hint indicating icons outside of the view
				float screenWidth = GetGame().GetWorkspace().GetWidth();
				float screenHeight = GetGame().GetWorkspace().GetHeight();
				float screenWUnscaled = GetGame().GetWorkspace().DPIUnscale(screenWidth);
				float screenHUnscaled = GetGame().GetWorkspace().DPIUnscale(screenHeight);
				float ctverecWidth, ctverecHeight;
				m_wWarningWidget.GetScreenSize(ctverecWidth, ctverecHeight);

				if (x < 0)
					FrameSlot.SetPos(m_wWarningWidget, 0, y);
				if (x > screenWUnscaled)
					FrameSlot.SetPos(m_wWarningWidget, screenWUnscaled - ctverecWidth * 2, y);

				if (y < 0)
					FrameSlot.SetPos(m_wWarningWidget, x, 0);
				if (y > screenHUnscaled)
					FrameSlot.SetPos(m_wWarningWidget, x, screenHUnscaled - ctverecHeight * 2);

				m_wWarningWidget.SetVisible(x < 0 || x > screenWUnscaled || y < 0 || y > screenHUnscaled);
			}

			FrameSlot.SetPos(widget, x, y);
		}
	}

	protected void InitSpawnPoints()
	{
		array<SCR_SpawnPoint> infos = {};
		if (!m_bIsEditor)
		{
			Faction playerFaction = m_PlyFactionAffilComp.GetAffiliatedFaction();
			if (!playerFaction)
				return;
			
			infos = SCR_SpawnPoint.GetSpawnPointsForFaction(playerFaction.GetFactionKey());
		}
		else
		{
			infos = SCR_SpawnPoint.GetSpawnPoints();
		}

		foreach (SCR_SpawnPoint info : infos)
		{
			ShowSpawnPoint(info);
		}

		UpdateIcons();
	}

	protected void ShowSpawnPoint(notnull SCR_SpawnPoint spawnPoint)
	{
		if (!m_bIsDeployMap && spawnPoint.GetVisibleInDeployMapOnly())
			return;

		if (!spawnPoint.IsSpawnPointVisibleForPlayer(SCR_PlayerController.GetLocalPlayerId()))
			return;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sSpawnPointElement, m_wIconsContainer);
		SCR_MapUISpawnPoint handler = SCR_MapUISpawnPoint.Cast(w.FindHandler(SCR_MapUISpawnPoint));
		if (!handler)
			return;

		handler.Init(spawnPoint);
		handler.SetParent(this);
		m_mIcons.Set(w, handler);

		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitTaskMarkers()
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;

		int taskCount;
		array<SCR_BaseTask> availableTasks = {};
		if (m_bIsEditor)
		{
			taskCount = taskManager.GetTasks(availableTasks);
		}
		else
		{
			taskCount = taskManager.GetFilteredTasks(availableTasks, SCR_FactionManager.SGetLocalPlayerFaction());
		}

		for (int i = 0; i < taskCount; ++i)
		{
			InitTaskIcon(availableTasks[i]);
		}

		UpdateIcons();
	}

	protected void InitTaskIcon(SCR_BaseTask task)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sTaskElement, m_wIconsContainer);
		if (!w)
			return;

		SCR_MapUITask handler = SCR_MapUITask.Cast(w.FindHandler(SCR_MapUITask));
		if (!handler)
			return;

		handler.SetParent(this);
		handler.InitTask(task);
		m_mIcons.Set(w, handler);

		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);
	}

	protected void OnTaskAdded(SCR_BaseTask task)
	{
		if (!m_bShowTasks)
			return;

		if (task.GetTargetFaction() != SCR_FactionManager.SGetLocalPlayerFaction())
			return;

		foreach (Widget w, SCR_MapUIElement e : m_mIcons)
		{
			SCR_MapUITask t = SCR_MapUITask.Cast(e);
			if (!t)
				continue;

			if (t.GetTask() == task)
				return;
		}

		InitTaskIcon(task);
		UpdateIcons();
	}

	protected void RemoveAllIcons()
	{
		Widget child = m_wIconsContainer.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			delete child;
			child = sibling;
		}

		m_mIcons.Clear();
	}

	void RemoveIcon(SCR_MapUIElement icon)
	{
		delete m_mIcons.GetKeyByValue(icon);
	}

	protected SCR_MapUIElement FindSpawnPointIconById(RplId id)
	{
		foreach (Widget w, SCR_MapUIElement icon : m_mIcons)
		{
			if (icon && icon.GetSpawnPointId() == id)
				return icon;
		}

		return null;
	}

	void OnSpawnPointSelectedExt(RplId id) // called when selected via deploy menu spinbox
	{
		SCR_MapUIElement icon = FindSpawnPointIconById(id);
		if (icon)
			icon.SelectIcon(false);
	}

	void OnElementSelected(notnull SCR_MapUIElement element)
	{
		GetOnElementSelected().Invoke(element);
	}

	void OnSpawnPointSelected(RplId spId = RplId.Invalid())
	{
		GetOnSpawnPointSelected().Invoke(spId);
	}
	
	protected void OnPlayerGroupChanged(SCR_AIGroup group)
	{
		int pid = SCR_PlayerController.GetLocalPlayerId();
		foreach (Widget w, SCR_MapUIElement e : m_mIcons)
		{
			SCR_MapUISpawnPoint spIcon = SCR_MapUISpawnPoint.Cast(e);
			if (!spIcon)
				continue;

			SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(spIcon.GetSpawnPointId());
			if (!sp)
				continue;

			if (!sp.IsSpawnPointVisibleForPlayer(pid))
				RemoveSpawnPoint(sp);
		}

		array<SCR_SpawnPoint> playerSpawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_PlyFactionAffilComp.GetAffiliatedFaction().GetFactionKey());
		foreach (SCR_SpawnPoint sp : playerSpawnPoints)
		{
			SCR_MapUISpawnPoint spIcon = FindSpawnPoint(sp.GetRplId()); // this is pretty bad sorry
			if (spIcon)
				continue;

			if (sp.IsSpawnPointVisibleForPlayer(SCR_PlayerController.GetLocalPlayerId()))
				AddSpawnPoint(sp);
		}
	}
	
	SCR_MapUISpawnPoint FindSpawnPoint(RplId id)
	{
		foreach (Widget w, SCR_MapUIElement e : m_mIcons)
		{
			SCR_MapUISpawnPoint sp = SCR_MapUISpawnPoint.Cast(e);
			if (!sp)
				continue;

			if (sp.GetSpawnPointId() == id)
				return sp;
		}

		return null;
	}

	ScriptInvoker GetOnSpawnPointSelected()
	{
		if (!m_OnSpawnPointSelected)
			m_OnSpawnPointSelected = new ScriptInvoker();

		return m_OnSpawnPointSelected;
	}

	ScriptInvoker GetOnElementSelected()
	{
		if (!m_OnElementSelected)
			m_OnElementSelected = new ScriptInvoker();

		return m_OnElementSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MapUIElementContainer()
	{
	}
}
