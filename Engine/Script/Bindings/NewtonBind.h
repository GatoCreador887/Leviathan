// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "BindHelpers.h"

namespace Leviathan{

bool BindNewton(asIScriptEngine* engine);
void RegisterNewton(asIScriptEngine* engine, std::map<int, std::string> &typeids);

}
