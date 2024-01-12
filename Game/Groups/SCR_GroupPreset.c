[BaseContainerProps()]
class SCR_GroupPreset
{	
	[Attribute(desc: "Name of the Group")]
	protected string m_sGroupName; 
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Flag icon of this particular group.", params: "edds")]
	private ResourceName m_sGroupFlag;
	
	[Attribute(desc: "Description of the Group")]
	protected string m_sGroupDescription;
	
	[Attribute(desc: "Count of group members.")]
	protected int m_iGroupSize;
	
	[Attribute(desc: "Radio frequency for communication in kHz.")]
	protected int m_iRadioFrequency;
	
	[Attribute(desc: "Group is private.")]
	protected bool m_bIsPrivate;
	
		
	//------------------------------------------------------------------------------------------------	
	void SetupGroup(SCR_AIGroup group)
	{
		group.SetCustomName(m_sGroupName, 0);
		group.SetRadioFrequency(m_iRadioFrequency);
		group.SetMaxGroupMembers(m_iGroupSize);
		group.SetPrivate(m_bIsPrivate);
		group.SetCustomDescription(m_sGroupDescription, 0);
		
		if (!m_sGroupFlag.IsEmpty())
			group.SetCustomGroupFlag(m_sGroupFlag);
	}
	
	//Getters
	//------------------------------------------------------------------------------------------------	
	string GetGroupName()
	{
		return m_sGroupName;
	}
	
	//------------------------------------------------------------------------------------------------	
	ResourceName GetGroupFlag()
	{
		return m_sGroupFlag;
	}
	
	//------------------------------------------------------------------------------------------------	
	int GetGroupSize()
	{
		return m_iGroupSize;
	}
	
	//------------------------------------------------------------------------------------------------	
	int GetRadioFrequency()
	{
		return m_iRadioFrequency;
	}	
	
	//------------------------------------------------------------------------------------------------	
	string GetGroupDescription()
	{
		return m_sGroupDescription;
	}
	
	//------------------------------------------------------------------------------------------------	
	bool IsPrivate()
	{
		return m_bIsPrivate;
	}	
	
	//Setters
	//------------------------------------------------------------------------------------------------
	
	void SetGroupName(string name)
	{
		m_sGroupName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupSize(int size)
	{
		m_iGroupSize = size;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadioFrequency(int freq)
	{
		m_iRadioFrequency = freq;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsPrivate(bool privacy)
	{
		m_bIsPrivate = privacy;
	}	
}
