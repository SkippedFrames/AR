class SCR_BaseCaptureMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnBaseCapture(SCR_MilitaryBaseComponent base, Faction faction)
	{
		PlayerController pc = GetGame().GetPlayerController();
		
		if (!pc)
			return;
		
		if (faction != SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetPlayerFaction(pc.GetPlayerId()))
			return;
		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONBASECAPTURE);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();
		
		if (!baseManager)
			return;
		
		baseManager.GetOnBaseFactionChanged().Insert(OnBaseCapture);
	}
	
	override void OnDelete()
	{
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance(false);
		
		if (!baseManager)
			return;
		
		baseManager.GetOnBaseFactionChanged().Remove(OnBaseCapture);
	}
}
