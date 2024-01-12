//------------------------------------------------------------------------------------------------
//! Contains various global constants
//------------------------------------------------------------------------------------------------

const float METERS_PER_SEC_TO_KILOMETERS_PER_HOUR = 3.6;
const float KILOMETERS_PER_HOUR_TO_METERS_PER_SEC = 0.277778;

const float METERS_TO_KNOTS = 1.943844;
const float KNOTS_TO_METERS = 0.514444;

const float COLOR_255_TO_1 = 0.003921568;
const float COLOR_1_TO_255 = 255;

const int TRACE_LAYER_CAMERA = EPhysicsLayerDefs.Camera;

class UIColors
{
	static const ref Color DARK_SAGE        			= Color.FromSRGBA(86, 92, 84, 255);
	static const ref Color DARK_GREY					= Color.FromSRGBA(85, 85, 85, 255);
	static const ref Color LIGHT_GREY             		= Color.FromSRGBA(255, 255, 255, 179);
	static const ref Color TRANSPARENT      			= Color.FromSRGBA(0, 0, 0, 0);

	// Refined UI colors
	static const ref Color CONTRAST_DISABLED			= Color.FromSRGBA(0, 0, 0, 38);
	static const ref Color CONTRAST_DEFAULT   			= Color.FromSRGBA(226, 167, 79, 76);
	static const ref Color CONTRAST_HOVERED				= Color.FromSRGBA(239, 199, 139, 102);
	static const ref Color CONTRAST_CLICKED  			= Color.FromSRGBA(226, 167, 79, 255);
	static const ref Color CONTRAST_CLICKED_HOVERED		= Color.FromSRGBA(226, 167, 79, 255);
	
	static const ref Color WHITE_DISABLED				= Color.FromSRGBA(255, 255, 255, 38);
	static const ref Color WHITE_DEFAULT				= Color.FromSRGBA(255, 255, 255, 25);
	static const ref Color WHITE_HOVERED				= Color.FromSRGBA(255, 255, 255, 102);
	
	static const ref Color BACKGROUND_DISABLED			= Color.FromSRGBA(0, 0, 0, 38);
	static const ref Color BACKGROUND_DEFAULT			= Color.FromSRGBA(0, 0, 0, 102);
	static const ref Color BACKGROUND_HOVERED			= Color.FromSRGBA(0, 0, 0, 153);
	
	static const ref Color WARNING_FOCUSED				= Color.FromSRGBA(249, 67, 67, 107);
	static const ref Color INFO							= Color.FromSRGBA(0, 128, 255, 255);
	
	// From Manual
	static const ref Color HIGHLIGHTED					= Color.FromSRGBA(255, 203, 123, 255);	//#FFCB7B
	static const ref Color NEUTRAL_INFORMATION			= Color.FromSRGBA(255, 255, 255, 255);	//#FFFFFF
	static const ref Color NEUTRAL_ACTIVE_STANDBY		= Color.FromSRGBA(189, 189, 188, 255); 	//#BDBDBC
	static const ref Color IDLE_ACTIVE					= Color.FromSRGBA(255, 255, 255, 179);	//#FFFFFF at 70% Alpha
	static const ref Color IDLE_DISABLED				= Color.FromSRGBA(77, 77, 77, 255);		//#4D4D4D
	static const ref Color IDLE_DISABLED_TRANSPARENT	= Color.FromSRGBA(77, 77, 77, 153);		//60% #4D4D4D
	static const ref Color WARNING						= Color.FromSRGBA(249, 67, 67, 255);	//#f94343
	static const ref Color CONFIRM						= Color.FromSRGBA(67, 194, 93, 255);	//#43C25D Manual calls it POSITIVE green
	static const ref Color CONTRAST_COLOR	 			= Color.FromSRGBA(226, 167, 79, 255); 	//#e2a74f REFORGER ORANGE
	static const ref Color ONLINE	 					= Color.FromSRGBA(0, 128, 255, 255); 	//#0080FF
	static const ref Color EDIT_WIDGET_BACKGROUND 		= Color.FromSRGBA(42, 42, 42, 255);		//#2a2a2a
	
	//~ editor
	static const ref Color EDITOR_ICON_COLOR_NEUTRAL	= Color.FromSRGBA(255, 255, 255, 255); ///< Colors for Editor Icons when an entity does not have an assigned faction and is not destroyed
	static const ref Color EDITOR_ICON_COLOR_DESTROYED	= Color(0.25, 0.25, 0.25, 1); ///< Colors for Editor Icons when an entity is dead or destroyed

	//------------------------------------------------------------------------------------------------
	// Returns the color in a format usable as default of Attributes
	static string GetColorAttribute(Color color)
	{
		if (color)
			return string.Format("%1 %2 %3 %4", color.R(), color.G(), color.B(), color.A());

		return "1 1 1 1";
	}
	
	//------------------------------------------------------------------------------------------------
	static string SRGBAFloatToInt(Color color)
	{
		if (!color)
			return "255, 255, 255, 255";
		
		Color returnColor = new Color(color.R(), color.G(), color.B(), color.A());
		
		//--- Convert to sRGBA format for rich text
		returnColor.LinearToSRGB();

		//--- Convert to ints, no fractions allowed in rich text
		int colorR = returnColor.R() * 255;
		int colorG = returnColor.G() * 255;
		int colorB = returnColor.B() * 255;
		int colorA = returnColor.A() * 255;
		
		return string.Format("%1, %2, %3, %4", colorR, colorG, colorB, colorA);
	}
};

class GUIColors
{
	static const ref Color DISABLED 				= Color.FromSRGBA(200, 200, 200, 100);				//WHITE with 30% alpha converted to GREY with 100% alpha
	static const ref Color DISABLED_GLOW 			= Color.FromSRGBA(0, 0, 0, 100);

	static const ref Color ENABLED 					= Color.FromSRGBA(255, 255, 255, 255);				//WHITE
	static const ref Color ENABLED_GLOW 			= Color.FromSRGBA(162, 162, 162, 255);				//GREY
	
	static const ref Color DEFAULT 					= Color.FromSRGBA(255, 255, 255, 255);				//WHITE
	static const ref Color DEFAULT_GLOW 			= Color.FromSRGBA(0, 0, 0, 255);						//BLACK																					  
	//------------------------------------------------------------------------------------------

	static const ref Color ORANGE 					= Color.FromSRGBA(226, 167, 80, 255);				//ORANGE, standard UI orange, warnings
	static const ref Color ORANGE_BRIGHT 			= Color.FromSRGBA(255, 207, 136, 255);				//ORANGE (bright)
	static const ref Color ORANGE_BRIGHT2 			= Color.FromSRGBA(255, 233, 200, 255);				//ORANGE (bright++)
	static const ref Color ORANGE_DARK 				= Color.FromSRGBA(162, 97, 0, 255);					//DARK ORANGE

	static const ref Color RED 						= Color.FromSRGBA(236, 80, 80, 255);					//RED, error states
	static const ref Color RED_BRIGHT 				= Color.FromSRGBA(255, 134, 134, 255);				//RED (bright)
	static const ref Color RED_BRIGHT2 				= Color.FromSRGBA(255, 150, 150, 255);				//RED (bright++)
	static const ref Color RED_DARK 				= Color.FromSRGBA(162, 0, 0, 255);				//DARK RED

	static const ref Color BLUE 					= Color.FromSRGBA(41, 127, 240, 255);				//BLUE
	static const ref Color BLUE_BRIGHT 				= Color.FromSRGBA(122, 175, 255, 255);				//BLUE (bright)
	static const ref Color BLUE_BRIGHT2 			= Color.FromSRGBA(184, 212, 255, 255);				//BLUE (bright++)
	static const ref Color BLUE_DARK 				= Color.FromSRGBA(27, 92, 189, 255);					//DARK BLUE	

	static const ref Color GREEN 					= Color.FromSRGBA(37, 209, 29, 255);					//GREEN
	static const ref Color GREEN_BRIGHT 			= Color.FromSRGBA(157, 250, 153, 255);				//GREEN (bright)
	static const ref Color GREEN_BRIGHT2			= Color.FromSRGBA(216, 255, 214, 255);				//GREEN (bright++)
	static const ref Color GREEN_DARK 				= Color.FromSRGBA(28, 157, 22, 255);					//DARK GREEN
}

class UIConstants
{
	static const float FADE_RATE_INSTANT = 0;
	static const float FADE_RATE_SUPER_FAST = 20;
	static const float FADE_RATE_FAST = 10;
	static const float FADE_RATE_DEFAULT = 5; // Used for near instant actions
	static const float FADE_RATE_SLOW = 1; // Used for fading out elements that should be visible for some time
	static const float FADE_RATE_SUPER_SLOW = 0.2; // Very slow fade out
	
	// Common labels
	static const string FAVORITE_LABEL_ADD = "#AR-Workshop_ButtonAddToFavourites";
	static const string FAVORITE_LABEL_REMOVE = "#AR-Workshop_ButtonRemoveFavourites";
	
	// Common icons
	static const ResourceName ICONS_IMAGE_SET = "{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset";
	static const ResourceName ICONS_GLOW_IMAGE_SET = "{00FE3DBDFD15227B}UI/Textures/Icons/icons_wrapperUI-glow.imageset";

	// Values
    const float DISABLED_WIDGET_OPACITY = 0.3;
    const float ENABLED_WIDGET_OPACITY = 1;
	
	const int LOADING_SCREEN_Z_ORDER = 1000;
	const int SPLASH_SCREEN_Z_ORDER = 1001;
	
	// Menu base actions
	static const string MENU_ACTION_LEFT = "MenuLeft";
	static const string MENU_ACTION_RIGHT = "MenuRight";
	static const string MENU_ACTION_UP = "MenuUp";
	static const string MENU_ACTION_DOWN = "MenuDown";
	static const string MENU_ACTION_BACK = "MenuBack";
};
