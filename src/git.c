#include "ted.h"

void git_setup(void) {
    char buf[4000];
    char cwd[1000];
    getcwd(cwd, 1000);

    snprintf(buf, 4000,
            "git --version > /dev/null 2> /dev/null || exit -1;"
            "cd %s > /dev/null 2> /dev/null;"
            "git rev-parse --git-dir > /dev/null 2> /dev/null || exit -2",
            cwd);

    int ec = system(buf);

    if (ec == -1)
        config.git_installed = 0, config.git_in_repo = 0;
    else if (ec == -2)
        config.git_installed = 1, config.git_in_repo = 0;
    else
        config.git_installed = 1, config.git_in_repo = 1;
}

bool git_in_repo(char *path) {
    if (!config.git_installed) return 0;
    char buf[4000];

    if (path == NULL) {
        char cwd[1000];
        if (getcwd(cwd, 1000) == NULL) return 0;
        snprintf(buf, 4000, "cd %s;", cwd);
    } else
        snprintf(buf, 4000, "cd %s;", path);

    strncat(buf, "git rev-parse --git-dir > /dev/null 2> /dev/null", 4000);
    return system(buf) == 0;
}

bool git_current_branch(char *buf, unsigned int buflen) {
    if (!config.git_installed || !config.git_in_repo) return 0;

    int stdout_pipe[2];
    if (pipe(stdout_pipe) < 0) return 0;

    pid_t pid = fork();
    if (pid < 0) {
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        return 0;
    }
    if (pid == 0) {//child process
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);

        execl("/bin/sh", "sh", "-c", "git rev-parse --abbrev-ref HEAD", NULL);
        exit(1);//unreachable
    }

    while (read(stdout_pipe[0], buf, buflen) == -1 && errno == EINTR);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    return 1;
}

bool git_tracked(char *fname) {
    if (!config.git_installed) return 0;
    char buf[4000];

    snprintf(buf, 4000, "git ls-files --error-unmatch %s", fname);
    return system(buf) == 0;
}
