#pragma once
// USE these commands to create a .cu file 
//nvcc -arch sm_89 -I"C:\ProgramData\NVIDIA Corporation\OptiX SDK 8.1.0\include" -ptx -lineinfo Accu.cu -o embedded_ptx_code_Accu.ptx
//nvcc -arch sm_89 -I"C:\ProgramData\NVIDIA Corporation\OptiX SDK 8.1.0\include" -ptx -lineinfo Pack.cu -o embedded_ptx_code_Pack.ptx
//bin2c embedded_ptx_code_Accu.ptx > Accu_embedded_ptx_code.c
//bin2c  embedded_ptx_code_Pack.ptx > Pack_embedded_ptx_code.c
//compute-sanitizer --tool memcheck C:/DirectX11/bin/Debug-windows-x86_64/Game/Game.exe