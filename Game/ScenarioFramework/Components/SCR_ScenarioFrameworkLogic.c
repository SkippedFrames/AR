//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkLogicInput
{
	[Attribute(defvalue: "1", desc: "Input connector", UIWidgets.Auto, category: "Input")];
	protected ref SCR_ScenarioFrameworkActionInputBase m_InputAction;
	
	[Attribute(defvalue: "0", desc: "Input connector", UIWidgets.Auto, category: "Input")];
	protected bool 			m_bLatch;
	
	protected bool			m_bSignal;
	protected IEntity		m_Entity;
	protected SCR_ScenarioFrameworkLogic		m_MasterLogic;
	
		
	//------------------------------------------------------------------------------------------------
	void Init(SCR_ScenarioFrameworkLogic logic)
	{
		m_MasterLogic = logic;
		if (m_InputAction)
			m_InputAction.Init(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivate(bool bSignal, IEntity entity)
	{
		if (m_MasterLogic)
			m_MasterLogic.OnInput(bSignal, entity);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogicClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogic: GenericEntity
{
	
	[Attribute(defvalue: "1", desc: "What causes the increase", UIWidgets.Auto, category: "Input")];
	protected ref array<ref SCR_ScenarioFrameworkLogicInput> m_aInputs;
		
	[Attribute(defvalue: "1", desc: "What to do once counter is reached", UIWidgets.Auto, category: "OnActivate")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;
	
	protected bool												m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		if (m_bIsTerminated)
			return;
		
		foreach (SCR_ScenarioFrameworkLogicInput input : m_aInputs)
			input.Init(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInput(bool pSignal = true, IEntity entity = null);
	
	//------------------------------------------------------------------------------------------------
	void OnActivate(IEntity entity)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
			action.OnActivate(entity);
	}
	
#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		api.RenameEntity(this, api.GenerateDefaultEntityName(api.EntityToSource(this)));
	}
#endif
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogicCounterClass : SCR_ScenarioFrameworkLogicClass
{
	// prefab properties here
};

class SCR_ScenarioFrameworkLogicCounter : SCR_ScenarioFrameworkLogic
{	
	[Attribute(defvalue: "1", desc: "Threshold", UIWidgets.Graph, category: "Counter")];
	protected int							m_iCountTo;
	
	[Attribute(defvalue: "1", desc: "What to do once value is increased", UIWidgets.Auto, category: "OnIncrease")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aOnIncreaseActions;
			
	int 							m_iCnt = 0;
	
	//------------------------------------------------------------------------------------------------
	override void OnInput(bool pSignal = true, IEntity entity = null)
	{
		Increase(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCounterValue()
	{
		return m_iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCounterValue(int value)
	{
		m_iCnt = value;
	}
	
	//------------------------------------------------------------------------------------------------
	void Increase(IEntity entity)
	{
		m_iCnt++;
		if (m_iCnt == m_iCountTo)
		{
			OnActivate(entity);
		}
		else
		{
			foreach (SCR_ScenarioFrameworkActionBase increaseAction : m_aOnIncreaseActions)
				increaseAction.OnActivate(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_iCnt = 0;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogicORClass : SCR_ScenarioFrameworkLogicClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogicOR : SCR_ScenarioFrameworkLogic
{
	protected int			m_iActivations = 0;
	
	//------------------------------------------------------------------------------------------------
	override void OnInput(bool pSignal = true, IEntity entity = null)
	{
		if (pSignal)
			m_iActivations = Math.Min(++m_iActivations, m_aInputs.Count());
		else
			m_iActivations--;
	}	
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogicSwitchClass : SCR_ScenarioFrameworkLogicClass
{
	// prefab properties here
};

class SCR_ScenarioFrameworkLogicSwitch : SCR_ScenarioFrameworkLogic
{
		
};
