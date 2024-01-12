//------------------------------------------------------------------------------------------------
class SCR_XPHandlerComponentClass : SCR_BaseGameModeComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_XPHandlerComponent : SCR_BaseGameModeComponent
{
	[Attribute("{E6FC4537B53EA00B}Configs/Campaign/XPRewards.conf", params: "conf class=SCR_XPRewardList")]
	private ResourceName m_sXPRewardsConfig;

	[Attribute("300", UIWidgets.EditBox, "How many XP a player needs to gain in a single life to get the Veterancy award. 0 = disabled.", params: "0 inf 1")]
	protected int m_iVeterancyXPAwardThreshold;

	[Attribute("1800", UIWidgets.EditBox, "If suicide is committed more than once in this time (seconds), a penalty is issued.", params: "0 inf 1")]
	protected int m_iSuicidePenaltyCooldown;

	//static const int SKILL_LEVEL_MAX = 10;
	//static const int SKILL_LEVEL_XP_COST = 1000;				// how much XP is needed for new level

	//static const float SKILL_LEVEL_XP_BONUS = 0.1;

	protected static const float TRANSPORT_POINTS_TO_XP_RATIO = 0.01;
	protected static const int TRANSPORT_XP_PAYOFF_THRESHOLD = 15;

	protected ref array<ref SCR_XPRewardInfo> m_aXPRewardList = {};

	protected ref map<int, float> m_mPlayerTransportPoints = new map<int, float>();

	protected float m_fXpMultiplier = 1;

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		if (!IsProxy())
		{
			SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));

			if (compartmentAccessComponent)
				compartmentAccessComponent.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
		}

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent compXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

		if (compXP)
			compXP.UpdatePlayerRank(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		m_mPlayerTransportPoints.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);

		if (IsProxy())
			return;

		if (playerId == killer.GetInstigatorPlayerID())
			ProcessSuicide(playerId);

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (pc)
		{
			SCR_PlayerXPHandlerComponent compXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

			if (compXP)
				compXP.OnPlayerKilled();
		}

		AwardTransportXP(playerId);
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(playerEntity.FindComponent(SCR_CompartmentAccessComponent));

		if (!compartmentAccessComponent)
			return;

		compartmentAccessComponent.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
	}

	//------------------------------------------------------------------------------------------------
	override void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnControllableDestroyed(entity, killerEntity, killer);

		// Handle XP for kill
		if (killer.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;

		int killerId = killer.GetInstigatorPlayerID();
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);

		SCR_ChimeraCharacter entityChar = SCR_ChimeraCharacter.Cast(entity);
		if (!entityChar)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		Faction factionKiller = Faction.Cast(factionManager.GetPlayerFaction(killerId));
		if (!factionKiller)
			return;

		if (factionKiller.IsFactionFriendly(entityChar.GetFaction()))
		{
			if (killerId != playerId)
				AwardXP(killerId, SCR_EXPRewards.FRIENDLY_KILL);
		}
		else
		{
			SCR_ChimeraCharacter instigatorChar = SCR_ChimeraCharacter.Cast(killerEntity);

			if (!instigatorChar || !instigatorChar.IsInVehicle())
				AwardXP(killerId, SCR_EXPRewards.ENEMY_KILL);
			else
				AwardXP(killerId, SCR_EXPRewards.ENEMY_KILL_VEH);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);

		if (!compartment)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(compartment.GetOccupant());

		if (playerId == 0)
			return;

		AwardTransportXP(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStatPointsAdded(int playerId, SCR_EDataStats stat, float amount, bool temp)
	{
		//Print(temp);
		//Print(SCR_Enum.GetEnumName(SCR_EDataStats, stat));

		if (!temp || stat != SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS)
			return;

		int newValue = m_mPlayerTransportPoints.Get(playerId) + amount;
		m_mPlayerTransportPoints.Set(playerId, newValue);

		//Print(newValue);

		if (newValue * TRANSPORT_POINTS_TO_XP_RATIO >= TRANSPORT_XP_PAYOFF_THRESHOLD)
			AwardTransportXP(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void ProcessSuicide(int playerId)
	{
#ifdef NO_SUICIDE_PENALTY
		return;
#endif
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent compXPPlayer = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

		if (!compXPPlayer)
			return;

		SCR_FactionManager fm = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (!fm)
			return;

		// Check for cooldown on the penalty
		float curTime = GetGame().GetWorld().GetWorldTime();
		float penaltyTimestamp = compXPPlayer.GetSuicidePenaltyTimestamp();
		compXPPlayer.SetSuicidePenaltyTimestamp(curTime + ((float)m_iSuicidePenaltyCooldown * 1000));

		if (curTime > penaltyTimestamp)
			return;

		// Don't make player renegade just by suiciding
		int penalty = GetXPRewardAmount(SCR_EXPRewards.SUICIDE);
		int playerXPWithPenalty = compXPPlayer.GetPlayerXP() + penalty;
		SCR_ECharacterRank newRank = fm.GetRankByXP(playerXPWithPenalty);

		if (fm.IsRankRenegade(newRank))
			return;

		AwardXP(pc.GetPlayerId(), SCR_EXPRewards.SUICIDE);
	}

	//------------------------------------------------------------------------------------------------
	protected void AwardTransportXP(int playerId)
	{
		float toAward = m_mPlayerTransportPoints.Get(playerId) * TRANSPORT_POINTS_TO_XP_RATIO;

		if (toAward <= 0)
			return;

		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!controller)
			return;

		m_mPlayerTransportPoints.Set(playerId, 0);

		AwardXP(controller, SCR_EXPRewards.TASK_TRANSPORT, toAward);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));

		return rplComponent && rplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	//! Add XP to given playerId
	void AwardXP(int playerId, SCR_EXPRewards rewardID, float multiplier = 1.0, bool volunteer = false, int customXP = 0)
	{
		if (IsProxy())
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (playerController)
			AwardXP(playerController, rewardID, multiplier, volunteer, customXP);
	}

	//------------------------------------------------------------------------------------------------
	//! Add XP to given controller
	void AwardXP(notnull PlayerController controller, SCR_EXPRewards rewardID, float multiplier = 1.0, bool volunteer = false, int customXP = 0)
	{
		if (IsProxy())
			return;

		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(controller.FindComponent(SCR_PlayerXPHandlerComponent));

		if (!comp)
			return;

		comp.AddPlayerXP(rewardID, multiplier, volunteer, customXP);

		// Do not handle veterancy award if it's being processed already or disabled
		if (rewardID == SCR_EXPRewards.VETERANCY || m_iVeterancyXPAwardThreshold == 0 || customXP != 0)
			return;

		int singleLifeXP = comp.GetPlayerXPSinceLastSpawn();
		float veterancyAwards = Math.Floor(singleLifeXP / m_iVeterancyXPAwardThreshold);

		if (veterancyAwards < 1)
			return;

		int leftoverXP = singleLifeXP % m_iVeterancyXPAwardThreshold;
		comp.SetPlayerXPSinceLastSpawn(leftoverXP);

		// Award veterancy bonus with a delay so the UI isn't overwritten immediately
		GetGame().GetCallqueue().CallLater(VeterancyAward, 2000, false, controller, veterancyAwards);
	}

	//------------------------------------------------------------------------------------------------
	void VeterancyAward(notnull PlayerController controller, float multiplier)
	{
		AwardXP(controller, SCR_EXPRewards.VETERANCY, multiplier);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward amount
	int GetXPRewardAmount(SCR_EXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();

		for (int i = 0; i < rewardsCnt; i++)
		{
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardXP();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward amount
	float GetXPMultiplier()
	{
		return m_fXpMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward name
	string GetXPRewardName(SCR_EXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();

		for (int i = 0; i < rewardsCnt; i++)
		{
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardName();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	bool AllowNotification(SCR_EXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();

		for (int i = 0; i < rewardsCnt; i++)
		{
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].AllowNotification();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward skill
	/*EProfileSkillID GetXPRewardSkill(SCR_EXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();

		for (int i = 0; i < rewardsCnt; i++)
		{
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardSkill();
		}

		return EProfileSkillID.GLOBAL;
	}*/

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		//Parse & register XP reward list
		Resource container = BaseContainerTools.LoadContainer(m_sXPRewardsConfig);
		SCR_XPRewardList list = SCR_XPRewardList.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
		list.GetRewardList(m_aXPRewardList);

		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());

		if (header)
			m_fXpMultiplier = header.m_fXpMultiplier;

		SCR_PlayerData.s_OnStatAdded.Insert(OnStatPointsAdded);
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_EXPRewards
{
	UNDEFINED,
	CHEAT,
	ENEMY_KILL,
	ENEMY_KILL_VEH,
	FRIENDLY_KILL,
	RELAY_DISCOVERED,
	RELAY_RECONFIGURED,
	BASE_SEIZED,
	BASE_DEFENDED,
	SUPPLIES_DELIVERED,
	SUPPORT_EVAC,
	SUPPORT_FUEL,
	TASK_DEFEND,
	TASK_TRANSPORT,
	SERVICE_BUILD,
	SUICIDE,
	VETERANCY,
	SPAWN_PROVIDER,
	FREE_ROAM_BUILDING_BUILT,
	CUSTOM_1,
	CUSTOM_2,
	CUSTOM_3,
	CUSTOM_4,
	CUSTOM_5,
	CUSTOM_6,
	CUSTOM_7,
	CUSTOM_8,
	CUSTOM_9,
	CUSTOM_10,
	CUSTOM_11,
	CUSTOM_12,
	CUSTOM_13,
	CUSTOM_14,
	CUSTOM_15,
	CUSTOM_16,
	CUSTOM_17,
	CUSTOM_18,
	CUSTOM_19,
	CUSTOM_20
}
