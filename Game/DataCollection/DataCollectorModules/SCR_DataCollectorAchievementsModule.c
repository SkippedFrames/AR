[BaseContainerProps()]
class SCR_DataCollectorAchievementsModule : SCR_DataCollectorModule
{
	
	//------------------------------------------------------------------------------------------------
	protected override void InitModule()
	{
		// Achievement COMBAT_HYGIENE
		SCR_FlushToilet.GetOnToiletFlushed().Insert(ToiletFlushed);
		
		// Achievement NUTCRACKER
		RplComponent rplComp = RplComponent.Cast(GetGame().GetGameMode().FindComponent(RplComponent));
		if (rplComp.IsMaster())
		{
			// Invoke this only as authority
			SCR_VehicleDamageManagerComponent.GetOnVehicleDestroyed().Insert(VehicleDestroyed);
		}
		
		// Achievement MAJOR_PROMOTION
		SCR_CharacterRankComponent.s_OnRankChanged.Insert(RankedUp);
		
		// Achievement MINED_OUT
		SCR_MineInventoryItemComponent.GetOnMinePlaced().Insert(MinePlaced);
		
		// Achievement IVORY_TICKLER || PIPING_UP
		SCR_PlayInstrument.GetOnInstrumentPlayed().Insert(InstrumentPlayed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		// Achievement HELPING_HAND || BATTLEFIELD_ANGEL
		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;
		
		characterController.m_OnItemUseEndedInvoker.Insert(OnItemUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Remove(OnItemUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	//Removing the invokers that do not belong to an entity
	protected void ~SCR_DataCollectorAchievementsModule()
	{
		SCR_FlushToilet.GetOnToiletFlushed().Remove(ToiletFlushed);
		SCR_VehicleDamageManagerComponent.GetOnVehicleDestroyed().Remove(VehicleDestroyed);
		SCR_CharacterRankComponent.s_OnRankChanged.Remove(RankedUp);	
		SCR_MineInventoryItemComponent.GetOnMinePlaced().Remove(MinePlaced);
		SCR_PlayInstrument.GetOnInstrumentPlayed().Remove(InstrumentPlayed);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		// Only players are counted towards stats & achievements
		if (killer.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
		{
			return;
		}
		
		const int killerId = killer.GetInstigatorPlayerID();
		
		// Self harm doesn't count
		if (playerId == killerId)
		{
			return;
		}
		
		// Handle achievement statistic PLAYER_KILLED (achievement TRUE_GRIT)
		// Note: PLAYER_KILLED is counted even for friendlies - oopsies makes fun
		IncrementAchievementProgress(playerId, AchievementStatId.PLAYER_KILLED);
		
		// Handle achievement statistic ENEMIES_NEUTRALIZED (achievements ONE_TO_WATCH, ARMY_OF_ONE)
		
		// Note: faction less games couldn't count towards achievement, this will need fixing
		// in case where player could be "kicked" out of the faction - e.g. becoming rogue
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
		{
			return;
		}

		Faction victimFaction = factionManager.GetPlayerFaction(playerId);
		if (!victimFaction)
		{
			return;
		}

		Faction killerFaction = factionManager.GetPlayerFaction(killerId);
		if (!killerFaction)
		{
			return;
		}

		// Teamkill? - doesn't count
		if (victimFaction.IsFactionFriendly(killerFaction))
		{
			return;
		}
		
		// Increment stat for the killer
		IncrementAchievementProgress(killerId, AchievementStatId.ENEMIES_NEUTRALIZED);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnd()
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		int playerCount = players.Count();
		
		SCR_GameModeCombatOpsManager combatOps = SCR_GameModeCombatOpsManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeCombatOpsManager));
		SCR_GameModeCampaign conflict = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		
		if (!combatOps && !conflict)
			return;
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		array<SCR_BaseTask> activeTasks = {};
		array<SCR_BaseTask> finishedTasks = {};
		taskManager.GetTasks(activeTasks);
		taskManager.GetFinishedTasks(finishedTasks);
		
		if (combatOps)
		{
			/* Achievement CLEAN_SWEEP || PAPER_PUSHER */
			
			if (finishedTasks.Count() >= activeTasks.Count())
			{	
				for(int i = 0; i < playerCount; i++)
				{
					CleanSweepCombatOps(players[i]);
					SecureIntelCombatOps(players[i]);
				}
				return;
			}
			/* Achievement 008 - meaningless number I cannot deduce */
			else
			{
				//unnecesary if finishedTasks are not nulls
				bool foundIntel = false;
				foreach (SCR_BaseTask task : finishedTasks)
				{
					//Currently, all tasks on finishedTasks are null
					//@todo Task system refactor will fix this
					if (!task)
						continue;
					
					if (task.GetTitle().Contains("Intel"))
					{			
						foundIntel = true;		
						for(int i = 0; i < playerCount; i++)
						{
							SecureIntelCombatOps(players[i]);
						}
						break;
					}
				}
				
				/* Doing this in case finishedTasks tasks are null, which should not happen. @todo fix */
				if (!foundIntel)
				{
					foreach (SCR_BaseTask task : activeTasks)
					{
						if (!task)
							continue;
						
						if (task.GetTaskState() == SCR_TaskState.FINISHED && task.GetTitle().Contains("Intel"))
						{
							foundIntel = true;		
							for(int i = 0; i < playerCount; i++)
							{
								SecureIntelCombatOps(players[i]);
							}
							break;
						}
					}
				}
				/* Achievement 009 - meaningless number */
			}
		}
		else if (conflict && conflict.IsTutorial())
		{
			/* Achievement SWEAT_SAVES_BLOOD */
			if (finishedTasks.Count() >= activeTasks.Count())
			{	
				for(int i = 0; i < playerCount; i++)
				{
					CleanSweepTutorial(players[i]);
				}
				return;
			}
			/* Achievement SWEAT_SAVES_BLOOD */
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemUsed(IEntity item, bool actionCompleted, SCR_ConsumableEffectAnimationParameters animParams)
	{
		// This is invoked for multiple items being used, including mines, but mines are handled separately
		if (!item || !actionCompleted)
		{
			return;
		}

		// We are interested only in consumables
		SCR_ConsumableItemComponent consumableComp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableComp)
		{
			return;
		}

		// We are interested only in medical consumables
		SCR_EConsumableType consumType = consumableComp.GetConsumableType();

		// All consumables are medical at this time, but I try to make check, that would skip any newly added consumables
		// and should produce error, if the existing medical consumable is removed/changed in SCR_EConsumableType
		// TODO: Turn into something more serious when general "medical" type is available
		if (consumType != SCR_EConsumableType.BANDAGE &&
			consumType != SCR_EConsumableType.HEALTH &&
			consumType != SCR_EConsumableType.TOURNIQUET &&
			consumType != SCR_EConsumableType.SALINE &&
			consumType != SCR_EConsumableType.MORPHINE &&
			consumType != SCR_EConsumableType.MED_KIT)
		{
			return;
		}

		IEntity userCharacter = consumableComp.GetCharacterOwner();
		if (!userCharacter)
		{
			return;
		}

		IEntity targetCharacter = consumableComp.GetTargetCharacter();
		if (!targetCharacter)
		{
			return;
		}

		// Self usage doesn't count
		if (userCharacter == targetCharacter)
		{
			return;
		}

		// Only player as a user is counted
		int userPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userCharacter);
		if (userPlayerId == 0)
		{
			return;
		}

		// Only player as a target is counted
		int targetPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(targetCharacter);
		if (targetPlayerId == 0)
		{
			return;
		}

		// Tracking for achievements HELPING_HAND, BATTLEFIELD_ANGEL
		IncrementAchievementProgress(userPlayerId, AchievementStatId.MEDICAL_ASSISTS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InstrumentPlayed(int playerId, SCR_EInstrumentType instrumentType)
	{
		// Achievement IVORY_TICKLER || PIPING_UP
		
		switch (instrumentType)
		{
			case SCR_EInstrumentType.PIANO: 
				PianoPlayed(playerId);
			break;
			case SCR_EInstrumentType.ORGAN:
				OrganPlayed(playerId);
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnlockAchievement(int playerId, AchievementId achievementId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;
		
		SCR_AchievementsHandler handler = SCR_AchievementsHandler.Cast(playerController.FindComponent(SCR_AchievementsHandler));
		if (!handler)
			return;
		
		handler.UnlockAchievement(achievementId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void IncrementAchievementProgress(int playerId, AchievementStatId achievementStatId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;
		
		SCR_AchievementsHandler handler = SCR_AchievementsHandler.Cast(playerController.FindComponent(SCR_AchievementsHandler));
		if (!handler)
			return;
		
		handler.IncrementAchievementProgress(achievementStatId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ToiletFlushed(int playerId)
	{		
		// Achievement COMBAT_HYGIENE
		Print("Player with id " + playerId + " flushed a toilet!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.COMBAT_HYGIENE);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VehicleDestroyed(int playerId)
	{
		// Achievement NUTCRACKER
		IncrementAchievementProgress(playerId, AchievementStatId.VEHICLES_DESTROYED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RankedUp(SCR_ECharacterRank prevRank, SCR_ECharacterRank newRank, IEntity playerEntity, bool silent)
	{
		// Achievement MAJOR_PROMOTION
		if (newRank != SCR_ECharacterRank.MAJOR || prevRank > newRank)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		if (playerId <= 0)
			return;
		
		Print("Player with id " + playerId + " ranked up to major!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.MAJOR_PROMOTION);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SuppliesDelivered(int playerId, int numberOfSupplies)
	{
		// Achievement TRUCKER_JOE
		// Delivered 'numberOfSupplies' supplies to captured locations in a single Conflict play session.
		// Help me Mario!
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CleanSweepCombatOps(int playerId)
	{
		// Achievement CLEAN_SWEEP
		Print("Player with id " + playerId + " cleanswept combat ops!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.CLEAN_SWEEP);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SecureIntelCombatOps(int playerId)
	{
		// Achievement PAPER_PUSHER
		Print("Player with id " + playerId + " has secured the intel!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.PAPER_PUSHER);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CleanSweepTutorial(int playerId)
	{
		//Achievement SWEAT_SAVES_BLOOD
		Print("Player with id " + playerId + " has cleanswept the tutorial!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.SWEAT_SAVES_BLOOD);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MinePlaced(int playerId)
	{
		// Achievement MINED_OUT
		Print("Player with id " + playerId + " placed a mine!", LogLevel.DEBUG);
		IncrementAchievementProgress(playerId, AchievementStatId.MINES_PLACED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PianoPlayed(int playerId)
	{
		// Achievement IVORY_TICKLER
		Print("Player with id " + playerId + " played a Piano!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.IVORY_TICKLER);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OrganPlayed(int playerId)
	{
		//Achievement PIPING_UP
		Print("Player with id " + playerId + " played an Organ!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.PIPING_UP);
	}
};
