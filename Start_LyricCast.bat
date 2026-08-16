@echo off
title LyricCast Backend Server
color 0b

echo ====================================================
echo            STARTING LYRICCAST SERVER
echo ====================================================
echo.

cd /d "%~dp0python"
set PYTHONIOENCODING=utf-8

echo [1/2] Launching Python Engine...
echo [2/2] Connecting to ESP32...
echo.
echo Dashboard is available at: http://localhost:8080/
echo Type 'stop' or 'quit' in this window to stop the server anytime.
echo.
echo ====================================================

python -u spotify_lyrics.py

echo.
echo Server has stopped.
pause
