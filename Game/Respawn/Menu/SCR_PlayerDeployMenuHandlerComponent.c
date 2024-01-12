void OnDeployMenuOpenDelegate();
typedef func OnDeployMenuOpenDelegate;
typedef ScriptInvokerBase<OnDeployMenuOpenDelegate> OnDeployMenuOpenInvoker;

void OnDeployMenuCloseDelegate();
typedef func OnDeployMenuCloseDelegate;
typedef ScriptInvokerBase<OnDeployMenuCloseDelegate> OnDeployMenuCloseInvoker;

//------------------------------------------------------------------------------------------------
class SCR_PlayerDeployMenuHandlerComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
/*!
	Component responsible for deploy menu management.
*/
class SCR_PlayerDeployMenuHandlerComponent : ScriptComponent
{
	private SCR_RespawnComponent m_RespawnComponent;
	private SCR_PlayerController m_PlayerController;
	private SCR_PlayerFactionAffiliationComponent m_PlyFactionAffil;
	private bool m_bReady = false;
	private bool m_bFirstOpen = true;
	private bool m_bWelcomeOpened;
	private bool m_bWelcomeClosed;
	
	private static ref OnDeployMenuCloseInvoker s_OnMenuClose;

	private DeployMenuSystem m_DeployMenuSystem;
	private SCR_MenuSpawnLogic m_SpawnLogic;

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SCR_RespawnSystemComponent rsc = SCR_RespawnSystemComponent.GetInstance();
		if (!GetGame().InPlayMode() || (rsc && !rsc.CanOpenDeployMenu()))
			return;				

		World world = GetOwner().GetWorld();		
		m_DeployMenuSystem = DeployMenuSystem.Cast(world.FindSystem(DeployMenuSystem));
		m_DeployMenuSystem.Register(this);

		m_RespawnComponent = SCR_RespawnComponent.Cast(owner.FindComponent(SCR_RespawnComponent));
		if (!m_RespawnComponent)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), SCR_RespawnComponent),
				LogLevel.ERROR);		
		}

		m_PlayerController = SCR_PlayerController.Cast(owner);
		if (!m_PlayerController)
		{
			Print(string.Format("%1 is not attached in %2 hierarchy! (%1 should be a child of %3!)",
				Type().ToString(), SCR_PlayerController, SCR_RespawnComponent),
				LogLevel.ERROR);
		}
		
		m_PlyFactionAffil = SCR_PlayerFactionAffiliationComponent.Cast(owner.FindComponent(SCR_PlayerFactionAffiliationComponent));

		m_RespawnComponent.GetOnRespawnReadyInvoker_O().Insert(OnRespawnReady);
		m_RespawnComponent.GetOnRespawnResponseInvoker_O().Insert(SetNotReady);
		m_SpawnLogic = SCR_MenuSpawnLogic.Cast(rsc.GetSpawnLogic());

		SCR_ReconnectSynchronizationComponent reconnectComp = SCR_ReconnectSynchronizationComponent.Cast(owner.FindComponent(SCR_ReconnectSynchronizationComponent));
		if (reconnectComp)
			reconnectComp.GetOnPlayerReconnect().Insert(OnPlayerReconnect);

		SCR_WelcomeScreenComponent welcomeScreenComp = SCR_WelcomeScreenComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_WelcomeScreenComponent));
		if (!welcomeScreenComp)
			SetWelcomeClosed();	
	}

	void Update(float timeSlice)
	{
#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
			return;
#endif
		if (CanOpenWelcomeScreen())
		{
			if (!IsWelcomeScreenOpen())
				GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.WelcomeScreenMenu);
			return;

		}
		else if (IsWelcomeScreenOpen())
		{
			CloseWelcomeScreen();
			return;
		}

		if (CanOpenMenu())
		{
			if (HasPlayableFaction() && !SCR_RoleSelectionMenu.GetRoleSelectionMenu())
			{
				if (!SCR_DeployMenuMain.GetDeployMenu())
				{
					SCR_RoleSelectionMenu.CloseRoleSelectionMenu();
					SCR_DeployMenuMain.OpenDeployMenu();
				}
			}
			else
			{
				if (!SCR_RoleSelectionMenu.GetRoleSelectionMenu())
				{
					SCR_DeployMenuMain.CloseDeployMenu();
					SCR_RoleSelectionMenu.OpenRoleSelectionMenu();
				}
				
				m_bFirstOpen = false;
			}
		}
		else if (IsMenuOpen())
			CloseMenu();
	}
	
	protected bool IsMenuOpen()
	{
		return SCR_DeployMenuMain.GetDeployMenu() || SCR_RoleSelectionMenu.GetRoleSelectionMenu();
	}

	protected bool IsWelcomeScreenOpen()
	{
		return GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
	}

	protected void CloseWelcomeScreen()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
	}

	protected void CloseMenu()
	{
		if (m_PlayerController.GetPlayerId() != GetGame().GetPlayerController().GetPlayerId())
			return;
		
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RoleSelectionDialog);

		if (s_OnMenuClose)
			s_OnMenuClose.Invoke();
	}
	
	protected bool HasPlayableFaction()
	{
		if (!m_PlyFactionAffil)
			return false;

		SCR_Faction faction = SCR_Faction.Cast(m_PlyFactionAffil.GetAffiliatedFaction());

		return (faction && faction.IsPlayable());
	}

	protected bool CanOpenMenu()
	{
		if (!m_bWelcomeClosed)
			return false;

		bool hasEntity = (m_PlayerController.IsPossessing() || m_PlayerController.GetControlledEntity());
		bool hasDeadEntity = false;
		ChimeraCharacter character = ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());
		if (character)
			hasDeadEntity = character.GetCharacterController().IsDead();
		
		bool canOpen = (
			!SCR_EditorManagerEntity.IsOpenedInstance() && 
			(!hasEntity || hasDeadEntity) && 
			(SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetState() != SCR_EGameModeState.POSTGAME)
		);

		return canOpen;
	}
	
	protected bool CanOpenWelcomeScreen()
	{
		bool hasEntity = (m_PlayerController.IsPossessing() || m_PlayerController.GetControlledEntity());

		if (m_bWelcomeClosed || hasEntity)
			return false;

		bool hasDeadEntity = false;
		ChimeraCharacter character = ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());
		if (character)
			hasDeadEntity = character.GetCharacterController().IsDead();
		
		bool canOpen = (
			!SCR_EditorManagerEntity.IsOpenedInstance() && 
			(!hasEntity || hasDeadEntity) && 
			(SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetState() != SCR_EGameModeState.POSTGAME)
		);

		return canOpen;		
	}

	protected void OnRespawnReady()
	{
		if (m_bFirstOpen)
		{
			SetReady();
			return;
		}
		
		// Delay to next frame as current character deleted replication could by applied later than the respawn system RPC this event is raised from.
		GetGame().GetCallqueue().Call(SetReadyDelayed);
	}

	protected void SetReadyDelayed()
	{
		IEntity controlledEntity = m_PlayerController.GetControlledEntity();
		if (!controlledEntity || controlledEntity.IsDeleted())
		{
			SetReady();
			return;
		}

		float delay = SCR_RespawnSystemComponent.GetInstance().GetDeployMenuOpenDelay_ms();
		GetGame().GetCallqueue().CallLater(SetReady, delay, false);
	}

	protected void SetReady()
	{
		if (m_SpawnLogic)
			m_SpawnLogic.DestroyLoadingPlaceholder();
		m_DeployMenuSystem.SetReady(true);
	}
	
	void SetWelcomeClosed()
	{
		m_bWelcomeClosed = true;
	}

	protected void OnPlayerReconnect(int state)
	{
		if (m_SpawnLogic)
			m_SpawnLogic.DestroyLoadingPlaceholder();
		m_bFirstOpen = false;
		SetWelcomeClosed();
	}

	protected void SetNotReady(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		if (response == SCR_ESpawnResult.OK)
		{
			SCR_DeployMenuMain.CloseDeployMenu();
			SCR_RoleSelectionMenu.CloseRoleSelectionMenu();
			CloseWelcomeScreen();
			SetWelcomeClosed();
			m_DeployMenuSystem.SetReady(false);
			m_bFirstOpen = false;
		}
	}

	bool UpdateLoadingPlaceholder(float dt)
	{
		return m_SpawnLogic.UpdateLoadingPlaceholder(dt);
	}

	static OnDeployMenuCloseInvoker SGetOnMenuClosed()
	{
		if (!s_OnMenuClose)
			s_OnMenuClose = new OnDeployMenuCloseInvoker();
		
		return s_OnMenuClose;
	}
	
	override protected void OnDelete(IEntity owner)
	{
		if (s_OnMenuClose)
			delete s_OnMenuClose;
		
		if (m_DeployMenuSystem)
			m_DeployMenuSystem.Unregister(this);
		super.OnDelete(owner);
	}
};
