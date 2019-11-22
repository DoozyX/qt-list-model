add_library(list-models INTERFACE)

cmake_policy(SET CMP0076 NEW)

#TODO(doozy): refactor QVariantListModel
target_sources(list-models INTERFACE SmartDataModels.h ObjectListModelBase.h ObjectListModel.h)
target_include_directories(list-models INTERFACE ./)
