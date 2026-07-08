@echo off
echo Starting 8 parallel Fenrir self-play instances...

REM Launch 8 instances in the background
start "Fenrir SelfPlay 1" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800
start "Fenrir SelfPlay 2" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800
start "Fenrir SelfPlay 3" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800
start "Fenrir SelfPlay 4" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800
start "Fenrir SelfPlay 5" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800
start "Fenrir SelfPlay 6" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800
start "Fenrir SelfPlay 7" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800
start "Fenrir SelfPlay 8" bin\Release\fenrir.exe --selfplay --games 1000000 --simulations 800

echo All instances launched in separate windows!
