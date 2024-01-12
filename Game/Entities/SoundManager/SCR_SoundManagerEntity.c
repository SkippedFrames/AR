[EntityEditorProps(category: "GameScripted/Sound", description: "")]
class SCR_SoundManagerEntityClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
/*!
SCR_SoundManagerEntity is created for playing simple one-shot sounds without need of SoundComponent on given entity.
SCR_SoundManagerEntity is not present on console app.
If functions in SCR_SoundManagerEntity are enough for given sound, prioritize it befor adding SoundComponent on the entity.
If entity already has SoundComponent because of some other sound, do not use SCR_SoundManagerEntity.
Do not use SCR_SoundManagerEntity for UI sounds.
Do not use SCR_SoundManagerEntity for managing looped sounds. Looped sounds always need SoundComponent to work properly
Signals for given sound can be set only befor the playback and is not possible to update them during the sound playback.
*/

class SCR_SoundManagerEntity : GenericEntity
{	
	//! Stores all playing AudioSources	with dynamic update
	private ref array<ref SCR_AudioSource> m_aAudioSource = {};
	//! SoundWorld
	private SoundWorld m_SoundWorld;
	
	//! Global signal names
	static const string G_INTERIOR_SIGNAL_NAME = "GInterior";
	static const string G_CURR_VEHICLE_COVERAGE_SIGNAL_NAME = "GCurrVehicleCoverage";
	static const string G_IS_THIRD_PERSON_CAM_SIGNAL_NAME = "GIsThirdPersonCam";
	static const string G_ROOM_SIZE = "GRoomSize";
	static const string DYNAMIC_RANGE_SIGNAL_NAME = "DynamicRange";
	static const string IN_EDITOR_SIGNAL_NAME = "InEditor";
	
	//! Global signal indexes
	private static int s_iGInteriorIdx;
	private static int s_iGCurrVehicleCoverageIdx;
	private static int s_iGIsThirdPersonCamIdx;
	private static int s_iGRoomSizeIdx;
	private static int s_iDynamicRangeIdx;
	private static int s_iInEditorIdx;

	#ifdef ENABLE_DIAG
	private static int s_iCreatedAudioSources;
	private static int s_iMaxActiveAudioSources;
	private static int s_iPlayedAudioSources;
	private static int s_iInvalidAudioSources;
	private static int s_iInaudibleAudioSources;
	private static int s_iTerminateAudioSourceCalls;
	private static int s_iCalculateInterirorAtCount;
	#endif
	
	//------------------------------------------------------------------------------------------------	
	/*!
	When creating AudioSource, basic distance and loudness check is performed to establish, if sound would be audible or not
	Use when having SCR_SoundDataComponent on the owner EntityID
	\param owner Entity soundEvent is linked to
	\param eventName Sound event to play
	*/
	SCR_AudioSource CreateAudioSource(IEntity owner, string eventName)
	{
		#ifdef ENABLE_DIAG
		s_iCreatedAudioSources++;
		#endif
		
		// Get SCR_AudioSourceConfiguration prefab data
		SCR_SoundDataComponent soundDataComponent = SCR_SoundDataComponent.Cast(owner.FindComponent(SCR_SoundDataComponent));
		if (!soundDataComponent)
			return null;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = soundDataComponent.GetAudioSourceConfiguration(eventName);
		if (!audioSourceConfiguration)
			return null;
					
		// Is sound source in audible range?
		float distance = AudioSystem.IsAudible(audioSourceConfiguration.m_sSoundProject, eventName, owner.GetOrigin());	
		if (distance < 0)
		{
			#ifdef ENABLE_DIAG
			s_iInaudibleAudioSources++;
			#endif
			
			return null;
		}
		
		return new SCR_AudioSource(owner, audioSourceConfiguration, distance);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	When creating AudioSource, basic distance and loudness check is performed to establish, if sound would be audible or not
	Use, when need to create custom audioSourceConfiguration
	\param owner Entity soundEvent is linked to
	\param audioSourceConfiguration Custom audio source configuration
	*/
	SCR_AudioSource CreateAudioSource(IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration)
	{		
		#ifdef ENABLE_DIAG
		s_iCreatedAudioSources++;
		#endif
		
		// Is sound source in audible range?
		float distance = AudioSystem.IsAudible(audioSourceConfiguration.m_sSoundProject, audioSourceConfiguration.m_sSoundEventName, owner.GetOrigin());
		if (distance < 0)
		{
			#ifdef ENABLE_DIAG
			s_iInaudibleAudioSources++;
			#endif
			
			return null;
		}
						
		return new SCR_AudioSource(owner, audioSourceConfiguration, distance);
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
	Creates and plays audiosource
	Use when having SCR_SoundDataComponent on the owner EntityID
	\param owner Entity soundEvent is linked to
	\param eventName Sound event that will be played
	*/
	void CreateAndPlayAudioSource(IEntity owner, string eventName)
	{
		SCR_AudioSource audioSource = CreateAudioSource(owner, eventName);
		
		if (!audioSource)
			return;
		
		PlayAudioSource(audioSource);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Creates and plays audiosource
	Use when having custom AudioSourceConfiguration
	\param owner Entity soundEvent is linked to
	\param audioSourceConfiguration Custom audio source configuration
	*/
	void CreateAndPlayAudioSource(IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration)
	{
		SCR_AudioSource audioSource = CreateAudioSource(owner, audioSourceConfiguration);
		
		if (!audioSource)
			return;
		
		PlayAudioSource(audioSource);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Creates and plays audiosource at given location
	Does not allow dynamic position update
	Use when having custom AudioSourceConfiguration and  custom transformation matrix.
	\param owner Entity soundEvent is linked to
	\param audioSourceConfiguration Custom audio source configuration
	\param mat Sound will be played using this transformation matrix. Owner entity position is ignored
	*/
	void CreateAndPlayAudioSource(IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration, vector mat[4])
	{
		SCR_AudioSource audioSource = CreateAudioSource(owner, audioSourceConfiguration);
		
		if (!audioSource)
			return;
		
		PlayAudioSource(audioSource, mat);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Plays audiosource
	AudioSource playback follows after AudioSource creation and signals setup (optional)
	Use for sounds, when sound is linked to owner entity position
	\param audioSource AudioSource that is supposed to be played
	*/
	void PlayAudioSource(SCR_AudioSource audioSource)
	{			
		// Update global occlusion signals
		audioSource.SetGlobalOcclusionSignals();
		
		// Get owner transformation
		vector mat[4];
		audioSource.m_Owner.GetTransform(mat);
		
		// Play event
		PlaySoundEvent(audioSource, mat);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Plays audiosource using custom transformation matrix.
	Does not allow dynamic position update
	AudioSource playback follows after AudioSource creation and signals setup (optional)
	\param audioSource AudioSource that is supposed to be played
	\param mat Sound will be played using this transformation matrix. Owner entity position is ignored
	*/
	void PlayAudioSource(SCR_AudioSource audioSource, vector mat[4])
	{		
		// Update global occlusion signals
		audioSource.SetGlobalOcclusionSignals();
		
		// Play event
		PlaySoundEvent(audioSource, mat);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Plays sound event configured in audioSource.
	\param audioSource AudioSource that is supposed to be played
	\param mat Sound will be played using this transformation matrix
	*/	
	private void PlaySoundEvent(SCR_AudioSource audioSource, vector mat[4])
	{
		if (m_SoundWorld)
		{
			audioSource.m_InteriorRequestCallback = new SCR_InteriorRequestCallback(audioSource);
			audioSource.SetTransformation(mat);
			m_SoundWorld.CalculateInterirorAt(mat[3], audioSource.m_InteriorRequestCallback);
			
			#ifdef ENABLE_DIAG			
			s_iCalculateInterirorAtCount++;
			#endif

			// Inset audio source to audio source pool
			m_aAudioSource.Insert(audioSource);
		}
		else
		{
			audioSource.SetTransformation(mat);
			
			// Inset audio source to audio source pool
			if (audioSource.Play())
				m_aAudioSource.Insert(audioSource);
		}
	}
			
	//------------------------------------------------------------------------------------------------
	/*!
	Terminates all playing sounds on given entity 
	\param entity All sound events on this entity are terminated
	*/
	void TerminateAudioSource(IEntity entity)
	{	
		#ifdef ENABLE_DIAG
		s_iTerminateAudioSourceCalls++;
		#endif
					
		for (int i = m_aAudioSource.Count() - 1; i >= 0; i--)
		{
			SCR_AudioSource audioSource = m_aAudioSource[i];
			
			if (audioSource.m_Owner == entity)
				audioSource.Ternimate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Terminates given audio source 
	\param audioSource AudioSource to terminate
	*/
	void TerminateAudioSource(SCR_AudioSource audioSource)
	{				
		// AudioSource was already played and removed
		if (!audioSource)
			return;	
		
		// Terminate sound
		audioSource.Ternimate();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Terminates all playing sounds that are not static
	*/
	void TerminateAll()
	{		
		foreach (SCR_AudioSource audioSource : m_aAudioSource)
		{
			audioSource.Ternimate();
		}
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
	Manages active AudioSources
	*/
	private void HandleAudioSources()
	{							
		for (int i = m_aAudioSource.Count() - 1; i >= 0; i--)
		{
			SCR_AudioSource audioSource = m_aAudioSource[i];
							
			// Remove audio source if sound finished playing
			if (AudioSystem.IsSoundPlayed(audioSource.m_AudioHandle) && !audioSource.m_InteriorRequestCallback)
			{
				m_aAudioSource.Remove(i);
				continue;
			}
					
			if (m_aAudioSource[i].m_Owner)
			{
				// Update audio source position				
				if (!SCR_Enum.HasFlag(audioSource.m_AudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static))
					audioSource.UpdateTransformation();			
			}
			else
			{
				// Remove audio source if parent entity was deleted
				if (!SCR_Enum.HasFlag(audioSource.m_AudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.FinishWhenEntityDestroyed) && !audioSource.m_InteriorRequestCallback)
				{
					audioSource.Ternimate();
					m_aAudioSource.Remove(i);
				}
			}
		}
		
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_SOUND_MANAGER))
		{
			int count = m_aAudioSource.Count();
			DbgUI.Begin("Sound Manager");
			DbgUI.Text("Active Audio Sources          :" + count);
			if (count > s_iMaxActiveAudioSources)
				s_iMaxActiveAudioSources = count;
			DbgUI.Text("Maximum Active Audio Sources  :" + s_iMaxActiveAudioSources);
			DbgUI.Text("Created Audio Sources         :" + s_iCreatedAudioSources);
			DbgUI.Text("Played Audio Sources          :" + s_iPlayedAudioSources);
			DbgUI.Text("Invalid Audio Sources         :" + s_iInvalidAudioSources);
			DbgUI.Text("Inaudible Audio Sources       :" + s_iInaudibleAudioSources);
			DbgUI.Text("TerminateAudioSource() Calls  :" + s_iTerminateAudioSourceCalls);
			DbgUI.Text("CalculateInterirorAt() Count  :" + s_iCalculateInterirorAtCount);
			DbgUI.End();
			
			// Print events for active audio sources
			if (m_aAudioSource.Count() != 0)
			{
				Print("--- " + m_aAudioSource.Count().ToString() + " playing sound events ---");
				foreach (SCR_AudioSource audioSource : m_aAudioSource)
				{
					Print(audioSource.m_AudioSourceConfiguration.m_sSoundEventName);
				}
			}
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_VARIABLES))
		{					
			Resource holder = BaseContainerTools.LoadContainer("{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf");			
			if (!holder)
				return;
			
			BaseContainer baseContainer = holder.GetResource().ToBaseContainer();
			if (!baseContainer)
				return;
						
			BaseContainerList list = baseContainer.GetObjectArray("Variables");
			if (!list)
				return;		
			
			DbgUI.Begin("Global Variables");
			for (int i = 0; i < list.Count(); i++)
			{
				string name = list[i].GetName();
				DbgUI.Text(name + ": " + AudioSystem.GetVariableValue(name, "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf").ToString(-1,2));
			}
			DbgUI.End();
		}		
		#endif
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
	Returns GInterior global signal index
	*/
	int GetGInteriorSignalIdx()
	{
		return s_iGInteriorIdx;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns GCurrVehicleCoverage global signal index
	*/	
	int GetGCurrVehicleCoverageSignalIdx()
	{
		return s_iGCurrVehicleCoverageIdx;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns GIsThirdPersonCam global signal index
	*/	
	int GetGIsThirdPersonCamSignalIdx()
	{
		return s_iGIsThirdPersonCamIdx;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns GRoomSize global signal index
	*/	
	int GetRoomSizeIdx()
	{
		return s_iGRoomSizeIdx;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets GDynamicRange signal value base of game settings
	*/	
	protected void SetDynamicRangeSignalValue()
	{			
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_AudioSettings");    		
    	if (!settings) 
			return;       
        
		float value;	
		settings.Get("m_fDynamicRange", value);    
		
		// DynamicRange has range <-1, 1>
		GetGame().GetSignalsManager().SetSignalValue(s_iDynamicRangeIdx, value * 0.01 - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets InEditor signal to 1
	*/
	protected void OnEditorOpen()
	{
		GetGame().GetSignalsManager().SetSignalValue(s_iInEditorIdx, 1);
	}
	//------------------------------------------------------------------------------------------------
	/*!
	Sets InEditor signal to 0
	*/
	protected void OnEditorClose()
	{
		GetGame().GetSignalsManager().SetSignalValue(s_iInEditorIdx, 0);
	}
	//------------------------------------------------------------------------------------------------
	/*!
	Registers to open and close editor event listenets
	*/	
	protected void OnEditorManagerInit(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Insert(OnEditorOpen);
		editorManager.GetOnClosed().Insert(OnEditorClose);
	}
	//------------------------------------------------------------------------------------------------
	/*!
	Unregisters from open and close editor event listenets
	*/	
	protected void OnEditorManagerExit(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Remove(OnEditorOpen);
		editorManager.GetOnClosed().Remove(OnEditorClose);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Editor manager events init
	*/	
	protected void EditorManagerEventsInit()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			OnEditorManagerInit(editorManager);
		else
		{
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore)
				editorManagerCore.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerInit);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Editor manager events exit
	*/
	protected void EditorManagerEventsExit()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			OnEditorManagerExit(editorManager);
		
		SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (editorManagerCore)
			editorManagerCore.Event_OnEditorManagerInitOwner.Remove(OnEditorManagerInit);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{		
		HandleAudioSources();
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		// Register SCR_SoundManagerEntity
		GetGame().RegisterSoundManagerEntity(this);
		
		// Get global signals indexes
		GameSignalsManager gameSignalsManager = GetGame().GetSignalsManager();
		
		s_iGInteriorIdx = gameSignalsManager.AddOrFindSignal(G_INTERIOR_SIGNAL_NAME);
		s_iGCurrVehicleCoverageIdx = gameSignalsManager.AddOrFindSignal(G_CURR_VEHICLE_COVERAGE_SIGNAL_NAME);
		s_iGIsThirdPersonCamIdx = gameSignalsManager.AddOrFindSignal(G_IS_THIRD_PERSON_CAM_SIGNAL_NAME);
		s_iGRoomSizeIdx = gameSignalsManager.AddOrFindSignal(G_ROOM_SIZE);
		s_iDynamicRangeIdx = gameSignalsManager.AddOrFindSignal(DYNAMIC_RANGE_SIGNAL_NAME);
		s_iInEditorIdx = gameSignalsManager.AddOrFindSignal(IN_EDITOR_SIGNAL_NAME);
		
		// Event listeners for audio game settings
		GetGame().OnUserSettingsChangedInvoker().Insert(SetDynamicRangeSignalValue);
		// Update DynamicRange signal
		SetDynamicRangeSignalValue();
		
		//Event listeners for editor manager
		EditorManagerEventsInit();
		
		// GetSoundWorld
		ChimeraWorld chimeraWorld = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (chimeraWorld)
			m_SoundWorld = chimeraWorld.GetSoundWorld();
		
		#ifdef ENABLE_DIAG	
		s_iCreatedAudioSources = 0;
		s_iMaxActiveAudioSources = 0;
		s_iPlayedAudioSources = 0;
		s_iInvalidAudioSources = 0;
		s_iInaudibleAudioSources = 0;
		s_iTerminateAudioSourceCalls = 0;
		s_iCalculateInterirorAtCount = 0;
		#endif	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SoundManagerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.POSTFRAME | EntityEvent.INIT);
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
		
		#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_SOUND_MANAGER, "", "Sound Manager", "Sounds");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_VARIABLES, "", "Global Variables", "Sounds");
		#endif	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SoundManagerEntity()
	{
		// Terminate all playing sounds
		TerminateAll();
		
		// Remove from audio game settings event
		GetGame().OnUserSettingsChangedInvoker().Remove(SetDynamicRangeSignalValue);
		
		// Remove from editor manager events
		EditorManagerEventsExit();
		
		#ifdef ENABLE_DIAG				
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_SOUND_MANAGER);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_VARIABLES);
		#endif
	}
};
