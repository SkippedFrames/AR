[ComponentEditorProps(category: "GameScripted/AIComponent", description: "Scripted component for synchronization", icon: HYBRID_COMPONENT_ICON)]
class SCR_LockContextComponentClass: AIComponentClass
{	
};

class SCR_Key : Managed
{
	string m_key;
	int m_numOfKeyHolders;
	int m_maxAllowedHolders = 1;
};


class SCR_LockContextComponent : AIComponent
{
	private ref array<ref SCR_Key> m_aKeys;	
	
	private int IsKeyRegistered (string key)
	{
		for (int i, lenght = m_aKeys.Count(); i < lenght; i++)
		{
			if (m_aKeys[i].m_key == key)
				return i;			
		}
		return -1;
	}
	
	void SCR_LockContextComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_aKeys = new ref array<ref SCR_Key>;
	}
	
	
	bool JoinKey (string key, int maxAllowedHolders)
	{
		int keyIndex = IsKeyRegistered(key);
		if ( keyIndex > -1 )
		{
			if ( m_aKeys[keyIndex].m_maxAllowedHolders >= m_aKeys[keyIndex].m_numOfKeyHolders + 1 )
			{	
				if ( maxAllowedHolders != m_aKeys[keyIndex].m_maxAllowedHolders )
				{
					Print("Error: key is used with different max allowed holders!");
					return false;
				}
				m_aKeys[keyIndex].m_numOfKeyHolders++;
				//Print("Semaphore is Opened");
				return true;
			}
			else
				//Print("Semaphore is Full");
				return false;
		}
		else
		{
			ref SCR_Key newKey = new ref SCR_Key;
			newKey.m_key = key;
			newKey.m_maxAllowedHolders = maxAllowedHolders;
			if (maxAllowedHolders < 1) 
			{
				Print("Error: key is set with no holders allowed!");
				return false;
			}	
			newKey.m_numOfKeyHolders = 1;
			m_aKeys.Insert(newKey);
			//Print("Semaphore is Opened");
			return true;
		} 	
	}
	
	bool LeaveKey (string key)
	{
		int keyIndex = IsKeyRegistered(key);
		if ( keyIndex > -1 )
		{
			if (m_aKeys[keyIndex].m_numOfKeyHolders == 1)
			{
				m_aKeys.Remove(keyIndex);
				//Print("Semaphore is Closed");
				return true;
			}
			else
			{
				m_aKeys[keyIndex].m_numOfKeyHolders--;
				return true;
			}
		}
		Print("Error: key that should be returned was not found!");
		return false;
	}
	
	void ~SCR_LockContextComponent()
	{
		if (m_aKeys) m_aKeys.Clear();
		m_aKeys = null;
	}
};
