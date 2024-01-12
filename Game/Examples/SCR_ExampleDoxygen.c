/**
@defgroup Examples
Examples of various features, not to be used in actual game.
*/
/*!
Example of how to document a class in Doxygen.
@ingroup Examples

	██████╗  ██████╗ ██╗  ██╗██╗   ██╗ ██████╗ ███████╗███╗   ██╗
	██╔══██╗██╔═══██╗╚██╗██╔╝╚██╗ ██╔╝██╔════╝ ██╔════╝████╗  ██║
	██║  ██║██║   ██║ ╚███╔╝  ╚████╔╝ ██║  ███╗█████╗  ██╔██╗ ██║
	██║  ██║██║   ██║ ██╔██╗   ╚██╔╝  ██║   ██║██╔══╝  ██║╚██╗██║
	██████╔╝╚██████╔╝██╔╝ ██╗   ██║   ╚██████╔╝███████╗██║ ╚████║
	╚═════╝  ╚═════╝ ╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚══════╝╚═╝  ╚═══╝
>  What is not documented does not exist!

# Why document?
- Provide info for other developers and modders
- Help your future self to remember
- Final sanity check before commit

# What is Doxygen?
- Automated tool that converts code comments into HTML library
- Docs stay together with the code and can be updated alongside it

# Local setup
1. Download and install [Doxygen]
2. Download and install [GraphViz]
3. In *Docs* folder, create *Generate.bat* file and put the following line inside
	(update the path to Doxygen installation with your path)
	~~~~
	"C:\Program Files\doxygen\bin\doxygen.exe" ArmaReforgerDoxygen.cfg
	~~~~
4. **Run the bat file.**
	Documentation will be generated in Docs\html folder
	You can access it by opening index.html.

	Consider creating a shortcut / bookmark both for Generate.bat file
	and for index.hml, so you can access them later with ease.

# What to document?
- Public variables and methods
- Protected variables and function you expect to be overriden in inherited classes
- Enum values

For more info, check the official Doxygen website: https://www.doxygen.nl/

[Doxygen]: http://www.doxygen.nl/download.html
[GraphViz]: https://www.graphviz.org/download/
*/
class SCR_ExampleDoxygen
{
	//------------------------------------------------------------------------------------------------
	/*!
	Example public variable.
	Ideally avoid using public variables and offer public methods instead.
	But if you must, please document them.
	*/
	int m_iExamplePublicVariable;

	//------------------------------------------------------------------------------------------------
	/*!
	Example protected variable.
	Document protected vars for example when you expect them to be used in inherited classes.
	*/
	protected int m_iExampleProtectedVariable;

	//------------------------------------------------------------------------------------------------
	//! Simple example function.
	void Example()
	{
		// comments without Doxygen markup like this one are ignored
		Print("Example");
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Example function with documented parameters.
	\param exampleParam Example parameter
	*/
	void ExampleWithParam(int exampleParam)
	{
		PrintFormat("ExampleWithParam: exampleParam1=%", exampleParam);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Example function with out parameter.
	\param[out] outExampleParam Example parameter to be overriden by the function
	*/
	void ExampleWithOutParam(out int outExampleParam)
	{
		outExampleParam = 42;
		PrintFormat("ExampleWithOutParam: outExampleParam=%1", outExampleParam);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Example function with return value.
	\return Example value
	*/
	int ExampleWithReturnValue()
	{
		Print("ExampleWithReturnValue");
		return 42;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Example function with a parameter, out parameter and returned value.
	\param exampleParam Example parameter
	\param[out] outExampleParam Example parameter to be overriden by the function
	\return Example value
	*/
	int ExampleWithEverything(int exampleParam, out int outExampleParam)
	{
		PrintFormat("ExampleWithEverything: exampleParam=%1, outExampleParam=%2", exampleParam, outExampleParam);
		outExampleParam = exampleParam;
		return exampleParam + outExampleParam;
	}
}; // When you have multiple classes or enums withing one file, end them with a semicolon, otherwise Doxygen will stop looking further!

/*!
Example of how to document an enum in Doxygen.
@ingroup Examples
*/
enum SCR_EExampleDoxygen
{
	EXAMPLE_1, //!< First value
	EXAMPLE_2, //!< Second value
};
