#define DEBUG
#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Debris", description: "Entity used to represent small chunks of debris. Automatically managed.", dynamicBox: true)]
class SCR_DebrisSmallEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! Entity used to represent small debris in the world. Is managed automatically and will never
//! allow the user to exceed the specified maximum amount. If new spawn is requested, priority
//! is evaluated. If the new debris has higher priority than any of the debris spawned before,
//! it will be replaced by the new one.
class SCR_DebrisSmallEntity : GenericEntity
{
#ifdef ENABLE_BASE_DESTRUCTION
	//! The maximum limit of small debris in the scene at any given time. This limit is never exceeded.
	private static int s_iDebrisMaximumCount = 1000;
	//! List of all the small debris in the world.
	private static ref array<SCR_DebrisSmallEntity> s_aDebrisSmallList = null;
	
	//! Whether this debris has reached end of its lifetime and should be deleted
	private bool m_bDelete = false;
	//! The priority of this debris. Higher priority debris will replace lower priority if limit is reached.
	private int m_iPriority = 0;
	//! The lifetime in seconds. 
	private float m_fLifeTime = 0.0;
	//! Entity age in seconds. After this time is bigger thatn m_fLifeTime, debris will despawn.
	private float m_fAgeTime = 0.0;
	//! The maximum distance from the camera in metres. If camera is beyond this distance, debris will despawn.
	private float m_fMaxDistance = 0.0;
	//! The physics attached to this debris.
	private Physics m_RigidBody = null;
	//! Material type of item for sound
	private SCR_EMaterialSoundTypeDebris m_eMaterialSoundType
	//! Position of last played sound
	private vector m_vSoundPositionLast;
	//! Minimal distance from last played sound
	private static const float MINIMAL_DISTANCE_SQ = 0.25;
	//! Minimum entity lifetime to play sound
	private static const float MINIMAL_AGE = 0.25;
	//! Stores last sound
	private AudioHandle m_AudioHandle = AudioHandle.Invalid;
	//! Sound threshold
	private float m_fSoundThreshold;
	//! Kinetic energy delta needed to trigger impact sound
	private static const int KINETIC_ENERGY_THRESHOLD = 12;
	#ifdef ENABLE_DIAG
	//! Cash contact velocity change
	private float m_fdVelocity;
	//!
	private ref DebugTextWorldSpace m_Text;
	//! Peak value
	private float m_fTextMax;
	//! Peak value age
	private float m_fTextAgeTime;
	#endif
	
	//! Count of debris spawned this frame
	static private int m_iSpawnedThisFrame = 0;
	//! Max debris per frame m_iSpawnedThisFrame
	static private int m_iDebrisPerFrameLimit = 128;
		
	//------------------------------------------------------------------------------------------------
	//! Get whether game is in play state or not.
	//! \return true if play mode, false otherwise
	private bool IsGamePlaying()
	{
		if (GetGame().GetWorldEntity())
			return true;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register debris into the list.
	private void RegisterDebris()
	{
		if (!IsGamePlaying())
			return;
		
		// If array is not created, create it
		if (!s_aDebrisSmallList)
		{
			s_aDebrisSmallList = new ref array<SCR_DebrisSmallEntity>();
		}
		// Insert element into the list
		if (s_aDebrisSmallList)
		{
			s_aDebrisSmallList.Insert(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregister debris from the list.
	private void UnregisterDebris()
	{
		if (!IsGamePlaying())
			return;
		
		// If the array exists only
		if (s_aDebrisSmallList)
		{
			// Remove the element, resize the array
			// If no debris is left in the list, delete it
			int idx = s_aDebrisSmallList.Find(this);
			int count = s_aDebrisSmallList.Count();
			if (idx >= 0)
			{
				s_aDebrisSmallList.Remove(idx);
				s_aDebrisSmallList.Resize(count-1);
				
				if (s_aDebrisSmallList.Count() <= 0)
				{
					s_aDebrisSmallList = null;
				}				
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private static float GetDistanceToCamera(BaseWorld world, vector position)
	{
		vector cameraMat[4];
		world.GetCurrentCamera(cameraMat);
		return vector.Distance(cameraMat[3], position);
	}
	
	//------------------------------------------------------------------------------------------------				
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_bDelete)
			return;
									
		float spdDiff = contact.GetRelativeNormalVelocityAfter() - contact.GetRelativeNormalVelocityBefore();
		
		// Play sound
		if (spdDiff > m_fSoundThreshold && vector.DistanceSq(m_vSoundPositionLast, contact.Position) >= MINIMAL_DISTANCE_SQ && m_eMaterialSoundType != 0 && m_fAgeTime > MINIMAL_AGE)
			PlaySound(contact.Position, spdDiff);
		
		// Sound debug
		#ifdef ENABLE_DIAG 
		m_fdVelocity = spdDiff;
		#endif
		
		if (spdDiff < 20)
			return;
		
		
		DeleteDebris();
	}
		
	//------------------------------------------------------------------------------------------------
	private void PlaySound(vector pos, float dVelocity)
	{		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = destructionManager.GetAudioSourceConfiguration();
		if (!audioSourceConfiguration)
			return;
		
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_MPD_ + typename.EnumToString(SCR_EMaterialSoundTypeDebris, m_eMaterialSoundType);					
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(this, audioSourceConfiguration);
		if (!audioSource)
			return;
		
		// Stop previous sound
		AudioSystem.TerminateSound(m_AudioHandle);
		
		// Set signals
		audioSource.SetSignalValue(SCR_AudioSource.COLLISION_D_V_SIGNAL_NAME, dVelocity - m_fSoundThreshold);
		audioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, m_RigidBody.GetMass());
				
		// Get sound position
		vector mat[4];		
		mat[3] = pos;		
		
		// Play sound
		soundManagerEntity.PlayAudioSource(audioSource, mat);		
		m_AudioHandle = audioSource.m_AudioHandle;	
		
		// Store position of the last played sound
		m_vSoundPositionLast = pos;
		
		// Sound Debug
		#ifdef ENABLE_DIAG 
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_MPDESTRUCTION_SHOW_IMPULSEVALUES))
		{
			m_Text = DebugTextWorldSpace.Create(GetWorld(), dVelocity.ToString(1, 2) + "/" + m_fSoundThreshold.ToString(1, 2) + "/" + m_RigidBody.GetMass().ToString(), DebugTextFlags.FACE_CAMERA, pos[0], pos[1], pos[2], 20, COLOR_BLUE);
			m_fTextAgeTime = m_fAgeTime + 1;
		}
		#endif
	}
	
	//------------------------------------------------------------------------------------------------	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Get debris age
		m_fAgeTime += timeSlice;
		
		// Delete this debris. (automatically unregisters)
		if (m_fAgeTime >= m_fLifeTime)
		{
			DeleteDebris();
		}
		
		// Check if within camera range, set to delete if not.
		if (!m_bDelete)
		{
			float distance = GetDistanceToCamera(owner.GetWorld(), owner.GetOrigin());
			if (distance >= m_fMaxDistance)
				DeleteDebris();
		}
		
		// If debris should be deleted then delete it.
		if (m_bDelete)
		{
			if (m_RigidBody)
			{
				m_RigidBody.Destroy();
				m_RigidBody = null;
			}
			
			delete this;
		}
		
		//Sound debug
		#ifdef ENABLE_DIAG 
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_MPDESTRUCTION_SHOW_IMPULSEVALUES))
		{
			// Get center of entity
			vector minsDebug;
			vector maxsDebug;
			owner.GetWorldBounds(minsDebug, maxsDebug);			
			vector centerDebug;
			for (int i = 0; i < 3; i++)
			{
				centerDebug[i] = minsDebug[i] + Math.AbsFloat(((maxsDebug[i] - minsDebug[i]) * 0.5));
			}
			
			// Hold peak velue for 1s
			if (m_fAgeTime - m_fTextAgeTime > 1 || m_fdVelocity > m_fTextMax)
			{
				m_Text = DebugTextWorldSpace.Create(GetWorld(), m_fdVelocity.ToString(1, 2) + "/" + m_fSoundThreshold.ToString(1, 2) + "/" + m_RigidBody.GetMass().ToString(), DebugTextFlags.FACE_CAMERA, centerDebug[0], centerDebug[1], centerDebug[2], 20);
				m_fTextMax = m_fdVelocity;
				m_fTextAgeTime = m_fAgeTime;
			}
		}
		#endif	
	}
		
	//------------------------------------------------------------------------------------------------
	//! Method for spawning debris. All is handled internally.
	//! \param mat Transformation matrix for the debris.
	//! \param model Model that will be used by the newly spawned debris.
	//! \param mass The mass in kilos the debris' physics will have.
	//! \param lifeTime The maximum lifetime of this debris in seconds.
	//! \param priority The priority of this debris. Higher priority overrides lower priority if at or over debris limit.
	//! \param linearVelocity Linear velocity of the debris (in m/s)
	//! \param angularVelocity Angular velocity of the debris (in deg/s)
	//! \param remap The materials to be remapped to, see SetObject() for more info
	static SCR_DebrisSmallEntity SpawnDebris(BaseWorld world, vector mat[4], ResourceName model, float mass = 10, float lifeTime = 10.0, float maxDistance = 256.0, int priority = 1, vector linearVelocity = "0 0 0", vector angularVelocity = "0 0 0", string remap = "", bool isStatic = false, SCR_EMaterialSoundTypeDebris materialSoundType = 0)
	{
		if (m_iSpawnedThisFrame >= m_iDebrisPerFrameLimit)
			return null;
		
		// Check if model is valid
		if (model == string.Empty)
			return null;
		
		// Check if model is not being spawned out of its range
		if (GetDistanceToCamera(world, mat[3]) > maxDistance)
			return null;
		
		SCR_DebrisSmallEntity entity = null;		
		
		// See if this is first entity or not, if so, create the list
		if (!s_aDebrisSmallList)
			s_aDebrisSmallList = new ref array<SCR_DebrisSmallEntity>();
		
		// If the list exists, check count. If over limit, replate debris with lower priority by this one.
		if (s_aDebrisSmallList)
		{
			int count = s_aDebrisSmallList.Count();
			// Over the limit
			if (count >= s_iDebrisMaximumCount)
			{
				for (int i = 0; i < s_aDebrisSmallList.Count(); i++)
				{
					if (s_aDebrisSmallList[i])
					{
						if (s_aDebrisSmallList[i].m_iPriority < priority)
						{
							entity = s_aDebrisSmallList[i];
							break;
						}
					}
				}
				// TODO: Try out if this impacts performance or not (or how heavily)
				if (entity)
				{
					if (entity.m_RigidBody)
					{
						entity.m_RigidBody.Destroy();
						entity.m_RigidBody = null;
					}
				}
			}
			// Below the limit, spawn new one.
			else
			{
				entity = SCR_DebrisSmallEntity.Cast(GetGame().SpawnEntity(SCR_DebrisSmallEntity));
			}
		}
	
		// Spawning has failed? There was no possible replacement? 
		if (!entity)
			return null; 
		
		m_iSpawnedThisFrame++;
		// Set newly spawned entity (or the one being reused)'s data to the new
		entity.SetTransform(mat);
		Resource resource = Resource.Load(model);
		if (!resource)
			return null;
		
		BaseResourceObject baseRes = resource.GetResource();
		if (!baseRes)
			return null;
		
		VObject obj = baseRes.ToVObject();
		entity.SetObject(obj, remap);
		
		entity.m_fLifeTime = lifeTime;
		entity.m_iPriority = priority;
		entity.m_fMaxDistance = maxDistance;
		entity.m_eMaterialSoundType = materialSoundType;
		
		// Get sound threshold	
		entity.m_fSoundThreshold =  Math.Sqrt(2 * KINETIC_ENERGY_THRESHOLD / mass);
		
		// Set debris init position	for sound	
		vector mins;
		vector maxs;
		entity.GetWorldBounds(mins, maxs);			
		entity.m_vSoundPositionLast = vector.Lerp(mins, maxs, 0.5);
		
		// Set physics
		if (!entity.m_RigidBody)
		{
			if (isStatic)
				entity.m_RigidBody = Physics.CreateStatic(entity, -1);
			else
			{
				entity.m_RigidBody = Physics.CreateDynamic(entity, mass, -1);
				if (entity.m_RigidBody)
				{
					entity.m_RigidBody.SetVelocity(linearVelocity);
					entity.m_RigidBody.SetAngularVelocity(angularVelocity * Math.DEG2RAD);
				}
			}
			if (entity.m_RigidBody)
				entity.m_RigidBody.SetInteractionLayer(EPhysicsLayerDefs.Debris);
		}
		
		return entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delete random debris from all existing debris.
	//1 \param count The amount of debris to delete.
	static void DeleteRandomDebris(int count = 1)
	{
		for (int i = 0; i < count; i++)
		{
			if (s_aDebrisSmallList)
			{
				if (s_aDebrisSmallList.Count() > 0)
				{
					SCR_DebrisSmallEntity ent = s_aDebrisSmallList.GetRandomElement();
					if (ent)
					{
						ent.DeleteDebris();
					}
				}
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Delete debris - unregisters it from the list and makes it scale down and delete.
	void DeleteDebris()
	{
		UnregisterDebris();
		m_bDelete = true;
		ClearEventMask(EntityEvent.CONTACT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		m_iSpawnedThisFrame = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! On initialization the debris is registered into the list.
	override void EOnInit(IEntity owner)
	{
		RegisterDebris();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DebrisSmallEntity(IEntitySource src, IEntity parent)
	{	
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.POSTFRAME | EntityEvent.CONTACT);
		SetFlags(EntityFlags.ACTIVE, true);		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DebrisSmallEntity()
	{
		UnregisterDebris();
	}
#endif
};
