
@echo off 
SET original="%appdata%\Spotify\chrome_elf.dll"
SET original_backup="%appdata%\Spotify\_chrome_elf.dll"

taskkill /F /IM Spotify.exe

echo f | xcopy /y /f %original_backup% %original%

echo successful uninstalling

:end_code
pause > nul