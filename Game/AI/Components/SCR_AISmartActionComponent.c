class SCR_AISmartActionComponentClass: AISmartActionComponentClass
{
};

void SCR_AIActionUserInvoker_Callback(AIAgent actionUser);

typedef func SCR_AIActionUserInvoker_Callback;
typedef ScriptInvokerBase<SCR_AIActionUserInvoker_Callback> SCR_AIActionUserInvoker;

class SCR_AISmartActionComponent : AISmartActionComponent
{
	ref SCR_AIActionUserInvoker Event_EOnActionEnd;	
	
	ref SCR_AIActionUserInvoker Event_EOnActionFailed;
	
	SCR_CharacterDamageManagerComponent m_pDamageManager;
	
	//------------------------------------------------------------------------------------------------
	void ReserveAction(AIAgent owner)
	{
		// listen on death of the owner
		IEntity ent = owner.GetControlledEntity();
		if (!ent)
			return;
		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(ent.FindComponent(SCR_CharacterDamageManagerComponent));
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
		SetActionAccessible(false);	
	}	
	
	//------------------------------------------------------------------------------------------------
	void ReleaseAction()
	{
		if(m_pDamageManager)
		{
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
			m_pDamageManager = null;
		}	
		SetActionAccessible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDamageStateChanged(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
			ReleaseAction();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionEnd(IEntity owner)
	{
		super.OnActionEnd(owner);
		ReleaseAction();
		if(Event_EOnActionEnd)
			Event_EOnActionEnd.Invoke(GetUser());
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed(IEntity owner)
	{
		super.OnActionFailed(owner);
		ReleaseAction();
		if(Event_EOnActionEnd)
			Event_EOnActionFailed.Invoke(GetUser());
	}
	
	//------------------------------------------------------------------------------------------------
	// Invokers for the completion of the smartaction to be used by the children // 
	SCR_AIActionUserInvoker GetOnActionEnd(bool createNew = true)
	{
		if (!Event_EOnActionEnd && createNew)
			Event_EOnActionEnd = new ref SCR_AIActionUserInvoker();
		return Event_EOnActionEnd;
	}	
	
	//------------------------------------------------------------------------------------------------
	SCR_AIActionUserInvoker GetOnActionFailed(bool createNew = true)
	{
		if (!Event_EOnActionFailed && createNew)
			Event_EOnActionFailed = new ref SCR_AIActionUserInvoker();
		return Event_EOnActionFailed;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
	}
};

