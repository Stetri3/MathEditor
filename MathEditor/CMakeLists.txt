# CMakeList.txt: progetto CMake per MathEditor. Includere l'origine e definire
# qui la logica specifica del progetto.
#

# Aggiungere l'origine all'eseguibile di questo progetto.
add_executable (MathEditor "MathEditor.cpp"
"MathEditor.h" "utils.h"
"utils.cpp" "fonts.h"
"fonts.cpp" "text.h"
"text.cpp" "widget/widgets.h"
"widget/widgets.inl" "widget/widget_catalog.h"
"widget/widget_def.h" "widget/widgets.cpp"
"widget/widgets_g.inl" "utils/file_utils.hpp"
"utils/type_utils.hpp"
"widget/debug_load.h" "widget/debug_load.cpp" "config.h" "rendering/render.h" "rendering/render.cpp" "rendering/render_def.h")
set_property(TARGET MathEditor PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET MathEditor PROPERTY CXX_STANDARD 20)
set_source_files_properties(widgets.inl PROPERTIES HEADER_FILE_ONLY TRUE)
# TODO: Aggiungere i test e, se necessario, installare le destinazioni.

target_precompile_headers(MathEditor PRIVATE config.h)

set(LIBDIR "C:/Dev/Libs")
set(SDL3_DIR "${LIBDIR}/sdl3_dist")
find_package(SDL3 REQUIRED)
set(LINKED SDL3::SDL3)

# add_subdirectory(third_party)

target_link_libraries(MathEditor PRIVATE ${LINKED})

#Aggiunge SLD3.lib all'out
if(WIN32)
    add_custom_command(TARGET MathEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3::SDL3>" 
        "$<TARGET_FILE_DIR:MathEditor>"
    )
endif()
add_custom_command(
    TARGET MathEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/asset"
            "$<TARGET_FILE_DIR:MathEditor>/asset"
    COMMENT "Copia della cartella assets in corso..."
)
