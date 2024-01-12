/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class TagManagerClass: GenericEntityClass
{
}

class TagManager: GenericEntity
{
	/*!
	Returns registered entities in given categories within a given range.
	\param categories Entity categories represented as flags
	*/
	proto external int GetTagsInRange( out notnull array<IEntity> entities, vector origin, float range, ETagCategory categories);
}

/*!
\}
*/
