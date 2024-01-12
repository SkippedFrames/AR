class SCR_DeathMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
		//~ When the player died
	protected void OnPlayerDied(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{	
		if (playerId != SCR_PlayerController.GetLocalPlayerId())
			return;
		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONDEATH);
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
			//~ On player spawned and died register
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerKilled().Insert(OnPlayerDied);
		}
		
	}
	
	override void OnDelete()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerKilled().Remove(OnPlayerDied);
		}
	}
}
