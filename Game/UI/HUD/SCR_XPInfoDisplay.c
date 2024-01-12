//------------------------------------------------------------------------------------------------
class SCR_XPInfoDisplay : SCR_InfoDisplayExtended
{
	protected static const int XP_INFO_DURATION = 10000;	//ms
	protected static const ResourceName RANK_ICON_IMAGESET = "{5D7F0C1AB551F610}UI/Textures/MilitaryIcons/MilitaryIcons.imageset";
	
	protected bool m_bNegativeXP;
	protected bool m_bInitDone;
	protected bool m_bIsInfoAllowed = true;
	
	protected SCR_XPHandlerComponent m_PlayerXPComponent;
	
	protected TextWidget m_wTitle;
	protected TextWidget m_wRank;
	protected TextWidget m_wRankNoIcon;
	protected TextWidget m_wSkill;
	protected ImageWidget m_wRankIcon;
	protected ProgressBarWidget m_wProgress;
	protected ProgressBarWidget m_wProgressDiff;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (!gameMode)
			return;
		
		m_PlayerXPComponent = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		return (m_PlayerXPComponent != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (m_bInitDone)
			return;
		
		m_bInitDone = true;
		
		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(owner.FindComponent(SCR_PlayerXPHandlerComponent));
		
		if (comp)
			comp.GetOnXPChanged().Insert(ShowXPInfo);
		
		SCR_UITaskManagerComponent.s_OnTaskListVisible.Insert(ToggleXPInfo);
		
		m_wTitle = TextWidget.Cast(m_wRoot.FindWidget("Title"));
		m_wRank = TextWidget.Cast(m_wRoot.FindWidget("Rank"));
		m_wRankNoIcon = TextWidget.Cast(m_wRoot.FindWidget("RankNoIcon"));
		m_wRankIcon = ImageWidget.Cast(m_wRoot.FindWidget("RankIcon"));
		m_wSkill = TextWidget.Cast(m_wRoot.FindWidget("Skill"));
		m_wProgress = ProgressBarWidget.Cast(m_wRoot.FindWidget("Progress"));
		m_wProgressDiff = ProgressBarWidget.Cast(m_wRoot.FindWidget("ProgressDiff"));
		
		m_wProgress.SetColor(UIColors.CONTRAST_COLOR);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RecolorXPBar()
	{
		AnimateWidget.Color(m_wProgressDiff, UIColors.CONTRAST_COLOR, UIConstants.FADE_RATE_SLOW);
	}
	
	//------------------------------------------------------------------------------------------------
	void AllowShowingInfo(bool allow)
	{
		m_bIsInfoAllowed = allow;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HideHUD()
	{
		Show(false, UIConstants.FADE_RATE_SLOW)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowXPInfo(int totalXP, SCR_EXPRewards rewardID, int XP, bool volunteer, bool profileUsed, int skillLevel)
	{
		if (!m_bIsInfoAllowed)
			return;
		
		bool toggled = (XP == 0);
		bool nonSpecific = (rewardID == SCR_EXPRewards.UNDEFINED);
		bool notify = !nonSpecific && m_PlayerXPComponent.AllowNotification(rewardID);
		
		// XP has been added without specified source, don't show UI (i.e. XP sync upon reconnect)
		if (nonSpecific && !toggled)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			
		if (!faction)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		if (!factionManager)
			return;
		
		SCR_ECharacterRank curRank = factionManager.GetRankByXP(totalXP);
		SCR_ECharacterRank prevRank = factionManager.GetRankByXP(totalXP - XP);
		string rankText = faction.GetRankName(curRank);
		string rankIconName = faction.GetRankInsignia(curRank);
		
		if (rankIconName.IsEmpty())
		{
			m_wRankNoIcon.SetTextFormat(rankText);
			m_wRankIcon.SetVisible(false);
			m_wRank.SetTextFormat(string.Empty);
		}
		else
		{
			m_wRankNoIcon.SetText(string.Empty);
			m_wRankIcon.LoadImageFromSet(0, RANK_ICON_IMAGESET, rankIconName);
			m_wRankIcon.SetColor(UIColors.CONTRAST_COLOR);
			m_wRankIcon.SetVisible(true);
			m_wRank.SetTextFormat(rankText);
		}
		
		int showXP = XP;
		m_bNegativeXP = false;
		
		if (toggled || !notify)
		{
			// We are just showing info on current XP, no change was made - hide reward name
			m_wTitle.SetText(string.Empty);
		}
		else
		{
			if (volunteer)
				m_wTitle.SetTextFormat("#AR-Campaign_RewardBonus_Volunteer", m_PlayerXPComponent.GetXPRewardName(rewardID));
			else
				m_wTitle.SetTextFormat(m_PlayerXPComponent.GetXPRewardName(rewardID));
		
			// Show green portion of the progress bar when XP gets added, red portion if substracted
			if (XP > 0)
			{
				m_wTitle.SetColor(UIColors.CONFIRM);
				m_wProgressDiff.SetColor(UIColors.CONFIRM);
			}
			else
			{
				m_wTitle.SetColor(UIColors.WARNING);
				m_wProgressDiff.SetColor(UIColors.WARNING);
				m_bNegativeXP = true;
			}
		}
		
		UpdateXPProgressBar(factionManager, curRank, prevRank, XP, totalXP, notify);
		
		// Show skill info
		// Disabled for now, funcitonality is being transfered to SCR_PlayerData
		/*if (XP > 0 && profileUsed)
		{
			LocalizedString skillName;
			
			switch (m_PlayerXPComponent.GetXPRewardSkill(rewardID))
			{
				case EProfileSkillID.WEAPON_HANDLER: {skillName = "#AR-Campaign_SkillWeaponSpecialist"; break;};
				case EProfileSkillID.DRIVER: {skillName = "#AR-Campaign_SkillDriver"; break;};
				case EProfileSkillID.SCOUT: {skillName = "#AR-Campaign_SkillScout"; break;};
				case EProfileSkillID.OPERATOR: {skillName = "#AR-Campaign_SkillOperator"; break;};
			}
			
			if (!skillName.IsEmpty())
				m_wSkill.SetTextFormat("#AR-Campaign_LevelInfo", skillName, skillLevel);
		}*/
		
		// We updated the UI even if we don't want to show it at this point
		// This way it's up to date when it gets toggled manually later
		if (!notify && !toggled)
			return;
		
		// Show the UI
		Show(true);
		
		if (!toggled)
		{
			// Hide the UI after a while
			GetGame().GetCallqueue().Remove(HideHUD);
			GetGame().GetCallqueue().Remove(RecolorXPBar);
			GetGame().GetCallqueue().CallLater(HideHUD, XP_INFO_DURATION);
			GetGame().GetCallqueue().CallLater(RecolorXPBar, XP_INFO_DURATION * 0.7);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateXPProgressBar(notnull SCR_FactionManager factionManager, SCR_ECharacterRank curRank, SCR_ECharacterRank prevRank, int XP, int totalXP, bool notify)
	{
		if (factionManager.GetRankNext(curRank) == SCR_ECharacterRank.INVALID)
		{
			// Player at max level, no gain to show
			m_wProgress.SetMin(0);
			m_wProgress.SetMax(1);
			m_wProgress.SetCurrent(1);
			m_wProgressDiff.SetMin(0);
			m_wProgressDiff.SetMax(1);
			m_wProgressDiff.SetCurrent(0);
		}
		else
		{
			if (factionManager.GetRankPrev(curRank) == SCR_ECharacterRank.INVALID && XP < 0)
			{
				// Player is renegade and losing XP, just show red bar
				m_wProgress.SetMin(0);
				m_wProgress.SetMax(1);
				m_wProgress.SetCurrent(0);
				m_wProgressDiff.SetMin(0);
				m_wProgressDiff.SetMax(1);
				m_wProgressDiff.SetCurrent(1);
			}
			else
			{
				int XPCurRank = factionManager.GetRequiredRankXP(curRank);
				int XPNextRank = factionManager.GetRequiredRankXP(factionManager.GetRankNext(curRank));
				
				if (curRank == prevRank)
				{
					if (factionManager.GetRankPrev(curRank) != SCR_ECharacterRank.INVALID)
					{
						// Standard XP change
						m_wProgress.SetMin(XPCurRank);
						m_wProgress.SetMax(XPNextRank);
						m_wProgressDiff.SetMin(XPCurRank);
						m_wProgressDiff.SetMax(XPNextRank);
					}
					else
					{
						// XP change as renegade, show progress towards a normal rank from current XP
						m_wProgress.SetMin(totalXP - XP);
						m_wProgress.SetMax(totalXP - XP + 1);
						m_wProgressDiff.SetMin(totalXP - XP);
						m_wProgressDiff.SetMax(XPNextRank);
					}
					
					// Progress bar setup
					if (notify)
					{
						if (XP > 0)
						{
							m_wProgress.SetCurrent(totalXP - XP);
							m_wProgressDiff.SetCurrent(totalXP);
						}
						else
						{
							m_wProgress.SetCurrent(totalXP);
							m_wProgressDiff.SetCurrent(totalXP - XP);
						}
					}
					else
					{
						m_wProgress.SetCurrent(totalXP);
					}
				}
				else
				{
					if (curRank > prevRank)
					{
						// Promotion
						m_wProgress.SetMin(0);
						m_wProgress.SetMax(1);
						m_wProgress.SetCurrent(0);
						m_wProgressDiff.SetMin(XPCurRank);
						m_wProgressDiff.SetMax(XPNextRank);
						m_wProgressDiff.SetCurrent(totalXP);
					}
					else
					{
						// Demotion
						m_wProgress.SetMin(XPCurRank);
						m_wProgress.SetMax(XPNextRank);
						m_wProgress.SetCurrent(totalXP);
						m_wProgressDiff.SetMin(0);
						m_wProgressDiff.SetMax(1);
						m_wProgressDiff.SetCurrent(1);
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleXPInfo(bool visible)
	{
		// Make sure UI is not hidden automatically if shown a moment before
		GetGame().GetCallqueue().Remove(HideHUD);
		
		if (visible)
		{
			SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		
			if (!comp)
				return;
			
			int totalXP = comp.GetPlayerXP();
			
			ShowXPInfo(totalXP, SCR_EXPRewards.UNDEFINED, 0, false, false, 0);
		}
		else
		{
			Show(false, UIConstants.FADE_RATE_SUPER_FAST);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_XPInfoDisplay()
	{
		SCR_UITaskManagerComponent.s_OnTaskListVisible.Remove(ToggleXPInfo);
	}
};
