
**Progg Shell** - ProggShell is a lightweight command-line shell written in C.<br>
It provides basic file management and system interaction.<br>

>Note: In future Shell will be updated. Dont think its end! :3
---

[Commands]<br>
ver    - Print Shell version<br>
hello  - Print "Hello world!"<br>
quit , q , exit - Exit from Shell<br>
fl,dir,ls - Show objects in current folder<br>
del <Path + Full File name> - Delete file<br>
delay <ms> - Wait some time <br>
prt <word> - Print string<br>
prtread <Path + Full File name> - Print file content<br>
noflag - Disable flags<br>
exflag - Enable flags<br>
run <command> - Send command to Windows<br>
crt <file name> - Creating or cleaning file<br>
cmd    - Open System Command Prompt<br>
clv    - Clear view<br>
cls    - Clear Screen<br>
clh    - Clear Terminal History<br>
cd  <folder name or path to folder> - Enter in the folder<br>
cop, cln <Path_To_File> <Path_To_Duplicate> - Create Duplicate of file<br>
create <object_name> - create object (For extensions developers)<br>
cpu-load - Show CPU load<br>
mem-load - Show RAM load<br>
mdr <folder name> - Create folder in current path<br>
mov <new_Path> - Move file<br>
exec <Path + Full File name> - Execute file<br>
export <name> - export Shell History file<br>
set-lng <language file name> - Sets language file to file in program folderм
ren <new_file_name> - Rename file<br>
neo <file_name or new_file_name> - Edit file with Neo<br>
if <arg1> <operator> <arg2> - Make logic if<br>


[Parameters]<br>
{q     - Quit from shell after finishing program<br>
{s     - Shut Down after finishing program<br>
{d     - Show executing string<br>

---
**Compilation**

<pre>gcc main.c modules/*.c -o ProggShell -s  -O2 -lreadline -lncursesw -lm   -ffunction-sections -fdata-sections  -Wl,--exclude-all-symbols -Wl,--strip-all -Wl,--gc-sections</pre>  

---


