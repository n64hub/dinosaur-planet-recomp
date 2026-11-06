# Patches lib/rt64, lib/N64ModernRuntime and lib/RecompFrontend: N64Recomp-browser's patches/ (its apply.cmake), then
# this game's own changes to them on top, runtime-patches/<project>-<n>-*.patch in order of name (README.md here).
# Doing it again changes nothing.
#
# The game's are taken out first and put back after: N64Recomp-browser's check that its patch is in, for a checkout it
# patched with a three-way merge, is that the checkout's changes are what they were when it went in, and the game's
# change them.
#
#   cmake -P runtime-patches/apply.cmake          from the repo's root (tools/build_n64recomp.sh)
#   include(runtime-patches/apply.cmake)          and dq_apply_patches() (CMakeLists.txt)

set(DQ_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
include("${DQ_ROOT}/lib/N64Recomp-browser/patches/apply.cmake")

# Where a project's checkout is: lib/<project>, but N64Recomp is N64ModernRuntime's submodule.
function(dq_patch_dir out root project)
    if (project STREQUAL "N64Recomp")
        set(${out} "${root}/lib/N64ModernRuntime/N64Recomp" PARENT_SCOPE)
    else()
        set(${out} "${root}/lib/${project}" PARENT_SCOPE)
    endif()
endfunction()

function(dq_apply_patches)
    get_filename_component(root "${DQ_ROOT}" ABSOLUTE)
    file(GLOB patches "${root}/runtime-patches/*.patch")
    list(SORT patches)
    list(REVERSE patches)
    foreach (patch IN LISTS patches)
        get_filename_component(name "${patch}" NAME)
        string(REGEX REPLACE "-[0-9]+-.*" "" project "${name}")
        dq_patch_dir(dir "${root}" "${project}")
        execute_process(COMMAND git apply --whitespace=nowarn --reverse "${patch}"
            WORKING_DIRECTORY "${dir}" RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
    endforeach()
    list(REVERSE patches)

    n64recomp_browser_apply_patches("${root}/lib")

    foreach (patch IN LISTS patches)
        get_filename_component(name "${patch}" NAME)
        string(REGEX REPLACE "-[0-9]+-.*" "" project "${name}")
        dq_patch_dir(dir "${root}" "${project}")
        execute_process(COMMAND git apply --whitespace=nowarn "${patch}"
            WORKING_DIRECTORY "${dir}" RESULT_VARIABLE result ERROR_VARIABLE error)
        if (NOT result EQUAL 0)
            message(FATAL_ERROR "${name} doesn't apply to ${dir}:\n${error}")
        endif()
    endforeach()
endfunction()

if (CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    dq_apply_patches()
endif()
