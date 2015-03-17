@echo off
call python build\vs_toolchain.py update
call python build\util\lastchange.py -o build\util\LASTCHANGE
call python build\util\lastchange.py -s third_party/WebKit -o build\util\LASTCHANGE.blink
call python build\gyp_chromium
