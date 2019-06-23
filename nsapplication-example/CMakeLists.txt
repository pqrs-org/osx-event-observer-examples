cmake_minimum_required (VERSION 3.9)
project(nsapplication-example)
enable_language(Swift)

add_executable(
  nsapplication-example
  MACOSX_BUNDLE
  src/AppDelegate.swift
  src/ExampleApplication.swift
  Resources/MainMenu.xib
  Resources/Assets.xcassets
  nsapplication_example.entitlements
)

set_source_files_properties(
  Resources/MainMenu.xib
  Resources/Assets.xcassets
  PROPERTIES
  MACOSX_PACKAGE_LOCATION
  Resources
)

set_target_properties(
  nsapplication-example
  PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST
  ${CMAKE_CURRENT_LIST_DIR}/Resources/Info.plist
)

set_target_properties(
  nsapplication-example
  PROPERTIES
  XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
  "@loader_path @loader_path/../Frameworks"
)
