#include "ted.h"

void config_dialog()
{
    char msg[1000];
    snprintf(msg, 1000, "Configure: ");

    char *answer = prompt(msg);

    if (answer == NULL)
    {
        return;
    }

    if (strcmp(answer, "tablen") == 0)
    {
        char msg1[1000];
        snprintf(msg1, 1000, "tablen: ");

        char *answer1 = prompt(msg1);

        if (answer1 == NULL)
        {
            char msg2[1000];
            snprintf(msg2, 1000, "Canceled");

            message(msg2);
        }
        else
        {
            int answer_int = atoi(answer1);

            if (answer_int > 0)
            {
                config.tablen = answer_int;
            }
            else {
                char msg2[1000];
                snprintf(msg2, 1000, "Needs to be greater than zero");
            
                message(msg2);
            }

            free(answer1);
        }
    }
    else if (strcmp(answer, "linebreak") == 0)
    {
        char msg1[1000];
        snprintf(msg1, 1000, "linebreak (LF, CR, CRLF): ");

        char *answer1 = prompt(msg1);

        if (answer1 == NULL)
        {
            message("Canceled");
        }
        else
        {
            if (strcmp(answer1, "LF") == 0)
            {
                config.line_break_type = 0;
            }
            else if (strcmp(answer1, "CRLF") == 0)
            {
                config.line_break_type = 1;
            }
            else if (strcmp(answer1, "CR") == 0)
            {
                config.line_break_type = 2;
            }
            else
            {
                message("Needs to be LF, CRLF or CR");
            }

            free(answer1);
        }
    }
    else if (strcmp(answer, "use_spaces") == 0)
    {
        char msg1[1000];
        snprintf(msg1, 1000, "use_spaces (0/FALSE, 1/TRUE): ");

        char *answer1 = prompt(msg1);

        if (answer1 == NULL)
        {
            message("Canceled");
        }
        else
        {
            if (strcmp(answer1, "TRUE") == 0 || strcmp(answer1, "1") == 0)
            {
                config.use_spaces = 1;
            }
            else if (strcmp(answer1, "FALSE") == 0 || strcmp(answer1, "0") == 0)
            {
                config.use_spaces = 0;
            }
            else
            {
                message("Invalid option");
            }

            free(answer1);
        }
    }
    else if (strcmp(answer, "autotab") == 0)
    {
        char msg1[1000];
        snprintf(msg1, 1000, "autotab (0/FALSE, 1/TRUE): ");
        char *answer1 = prompt(msg1);
        
        if (answer1 == NULL)
            message("Canceled");
        else
        {
            if (strcmp(answer1, "TRUE") == 0 || strcmp(answer1, "1") == 0)
                config.autotab = 1;
            else if (strcmp(answer1, "FALSE") == 0 || strcmp(answer1, "0") == 0)
                config.autotab = 0;
            else
                message("Incalid option");
            free(answer1);
        }
    }
    else {
        char msg1[1000];
        snprintf(msg1, 1000, "This option does not exist");

        message(msg1);
    }

    free(answer);
}
