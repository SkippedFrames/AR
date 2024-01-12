// Class for one entry of the magazine configuration
// It contains images for mag outline, progress bar and alpha mask.
[BaseContainerProps(configRoot: true)]
class SCR_MagazineIndicatorConfiguration 
{
	// ---- Imageset with magazine standard textures
	[Attribute("{3BB05C675B05A74B}UI/Textures/WeaponInfo/icons_weaponInfo.imageset", UIWidgets.ResourceNamePicker, "Imageset with foreground textures", "imageset")]
	ResourceName m_sImagesetIcons;
	
	// ---- Imageset with magazine glow textures
	[Attribute("{4E003F94B2A00561}UI/Textures/WeaponInfo/icons_weaponInfo-glow.imageset", UIWidgets.ResourceNamePicker, "Imageset with glow textures", "imageset")]
	ResourceName m_sImagesetGlows;
	
	// ---- Magazine outline image ----
	[Attribute("magazine-default-outline", UIWidgets.EditBox, "Magazine icon/outline image")]
	string m_sOutline;
	
	[Attribute("true", UIWidgets.EditBox, "Use progress bar. If false, only outline image will be used.", params: "imageset")]
	bool m_bProgressBar;	
	
	[Attribute("magazine-default-background", UIWidgets.EditBox, "Magazine background image")]
	string m_sBackground;	
		
	// ---- Magazine progress image ----
	[Attribute("magazine-default-fill", UIWidgets.EditBox, "Magazine progress bar image")]
	string m_sProgress;	
	
	// ---- Magazine alpha mask image ----
	[Attribute("magazine-default-alpha", UIWidgets.EditBox, "Magazine progress bar alpha mask")]
	ResourceName m_sProgressAlphaMask;
};
