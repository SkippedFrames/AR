//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_RecruitPlayerGroupCommand : SCR_BaseGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient)
		{
			if (playerID != SCR_PlayerController.GetLocalPlayerId())
				return false;
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
			if (!playerController)
				return false;
			
			SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
			if (!groupController)
				return false;
			
			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(cursorTarget);
			if (!character)
				return false;
			
			int targetPlayerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
			if (targetPlayerID <= 0)
				return false;
			
			groupController.InvitePlayer(targetPlayerID);
			
			return true;
		}
				
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		if (!CanBeShownInCurrentLifeState())
			return false;
		
		PlayerCamera camera = GetGame().GetPlayerController().GetPlayerCamera();
		if (!camera)
			return false;
		
		IEntity cursorTarget = camera.GetCursorTarget();
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(cursorTarget);
		if (!character)
			return false;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return false;
		
		SCR_RespawnSystemComponent respawnComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnComponent)
			return false;
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;
		
		if (!CanRoleShow())
			return false;
		
		if (character.GetFaction() != playerController.GetLocalControlledEntityFaction())
			return false;
		
		int targetPlayerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		if (targetPlayerID <= 0)
			return false;
		
		if (!groupController.CanInvitePlayer(targetPlayerID))
			return false;
		
		return true;
	}
}
