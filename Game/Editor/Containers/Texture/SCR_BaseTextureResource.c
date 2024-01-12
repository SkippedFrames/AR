[BaseContainerProps()]
class SCR_BaseTextureResource
{
	void ApplyTo(ImageWidget widget)
	{
		if (widget) widget.LoadImageTexture(0, "{AC7E384FF9D8016A}Common/Textures/placeholder_BCR.edds"); //--- Hard-coded placeholder. Ugly, but necessary.
	}
};
