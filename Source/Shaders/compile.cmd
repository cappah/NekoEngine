@ECHO OFF

set OUTPUT_DIRECTORY=..\..\Resources\Data\Shaders

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V depth_vertex.vert -o %OUTPUT_DIRECTORY%\depth_vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V depth_anim_vertex.vert -o %OUTPUT_DIRECTORY%\depth_anim_vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V depth_terrain_vertex.vert -o %OUTPUT_DIRECTORY%\depth_terrain_vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V depth_fragment.frag -o %OUTPUT_DIRECTORY%\depth_fragment.spv

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V light_culling.comp -o %OUTPUT_DIRECTORY%\light_culling.spv

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V vertex.vert -o %OUTPUT_DIRECTORY%\vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V anim_vertex.vert -o %OUTPUT_DIRECTORY%\anim_vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V terrain_vertex.vert -o %OUTPUT_DIRECTORY%\terrain_vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V phong.frag -o %OUTPUT_DIRECTORY%\phong.spv

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V skysphere_vertex.vert -o %OUTPUT_DIRECTORY%\skysphere_vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V skysphere_fragment.frag -o %OUTPUT_DIRECTORY%\skysphere_fragment.spv

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V gui_vertex.vert -o %OUTPUT_DIRECTORY%\gui_vertex.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V gui_fragment.frag -o %OUTPUT_DIRECTORY%\gui_fragment.spv

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V fullscreen_vertex.vert -o %OUTPUT_DIRECTORY%\fullscreen_vertex.spv

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V pp_hdr.frag -o %OUTPUT_DIRECTORY%\pp_hdr.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V pp_blur.frag -o %OUTPUT_DIRECTORY%\pp_blur.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V pp_dof.frag -o %OUTPUT_DIRECTORY%\pp_dof.spv

"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V ssao.frag -o %OUTPUT_DIRECTORY%\ssao.spv
"%VK_SDK_PATH%\Bin\glslangValidator.exe" -V ssao_blur.frag -o %OUTPUT_DIRECTORY%\ssao_blur.spv
pause
