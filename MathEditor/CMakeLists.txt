# CMakeList.txt: progetto CMake per MathEditor. Includere l'origine e definire
# qui la logica specifica del progetto.
#

# Aggiungere l'origine all'eseguibile di questo progetto.
add_executable (MathEditor "MathEditor.cpp"
"MathEditor.h" "utils.h"
"utils.cpp" "fonts.h"
"fonts.cpp" "text.h"
"text.cpp" "widgets.h"
"widgets.inl" "widget_catalog.h"
"widget_def.h" "widgets.cpp"
"widgets_g.inl" "utils/file_utils.hpp")
set_property(TARGET MathEditor PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET MathEditor PROPERTY CXX_STANDARD 20)
set_source_files_properties(widgets.inl PROPERTIES HEADER_FILE_ONLY TRUE)
# TODO: Aggiungere i test e, se necessario, installare le destinazioni.

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
