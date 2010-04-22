#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <set>
#include <queue>
#include "PipeBuffer.h"
#include "dcloudgahp_commands.h"
#include "dcloudgahp_common.h"

#define DCLOUD_GAHP_VERSION "0.0.1"

#define GAHP_COMMAND_ASYNC_MODE_ON			"ASYNC_MODE_ON"
#define GAHP_COMMAND_ASYNC_MODE_OFF			"ASYNC_MODE_OFF"
#define GAHP_COMMAND_RESULTS				"RESULTS"
#define GAHP_COMMAND_QUIT					"QUIT"
#define GAHP_COMMAND_VERSION				"VERSION"
#define GAHP_COMMAND_COMMANDS				"COMMANDS"

#define GAHP_RESULT_SUCCESS					"S"
#define GAHP_RESULT_ERROR					"E"
#define GAHP_RESULT_FAILURE					"F"

#define DCLOUD_COMMAND_VM_SUBMIT			"DCLOUD_VM_SUBMIT"
#define DCLOUD_COMMAND_VM_STATUS_ALL			"DCLOUD_VM_STATUS_ALL"
#define DCLOUD_COMMAND_VM_ACTION			"DCLOUD_VM_ACTION"
#define DCLOUD_COMMAND_VM_INFO				"DCLOUD_VM_INFO"
#define DCLOUD_COMMAND_VM_FIND				"DCLOUD_VM_FIND"

const char * version = "$GahpVersion " DCLOUD_GAHP_VERSION " Feb 4 2010 Condor\\ DCLOUDGAHP $";

static std::set<DcloudGahpCommand*> dcloud_gahp_commands;

FILE *logfp;

static PipeBuffer m_stdin_buffer;

static pthread_mutex_t async_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool async_mode = false;
static bool async_results_signalled = false;
static std::queue<std::string> results_list;
static pthread_mutex_t results_list_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

static void safe_printf(const char *fmt, ...)
{
    va_list va_args;

    pthread_mutex_lock(&stdout_mutex);
    va_start(va_args, fmt);
    vprintf(fmt, va_args);
    va_end(va_args);
    fflush(stdout);
    pthread_mutex_unlock(&stdout_mutex);
}

static int parse_gahp_command(const char *raw, Gahp_Args *args)
{
    int beginning = 0;
    int len = strlen(raw);
    char *buff;
    int buff_len = 0;

    if (!raw) {
        dcloudprintf("NULL command!\n");
        return FALSE;
    }

    buff = (char *)malloc(len+1);
    if (buff == NULL) {
        dcloudprintf("Failed to allocate memory\n");
        return FALSE;
    }

    args->reset();

    for (int i = 0; i<len; i++) {

        if ( raw[i] == '\\' ) {
            i++; 			// skip this char
            if (i<(len-1))
                buff[buff_len++] = raw[i];
            continue;
        }

        /* Check if character read was whitespace */
        if ( raw[i]==' ' || raw[i]=='\t' || raw[i]=='\r' || raw[i] == '\n') {

            /* Handle Transparency: we would only see these chars
             * if they WEREN'T escaped, so treat them as arg separators
             */
            buff[buff_len++] = '\0';
            args->add_arg( strdup(buff) );
            buff_len = 0;	// re-set temporary buffer

            beginning = i+1; // next char will be one after whitespace
        }
        else {
            // It's just a regular character, save it
            buff[buff_len++] = raw[i];
        }
    }

    /* Copy the last portion */
    buff[buff_len++] = '\0';
    args->add_arg( strdup(buff) );

    free (buff);
    return TRUE;
}

static void gahp_output_return(const char **results, const int count)
{
    int i;

    for (i = 0; i < count; i++) {
        safe_printf("%s", results[i]);
        if (i < (count - 1))
            safe_printf(" ");
    }

    safe_printf("\n");
}

static void gahp_output_return_success(void)
{
	const char *result[] = {GAHP_RESULT_SUCCESS};
	gahp_output_return(result, 1);
}

static void gahp_output_return_error(void)
{
	const char* result[] = {GAHP_RESULT_ERROR};
	gahp_output_return(result, 1);
}

static void unregisterAllDcloudCommands(void)
{
    std::set<DcloudGahpCommand*>::iterator itr;

    for (itr = dcloud_gahp_commands.begin(); itr != dcloud_gahp_commands.end(); itr++) {
        delete *itr;
    }
}

static void handle_command_results(Gahp_Args *args)
{
    // Print number of results
    if (args->argc != 1) {
        dcloudprintf("Expected 1 argument, saw %d\n", args->argc);
        gahp_output_return_error();
        return;
    }

    pthread_mutex_lock(&results_list_mutex);
    safe_printf("%s %d\n", GAHP_RESULT_SUCCESS, results_list.size());

    while (!results_list.empty()) {
        safe_printf("%s", results_list.front().c_str());
        results_list.pop();
    }
    pthread_mutex_unlock(&results_list_mutex);
    async_results_signalled = false;
}

static void handle_command_version(Gahp_Args *args)
{
    if (args->argc != 1) {
        dcloudprintf("Expected 1 argument, saw %d\n", args->argc);
        gahp_output_return_error();
        return;
    }

    safe_printf(GAHP_RESULT_SUCCESS " %s\n", version);
 }

static void handle_command_async_on(Gahp_Args *args)
{
    // Turn on async mode
    if (args->argc != 1) {
        dcloudprintf("Expected 1 argument, saw %d\n", args->argc);
        gahp_output_return_error();
        return;
    }

    pthread_mutex_lock(&async_mutex);
    async_mode = true;
    async_results_signalled = false;
    pthread_mutex_unlock(&async_mutex);
    gahp_output_return_success();
}

static void handle_command_async_off(Gahp_Args *args)
{
    // Turn off async mode
    if (args->argc != 1) {
        dcloudprintf("Expected 1 argument, saw %d\n", args->argc);
        gahp_output_return_error();
        return;
    }

    pthread_mutex_lock(&async_mutex);
    async_mode = false;
    pthread_mutex_unlock(&async_mutex);
    gahp_output_return_success();
}

static void handle_command_commands(Gahp_Args *args)
{
    const char **commands;
    int i = 0;
    const char **tmp;

    if (args->argc != 1) {
        dcloudprintf("Expected 1 argument, saw %d\n", args->argc);
        gahp_output_return_error();
        return;
    }

    commands = (const char **)malloc(7 * sizeof(char *));
    if (commands == NULL) {
        dcloudprintf("failed to allocate memory\n");
        gahp_output_return_error();
        return;
    }
    commands[i++] = GAHP_RESULT_SUCCESS;
    commands[i++] = GAHP_COMMAND_ASYNC_MODE_ON;
    commands[i++] = GAHP_COMMAND_ASYNC_MODE_OFF;
    commands[i++] = GAHP_COMMAND_RESULTS;
    commands[i++] = GAHP_COMMAND_QUIT;
    commands[i++] = GAHP_COMMAND_VERSION;
    commands[i++] = GAHP_COMMAND_COMMANDS;

    std::set<DcloudGahpCommand*>::iterator itr;
    for (itr = dcloud_gahp_commands.begin(); itr != dcloud_gahp_commands.end();
         itr++ ) {
        tmp = (const char **)realloc(commands, (i+1) * sizeof(char *));
        if (tmp == NULL) {
            dcloudprintf("failed to realloc memory\n");
            gahp_output_return_error();
            free(commands);
            return;
        }
        commands = tmp;
        commands[i++] = (*itr)->command.c_str();
    }

    gahp_output_return(commands, i);
    free(commands);
}

struct workerdata {
    char *fullcommand;
    workerfn worker;
};

static void *worker_function(void *ptr)
{
    struct workerdata *data = (struct workerdata *)ptr;
    Gahp_Args args;
    std::string output_string = "";

    if (!parse_gahp_command(data->fullcommand, &args)) {
        /* this should really never happen; we successfully parsed it
         * earlier, so there is no reason we can't parse it again.
         */
        dcloudprintf("Failed to parse command again\n");
        output_string = create_failure(0, "Command_Parse_Failure");
        goto cleanup;
    }

    /* even if the worker function fails, there should still be an error
     * message in output_string that we want to add
     */
    dcloudprintf("Worker started\n");

    data->worker(args.argc, args.argv, output_string);

    dcloudprintf("Worker done!\n");

cleanup:
    pthread_mutex_lock(&results_list_mutex);
    results_list.push(output_string);
    pthread_mutex_unlock(&results_list_mutex);
    pthread_mutex_lock(&async_mutex);
    if (async_mode && !async_results_signalled) {
        safe_printf("R\n");
        async_results_signalled = true;
    }
    pthread_mutex_unlock(&async_mutex);

    free(data->fullcommand);
    free(data);

    return NULL;
}

static void handle_dcloud_commands(const char *cmd, const char *fullcommand)
{
    pthread_t thread;
    struct workerdata *data;

    std::set<DcloudGahpCommand*>::iterator itr;
    for (itr = dcloud_gahp_commands.begin(); itr != dcloud_gahp_commands.end();
         itr++ ) {
        if (STRCASEEQ((*itr)->command.c_str(), cmd)) {
            data = (struct workerdata *)malloc(sizeof(struct workerdata));
            if (data == NULL) {
                dcloudprintf("Failed to allocate memory for new thread\n");
                gahp_output_return_error();
                return;
            }
            data->fullcommand = strdup(fullcommand);
            if (data->fullcommand == NULL) {
                dcloudprintf("Failed to allocate memory for command\n");
                gahp_output_return_error();
                free(data);
                return;
            }
            data->worker = (*itr)->workerfunction;
            if (pthread_create(&thread, NULL, worker_function, data) < 0) {
                dcloudprintf("Failed to create new thread\n");
                gahp_output_return_error();
                free(data->fullcommand);
                free(data);
                return;
            }
            pthread_detach(thread);
            gahp_output_return_success();
            return;
        }
    }

    /* if we reached here, we didn't find the command to execute */
    gahp_output_return_error();
}

static void handlePipe()
{
    std::string *line;

    while ((line = m_stdin_buffer.GetNextLine()) != NULL) {
        const char *command = line->c_str();
        Gahp_Args args;

        dcloudprintf("Handling line %s\n", command);

        if (parse_gahp_command(command, &args)) {
            if (STRCASEEQ(args.argv[0], GAHP_COMMAND_RESULTS))
                handle_command_results(&args);
            else if (STRCASEEQ(args.argv[0], GAHP_COMMAND_VERSION))
                handle_command_version(&args);
            else if (STRCASEEQ(args.argv[0], GAHP_COMMAND_QUIT)) {
                gahp_output_return_success();
                unregisterAllDcloudCommands();
                /* Since we are exiting, free the line to avoid a
                 * valgrind warning
                 */
                args.reset();
                delete line;
                exit(0);
            }
            else if (STRCASEEQ(args.argv[0], GAHP_COMMAND_ASYNC_MODE_ON))
                handle_command_async_on(&args);
            else if (STRCASEEQ(args.argv[0], GAHP_COMMAND_ASYNC_MODE_OFF))
                handle_command_async_off(&args);
            else if (STRCASEEQ(args.argv[0], GAHP_COMMAND_COMMANDS))
                handle_command_commands(&args);
            else
                /* note that we pass "command" into handle_dcloud_commands
                 * instead of the already parsed Gahp_Args.  This is because
                 * the dcloud commands are all going to be handled in a
                 * separate thread, so there is no elegant way for us to handle
                 * this.  Therefore, the dcloud_commands will just re-parse
                 * the command.  Kind of ugly, but really better than the
                 * alternatives.
                 */
                handle_dcloud_commands(args.argv[0], command);
        }
        else
            gahp_output_return_error();

        delete line;
    }

	// check if GetNextLine() returned NULL because of an error or EOF
	if (m_stdin_buffer.IsError() || m_stdin_buffer.IsEOF()) {
		dcloudprintf("stdin buffer closed, exiting\n");
		exit(1);
	}
}

static void registerDcloudGahpCommand(const char *command, workerfn workerfunc)
{
    DcloudGahpCommand *newcommand;

    if (command == NULL) {
        dcloudprintf("tried to register NULL command\n");
        return;
    }

    newcommand = new DcloudGahpCommand(command, workerfunc);

    dcloud_gahp_commands.insert(newcommand);
}

static void registerAllDcloudCommands(void)
{
    dcloudprintf("\n");

    if (dcloud_gahp_commands.size() > 0) {
        dcloudprintf("already called\n");
        return;
    }

    registerDcloudGahpCommand(DCLOUD_COMMAND_VM_SUBMIT,
                              dcloud_start_worker);

    registerDcloudGahpCommand(DCLOUD_COMMAND_VM_ACTION,
                              dcloud_action_worker);

    registerDcloudGahpCommand(DCLOUD_COMMAND_VM_INFO,
                              dcloud_info_worker);

    registerDcloudGahpCommand(DCLOUD_COMMAND_VM_STATUS_ALL,
                              dcloud_statusall_worker);

    registerDcloudGahpCommand(DCLOUD_COMMAND_VM_FIND,
                              dcloud_find_worker);
}

int main(int argc, char *argv[])
{
    logfp = fopen("/tmp/dcloud_gahp.debug", "a");
    if (!logfp) {
        fprintf(stderr, "Could not open log file /tmp/dcloud_gahp.debug: %s\n",
                strerror(errno));
        return 1;
    }

    dcloudprintf("Starting dcloud GAHP\n");

    registerAllDcloudCommands();

    safe_printf("%s\n", version);

    m_stdin_buffer.setPipeEnd(0);

    while (1)
        /* handle input from stdin */
        handlePipe();

    return 0;
}
