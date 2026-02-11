function(target_enable_clang_format target)
    if(CLANG_FORMAT_EXE)
        get_target_property(TARGET_SOURCES ${target} SOURCES)
        if(NOT TARGET_SOURCES)
            return()
        endif()

        add_custom_target(format_${target}
            COMMAND ${CLANG_FORMAT_EXE}
                    -i
                    --style=file
                    ${TARGET_SOURCES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Running clang-format on target ${target}"
            COMMAND_EXPAND_LISTS
        )

        add_dependencies(${target} format_${target})
    else()
        message(STATUS "Formatting disabled for ${target}")
    endif()
endfunction()

function(target_enable_clang_tidy target)
    if(ENABLE_CLANG_TIDY)
        get_target_property(TARGET_SOURCES ${target} SOURCES)
        if(NOT TARGET_SOURCES)
            return()
        endif()

        add_custom_target(tidy_${target}
            COMMAND ${CLANG_TIDY_EXE}
                    -p "${CMAKE_BINARY_DIR}"
                    ${TARGET_SOURCES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Running clang-tidy on target ${target}"
            COMMAND_EXPAND_LISTS
        )

        add_dependencies(${target} tidy_${target})
    else()
        message(STATUS "Analysis disabled for ${target}")
    endif()
endfunction()

function(target_compiler_warnings target)
    # https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md#compilers
    set(MSVC_WARNINGS
        /W4 # Baseline reasonable warnings
        /w14242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        /w14263 # 'function': member function does not override any base class virtual member function
        /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
                # be destructed correctly
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
                # the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555 # expression has no effect; expected expression with side- effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        /permissive- # standards conformance mode for MSVC compiler.
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
        # catch hard to track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wpedantic # warn if non-standard C++ is used
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
    )

    if(WARNINGS_AS_ERRORS)
        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
    endif()

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
        -Wsuggest-override # warn if an overridden member function is not marked 'override' or 'final'
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${target} PRIVATE ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE ${GCC_WARNINGS})
    else()
        message(WARNING "No compiler warnings set for CXX compiler: '${CMAKE_CXX_COMPILER_ID}'")
    endif()
endfunction()

function(target_export_api target api_name)
    if(BUILD_SHARED_LIBS)
        set_target_properties(${target} PROPERTIES API ${api_name})

        if(MSVC)
            target_compile_definitions(
                ${target} PRIVATE "${api_name}=__declspec(dllexport)"
            )
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_definitions(
                ${target} PRIVATE "${api_name}=__attribute__((visibility(\"default\")))"
            )
        endif()
    else()
        target_compile_definitions(
            ${target} PUBLIC ${api_name}=
        )
    endif()
endfunction()

function(target_import_api target target_to_import)
    if(BUILD_SHARED_LIBS)
        get_target_property(api_name ${target_to_import} API)

        if(NOT api_name)
            message(FATAL_ERROR "Could not find API property on ${target_to_import}. Did you call target_export_api on it first?")
        endif()

        if(MSVC)
            target_compile_definitions(
                ${target} PUBLIC "${api_name}=__declspec(dllimport)"
            )
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_definitions(
                ${target} PUBLIC "${api_name}=__attribute__((visibility(\"default\")))"
            )
        endif()
    endif()
endfunction()
