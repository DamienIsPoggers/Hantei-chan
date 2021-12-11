SET(imgui_PATH "${PROJECT_SOURCE_DIR}/imgui")

if(NOT EXISTS "${imgui_PATH}/imgui.h")
	message(FATAL_ERROR "Imgui not found. Did you forget to init the submodules?")
endif()

file(GLOB imgui_CPP
	"${imgui_PATH}/*.cpp"
)

add_library(imgui
	"${imgui_PATH}/backends/imgui_impl_opengl3.cpp"
	"${imgui_PATH}/backends/imgui_impl_win32.cpp"
	"${PROJECT_SOURCE_DIR}/imgui_text/imgui_input_text.cpp"
	${imgui_CPP}
)

target_include_directories(imgui PUBLIC
	"${imgui_PATH}"
	"${imgui_PATH}/backends"
	"${PROJECT_SOURCE_DIR}/imgui_text/"
	config
)


target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD IMGUI_IMPL_OPENGL_ES2 IMGUI_USER_CONFIG=<config.h> )
target_link_libraries(imgui PUBLIC glad Imm32 Dwmapi)