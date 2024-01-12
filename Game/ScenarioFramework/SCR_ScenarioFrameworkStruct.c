//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkStruct : SCR_JsonApiStruct
{
	protected int m_iHours = -1;
	protected int m_iMinutes = -1;
	protected int m_iSeconds = -1;
	protected bool m_bMatchOver;
	protected string m_sWeatherState;
	protected ref array<ref SCR_ScenarioFrameworkAreaStruct> m_aAreasStructs = {};
	
	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return false;
		
		m_bMatchOver = manager.GetIsMatchOver();
		
		m_aAreasStructs.Clear();
		
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		
		if (timeManager)
		{
			timeManager.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
			WeatherStateTransitionManager transitionManager = timeManager.GetTransitionManager();
			
			if (transitionManager)
				m_sWeatherState = transitionManager.GetCurrentState().GetStateName();
		}
		
		manager.StoreAreaStates(m_aAreasStructs);
		ClearEmptyAreaStructs(m_aAreasStructs);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		if (m_aAreasStructs.IsEmpty())
			return false;
		
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return false;
		
		// Game was saved after match was over, don't load
		if (m_bMatchOver)
			return false;
		
		SCR_TimeAndWeatherHandlerComponent timeHandler = SCR_TimeAndWeatherHandlerComponent.GetInstance();
		
		// Weather has to be changed after init
		if (timeHandler && m_iHours >= 0 && m_iMinutes >= 0)
		{
			GetGame().GetCallqueue().Remove(timeHandler.SetupDaytimeAndWeather);
			GetGame().GetCallqueue().CallLater(timeHandler.SetupDaytimeAndWeather, 500, false, m_iHours, m_iMinutes, m_iSeconds, m_sWeatherState, true);
		}
		
		manager.LoadAreaStates(m_aAreasStructs);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearEmptyAreaStructs(array<ref SCR_ScenarioFrameworkAreaStruct> areaStructsToClear)
	{
		array<ref SCR_ScenarioFrameworkAreaStruct> areasStructsCopy = {};
		foreach (SCR_ScenarioFrameworkAreaStruct areaStructToCopy : areaStructsToClear)
		{
			areasStructsCopy.Insert(areaStructToCopy);
		}
		
		foreach (SCR_ScenarioFrameworkAreaStruct areaStruct : areasStructsCopy)
		{
			if (areaStruct.GetStructVarCount() <= 1)
			{
				areaStructsToClear.RemoveItem(areaStruct);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkStruct()
	{
		RegV("m_iHours");
		RegV("m_iMinutes");
		RegV("m_iSeconds");
		RegV("m_bMatchOver");
		RegV("m_sWeatherState");
		RegV("m_aAreasStructs");
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkAreaStruct : SCR_ScenarioFrameworkLayerStruct
{
	protected bool m_bAreaSelected;
	protected string m_sItemDeliveryPointName;
	protected string m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	bool GetAreaSelected()
	{
		return m_bAreaSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDeliveryPointNameForItem()
	{
		return m_sItemDeliveryPointName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetLayerTaskname()
	{
		return m_sLayerTaskName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAreaSelected(bool selected)
	{
		m_bAreaSelected = selected;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeliveryPointNameForItem(string name)
	{
		m_sItemDeliveryPointName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayerTaskName(string layerTaskName)
	{
		m_sLayerTaskName = layerTaskName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkAreaStruct()
	{
		RegV("m_iRepeatedSpawnNumber");
		RegV("m_bEnableRepeatedSpawn");
		RegV("m_bAreaSelected");
		RegV("m_sName");	
		RegV("m_sLayerTaskName");
		RegV("m_aLayersStructs");
		RegV("m_aLogicStructs");
		RegV("m_bIsTerminated");
		RegV("m_sItemDeliveryPointName");
		RegV("m_aRandomlySpawnedChildren");	
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLayerStruct : SCR_JsonApiStruct
{
	protected string											m_sName;
	protected ResourceName 										m_sRandomlySpawnedObject;
	protected ref array<ref SCR_ScenarioFrameworkLayerStruct>	m_aLayersStructs = {};
	protected ref array<ref SCR_ScenarioFrameworkLogicStruct> 	m_aLogicStructs = {};
	protected ref array<SCR_ScenarioFrameworkLayerBase>			m_aChildren = {};
	protected ref array<SCR_ScenarioFrameworkLogic>				m_aLogic = {};
	protected ref array<ResourceName>							m_aAIPrefabsForRemoval = {};
	protected ref array<string>									m_aRandomlySpawnedChildren = {};
	protected int 												m_iLayerTaskState;
	protected bool 												m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	protected int 												m_iRepeatedSpawnNumber;
	protected bool 												m_bEnableRepeatedSpawn;
	protected int												m_iStructVarCount;
	
	//------------------------------------------------------------------------------------------------
	void StoreLayerState(SCR_ScenarioFrameworkLayerBase layer)
	{
		SCR_ScenarioFrameworkLayerStruct layerStruct = new SCR_ScenarioFrameworkLayerStruct();
		
		//Checks if layer is named. If it is not, we cannot use it for serialization
		if (layer.GetName().IsEmpty())
		{
			delete layerStruct;
			return;
		}
		
		layerStruct.IncreaseStructVarCount();
		layerStruct.SetName(layer.GetName());
		
		StoreTerminationStatus(layer, layerStruct);
		StoreRepeatedSpawn(layer, layerStruct);
		StoreLayerTask(layer, layerStruct);
		
		bool handledLayers;
		StoreChildren(layer, layerStruct, handledLayers);
		bool handledLogics;
		StoreLogic(layer, layerStruct, handledLogics);
		
		StoreSlotAndRandomObject(layer, layerStruct);
		
		CleanEmptyStoredLayers(layer, layerStruct, handledLayers);
		CleanEmptyStoredLogic(layer, layerStruct, handledLogics);
		
		//Final insertion
		if (layerStruct && layerStruct.GetStructVarCount() > 1)
			m_aLayersStructs.Insert(layerStruct);
		
		CleanEmptyStructs();
	}

	//------------------------------------------------------------------------------------------------
	//! Marks if this was terminated - either by death or deletion
	void StoreTerminationStatus(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layer.GetIsTerminated())
		{
			layerStruct.SetIsTerminated(true);
			layerStruct.IncreaseStructVarCount();
		}
		else
		{
			layerStruct.UnregV("m_bIsTerminated");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Repeated spawn handling
	void StoreRepeatedSpawn(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layer.GetEnableRepeatedSpawn())
		{
			layerStruct.SetEnableRepeatedSpawn(layer.GetEnableRepeatedSpawn());
			layerStruct.IncreaseStructVarCount();
			if (layer.GetRepeatedSpawnNumber() != -1)
			{
				layerStruct.IncreaseStructVarCount();
				layerStruct.SetRepeatedSpawnNumber(layer.GetRepeatedSpawnNumber());
			}
			else
			{
				layerStruct.UnregV("m_iRepeatedSpawnNumber");
			}
		}
		else
		{
			layerStruct.UnregV("m_bEnableRepeatedSpawn");
			layerStruct.UnregV("m_iRepeatedSpawnNumber");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Layer Task handling
	void StoreLayerTask(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		if (layerTask && layerTask.GetLayerTaskState() != 0)
		{
			layerStruct.SetLayerTaskState(layerTask.GetLayerTaskState());
			layerStruct.IncreaseStructVarCount();
		}
		else
		{
			layerStruct.UnregV("m_iLayerTaskState");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Children handling
	void StoreChildren(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct, bool handledLayers)
	{
		m_aChildren = layer.GetChildrenEntities();
		if (m_aChildren.IsEmpty())
		{
			layerStruct.UnregV("m_aLayersStructs");
		}
		else
		{
			if (layer.GetSpawnChildrenType() != SCR_EScenarioFrameworkSpawnChildrenType.ALL)
			{
				array <SCR_ScenarioFrameworkLayerBase> m_aRandomlySpawnedChildrenLayerBases = layer.GetRandomlySpawnedChildren();
				foreach (SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildrenLayerBases)
				{
					layerStruct.InsertRandomlySpawnedChildren(child.GetName());
				}
				layerStruct.IncreaseStructVarCount();
			}
			else
				layerStruct.UnregV("m_aRandomlySpawnedChildren");
			
			layerStruct.IncreaseStructVarCount();
			handledLayers = true;
			foreach (SCR_ScenarioFrameworkLayerBase layerToCycle : m_aChildren)
			{
				layerStruct.StoreLayerState(layerToCycle);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Logics handling
	void StoreLogic(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct, bool handledLogics)
	{
		m_aLogic = layer.GetSpawnedLogics();
		if (m_aLogic.IsEmpty())
		{
			layerStruct.UnregV("m_aLogicStructs");
		}
		else
		{
			layerStruct.IncreaseStructVarCount();
			handledLogics = true;
			foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
			{
				layerStruct.StoreLogicState(logic);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Logics handling
	void StoreSlotAndRandomObject(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		SCR_ScenarioFrameworkSlotBase slot = SCR_ScenarioFrameworkSlotBase.Cast(layer);
		if (slot)
		{
			SCR_ScenarioFrameworkSlotAI slotAI = SCR_ScenarioFrameworkSlotAI.Cast(slot);
			if (slotAI && !slotAI.GetAIPrefabsForRemoval().IsEmpty() && !slotAI.GetIsTerminated())
			{
				layerStruct.SetAIPrefabsForRemoval(slotAI.GetAIPrefabsForRemoval());
				layerStruct.IncreaseStructVarCount();
			}
			else
			{
				layerStruct.UnregV("m_aAIPrefabsForRemoval");
			}
			
			if (slot.GetRandomlySpawnedObject())
			{
				layerStruct.SetRandomlySpawnedObject(slot.GetRandomlySpawnedObject());
				layerStruct.IncreaseStructVarCount();
			}
			else
			{
				layerStruct.UnregV("m_sRandomlySpawnedObject");
			}
		}
		else
		{
			layerStruct.UnregV("m_aAIPrefabsForRemoval");
			layerStruct.UnregV("m_sRandomlySpawnedObject");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! //Cleaning empty layers
	void CleanEmptyStoredLayers(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct, bool handledLayers)
	{
		if (!layerStruct.GetLayerStructs() || layerStruct.GetLayerStructs().IsEmpty() || layerStruct.GetStructVarCount() <= 1)
			layerStruct.UnregV("m_aLayersStructs");

		if (layerStruct.GetLayerStructs())
		{
			if (layerStruct.GetLayerStructs().IsEmpty())
			{
				layerStruct.UnregV("m_aLayersStructs");
				if (handledLayers)
					layerStruct.DecreaseStructVarCount();
			}
		}
		else
		{
			if (handledLayers)
				layerStruct.DecreaseStructVarCount();
			
			layerStruct.UnregV("m_aLayersStructs");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! //Cleaning empty Logic
	void CleanEmptyStoredLogic(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct, bool handledLogics)
	{
		if (layerStruct.GetLogicStructs())
		{
			if (layerStruct.GetLogicStructs().IsEmpty())
			{
				layerStruct.UnregV("m_aLogicStructs");
				if (handledLogics)
					layerStruct.DecreaseStructVarCount();
			}
		}
		else
		{
			if (handledLogics)
				layerStruct.DecreaseStructVarCount();
			
			layerStruct.UnregV("m_aLogicStructs");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CleanEmptyStructs()
	{
		//Clears empty layer structs which are unnecessary to be saved
		ClearEmptyLayerStructs(m_aLayersStructs);
		
		//Clears empty logic structs which are unnecessary to be saved
		ClearEmptyLogicStructs(m_aLogicStructs);
	
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreLogicState(SCR_ScenarioFrameworkLogic logic)
	{
		SCR_ScenarioFrameworkLogicStruct logicStruct = new SCR_ScenarioFrameworkLogicStruct();
		
		SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(logic);
		if (logicCounter)
		{
			//Checks if layer is named. If it is not, we cannot use it for serialization
			if (logicCounter.GetName())
			{
				logicStruct.IncreaseStructVarCount();
				logicStruct.SetName(logicCounter.GetName());
			}
			else
			{
				DecreaseStructVarCount();
				delete logicStruct;
				return;
			}
			
			StoreLogicCounterValue(logicCounter, logicStruct);
			StoreLogicCounterTermination(logicCounter, logicStruct);
			
			if (logicStruct && logicStruct.GetStructVarCount() > 1)
			{
				m_aLogicStructs.Insert(logicStruct);
				return;	
			}
		}
		
		DecreaseStructVarCount();
		delete logicStruct;
	}

	//------------------------------------------------------------------------------------------------
	//! Handling logic counter value
	void StoreLogicCounterValue(SCR_ScenarioFrameworkLogicCounter logicCounter, SCR_ScenarioFrameworkLogicStruct logicStruct)
	{
		if (logicCounter.GetCounterValue() == 0)
		{
			logicStruct.UnregV("m_iCounterValue");
		}
		else
		{
			logicStruct.IncreaseStructVarCount();
			logicStruct.SetCounterValue(logicCounter.GetCounterValue());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Marks if this was terminated - either by death or deletion
	void StoreLogicCounterTermination(SCR_ScenarioFrameworkLogicCounter logicCounter, SCR_ScenarioFrameworkLogicStruct logicStruct)
	{
		if (logicCounter.GetIsTerminated())
		{
			logicStruct.IncreaseStructVarCount();
			logicStruct.SetIsTerminated(true);
		}
		else
		{
			logicStruct.UnregV("m_bIsTerminated");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearEmptyLayerStructs(array<ref SCR_ScenarioFrameworkLayerStruct> layerStructsToClear)
	{
		array<ref SCR_ScenarioFrameworkLayerStruct> layersStructsCopy = {};
		foreach (SCR_ScenarioFrameworkLayerStruct layerStructToCopy : layerStructsToClear)
		{
			layersStructsCopy.Insert(layerStructToCopy);
		}
		
		foreach (SCR_ScenarioFrameworkLayerStruct layerStruct : layersStructsCopy)
		{
			if (layerStruct.GetStructVarCount() <= 1)
				layerStructsToClear.RemoveItem(layerStruct);
			else
				ClearEmptyLayerStructs(layerStruct.GetLayerStructs());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearEmptyLogicStructs(array<ref SCR_ScenarioFrameworkLogicStruct> logicStructsToClear)
	{
		array<ref SCR_ScenarioFrameworkLogicStruct> logicsStructsCopy = {};
		foreach (SCR_ScenarioFrameworkLogicStruct logicStructToCopy : logicStructsToClear)
		{
			logicsStructsCopy.Insert(logicStructToCopy);
		}
		
		foreach (SCR_ScenarioFrameworkLogicStruct logicStruct : logicsStructsCopy)
		{
			if (logicStruct.GetStructVarCount() <= 1)
				logicStructsToClear.RemoveItem(logicStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void IncreaseStructVarCount()
	{
		m_iStructVarCount += 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	void DecreaseStructVarCount()
	{
		m_iStructVarCount -= 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkLayerStruct> GetLayerStructs()
	{
		return m_aLayersStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkLogicStruct> GetLogicStructs()
	{
		return m_aLogicStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	array<string> GetRandomlySpawnedChildren()
	{
		return m_aRandomlySpawnedChildren;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetStructVarCount()
	{
		return m_iStructVarCount;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	ref array<ResourceName> GetAIPrefabsForRemoval()
	{
		return m_aAIPrefabsForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetLayerTaskState()
	{
		return m_iLayerTaskState;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRandomlySpawnedObject()
	{
		return m_sRandomlySpawnedObject;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRepeatedSpawnNumber()
	{
		return m_iRepeatedSpawnNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}
	
	//--------------------- Setters:
	
	//------------------------------------------------------------------------------------------------
	void InsertRandomlySpawnedChildren(string child)
	{
		m_aRandomlySpawnedChildren.Insert(child);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name)
	{
		m_sName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayerTaskState(int state)
	{
		m_iLayerTaskState = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAIPrefabsForRemoval(array<ResourceName> arrayForRemoval)
	{
		m_aAIPrefabsForRemoval = arrayForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRandomlySpawnedObject(ResourceName name)
	{
		m_sRandomlySpawnedObject = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRepeatedSpawnNumber(int number)
	{
		m_iRepeatedSpawnNumber = number;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnableRepeatedSpawn(bool value)
	{
		m_bEnableRepeatedSpawn = value;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerStruct()
	{
		RegV("m_sName");
		RegV("m_sRandomlySpawnedObject");
		RegV("m_bIsTerminated");
		RegV("m_aLayersStructs");	
		RegV("m_aLogicStructs");
		RegV("m_iRepeatedSpawnNumber");
		RegV("m_aAIPrefabsForRemoval");
		RegV("m_bEnableRepeatedSpawn");	
		RegV("m_aRandomlySpawnedChildren");	
		RegV("m_iLayerTaskState");	
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogicStruct : SCR_JsonApiStruct
{
	protected string m_sName;
	protected int m_iCounterValue;
	protected bool m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	protected int m_iStructVarCount;
	
	//------------------------------------------------------------------------------------------------
	void IncreaseStructVarCount()
	{
		m_iStructVarCount += 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	int GetStructVarCount()
	{
		return m_iStructVarCount;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCounterValue()
	{
		return m_iCounterValue;
	}
	
	//--------------------- Setters:
	
	//------------------------------------------------------------------------------------------------
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name)
	{
		m_sName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCounterValue(int value)
	{
		m_iCounterValue = value;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLogicStruct()
	{
		RegV("m_sName");
		RegV("m_bIsTerminated");
		RegV("m_iCounterValue");	
	}
};
