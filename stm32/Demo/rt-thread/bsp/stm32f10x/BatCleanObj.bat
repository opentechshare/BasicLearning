@rem ɾ��mdk��ʱ�ļ�
del /Q %project.uvgui*
del /Q %project.uvopt
del /Q %project_*.dep

@rem ɾ��scons��ʱ�ļ�
del /Q %.sconsign.dblite
del /Q %rtconfig.pyc
del /Q %rtthread-stm32.axf
del /Q %rtthread-stm32.map

@rem ɾ��������Ŀ���ļ�
del /Q %rtthread.bin

@rem ɾ����ʱ�ļ���
rd /Q /S %build

PAUSE