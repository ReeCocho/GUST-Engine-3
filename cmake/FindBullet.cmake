# - Try to find libBullet
#
#  Bullet_FOUND - system has libBullet
#  Bullet_INCLUDE_DIRS - the libBullet include directories
#  Bullet_LIBRARIES - link these to use libBullet in release mode

FIND_PATH(
  Bullet_BASE
  NAMES btBulletDynamicsCommon.h
  PATHS ${CMAKE_SOURCE_DIR}/../bullet/src
  /usr/include/bullet
  /usr/local/include/bullet
)



FIND_LIBRARY(
  BulletDynamics_LIBRARY
  NAMES BulletDynamics
  PATHS ${Bullet_BASE}/.libs
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY(
  BulletCollision_LIBRARY
  NAMES BulletCollision
  PATHS ${Bullet_BASE}/.libs
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY(
  LinearMath_LIBRARY
  NAMES LinearMath
  PATHS ${Bullet_BASE}/.libs
  /usr/lib
  /usr/local/lib
)



FIND_LIBRARY(
  BulletDynamics_LIBRARY_DEBUG
  NAMES BulletDynamics_Debug
  PATHS ${Bullet_BASE}/.libs
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY(
  BulletCollision_LIBRARY_DEBUG
  NAMES BulletCollision_Debug
  PATHS ${Bullet_BASE}/.libs
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY(
  LinearMath_LIBRARY_DEBUG
  NAMES LinearMath_Debug
  PATHS ${Bullet_BASE}/.libs
  /usr/lib
  /usr/local/lib
)

IF(Bullet_BASE AND BulletDynamics_LIBRARY AND BulletCollision_LIBRARY AND LinearMath_LIBRARY AND 
BulletDynamics_LIBRARY_DEBUG AND BulletCollision_LIBRARY_DEBUG AND LinearMath_LIBRARY_DEBUG)
   SET(
	Bullet_LIBRARIES 
	optimized ${BulletDynamics_LIBRARY} 
	optimized ${BulletCollision_LIBRARY} 
	optimized ${LinearMath_LIBRARY}
	debug ${BulletDynamics_LIBRARY_DEBUG} 
	debug ${BulletCollision_LIBRARY_DEBUG} 
	debug ${LinearMath_LIBRARY_DEBUG}
   )
   SET(Bullet_FOUND TRUE)
   SET(Bullet_INCLUDE_DIRS ${Bullet_BASE})
ENDIF(Bullet_BASE AND BulletDynamics_LIBRARY AND BulletCollision_LIBRARY AND LinearMath_LIBRARY AND 
BulletDynamics_LIBRARY_DEBUG AND BulletCollision_LIBRARY_DEBUG AND LinearMath_LIBRARY_DEBUG)

IF(Bullet_FOUND)
   IF(NOT Bullet_FIND_QUIETLY)
      MESSAGE(STATUS "Found Bullet: ${Bullet_LIBRARIES}")
   ENDIF(NOT Bullet_FIND_QUIETLY)
ELSE(Bullet_FOUND)
   IF(Bullet_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Bullet")
   ENDIF(Bullet_FIND_REQUIRED)
ENDIF(Bullet_FOUND)