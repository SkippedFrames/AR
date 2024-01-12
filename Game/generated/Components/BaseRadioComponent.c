/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseRadioComponentClass: GameComponentClass
{
}

class BaseRadioComponent: GameComponent
{
	/*!
	\return Entity this component is atached to.
	*/
	proto external IEntity GetOwner();
	//! \return Number of transceivers.
	proto external int TransceiversCount();
	//! \return Transceiver with given index.
	proto external BaseTransceiver GetTransceiver(int idx);
	/*!
	\param key Key used for transmission encryption
	*/
	proto external void SetEncryptionKey(string key);
	/*!
	\return Key used for transmission encryption
	*/
	proto external string GetEncryptionKey();
	/*!
	Sets the power state of whole radio.
	*/
	proto external void SetPower(bool powered);
	/*!
	\return Power state of whole radio
	*/
	proto external bool IsPowered();
}

/*!
\}
*/
