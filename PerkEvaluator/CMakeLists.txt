#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.7)

include(ExternalProject)

#-----------------------------------------------------------------------------
set(MODULE_NAME "PerkEvaluator")
set(MODULE_TITLE "Perk Evaluator")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)


#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${vtkSlicerTransformRecorderModuleLogic_INCLUDE_DIRS}
  ${qSlicerTransformRecorderModuleWidgets_INCLUDE_DIRS}
  ${vtkSlicerSequencesModuleMRML_INCLUDE_DIRS}  
  ${vtkSlicerSequenceBrowserModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSequencesModuleLogic_INCLUDE_DIRS}  
  ${vtkSlicerSequenceBrowserModuleLogic_INCLUDE_DIRS}
  ${qSlicerSequenceBrowserModuleWidgets_INCLUDE_DIRS}
  )

set(MODULE_SRCS
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.cxx
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicerMetricScriptReader.cxx
  qSlicerMetricScriptReader.h
  )

set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicerMetricScriptReader.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicer${MODULE_NAME}Module.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  qSlicer${MODULE_NAME}ModuleWidgets
  vtkSlicerTransformRecorderModuleLogic
  qSlicerTransformRecorderModuleWidgets
  vtkSlicerSequencesModuleMRML
  vtkSlicerSequenceBrowserModuleMRML
  vtkSlicerSequencesModuleLogic
  vtkSlicerSequenceBrowserModuleLogic
  qSlicerSequenceBrowserModuleWidgets
  )
  
#-----------------------------------------------------------------------------
set(MODULE_RESOURCES
  Resources/qSlicer${MODULE_NAME}Module.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

