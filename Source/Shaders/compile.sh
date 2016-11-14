#!/bin/sh

OUTPUT_DIRECTORY=../../Resources/Data/Shaders

glslangValidator -V depth_vertex.vert -o ${OUTPUT_DIRECTORY}/depth_vertex.spv
glslangValidator -V depth_anim_vertex.vert -o ${OUTPUT_DIRECTORY}/depth_anim_vertex.spv
glslangValidator -V depth_terrain_vertex.vert -o ${OUTPUT_DIRECTORY}/depth_terrain_vertex.spv
glslangValidator -V depth_fragment.frag -o ${OUTPUT_DIRECTORY}/depth_fragment.spv

glslangValidator -V light_culling.comp -o ${OUTPUT_DIRECTORY}/light_culling.spv

glslangValidator -V vertex.vert -o ${OUTPUT_DIRECTORY}/vertex.spv
glslangValidator -V anim_vertex.vert -o ${OUTPUT_DIRECTORY}/anim_vertex.spv
glslangValidator -V terrain_vertex.vert -o ${OUTPUT_DIRECTORY}/terrain_vertex.spv
glslangValidator -V phong.frag -o ${OUTPUT_DIRECTORY}/phong.spv

glslangValidator -V skysphere_vertex.vert -o ${OUTPUT_DIRECTORY}/skysphere_vertex.spv
glslangValidator -V skysphere_fragment.frag -o ${OUTPUT_DIRECTORY}/skysphere_fragment.spv

glslangValidator -V gui_vertex.vert -o ${OUTPUT_DIRECTORY}/gui_vertex.spv
glslangValidator -V gui_fragment.frag -o ${OUTPUT_DIRECTORY}/gui_fragment.spv

glslangValidator -V fullscreen_vertex.vert -o ${OUTPUT_DIRECTORY}/fullscreen_vertex.spv

glslangValidator -V pp_hdr.frag -o ${OUTPUT_DIRECTORY}/pp_hdr.spv
glslangValidator -V pp_blur.frag -o ${OUTPUT_DIRECTORY}/pp_blur.spv
glslangValidator -V pp_dof.frag -o ${OUTPUT_DIRECTORY}/pp_dof.spv

glslangValidator -V ssao.frag -o ${OUTPUT_DIRECTORY}/ssao.spv
glslangValidator -V ssao_blur.frag -o ${OUTPUT_DIRECTORY}/ssao_blur.spv
