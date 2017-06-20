@REM Add pre-link commands below.

@if not [%SYSGEN_CMEM%] == [1] goto:EOF

build -c
