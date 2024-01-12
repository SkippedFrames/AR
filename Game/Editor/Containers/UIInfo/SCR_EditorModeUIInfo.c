[BaseContainerProps()]
class SCR_EditorModeUIInfo: SCR_UIInfo
{
	[Attribute()]
	protected ref Color m_ModeColor;
	
	[Attribute()]
	protected int m_iOrder;
	
	Color GetModeColor()
	{
		//~Todo: Figure out why alpha is sometimes set to 0.392
		m_ModeColor.SetA(1);
		return m_ModeColor;
	}
	
	int GetOrder()
	{
		return m_iOrder; 
	}
};
