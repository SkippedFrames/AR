//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_SoundManagerEntityCore: SCR_GameCoreBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	private ResourceName m_SoundManagerPrefab;
			
	//------------------------------------------------------------------------------------------------
	//! Spawn SoundManager entity
	override void OnGameStart()
	{		
		if (!System.IsConsoleApp())
			GetGame().SpawnEntityPrefab(Resource.Load(m_SoundManagerPrefab));
	}
};
