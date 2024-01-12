//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_UICore: SCR_GameCoreBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "UI sounds entity", "et")]
	private ResourceName m_UISoundEntityPrefab;
	
	[Attribute("5")]
	private int m_iMaxRecentGames;
	
	private static SCR_UICore s_Instance;
	
	//------------------------------------------------------------------------------------------------
	//! Spawn entities for WidgetAnimator and SCR_UISoundsEntity
	override void OnGameStart()
	{
		if (!s_Instance)
			s_Instance = this;
		
		if (!AnimateWidgetEntity.GetInstance())
		{
			IEntity animateWidgetEntity = GetGame().SpawnEntity(AnimateWidgetEntity);
			if (animateWidgetEntity)
			{
				ChimeraWorld world = GetGame().GetWorld();
				if (world)
				{
					world.RegisterEntityToBeUpdatedWhileGameIsPaused(animateWidgetEntity);
				}
			}
		}

		if (!SCR_UISoundEntity.GetInstance())
			GetGame().SpawnEntityPrefab(Resource.Load(m_UISoundEntityPrefab));
		
		if (!SCR_TooltipManagerEntity.GetInstance())
			GetGame().SpawnEntity(SCR_TooltipManagerEntity);
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, "", "Show all settings", "UI");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		if (s_Instance == this)
			s_Instance = null;
	}
};
