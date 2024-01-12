class SCR_TestScriptedRadioMsgUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		GenericEntity owner = GenericEntity.Cast(pOwnerEntity);
		BaseRadioComponent radioComp = BaseRadioComponent.Cast(owner.FindComponent(BaseRadioComponent));
		if (!radioComp)
			return;

		BaseTransceiver transmitter = radioComp.GetTransceiver(0);
		if (!transmitter)
			return;
		
		if (radioComp)
		{
			//ScriptedRadioMessage msg();
			SCR_RequestTransportMessage msg();
			transmitter.BeginTransmission(msg);
		}
	}
};
