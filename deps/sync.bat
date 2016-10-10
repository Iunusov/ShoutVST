@echo off

echo Sync started.

echo libmp3lame...
if not exist libmp3lame-CMAKE (
    (git clone https://github.com/R-Tur/libmp3lame-CMAKE.git)
) else (
    (cd libmp3lame-CMAKE) && (git pull) && (cd ..)
)
echo OGG-Vorbis...
if not exist OGG-Vorbis-CMAKE (
    (git clone https://github.com/R-Tur/OGG-Vorbis-CMAKE.git)
) else (
    (cd OGG-Vorbis-CMAKE) && (git pull) && (cd ..)
)
echo libshout...
if not exist libshout-CMAKE (
    (git clone https://github.com/R-Tur/libshout-CMAKE.git)
) else (
    (cd libshout-CMAKE) && (git pull) && (cd ..)
)

echo FLTK...
if not exist FLTK (
    (git clone https://github.com/IngwiePhoenix/FLTK.git)
) else (
    (cd FLTK) && (git pull) && (cd ..)
)

echo VST_SDK_2.4...
if not exist VST_SDK_2.4 (
    (git clone https://github.com/R-Tur/VST_SDK_2.4.git)
) else (
    (cd VST_SDK_2.4) && (git pull) && (cd ..)
)


echo Done.
