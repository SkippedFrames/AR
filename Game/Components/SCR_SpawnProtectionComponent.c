[EntityEditorProps(category: "GameScripted/GameMode", description: "Spawn protection component for gamemode.", color: "0 0 255 255")]
class SCR_SpawnProtectionComponentClass : SCR_BaseGameModeComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_SpawnProtectionComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "0.1", desc: "How long should be player protected?")]
	protected float m_fProtectionTime;

	[Attribute(defvalue: "1", desc: "Should be protection disabled if player shoots?")]
	protected bool m_bDisableOnAttack;

	[Attribute(defvalue: "0", desc: "Allow player spawnpoints?")]
	protected bool m_bAllowPlayerSpawnpoints;

	//------------------------------------------------------------------------------------------------
	/*!
	Begin spawn protection immediately after entity is spawned.
	*/
	override bool PreparePlayerEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::PreparePlayerEntity_S(playerId: %2, entity: %3)", Type().ToString(),
					requestComponent.GetPlayerId(), entity);
		#endif

		SCR_PlayerController controller = SCR_PlayerController.Cast(requestComponent.GetPlayerController());

		// Check for radio respawns
		if (!m_bAllowPlayerSpawnpoints)
		{
			SCR_SpawnPointSpawnData spawnData = SCR_SpawnPointSpawnData.Cast(data);
			if (spawnData)
			{
				// If not allowed on player spawns, skip
				if (SCR_PlayerRadioSpawnPoint.Cast(spawnData.GetSpawnPoint()))
					return true;
			}
		}

		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (!damageManager)
			return true;

		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("  %1::EnableDamageHandling(false, playerId: %2, entity: %3)", Type().ToString(),
					requestComponent.GetPlayerId(), entity);
		#endif

		// Disable damage handling
		damageManager.EnableDamageHandling(false);

		// TODO: Check also for and grenade throw
		if (m_bDisableOnAttack)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(entity.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
				eventHandlerManager.RegisterScriptHandler("OnAmmoCountChanged", entity, OnPlayerAmmoChangeCallback);

			SCR_MeleeComponent meleeComp = SCR_MeleeComponent.Cast(entity.FindComponent(SCR_MeleeComponent));
			if (meleeComp)
				meleeComp.GetOnMeleePerformed().Insert(DisablePlayerProtection);
		}

		//Using protection time parameter as delay for CallLater, DisablePlayerProtection is called to disable protection on specific entity.
		GetGame().GetCallqueue().CallLater(DisablePlayerProtection, m_fProtectionTime * 1000, false, entity);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSpawnPlayerEntityFailure_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnData data, SCR_ESpawnResult reason)
	{
		DisablePlayerProtection(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback to be used by eventHandler in PreparePlayerEntity_S.
	protected void OnPlayerAmmoChangeCallback(BaseWeaponComponent currentWeapon, BaseMuzzleComponent currentMuzzle, BaseMagazineComponent magazine, int ammoCount, bool isChambered)
	{
		if (!currentWeapon)
			return;
		
		//Get weapon entity to which is component assigned
		IEntity weaponEntity = currentWeapon.GetOwner();
		if (!weaponEntity)
			return;

		//Get parent of weapon, and check if it is character or not.
		IEntity parent = weaponEntity.GetParent();
		if (parent && parent.IsInherited(ChimeraCharacter))
			DisablePlayerProtection(parent);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisablePlayerProtection(IEntity playerEntity)
	{
		if (!playerEntity)
			return;

		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(playerEntity.FindComponent(SCR_DamageManagerComponent));
		if (!damageManager && damageManager.IsDamageHandlingEnabled())
			return;

		damageManager.EnableDamageHandling(true);

		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("  %1::EnableDamageHandling(true, playerId: %2, entity: %3)", Type().ToString(),
					GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity), playerEntity);
		#endif

		if (m_bDisableOnAttack)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(playerEntity.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
				eventHandlerManager.RemoveScriptHandler("OnAmmoCountChanged", playerEntity, OnPlayerAmmoChangeCallback);

			SCR_MeleeComponent meleeComp = SCR_MeleeComponent.Cast(playerEntity.FindComponent(SCR_MeleeComponent));
			if (meleeComp)
				meleeComp.GetOnMeleePerformed().Remove(DisablePlayerProtection);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
	}
}
