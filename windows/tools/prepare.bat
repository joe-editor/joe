@echo off

:: Top directory
cd %~dp0\..\..

:: Get rid of some top-level directories
rd /s /q .hg charmaps desktop htdocs man tests

:: Keep only some files at the top
mkdir keep
for %%f in (.editorconfig,NEWS.md,README.md,README1.md,Makefile.am,autojoe,configure.ac) DO (move %%f keep)
del /q *.*
move keep\*.* .
rd /s /q keep

:: Copy some more things into the top-level directory
copy windows\building.txt .
copy windows\docs\COPYING.rtf .

:: Random deletes
del windows\docs\backlog.txt

