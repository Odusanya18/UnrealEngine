#
# Build LowLevel
#

SET(PHYSX_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../)

SET(LL_SOURCE_DIR ${PHYSX_SOURCE_DIR}/LowLevel)

FIND_PACKAGE(nvToolsExt REQUIRED)

SET(LOWLEVEL_PLATFORM_INCLUDES
	${NVTOOLSEXT_INCLUDE_DIRS}
	${PHYSX_SOURCE_DIR}/Common/src/ios
	${PHYSX_SOURCE_DIR}/LowLevel/software/include/ios
	${PHYSX_SOURCE_DIR}/LowLevelDynamics/include/ios
	${PHYSX_SOURCE_DIR}/LowLevel/common/include/pipeline/ios
)

SET(LOWLEVEL_COMPILE_DEFS
	# Common to all configurations
	${PHYSX_IOS_COMPILE_DEFS};PX_PHYSX_STATIC_LIB

	$<$<CONFIG:debug>:${PHYSX_IOS_DEBUG_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=DEBUG;>
	$<$<CONFIG:checked>:${PHYSX_IOS_CHECKED_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=CHECKED;>
	$<$<CONFIG:profile>:${PHYSX_IOS_PROFILE_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=PROFILE;>
	$<$<CONFIG:release>:${PHYSX_IOS_RELEASE_COMPILE_DEFS};>
)

# include common low level settings
INCLUDE(../common/LowLevel.cmake)

SET_TARGET_PROPERTIES(LowLevel PROPERTIES LINK_FLAGS ""	)

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(LowLevel PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
