A simple shell simulation(much like the bash shell of Linux). A shell takes
in user input, forks one or more child processes using the fork system call, calls
exec() from these children to execute user commands, and reaps the dead children
using the wait system call.

On executing, the shell starts as "<username>:\~$"
where “~” is the path where we are currently at. The shell will terminate on pressing
Ctrl+C. An incorrect number of arguments or incorrect command format prints an error
in the shell. The shell only executes simple commands (commands which are readily available as
executables and can be invoked using exec system calls) like ls, cat, echo, sleep.

Implemented a "history" command which returns the last 5 commands.
