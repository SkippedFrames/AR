[BaseContainerProps()]
class SCR_ImageSetResource: SCR_BaseTextureResource
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "imageset")]
	protected ResourceName m_Set;
	
	[Attribute()]
	protected string m_sImage;
	
	override void ApplyTo(ImageWidget widget)
	{
		if (m_Set.IsEmpty())
		{
			Print("Image set not defined!", LogLevel.ERROR);
			return;
		}
		if (widget) widget.LoadImageFromSet(0, m_Set, m_sImage);
	}
};
