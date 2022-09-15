#include "input.h"

void die(const char *s) {
    perror(s);
    exit(1);
}

struct termios orig_termios;

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

// int max(int a, int b){
//     return (a<b)?b:a;    
// }
// int min(int a, int b){
//     return (a>b)?b:a;    
// }
/**
 * Enable row mode for the terminal
 * The ECHO feature causes each key you type to be printed to the terminal, so you can see what you’re typing.
 * Terminal attributes can be read into a termios struct by tcgetattr().
 * After modifying them, you can then apply them to the terminal using tcsetattr().
 * The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.
 * The c_lflag field is for “local flags”
 */
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/**
 * stdout and stdin are buffered we disable buffering on that
 * After entering in raw mode we read characters one by one
 * Up arrow keys and down arrow keys are represented by 3 byte escape codes
 * starting with ascii number 27 i.e. ESC key
 * This way we interpret arrow keys
 * Tabs are usually handled by the term, but here we are simulating tabs for the sake of simplicity
 * Backspace move the cursor one control character to the left
 * @return
 */
int input_fancy(char* cmd, char* home) {
    int bufsize = 10000;
    char *inp = malloc(sizeof(char) * bufsize);
    char* save_inp = malloc(sizeof(char) * bufsize);
    char c;
    // while (1) {
        setbuf(stdout, NULL);
        enableRawMode();
        printf("|====$ ");
        
        // fputs(CSI "1;1", stdout);
        memset(inp, '\0', bufsize);
        strcpy(inp, "|====$ ");
        int min_index = 7;
        int pt = min_index;
        int len = min_index;
        int cmd_index = 0;
        while (read(STDIN_FILENO, &c, 1) == 1) {
            printf("\r");
            for(int i = 0; i < 100; i++)
                printf(" ");
            if (iscntrl(c)) {
                if (c == '\n') break;
                else if (c == 27) { // ^] of ctrl
                    char buf[3]; // ]A\0
                    buf[2] = 0;
                    if (read(STDIN_FILENO, buf, 2) == 2) { // length of escape code
                        // printf("\rarrow key: %s", buf);
                        if(buf[1] == 'D'){
                            pt--;
                            if(pt  < min_index){
                                printf("%c", (char)7);
                            }
                            pt = max(min_index, pt);
                            // printf("left arrow");
                        }
                        if(buf[1] == 'C'){
                            pt++;
                            if(pt  >= len){
                                printf("%c", (char)7);
                            }
                            pt = min(len, pt);
                        }
                        if(buf[1] == 'A'){
                            if(cmd_index == 0){
                                strcpy(save_inp, &inp[min_index]);
                            }
                            cmd_index++;
                            cmd_index = min(20, cmd_index);
                            get_history(&inp[min_index], cmd_index, home);
                            len = strlen(inp);
                            pt = len;
                        }
                        if(buf[1] == 'B'){
                            cmd_index--;
                            cmd_index = max(0, cmd_index);
                            if(cmd_index == 0){
                                strcpy(&inp[min_index], save_inp);
                            }
                            else{
                                get_history(&inp[min_index], cmd_index, home);
                            }
                            len = strlen(inp);
                            pt = len;
                        }
                    }
                }
                else if (c == 127) { // backspace
                    if (pt > min_index) {
                        // if (inp[pt-1] == 9) {
                        //     for (int i = 0; i < 7; i++) {
                        //         printf("\b");
                        //     }
                        // }
                        // inp[--pt] = '\0'
                        for(int i = pt; inp[i]!= 0; i++){
                            inp[i] = inp[i+1];
                        }
                        pt--;
                        for(int i = len-1; i < 100; i++)
                            inp[i] = ' ';
                        len--;
                        // printf("\b \b");
                    }
                } 
                else if (c == '\t') { // TAB character
                    for(int k= 0; k < 8; k++){
                        for(int i = len; i >= pt; i--){
                            inp[i] = inp[i-1];
                        }
                        inp[pt++] = ' ';
                        len++;
                    }
                } 
                else if (c == 4) { // End of Transmission
                    printf("\r%s\n", inp);
                    return EOF;
                }
                else { // others
                    printf("%d\n", c);
                }
            } 
            else {
                for(int i = len; i >= pt; i--){
                    inp[i] = inp[i-1];
                }
                inp[pt++] = c;
                len++;
            }
                // printf("%c", c);
                // printf("\rInput Read: %s", inp);
                
                printf("\r");
                for(int i = 0; i < pt; i++){
                    printf("%c", inp[i]);
                }
                // printf("|");
                for(int i = pt; inp[i] != 0; i++){
                    printf("%c", inp[i]);
                    // printf(CSI "2C");
                }
                for(int i = pt; inp[i] != 0; i++){
                    printf(CSI "1D");
                }
        }
        printf("\e[?25h");
        disableRawMode();
        inp[len] = 0;
        printf("\r%s\n", inp);
        // printf("\nInput Read: [%s]\n", inp);
    // }
    strcpy(cmd, &inp[min_index]);
    return 0;
}
