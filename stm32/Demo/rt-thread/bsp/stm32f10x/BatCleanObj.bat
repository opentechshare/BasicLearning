@rem 删除mdk临时文件
del /Q %project.uvgui*
del /Q %project.uvopt
del /Q %project_*.dep

@rem 删除scons临时文件
del /Q %.sconsign.dblite
del /Q %rtconfig.pyc
del /Q %rtthread-stm32.axf
del /Q %rtthread-stm32.map

@rem 删除构建的目标文件
del /Q %rtthread.bin

@rem 删除临时文件夹
rd /Q /S %build

PAUSE