crinkler.exe /OUT:4klang.exe /CRINKLER /PRIORITY:NORMAL /COMPMODE:SLOW /PROGRESSGUI /TRANSFORM:CALLS /PRINT:IMPORTS /PRINT:LABELS /REPORT:report.html /ORDERTRIES:10000 /HASHTRIES:50 /HASHSIZE:200 /TRUNCATEFLOATS:32 main.obj 4klang.obj user32.lib kernel32.lib winmm.lib