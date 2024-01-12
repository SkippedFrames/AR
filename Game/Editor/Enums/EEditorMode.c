/** @ingroup Editor_Entities
*/

//! Editor mode that defines overall functionality.
enum EEditorMode
{
	EDIT = 1, ///< Editing and placing entities, configuring attributes
	STRATEGY = 2, ///< Commanding AI and players
	SPECTATE = 4, ///< Observing players
	PHOTO = 8, ///< Photo mode
	ADMIN = 16, ///< Server administration
	BUILDING = 32 ///< Building mode
};
