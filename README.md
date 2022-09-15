## BUSH - Bhargav's Useless SHell

### Compiling:
In the directory in which the code is present execute
> $ make all  

### Running:
> ./bush  

or pass a file like  
> ./bush a.sh  

and it will execute it although its not very useful now.


### Progress:

Commands:  
- `echo` has been implemented, but no escape sequences, quotes or flags other than -n have been implemented. Environment variables have not been implemented.
- `cd` takes the directory in which the shell was invoked as the HOME directory. `.`, `..`, `~`, `-` have been implemented.
- The working of `cd` is slightly different. Eg: `~/../~` would give error in bash but takes you to home directory in bush. Can modify the code to check for ~ after first char but going to home directory makes more sense imo. Also this scenario is unlikely to happen.
- `pwd` prints the absolute path of the current working directory
- `ls` will list all files in folder. There is some problem with symlinks. They are not getting recognised. I did not follow any specification, so may not be identical to any standard shell. Executables are colored green if any of the creator, group or others have the execute permission.
- `pinfo`. Without any pid, will show the info of bush, otherwise gives info of the given pid. Getting info from `/proc/pid/stat`. 
- `history` stores the command history in `.bush_history` in the home folder (the pwd when you started bush). So starting bush from different places would lead to different histories. Somewhat unstable, the cached history file tends to mess things up a bit. Look at plans.  
- `discover` if file name is given in "quotes", it will search for that filename, either total match or a substring match. `-d` will only display directories .`-f` only files, unless both are set. Set the `MAX_DEPTH` appropriately as I am loading all the file names into main memory and you might run out of it. discover in root directory hangs my laptop, so cant really debug there.
- `exit` will exit.
- Did not handle spaces in file names in any of the commands.
- There might be a lot of edge cases I missed.
- All other commands are executed by creating a child process and then using `execvp`.
- Upcoming builtins - `fg`, `bg`.
- Any command which takes more than 1 sec will have its time take printed in the next prompt.

Jobs:
- Managed to send processes to background by changing their process group id
- A uniq number is given to each of the process (fg or bg)
- `jobs` command will list all the non-dead process spawned by the sheell.
- `&` will put the preceding process it in the background.
- `;` will run the preceding process in foreground in the foreground.
- Exitcodes and Signals are printed for processes not done normally. Using `strsignal` to print signal message. In higher GLIBC versions, `sigdescpr_np` would be a better option ig, but i dont have a higher GLIBC version.
- Using this [link](https://spin0r.wordpress.com/2012/12/28/terminally-confused-part-seven/) as reference to implement jobs.
- When a background process ends, it is printed along with job id and exit code if exited abnormally.  

### Signals:
- blocked SIGINT, SIGTSP
- Exits if Ctrl-D (ik not signal, just EOF)


### Plans:  
- Fix bugs in histoy (fadvise?)
- arrow movement
- tab completion
- `aliases`
- Pipes