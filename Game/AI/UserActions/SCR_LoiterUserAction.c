enum ELoiteringType
{
	NONE, 	  
	SIT,	  
	LEAN_LEFT,
	LEAN_RIGHT
}

class SCR_LoiterUserAction: SCR_UserActionWithOccupancy
{
	[Attribute( defvalue: "0", uiwidget: UIWidgets.EditBox, desc: "Provide index of related SmartActionSentinel" )]
	protected int m_iSmartActionId;
	
	protected vector m_vActionTransform[4];
	protected ELoiteringType m_eLoiteringType;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!GetGame().InPlayMode())
			return;
		array<Managed> aComponents = {};
		pOwnerEntity.FindComponents(SCR_AISmartActionSentinelComponent, aComponents);
		if (m_iSmartActionId > aComponents.Count() - 1)
			Print("Invalid SmartAction id!",LogLevel.ERROR);
		SCR_AISmartActionSentinelComponent smartAction = SCR_AISmartActionSentinelComponent.Cast(aComponents[m_iSmartActionId]);
		if (smartAction)
		{
			vector mat[4];
			pOwnerEntity.GetWorldTransform(mat); 
			vector desiredPos = mat[3] + smartAction.GetActionOffset().Multiply3(mat);		// position of the action
			vector desiredLookPos = mat[3] + smartAction.GetLookPosition().Multiply3(mat); 	// position to rotate to
			vector actionEntityUp = mat[1];										// Up vector of entity that owns the action
			
			Math3D.DirectionAndUpMatrix(desiredLookPos - desiredPos, actionEntityUp, m_vActionTransform); 	// now I have rotation of entity to look towards desiredLookPos
			m_vActionTransform[3] = desiredPos; 															// standing at desired desiredPos
			m_eLoiteringType = LeaningTypeToLoiteringType(smartAction.GetLeaningType());
		}
		else
			pOwnerEntity.GetWorldTransform(m_vActionTransform);
	}
	
	//------------------------------------------------------------------------------------------------
	override void StartAction(IEntity pUserEntity)
	{
		SCR_CharacterControllerComponent contr = SCR_CharacterControllerComponent.Cast(pUserEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!contr)
			return;
		contr.StartLoitering(m_eLoiteringType, false, true, true, m_vActionTransform);		
	}
	
	//------------------------------------------------------------------------------------------------
	override void StopAction(IEntity pUserEntity)
	{
		SCR_CharacterControllerComponent contr = SCR_CharacterControllerComponent.Cast(pUserEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!contr)
			return;
		contr.StopLoitering(false);	
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return !m_Occupant || m_Occupant == user;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = ("#AR-UserAction_Loiter");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	ELoiteringType LeaningTypeToLoiteringType (ELeaningType leaning)
	{
		switch (leaning)
		{
			case ELeaningType.NONE:
			{
				return ELoiteringType.SIT;
			};
			case ELeaningType.LEFT:
			{
				return ELoiteringType.LEAN_LEFT;
			}; 
			case ELeaningType.RIGHT:
			{
				return ELoiteringType.LEAN_RIGHT;
			};
		}
		return ELoiteringType.NONE;	
	}
}
