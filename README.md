## BUSH - Bhargav's Useless SHEll

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
- All other commands are executed by creating a child process and then using `execvp`.
- Upcoming builtins - `ls`, `fg`, `bg`, `jobs`, `exit`, `pinfo`, `history`, `discover`.

Jobs:
- The jobs system is incomplete.
- Managed to send processes to background by changing their process group id, but have to implement a system to keep track of these and implement the fg and bg commands to move processes back and forth.
- `&` will put the preceding process it in the background.
- `;` will run the preceding process in foreground in the foreground.
- Using this [link](https://spin0r.wordpress.com/2012/12/28/terminally-confused-part-seven/) as reference to implement jobs.

### Plans:  
- The plan is to sort out the jobs system first, then start implementing the builtins.
- Deal with left and right arrows (eg: compiling with readline)
- `aliases`, autosuggestions, tab, up arrow.
- Pipes