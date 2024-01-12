//! Holds animation resource
[EntityEditorProps(category: "Animation", description: "Holds animation resource")]
class SCR_AnimationResourceHolderComponentClass : ScriptComponentClass
{
}

class SCR_AnimationResourceHolderComponent: GenericComponent
{
	[Attribute(params: "anm", desc: "Animation resource")]
	ResourceName 	m_sAnimation;
}
