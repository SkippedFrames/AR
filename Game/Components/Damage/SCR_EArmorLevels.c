/*!
Types of armor and their corresponding protection level. Protection level is int value used for damage reduction.
@ingroup ArmorLevels
*/
enum SCR_EArmorLevels
{
	NONE = 0,			//!< No protection value
	CUSTOM_01 = 10,		//!< Arbitrary protection values
	CUSTOM_02 = 15,
	CUSTOM_03 = 20,
	CUSTOM_04 = 24,
	TYPE_I = 25,		//!< Balanced to conform to NIJ standard protection values
	TYPE_IIA = 30,
	TYPE_II = 35,
	TYPE_IIIA = 40,
	TYPE_III = 60,
	CLASS_1 = 31,		//!< Balanced to conform to GOST standard protection values
	CLASS_2 = 41,
	CLASS_2A = 46,
	CLASS_3 = 80,
	CLASS_4 = 90,
	CLASS_5 = 121
};
