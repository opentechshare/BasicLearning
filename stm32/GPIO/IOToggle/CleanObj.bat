del /Q %~dp0\Project\MDK-ARM(uV4)\Obj\*.*
del /Q %~dp0\Project\MDK-ARM(uV4)\List\*.*
del /Q %~dp0\Project\MDK-ARM(uV4)\JLink*
del /Q %~dp0\Project\MDK-ARM(uV4)\IOToggle.uvgui.*
del /Q %~dp0\Project\MDK-ARM(uV4)\*.bak
del /Q %~dp0\Project\MDK-ARM(uV4)\*.dep
rd /Q /S %~dp0\Project\MDK-ARM(uV4)\Obj
rd /Q /S %~dp0\Project\MDK-ARM(uV4)\List

del /Q %~dp0\Project\EWARMv5\IOToggle.dep
del /Q %~dp0\Project\EWARMv5\Flash
del /Q %~dp0\Project\EWARMv5\CpuRam
del /Q %~dp0\Project\EWARMv5\ExtSram
rd /Q /S %~dp0\Project\EWARMv5\Flash
rd /Q /S %~dp0\Project\EWARMv5\CpuRam
rd /Q /S %~dp0\Project\EWARMv5\ExtSram

PAUSE