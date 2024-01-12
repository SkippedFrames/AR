#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_MineAwarenessComponentClass : ScriptComponentClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_MineDetectionData
{
	#ifndef AR_MINE_DETECTION_TIMESTAMP
	float m_fLastTime;
	#else
	WorldTimestamp m_fLastTime;
	#endif
	float m_fTimeElapsed;
}

//------------------------------------------------------------------------------------------------
class SCR_MineAwarenessComponent : ScriptComponent
{
	protected ref set<RplId> m_aDetectedMines;
	protected ref map<RplId, ref SCR_MineDetectionData> m_mMineDetection = new map<RplId, ref SCR_MineDetectionData>();
	
	//------------------------------------------------------------------------------------------------
	static SCR_MineAwarenessComponent GetLocalInstance()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;
		
		return SCR_MineAwarenessComponent.Cast(playerController.FindComponent(SCR_MineAwarenessComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void Detect(RplId id, bool instant = false)
	{
		if (instant)
		{
			m_aDetectedMines.Insert(id);
			return;
		}
		
		// Only called for mines which are not in m_aDetectedMines
		if (!m_mMineDetection.Contains(id))
			m_mMineDetection.Insert(id, new SCR_MineDetectionData);
		
		// Check if there is some start time
		#ifndef AR_MINE_DETECTION_TIMESTAMP
		float currentTime = Replication.Time();
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		#endif
		SCR_MineDetectionData data = m_mMineDetection.Get(id);
		if (data.m_fLastTime == 0)
			data.m_fLastTime = currentTime;
		
		#ifndef AR_MINE_DETECTION_TIMESTAMP
		if (currentTime - data.m_fLastTime < 1000)
			data.m_fTimeElapsed += currentTime - data.m_fLastTime;
		#else
		float sinceLastTime = currentTime.DiffMilliseconds(data.m_fLastTime);
		if (sinceLastTime < 1000)
			data.m_fTimeElapsed += sinceLastTime;
		#endif
		
		data.m_fLastTime = currentTime;
		
		if (data.m_fTimeElapsed > 2000)
		{
			m_aDetectedMines.Insert(id);
			m_mMineDetection.Remove(id);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDetected(IEntity entity)
	{
		RplComponent rplComponent = RplComponent.Cast(entity.FindComponent(RplComponent));
		if (!rplComponent)
			return true; // visible by default when entity doesn't have any rpl component
		
		RplId id = rplComponent.Id();
		bool isDetected = m_aDetectedMines.Contains(id);
		
		if (!isDetected)
			Detect(id);
		
		return isDetected;
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void EOnInit(IEntity owner)
	{
		m_aDetectedMines = new set<RplId>();
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnPostInit(IEntity owner)
	{
		SetEventMask(GetOwner(), EntityEvent.INIT);
	}
};
