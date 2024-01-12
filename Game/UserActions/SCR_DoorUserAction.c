//! User action that ought to be attached to an entity with door component.
//! When performed either opens or closes the door based on the previous state of the door.
class SCR_DoorUserAction : DoorUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		DoorComponent doorComponent = GetDoorComponent();
		if (doorComponent)
		{
			vector doorOpeningVecWS = pOwnerEntity.VectorToParent(vector.Forward);
			if (doorComponent.GetAngleRange() < 0.0)
				doorOpeningVecWS = -1.0 * doorOpeningVecWS;
			
			vector userMat[4];
			pUserEntity.GetWorldTransform(userMat);
			float dotP = vector.Dot(doorOpeningVecWS, userMat[3] - doorComponent.GetDoorPivotPointWS());
			
			// Flip the control value if it is above a threshold
			float controlValue = 1.0;
			float currentState = doorComponent.GetDoorState();
			if ((dotP < 0.0 && currentState <= 0.0)  || (dotP > 0.0 && currentState < 0.0))
				controlValue = -1.0;
			if (Math.AbsFloat(doorComponent.GetControlValue()) > 0.5)
				controlValue = 0.0;
			
			//Print(controlValue);
			doorComponent.SetActionInstigator(pUserEntity);
			doorComponent.SetControlValue(controlValue);
		}
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		DoorComponent doorComponent = GetDoorComponent();
		if (doorComponent)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		auto doorComponent = GetDoorComponent();
		// Logic here is flipped since method returns the opposite of what we expect
		if (doorComponent && Math.AbsFloat(doorComponent.GetControlValue()) >= 0.5)
			outName = "#AR-UserAction_Close";
		else
			outName = "#AR-UserAction_Open";
		
		return true;
	}
	
};

class SCR_LadderDoorUserAction : SCR_DoorUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		DoorComponent door = GetDoorComponent();
		if (door)
		{
			LadderComponent ladder = LadderComponent.Cast(door.FindComponent(LadderComponent));
			if (ladder)
			{
				if (door.GetControlValue() < 0.5)
				{
					ladder.SetEnabledEntry(false);
				}
				else
				{
					ladder.SetEnabledEntry(true);
				}
			}
		}
	}
};
