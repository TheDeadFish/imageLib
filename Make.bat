@setlocal
@call egcc.bat
@pushd obj
@SET SRC=..\imagelib\color.cpp ..\imagelib\imageLib.cc ..\imagelib\drawing.cpp ..\imagelib\quantize\quantize.cpp ..\imagelib\resample\resample.cpp
@gcc %SRC% %CCFLAGS2% -I..\imagelib -c
@if %errorlevel% neq 0 goto END
@popd

@md %PROGRAMS%\local\include\imagelib
@copy /Y imagelib\*.h %PROGRAMS%\local\include\imagelib
@copy /Y imagelib\quantize\*.h %PROGRAMS%\local\include\imagelib
@copy /Y imagelib\resample\*.h %PROGRAMS%\local\include\imagelib
@ar -rcs  %PROGRAMS%\local\lib32\libimglib.a obj\*.o
:END
@endlocal
