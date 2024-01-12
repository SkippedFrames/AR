class SCR_AdjustSignalAction: ScriptedSignalUserAction
{	
	//! Adjustment step of normalized value
	[Attribute( uiwidget: UIWidgets.Auto, defvalue: "0.1", desc: "Adjustment step [0-1]")]
	protected float m_fAdjustmentStep;
	
	//! Name of action to control the input
	[Attribute( uiwidget: UIWidgets.Auto, defvalue: "SelectAction", desc: "Input action for increase")]
	protected string m_sActionIncrease;
	
	//! Name of action to control the input
	[Attribute( uiwidget: UIWidgets.Auto, defvalue: "", desc: "Input action for decrease")]
	protected string m_sActionDecrease;
	
	//! Action start sound event name
	[Attribute( uiwidget: UIWidgets.Auto, defvalue: "", desc: "Action start sound event name")]
	protected string m_sActionStartSoundEvent;
	
	//! Action canceled sound event name
	[Attribute( uiwidget: UIWidgets.Auto, defvalue: "", desc: "Action canceled sound event name")]
	protected string m_sActionCanceledSoundEvent;
	
	//! Movement sound event name
	[Attribute( uiwidget: UIWidgets.Auto, defvalue: "", desc: "Movement sound event name")]
	protected string m_sMovementSoundEvent;
	
	//! Movement stop sound event name
	[Attribute( uiwidget: UIWidgets.Auto, defvalue: "", desc: "Movement stop sound event name")]
	protected string m_sMovementStopSoundEvent;
	
	//! Normalized current value
	protected float m_fTargetValue;
	
	//! Interacted with by main entity. Allows reading input actions.
	protected bool m_bIsAdjustedByPlayer;
	
	//! Sound component on owner entity
	protected SoundComponent m_SoundComponent;
	
	//! Last lerp value
	protected float m_fLerpLast;
	
	//! Movement sound AudioHandle
	protected AudioHandle m_MovementAudioHandle;
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		// Play sound
		if (!m_SoundComponent)
			m_SoundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		
		if (m_SoundComponent && m_sActionStartSoundEvent != string.Empty)
			m_SoundComponent.SoundEvent(m_sActionStartSoundEvent);
				
		m_bIsAdjustedByPlayer = SCR_PlayerController.GetLocalControlledEntity() == pUserEntity;
		
		if (!m_bIsAdjustedByPlayer)
			return;
		
		if (!m_sActionIncrease.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_sActionIncrease, EActionTrigger.VALUE, HandleActionIncrease);
		
		if (!m_sActionDecrease.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_sActionDecrease, EActionTrigger.VALUE, HandleActionDecrease);
		
		
		m_fTargetValue = Math.InverseLerp(GetMinimumValue(), GetMaximumValue(), GetCurrentValue());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Play sound
		if (m_SoundComponent && m_sActionCanceledSoundEvent != string.Empty)
			m_SoundComponent.SoundEvent(m_sActionCanceledSoundEvent);
		
		if (!m_bIsAdjustedByPlayer)
			return;
		
		m_bIsAdjustedByPlayer = false;
		
		if (!m_sActionIncrease.IsEmpty())
			GetGame().GetInputManager().RemoveActionListener(m_sActionIncrease, EActionTrigger.VALUE, HandleActionIncrease);
		
		if (!m_sActionDecrease.IsEmpty())
			GetGame().GetInputManager().RemoveActionListener(m_sActionDecrease, EActionTrigger.VALUE, HandleActionDecrease);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increment by adjustment step
	void HandleActionIncrease(float value)
	{
		if (m_fAdjustmentStep <= 0)
			return;
		
		if (value > 0.5)
			value = m_fAdjustmentStep;
		else if (value < -0.5)
			value = -m_fAdjustmentStep;
		else
			return;
		
		// Limit to normalized current value +/- adjustment limit
		float previousValue = m_fTargetValue;
		m_fTargetValue += value;
		
		// Round to adjustment step
		m_fTargetValue = Math.Round(m_fTargetValue / m_fAdjustmentStep) * m_fAdjustmentStep;
		
		// Limit to min/max value
		m_fTargetValue = Math.Clamp(m_fTargetValue, 0, 1);
		
		if (!float.AlmostEqual(m_fTargetValue, previousValue))
			SetSendActionDataFlag();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Decrement by adjustment step
	void HandleActionDecrease(float value)
	{
		HandleActionIncrease(-value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays movement and stop movement sound events
	void PlayMovementAndStopSound(float lerp)
	{
		if (!m_SoundComponent)
			return;
		
		if (m_fLerpLast == lerp)
			return;
				
		if (float.AlmostEqual(lerp, 1))
		{
			if (!float.AlmostEqual(m_fLerpLast, 1))
			{
				m_SoundComponent.Terminate(m_MovementAudioHandle);
				if (m_sMovementStopSoundEvent != string.Empty)
					m_SoundComponent.SoundEvent(m_sMovementStopSoundEvent);	
			}
		}
		else if (float.AlmostEqual(lerp, 0))
		{
			if (!float.AlmostEqual(m_fLerpLast, 0))
			{
				m_SoundComponent.Terminate(m_MovementAudioHandle);
				if (m_sMovementStopSoundEvent != string.Empty)
					m_SoundComponent.SoundEvent(m_sMovementStopSoundEvent);	
			}
		}
		else
		{
			if (m_SoundComponent.IsFinishedPlaying(m_MovementAudioHandle) && m_sMovementStopSoundEvent != string.Empty)
				m_MovementAudioHandle = m_SoundComponent.SoundEvent(m_sMovementSoundEvent);
		}
					
		m_fLerpLast = lerp;
	}
	
	//! Is the script broadcast to the server?
	override bool HasLocalEffectOnlyScript() { return false; };
	
	//! If HasLocalEffectOnly() is true this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript() { return true; };
	
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		float lerp = Math.Lerp(GetMinimumValue(), GetMaximumValue(), m_fTargetValue);
		writer.WriteFloat01(lerp);
		
		SetSignalValue(m_fTargetValue);
				
		PlayMovementAndStopSound(lerp);
				
		return true;
	};
	
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		if (m_bIsAdjustedByPlayer)
			return true;

		float lerp;
		reader.ReadFloat01(lerp);

		m_fTargetValue = Math.InverseLerp(GetMinimumValue(), GetMaximumValue(), lerp);
		SetSignalValue(m_fTargetValue);
		
		PlayMovementAndStopSound(lerp);
		
		return true;
	};
};
