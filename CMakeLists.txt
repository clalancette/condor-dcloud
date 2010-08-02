##########################################################################
# Set project
project(CONDOR)
cmake_minimum_required(VERSION 2.8)

##########################################################################
#Setup package meta-data (used in packaging)
set(PACKAGE "condor")
set(VERSION "7.5.4")
set(PACKAGE_BUGREPORT "http://condor-wiki.cs.wisc.edu/index.cgi/tktnew")
set(PACKAGE_NAME ${PACKAGE} )
set(PACKAGE_VERSION ${VERSION} )
set(PACKAGE_STRING ${PACKAGE} )
string( TOUPPER "${CMAKE_SYSTEM_NAME}" OS_NAME )
string( TOUPPER "${CMAKE_SYSTEM_VERSION}" OS_VER )
string( TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" SYS_ARCH )
option(_DEBUG "Enables spewing of interal dprint statements" ON)

###########################################
# start cmake module inclusion
list(APPEND CMAKE_MODULE_PATH ${CONDOR_SOURCE_DIR}/build/cmake)
include (dprint) # set debug module 1st to all spewing in other modules if needed
include (AppendVar)
include (CommandTarget)
include (gSOAPGen)
include (CloneInstall)
include (FindMultiple)
include (ExternalProjectPatched)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)
include (CheckStructHasMember)
include (CheckTypeExists)
include (CheckLibraryExists)
include (CheckTypeSize)
include (PackageDate)
include (SystemSpecificInformations)
include (CondorPackageConfig) # setup packaging variables needed prior to install targets
include (CondorGlob)
include (CondorSetLinkLibs)
include (CondorExe)
include (CondorDaemon)
include (CondorStaticLib)
include (CheckCXXCompilerFlag)
include (CheckCXXSourceCompiles)
include (CondorConfigure)     # will initiate the configure
include (AddSubdirectories)
include (CondorPLTest)
include (CondorExeTest)

##########################################################################
# we only include the single sub to allow make at two levels.
# root - includes externals
# src - src -> only
# this way clean @ root cleans externals while clean @src cleans just the src.
add_subdirectory (src)

##########################################################################
# Must in included post *install targets
include ( CPack )

















