// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "OgreMesh2.h"

namespace Leviathan {

//! \brief Helpers for generating procedural geometry
class GeometryHelpers {
public:
    //! \brief Creates a screen space plane with UV coordinates
    //!
    //! The coordinates are in screen space -1 - 1 where -1 is the left and top of the screen
    //! so a full screen quad would be CreateScreenSpaceQuad(-1, -1, 2, 2)
    //! \param meshname The name of the created mesh, must be unique
    //! \todo Parameter to specify if shadow geometry is needed
    DLLEXPORT static Ogre::MeshPtr CreateScreenSpaceQuad(
        const std::string& meshname, float x, float y, float width, float height);

    //! \brief Creates a plane with specific size and UV coordinates with 0, 0 on top left
    //!
    //! This plane is flat on the Y axis. The actual width of the mesh will be double of width
    //! to make it easier to calculate
    //! \todo Merge the shared code with CreateScreenSpaceQuad
    //! \todo This needs to be verified that this generates good geometry (currently crashes
    //! when recorded with RenderDoc)
    DLLEXPORT static Ogre::MeshPtr CreateXZPlane(
        const std::string& meshname, float width, float height);

    GeometryHelpers() = delete;
};

} // namespace Leviathan
