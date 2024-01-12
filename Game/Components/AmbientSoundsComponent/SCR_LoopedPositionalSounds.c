/*
Handles looped sounds such as leaves rustles, or crickets played on entities close to camera
*/
[BaseContainerProps(configRoot: true)]
class SCR_LoopedPositionalSounds: SCR_AmbientSoundsEffect
{
	private const int MINIMUM_MOVE_DISTANCE_SQ = 2;
	private const int INVALID = -1;	
	private const int LOOP_SOUND_HEIGHT_LIMIT = 25;
	private const int LOOP_SOUND_COUNT = 9;
	private const int UPDATE_POSITION_THRESHOLD = 5;
    private const float BOUNDING_BOX_CROP_FACTOR = 0.3;
	private const int BUSH_LOOP_SOUND_COUNT_LIMIT = 4;
	private const string ENTITY_SIZE_SIGNAL_NAME = "EntitySize";
	
	private int m_iEntitySizeSignalIdx;
	
	//! Camera position where looped sounds were processed the last time	
	private vector m_vCameraPosLooped;
	//! All closest entities arround camera				
	private ref array<IEntity>  m_aClosestEntityID = new array<IEntity>;
	//! Entities with plaing looped sound	
	private ref array<IEntity>  m_aLoopedSoundID = new array<IEntity>;
	//! Data for playing looped sounds
	private ref array<ref SCR_TreeFoliage> m_aTreeFoliage = new array<ref SCR_TreeFoliage>;
		
	//------------------------------------------------------------------------------------------------
	/*
	Called by SCR_AmbientSoundComponent in UpdateSoundJob()
	*/
	override void Update(float worldTime, vector cameraPos)
	{
		HandleLoopedSounds(worldTime, cameraPos);
		UpdateTreeFoliageSoundPosition(cameraPos);
	}
	
	//------------------------------------------------------------------------------------------------
	private void HandleLoopedSounds(float worldTime, vector cameraPos)
	{							
		if (vector.DistanceSqXZ(m_vCameraPosLooped, cameraPos) < MINIMUM_MOVE_DISTANCE_SQ)
			return;
		
		m_vCameraPosLooped = cameraPos;
		
		m_aClosestEntityID.Clear();
					
		//Get closest entities array
		UpdateClosesEntitieyArray();
		
		// Stop sounds that are no longer among closest entities
		StopLoopedSounds();
		
		// Play sounds on entities, that become closest				
		PlayLoopedSounds(cameraPos);
	}
	
	//------------------------------------------------------------------------------------------------
	private void StopLoopedSounds()
	{		
		for (int i = m_aLoopedSoundID.Count() - 1; i >= 0; i--)
		{				
			int index = m_aClosestEntityID.Find(m_aLoopedSoundID[i]);
			
			if (index == INVALID)
			{
				m_aLoopedSoundID.Remove(i);
				m_aTreeFoliage.Remove(i);			
			}		
		}
	}	
	
	//------------------------------------------------------------------------------------------------			
	private void PlayLoopedSounds(vector cameraPos)
	{		
		foreach(IEntity entity: m_aClosestEntityID)
		{
			int index = m_aLoopedSoundID.Find(entity);
			
			if (index == INVALID && entity != null)
			{
				m_aLoopedSoundID.Insert(entity);
				
				float treeHeight, foliageHight;
				ETreeSoundTypes treeSoundType				
				GetTreeProperties(entity, treeSoundType, foliageHight);

				// Create tree foliage class			
				SCR_TreeFoliage treeFoliage;
				string eventName;
								
				if (treeSoundType == ETreeSoundTypes.Leafy)
				{
					treeFoliage = CreateTreeFoliage(entity, foliageHight, treeHeight);	
					eventName = GetTreeSoundEventName(foliageHight, treeSoundType);
				}
				else
				{
					treeFoliage = CreateBushFoliage(entity);
					eventName = SCR_SoundEvent.SOUND_BUSH_LP;
				}
											
				// Get sound init position
				vector mat[4];
				if (treeFoliage.m_bUpdatePosition)
					mat[3] = GetTreeFoliageSoundPosition(treeFoliage, cameraPos);	
				else
					mat[3] = treeFoliage.m_vCenter;					
		
				// Play sound					
				treeFoliage.m_AudioHandle = m_AmbientSoundsComponent.SoundEventTransform(eventName, mat);
				m_aTreeFoliage.Insert(treeFoliage);									
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	private void UpdateClosesEntitieyArray()
	{
		m_AmbientSoundsComponent.GetClosestEntities(EQueryType.TreeBush, BUSH_LOOP_SOUND_COUNT_LIMIT, m_aClosestEntityID);
		m_AmbientSoundsComponent.GetClosestEntities(EQueryType.TreeLeafy, LOOP_SOUND_COUNT, m_aClosestEntityID);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetTreeProperties(IEntity entity, out ETreeSoundTypes soundType, out float foliageHeight)
	{
		// Get tree entity
		Tree tree = Tree.Cast(entity);
		if (!tree)
			return;
		
		// Get prefab data
		TreeClass treeClass = TreeClass.Cast(tree.GetPrefabData());
		if (!treeClass)
			return;
		
		foliageHeight = treeClass.m_iFoliageHeight * entity.GetScale();	
		soundType = treeClass.SoundType;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_TreeFoliage CreateTreeFoliage(IEntity entity,float foliageHight, out float treeHight = 0, )
	{		
		// Get world vounding box
		vector mins, maxs;			
		entity.GetWorldBounds(mins, maxs);
		
		treeHight = maxs[1] - mins[1];
						
		// Set Tree treeFoliage	
		ref SCR_TreeFoliage treeFoliage = new SCR_TreeFoliage();
		vector mat[4];
				
		if (treeHight < UPDATE_POSITION_THRESHOLD)
			treeFoliage.m_vCenter = vector.Lerp(mins, maxs, 0.5);
		else
		{							
			// Set update position flag
			treeFoliage.m_bUpdatePosition = true;
			
			// Get max/min height
			treeFoliage.m_fMaxY = maxs[1];			
			treeFoliage.m_fMinY = mins[1] + foliageHight;
			
			// Get average width
			float widthX = (maxs[0] - mins[0]) * 0.5;
			float widthZ = (maxs[2] - mins[2]) * 0.5;
			treeFoliage.m_fWidth = 0.5 * (widthX + widthZ) * BOUNDING_BOX_CROP_FACTOR;
						
			// Get Center 2D
			vector center;					
			center[0] = mins[0] + widthX;
			center[2] = mins[2] + widthZ;						
			treeFoliage.m_vCenter = center;
		}
		
		return treeFoliage;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_TreeFoliage CreateBushFoliage(IEntity entity)
	{
		// Get tree entity
		Tree tree = Tree.Cast(entity);
		if (!tree)
			return null;
		
		// Get scaled foliage above ground height
		TreeClass treeClass = TreeClass.Cast(tree.GetPrefabData());
		if (!treeClass)
			return null;
		
		// Get world vounding box
		vector mins, maxs;			
		entity.GetWorldBounds(mins, maxs);
										
		// Set Tree treeFoliage	
		ref SCR_TreeFoliage treeFoliage = new SCR_TreeFoliage();
		vector mat[4];
				
		treeFoliage.m_vCenter = vector.Lerp(mins, maxs, 0.5);

		return treeFoliage;
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetTreeSoundEventName(float foliageHeight, ETreeSoundTypes treeSoundType)
	{
			if (foliageHeight < 3)
				return SCR_SoundEvent.SOUND_LEAFYTREE_SMALL_LP;
			else if (foliageHeight < 5)
				return SCR_SoundEvent.SOUND_LEAFYTREE_MEDIUM_LP;
			else if (foliageHeight < 7)
				return SCR_SoundEvent.SOUND_LEAFYTREE_LARGE_LP;
			else
				return SCR_SoundEvent.SOUND_LEAFYTREE_VERYLARGE_LP;	
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateTreeFoliageSoundPosition(vector cameraPos)
	{		
		for (int i = 0, size = m_aLoopedSoundID.Count(); i < size; i++)
		{
			if (m_aTreeFoliage[i].m_bUpdatePosition == true)
			{		
				vector mat[4];										
				mat[3] = GetTreeFoliageSoundPosition(m_aTreeFoliage[i], cameraPos);
	
				// Update sound position
				m_AmbientSoundsComponent.SetSoundTransformation(m_aTreeFoliage[i].m_AudioHandle, mat);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	private vector GetTreeFoliageSoundPosition(SCR_TreeFoliage treeFoliage, vector cameraPos)
	{
		vector position;
		
		// Set distance
		float cameraDistance = vector.DistanceXZ(cameraPos, treeFoliage.m_vCenter);
			
		if (cameraDistance <= treeFoliage.m_fWidth)
			position = cameraPos;
		else
		{					
			vector cameraPos2D = cameraPos;
			cameraPos2D[1] =0 ;
			
			position = treeFoliage.m_vCenter + (cameraPos2D - treeFoliage.m_vCenter).Normalized() * treeFoliage.m_fWidth;
		}
			
		// Set height
		position[1] = Math.Clamp(cameraPos[1], treeFoliage.m_fMinY, treeFoliage.m_fMaxY);				
		
		return position;
	}

	//------------------------------------------------------------------------------------------------		
	private float VectorToRandomNumber(vector v)
	{		
		int i = v[0];
		int j = v[2];		
		int mod = (i * j) % 100;
		
		return Math.AbsInt(mod) * 0.01;
	}

	//------------------------------------------------------------------------------------------------	
	/*
	Called by SCR_AmbientSoundComponent in EOnPostInit()
	*/	
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, signalsManagerComponent);

		m_iEntitySizeSignalIdx = signalsManagerComponent.AddOrFindSignal(ENTITY_SIZE_SIGNAL_NAME);
	}
}
