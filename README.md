
**Progg Shell** - ProggShell is a lightweight, cross platform (Windows, Linux) command-line shell <br>

>Note: In future Shell will be updated. Dont think its end! :3
---

>Note: Basic commands all commands will be added from PATH env

[Commands]<br>


ver    - Print Shell version<br>
quit , q , exit - Exit from Shell<br>
delay <ms> - Wait some time <br>
noflag - Disable flags<br>
exflag - Enable flags<br>
clv    - Clear view<br>
clh    - Clear Terminal History<br>
cd  <folder name or path to folder> - Enter in the folder<br>
histexport <name> - export Shell History file<br>
set-lng <language file name> - Sets language file to file in program folderм
if <arg1> <operator> <arg2> - Make logic if<br>


---
**Compilation**

<pre>gcc main.c modules/*.c -o ProggShell -s  -O2 -ledit -ffunction-sections -fdata-sections -Wl,--strip-all -Wl,--gc-sections -fno-asynchronous-unwind-tables -fno-ident</pre>  

<pre>clang main.c modules/*.c -o ProggShell -s  -O2 -ledit -ffunction-sections -fdata-sections -Wl,--strip-all -Wl,--gc-sections -fno-asynchronous-unwind-tables -fno-ident -Wl,--allow-multiple-definition</pre>
---


