# 
# GLM is a header-only library. 
#  On windows, set env var GLM_ROOT_DIR 
# 
# Caution, old versions of GLM use .h extension 
# 
 
IF(WIN32) 
    FIND_PATH( GLM_INCLUDE_DIR glm/glm.hpp 
            $ENV{PROGRAMFILES}/glm 
            ${GLM_ROOT_DIR}/include 
            $ENV{GLM_ROOT_DIR}/include 
            $ENV{GLM_ROOT_DIR} 
            DOC "The directory where glm/glm.hpp resides") 
ELSE(WIN32) 
    FIND_PATH( GLM_INCLUDE_DIR glm/glm.hpp 
            /usr/include 
            /usr/local/include 
            /sw/include 
            /opt/local/include 
            ${GLM_ROOT_DIR}/include 
            $ENV{GLM_ROOT_DIR}/include 
            $ENV{GLM_ROOT_DIR} 
            DOC "The directory where glm/glm.hpp resides" 
            ) 
ENDIF(WIN32) 
 
IF(GLM_INCLUDE_DIR) 
    SET(GLM_FOUND "YES") 
    MESSAGE(STATUS "Found GLM.") 
ENDIF(GLM_INCLUDE_DIR)
