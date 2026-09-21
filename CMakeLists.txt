cmake_minimum_required(VERSION 3.16)

project(calamares-modules-nitrux VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$" AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
    check_cxx_compiler_flag("-march=x86-64-v3" NITRUX_HAS_X86_64_V3)
    if(NITRUX_HAS_X86_64_V3)
        add_compile_options(-march=x86-64-v3)
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Calamares 3.4.2 CONFIG REQUIRED)

if(Calamares_VERSION VERSION_GREATER_EQUAL 3.5.0)
    message(FATAL_ERROR "This module vendors the Calamares 3.4 partition backend")
endif()

if(NOT Calamares_WITH_QML)
    message(FATAL_ERROR "The installed Calamares package was built without QML support")
endif()

if(NOT Calamares_WITH_QT6)
    message(FATAL_ERROR "The installed Calamares package was not built with Qt 6 support")
endif()

find_package(Qt6 CONFIG REQUIRED Concurrent DBus)
find_package(KF6I18n CONFIG REQUIRED)
find_package(KF6WidgetsAddons CONFIG REQUIRED)
find_package(KPMcore 24.01.75 REQUIRED)

if(KPMcore_VERSION VERSION_GREATER_EQUAL 27.0.0)
    message(FATAL_ERROR "KPMcore 27.x and newer are not validated with the vendored Calamares 3.4 partition backend")
endif()

include(CalamaresAddPlugin)

set(_partition_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/partition")

calamares_add_plugin(partitionq
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        src/partitionq/ChoicePageQml.cpp
        src/partitionq/FlatPartitionModel.cpp
        src/partitionq/PartitionQmlViewStep.cpp
        ${_partition_source_dir}/Config.cpp
        ${_partition_source_dir}/core/BootLoaderModel.cpp
        ${_partition_source_dir}/core/ColorUtils.cpp
        ${_partition_source_dir}/core/DeviceList.cpp
        ${_partition_source_dir}/core/DeviceModel.cpp
        ${_partition_source_dir}/core/DirFSRestrictLayout.cpp
        ${_partition_source_dir}/core/KPMHelpers.cpp
        ${_partition_source_dir}/core/OsproberEntry.cpp
        ${_partition_source_dir}/core/PartUtils.cpp
        ${_partition_source_dir}/core/PartitionActions.cpp
        ${_partition_source_dir}/core/PartitionCoreModule.cpp
        ${_partition_source_dir}/core/PartitionInfo.cpp
        ${_partition_source_dir}/core/PartitionLayout.cpp
        ${_partition_source_dir}/core/PartitionModel.cpp
        ${_partition_source_dir}/jobs/AutoMountManagementJob.cpp
        ${_partition_source_dir}/jobs/ChangeFilesystemLabelJob.cpp
        ${_partition_source_dir}/jobs/ClearMountsJob.cpp
        ${_partition_source_dir}/jobs/ClearTempMountsJob.cpp
        ${_partition_source_dir}/jobs/CreatePartitionJob.cpp
        ${_partition_source_dir}/jobs/CreatePartitionTableJob.cpp
        ${_partition_source_dir}/jobs/CreateVolumeGroupJob.cpp
        ${_partition_source_dir}/jobs/DeactivateVolumeGroupJob.cpp
        ${_partition_source_dir}/jobs/DeletePartitionJob.cpp
        ${_partition_source_dir}/jobs/FillGlobalStorageJob.cpp
        ${_partition_source_dir}/jobs/FormatPartitionJob.cpp
        ${_partition_source_dir}/jobs/PartitionJob.cpp
        ${_partition_source_dir}/jobs/RemoveVolumeGroupJob.cpp
        ${_partition_source_dir}/jobs/ResizePartitionJob.cpp
        ${_partition_source_dir}/jobs/ResizeVolumeGroupJob.cpp
        ${_partition_source_dir}/jobs/SetPartitionFlagsJob.cpp
    LINK_PRIVATE_LIBRARIES
        Qt6::Concurrent
        Qt6::DBus
        KF6::I18n
        KF6::WidgetsAddons
        kpmcore
    SHARED_LIB
)

target_include_directories(${partitionq_TARGET}
    PRIVATE
        "${_partition_source_dir}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/partitionq"
)

if(DEFINED KPMcore_VERSION_MAJOR AND DEFINED KPMcore_VERSION_MINOR AND DEFINED KPMcore_VERSION_PATCH)
    math(EXPR _kpmcore_api_version
        "(0x${KPMcore_VERSION_MAJOR} * 0x10000) + (0x${KPMcore_VERSION_MINOR} * 0x100) + 0x${KPMcore_VERSION_PATCH}"
        OUTPUT_FORMAT HEXADECIMAL
    )
    target_compile_definitions(${partitionq_TARGET} PRIVATE WITH_KPMcore=${_kpmcore_api_version})
else()
    message(FATAL_ERROR "The KPMcore package did not provide a complete version")
endif()


add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            "${CMAKE_INSTALL_FULL_LIBDIR}/calamares/modules/partitionq"
)
