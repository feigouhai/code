#include <stdio.h>
#include <stdlib.h>
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
                    test_config_line(line, &current_test);
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

static void process_macros(const char *input, char *output)
{
    char c, prev;
    const char *varname_start = NULL;
    int inputcnt = strlen(input);
    int outputcnt = CONFIG_SYS_CBSIZE;
    int state = 0;          /* 0 = waiting for '$'  */

    /* 1 = waiting for '(' or '{' */
    /* 2 = waiting for ')' or '}' */
    /* 3 = waiting for '''  */

    prev = '\0';            /* previous character   */

    while (inputcnt && outputcnt) {
        c = *input++;
        inputcnt--;

        if (state != 3) {
            /* remove one level of escape characters */
            if ((c == '\\') && (prev != '\\')) {
                if (inputcnt-- == 0)
                    break;
                prev = c;
                c = *input++;
            }
        }

        switch (state) {
        case 0: /* Waiting for (unescaped) $    */
            if ((c == '\'') && (prev != '\\')) {
                state = 3;
                break;
            }
            if ((c == '$') && (prev != '\\')) {
                state++;
            } else {
                *(output++) = c;
                outputcnt--;
            }
            break;
        case 1: /* Waiting for (        */
            if (c == '(' || c == '{') {
                state++;
                varname_start = input;
            } else {
                state = 0;
                *(output++) = '$';
                outputcnt--;

                if (outputcnt) {
                    *(output++) = c;
                    outputcnt--;
                }
            }
            break;
        case 2: /* Waiting for )        */
            if (c == ')' || c == '}') {
                int i;
                char envname[CONFIG_SYS_CBSIZE], *envval;
                int envcnt = input - varname_start - 1; /* Varname # of chars */

                /* Get the varname */
                for (i = 0; i < envcnt; i++) {
                    envname[i] = varname_start[i];
                }
                envname[i] = 0;

                /* Get its value */
                envval = NULL; // getenv (envname);

                /* Copy into the line if it exists */
                if (envval != NULL)
                    while ((*envval) && outputcnt) {
                        *(output++) = *(envval++);
                        outputcnt--;
                    }
                /* Look for another '$' */
                state = 0;
            }
            break;
        case 3: /* Waiting for '        */
            if ((c == '\'') && (prev != '\\')) {
                state = 0;
            } else {
                *(output++) = c;
                outputcnt--;
            }
            break;
        }
        prev = c;
    }

    if (outputcnt)
        *output = 0;
    else
        *(output - 1) = 0;

}

int parse_line(char *line, char *argv[])
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

    printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

    return nargs;
}


int test_config_line(const char *cmd, int *test_num)
{
    char cmdbuf[CONFIG_SYS_CBSIZE]; /* working copy of cmd		*/
    char *token;                    /* start of token in cmdbuf	*/
    char *sep;                      /* end of token (separator) in cmdbuf */
    char finaltoken[CONFIG_SYS_CBSIZE];
    char *str = cmdbuf;
    char *argv[CONFIG_SYS_MAXARGS + 1];     /* NULL terminated	*/
    int argc, inquotes;
    int rc = 0;
    int index;
    int i;
    static int overheat_protect = 0;
    static int index1 = -1;

    if (!cmd || !*cmd) {
        return -1;      /* empty command */
    }

    strcpy(cmdbuf, cmd);

    /* Process separators and check for invalid
     * repeatable commands
     */
    while (*str) {

        /*
         * Find separator, or string end
         * Allow simple escape of ';' by writing "\;"
         */
        for (inquotes = 0, sep = str; *sep; sep++) {
            if ((*sep == '\'') &&
                    (*(sep - 1) != '\\'))
                inquotes = !inquotes;

            if (!inquotes &&
                    (*sep == ';') &&    /* separator		*/
                    ( sep != str) &&    /* past string start	*/
                    (*(sep - 1) != '\\')) /* and NOT escaped	*/
                break;
        }

        /*
         * Limit the token to data between separators
         */
        token = str;
        if (*sep) {
            str = sep + 1;  /* start of command for next pass */
            *sep = '\0';
        } else
            str = sep;      /* no more commands for next pass */

        /* find macros in this token and replace them */
        process_macros(token, finaltoken);

        /* Extract arguments */
        if ((argc = parse_line(finaltoken, argv)) == 0) {
            rc = -1;        /* no command at all */
            continue;
        }
        printf("#### %s %s \n", argv[0],argv[1]);
    }

    return 0;
}

void preinit_tvad() {
    char commands[1000];
    int count;

    fdump = fopen("/home/brianwang/project/code/enabledump.txt", "rb");
    if(!fdump) {
        printf("can't open enabledump \n");
        return;
    }
    count = fread(commands, sizeof(char), 100,  fdump);
//    printf("print file cn %d: \n%s \n",count, commands);

    parse_config_line(commands);
    printf("############## \n");
}

int main() {
    preinit_tvad();
    return 0;
}
