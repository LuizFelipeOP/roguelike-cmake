@echo off
echo Iniciando servidor em http://localhost:8766
echo Pressione Ctrl+C para parar.
echo.
python -m http.server 8766 --directory "%~dp0.."
