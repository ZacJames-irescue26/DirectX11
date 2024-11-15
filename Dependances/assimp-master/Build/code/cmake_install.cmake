# Install script for directory: D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Assimp")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp5.4.3-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/lib/Debug/assimp-vc143-mtd.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/lib/Release/assimp-vc143-mt.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/lib/MinSizeRel/assimp-vc143-mt.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/lib/RelWithDebInfo/assimp-vc143-mt.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp5.4.3" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/Debug/assimp-vc143-mtd.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/Release/assimp-vc143-mt.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/MinSizeRel/assimp-vc143-mt.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/RelWithDebInfo/assimp-vc143-mt.dll")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/anim.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/aabb.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/ai_assert.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/camera.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/color4.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/color4.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/code/../include/assimp/config.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/ColladaMetaData.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/commonMetaData.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/defs.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/cfileio.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/light.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/material.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/material.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/matrix3x3.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/matrix3x3.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/matrix4x4.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/matrix4x4.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/mesh.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/ObjMaterial.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/pbrmaterial.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/GltfMaterial.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/postprocess.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/quaternion.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/quaternion.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/scene.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/metadata.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/texture.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/types.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/vector2.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/vector2.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/vector3.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/vector3.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/version.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/cimport.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/AssertHandler.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/importerdesc.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Importer.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/DefaultLogger.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/ProgressHandler.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/IOStream.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/IOSystem.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Logger.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/LogStream.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/NullLogger.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/cexport.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Exporter.hpp"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/DefaultIOStream.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/DefaultIOSystem.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/ZipArchiveIOSystem.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/SceneCombiner.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/fast_atof.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/qnan.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/BaseImporter.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Hash.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/MemoryIOWrapper.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/ParsingUtils.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/StreamReader.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/StreamWriter.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/StringComparison.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/StringUtils.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/SGSpatialSort.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/GenericProperty.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/SpatialSort.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/SkeletonMeshBuilder.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/SmallVector.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/SmoothingGroups.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/SmoothingGroups.inl"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/StandardShapes.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/RemoveComments.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Subdivision.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Vertex.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/LineSplitter.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/TinyFormatter.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Profiler.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/LogAux.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Bitmap.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/XMLTools.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/IOStreamBuffer.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/CreateAnimMesh.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/XmlParser.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/BlobIOSystem.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/MathFunctions.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Exceptional.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/ByteSwapper.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Base64.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Compiler/pushpack1.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Compiler/poppack1.h"
    "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/code/../include/assimp/Compiler/pstdint.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/Debug/assimp-vc143-mtd.pdb")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/Release/assimp-vc143-mt.pdb")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/MinSizeRel/assimp-vc143-mt.pdb")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/bin/RelWithDebInfo/assimp-vc143-mt.pdb")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "D:/DirectX GameEngine/Tutorial/Dependances/assimp-master/Build/code/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
