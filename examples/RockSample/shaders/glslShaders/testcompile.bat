C:\VulkanSDK\1.3.275.0\Bin\glslc.exe terrain.frag -o ../terrain.frag.spv
C:\VulkanSDK\1.3.275.0\Bin\glslc.exe terrain.vert -o ../terrain.vert.spv
C:\VulkanSDK\1.3.275.0\Bin\glslc.exe terrain.tese -o ../terrain.tese.spv
C:\VulkanSDK\1.3.275.0\Bin\glslc.exe terrain.tesc -o ../terrain.tesc.spv

REM C:\VulkanSDK\1.3.275.0\Bin\glslc.exe -x glsl -fshader-stage=mesh --target-spv=spv1.5 grass.mesh -o ../grass.mesh.spv
C:\VulkanSDK\1.3.275.0\Bin\glslc.exe -x glsl -fshader-stage=mesh --target-spv=spv1.5 grass_bob.mesh -o ../grass.mesh.spv
C:\VulkanSDK\1.3.275.0\Bin\glslc.exe -x glsl -fshader-stage=task --target-spv=spv1.5 grass.task -o ../grass.task.spv
C:\VulkanSDK\1.3.275.0\Bin\glslc.exe grass.frag -o ../grass.frag.spv