#pragma once


// This is static polymorphism. Declare a templated method named registerCurrentOnScript inside the child to hide this method and not trigger the compile error.
// This method implementation should be done inside the endpoint library (.exe) that defines the SingletonAllocator.
// The reason I did it like this is to abstract my internal scripter from all modules (I don't want to be stuck with either lua or python, or ... as a scritping language
// But be able to switch easily between them without redoing all my code bindings... But wrappers libraries weren't coded with this mindset, therefore I needed to make some specific hack.

#define STORM_IS_SCRIPTABLE_ITEM																			\
public:																										\
	template<class IScriptWrapperInterface> void registerCurrentOnScript(IScriptWrapperInterface &) const	\

