#!/usr/bin/env bash
set -e

echo "============================================"
echo "  Build Minecraft Clone -> executable"
echo "============================================"

echo "[1/3] Installation des dependances..."
pip install -r requirements.txt

echo "[2/3] Compilation avec PyInstaller..."
pyinstaller --onefile --windowed --name "MinecraftClone" main.py

echo "[3/3] Termine ! Executable : dist/MinecraftClone"
