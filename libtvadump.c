#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

FILE *fdump = NULL;

#define CONFIG_SYS_CBSIZE 1000
#define CONFIG_SYS_MAXARGS 100

void parse_config_line(char *cmd)
{
    char *line = cmd;
    char *next = cmd;
    int eof = 0, current_test = -1;
    while (1) {
        if (*next == '\n' || *next == '\0') {
            if (*next == '\0')
                eof = 1;
            *next = '\0';
            if ((next - line) > 1) {
                parse_content_of_line(line, &current_test);
                if (eof)
                    break;
            } else {
                if (eof)
                    break;
            }
            line = next + 1;
        }
        ++next;
    }
}


int parse_content(char *line, char *argv[])
{
    int nargs = 0;

    while (nargs < CONFIG_SYS_MAXARGS) {
        /* skip any white space */
        while ((*line == ' ') || (*line == '\t') || (*line == '=')) {
            ++line;
        }

        if (*line == '\0') {    /* end of line, no more args	*/
            argv[nargs] = NULL;
            return nargs;
        }

        argv[nargs++] = line;   /* begin of argument string	*/

        /* find end of string */
        while (*line && (*line != ' ') && (*line != '\t') && (*line != '='))
            ++line;

        if (*line == '\0') {    /* end of line, no more args	*/
            argv[nargs] = NULL;
            return nargs;
        }

        *line++ = '\0';         /* terminate current arg	 */
    }

    return nargs;
}


int parse_content_of_line(const char *cmd, int *test_num)
{
    char cmdbuf[CONFIG_SYS_CBSIZE]; /* working copy of cmd		*/
    char *token;                    /* start of token in cmdbuf	*/
    char *str = cmdbuf;
    char *argv[CONFIG_SYS_MAXARGS + 1];     /* NULL terminated	*/
    int argc;
    int i;

    if (!cmd || !*cmd) {
        return -1;      /* empty command */
    }

    strcpy(cmdbuf, cmd);
    argc = parse_content(str, argv);
    if(!strncmp("DUMP_A", argv[0], sizeof(argv[0])) &&
            !strncmp("enable", argv[1], sizeof(argv[1]))) {
        printf("argv %s %s \n", argv[0],argv[1]);
    }
    if(!strncmp("DUMP_B", argv[2], sizeof(argv[2])) &&
            !strncmp("enable", argv[3], sizeof(argv[3]))) {
        printf("argv %s %s \n", argv[2],argv[3]);
    }
    return 0;
}

int preinit_tvad() {
    char commands[CONFIG_SYS_CBSIZE];
    int count;

    fdump = fopen("/home/brianwang/project/code/enabledump.txt", "rb");
    if(!fdump) {
        return -1;
    }
    count = fread(commands, sizeof(char), 100,  fdump);
    parse_config_line(commands);
    return 0;
}

int main() {
    int ret;
    ret = preinit_tvad();
    if(ret<0)
        printf("can't open enabledump.txt \n");
    else
        printf("success parse enabledump.txt \n");

    return 0;
}
