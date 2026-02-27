#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#define SSIZE_MAX  LLONG_MAX
typedef long long ssize_t;
static void platform_sleep(double seconds)
{
	Sleep((DWORD)(seconds * 1000));
}
#else
#include <unistd.h>
static void platform_sleep(double seconds)
{
	usleep((useconds_t)(seconds * 1000000));
}
#endif

typedef struct
{
	size_t	sz;
	ssize_t	iters;
	int		force_commit;
	int		should_print;
	int		stop_on_fail;
	int		needs_help;
	double	interval;
}	args_t;

static void format_size(char* buf, size_t buf_sz, size_t bytes)
{
	if (bytes >= 1024ULL * 1024 * 1024 && bytes % (1024ULL * 1024 * 1024) == 0)
		snprintf(buf, buf_sz, "%lluGB", bytes / (1024ULL * 1024 * 1024));

	else if (bytes >= 1024ULL * 1024 && bytes % (1024ULL * 1024) == 0)
		snprintf(buf, buf_sz, "%lluMB", bytes / (1024ULL * 1024));

	else if (bytes >= 1024 && bytes % 1024 == 0)
		snprintf(buf, buf_sz, "%zuKB", bytes / 1024);

	else
		snprintf(buf, buf_sz, "%zu bytes", bytes);
}

static void help(void)
{
	printf("Usage: leaker --size=<bytes> [options]\n\n");
	printf("Options:\n");
	printf("  --size=<value>[suffix]  Size of each allocation (required)\n");
	printf("                         Suffixes: b, kb, mb, gb (default: b)\n");
	printf("  --iters=<count>        Number of iterations, -1 for infinite (default: -1)\n");
	printf("  --interval=<seconds>   Sleep interval between allocations (default: 0)\n");
	printf("  --force-commit         Forces the OS to commit the memory\n");
	printf("  --silent               Suppress output\n");
	printf("  --stop-on-fail         Stop when an allocation fails\n");
	printf("  --help                 Show this help message\n");
}

static args_t* parse_args(const int argc, const char** argv)
{
	args_t* args = (args_t*)malloc(sizeof(args_t));

	if (!args)
	{
		printf("Failed to allocate arguments\n");
		return NULL;
	}

	args->sz			= 0;
	args->iters			= -1;
	args->force_commit	= 0;
	args->should_print	= 1;
	args->stop_on_fail	= 0;
	args->needs_help	= 0;
	args->interval		= 0.0;

	int size_provided = 0;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--silent") == 0)
			args->should_print = 0;

		else if (strcmp(argv[i], "--stop-on-fail") == 0)
			args->stop_on_fail = 1;

		else if (strcmp(argv[i], "--help") == 0)
			args->needs_help = 1;

		else if (strcmp(argv[i], "--force-commit") == 0)
			args->force_commit = 1;

		else if (strncmp(argv[i], "--size=", 7) == 0)
		{
			char* end = 0;
			const char* val = argv[i] + 7;

			errno = 0;

			unsigned long long tmp = strtoull(val, &end, 10);

			if (*val == '-' || end == val || errno == ERANGE)
			{
				printf("Invalid --size value: %s\n", val);
				free(args);
				return NULL;
			}

			unsigned long long multiplier = 1;

			if (*end)
			{
				if (strcasecmp(end, "b") == 0)
					multiplier = 1;
				else if (strcasecmp(end, "kb") == 0)
					multiplier = 1024;
				else if (strcasecmp(end, "mb") == 0)
					multiplier = 1024 * 1024;
				else if (strcasecmp(end, "gb") == 0)
					multiplier = 1024ULL * 1024 * 1024;
				else
				{
					printf("Invalid --size suffix: %s\n", end);
					free(args);
					return NULL;
				}
			}

			if (tmp > (unsigned long long)SIZE_MAX / multiplier)
			{
				printf("--size value too large: %s\n", val);
				free(args);
				return NULL;
			}

			args->sz = (size_t)(tmp * multiplier);
			size_provided = 1;
		}

		else if (strncmp(argv[i], "--iters=", 8) == 0)
		{
			char* end = 0;
			const char* val = argv[i] + 8;

			errno = 0;
			long long tmp = strtoll(val, &end, 10);

			if (end == val || *end || errno == ERANGE ||
				tmp < -(long long)SSIZE_MAX - 1 || tmp > (long long)SSIZE_MAX)
			{
				printf("Invalid --iters value: %s\n", val);
				free(args);
				return NULL;
			}

			args->iters = (ssize_t)tmp;
		}

		else if (strncmp(argv[i], "--interval=", 11) == 0)
		{
			char* end = 0;
			const char* val = argv[i] + 11;

			errno = 0;
			double tmp = strtod(val, &end);

			if (end == val || *end || errno == ERANGE || !isfinite(tmp))
			{
				printf("Invalid --interval value: %s\n", val);
				free(args);
				return NULL;
			}

			args->interval = tmp;
		}

		else
		{
			printf("Unknown argument: %s\n", argv[i]);
			free(args);
			return NULL;
		}
	}

	if (!args->needs_help && !size_provided)
	{
		printf("Error: --size=<bytes> is required\n\n");
		help();
		free(args);
		return NULL;
	}

	return args;
}

static int leak(
	const size_t	sz,
	const int		force_commit,
	const int		should_print,
	const int		stop_on_fail)
{
	void* alloc = malloc(sz);

	if (!alloc)
	{
		if (should_print)
			printf("Allocation failure!\n");

		if (stop_on_fail)
		{
			if (should_print)
				printf("Stopping...\n");
			return -1;
		}

		return 0;
	}

	if (force_commit)
		memset(alloc, 0xAA, sz);

	if (should_print)
	{
		char buf[64];
		format_size(buf, sizeof(buf), sz);
		printf("Leaked %s\n", buf);
	}

	return 1;
}

static size_t iterative_leak(const args_t* args)
{
	size_t total = 0;

	if (args->iters == -1)
	{
		while (1)
		{
			const int ret = leak(
				args->sz,
				args->force_commit,
				args->should_print,
				args->stop_on_fail
			);

			if (ret < 0)
				return total;

			if (ret > 0)
				total += args->sz;

			if (args->interval > 0)
				platform_sleep(args->interval);
		}
	}
	else
	{
		for (ssize_t i = 0; i < args->iters; ++i)
		{
			const int ret = leak(
				args->sz,
				args->force_commit,
				args->should_print,
				args->stop_on_fail
			);

			if (ret < 0)
				return total;

			if (ret > 0)
				total += args->sz;

			if (args->interval > 0)
				platform_sleep(args->interval);
		}
	}

	return total;
}

int main(const int argc, const char** argv)
{
	if (argc < 2)
	{
		help();
		return 1;
	}

	args_t* args = parse_args(argc, argv);

	if (!args)
		return 1;

	if (args->needs_help)
	{
		free(args);
		help();
		return 0;
	}

	if (args->should_print)
		printf("Warning: Leaking only persists within the program's lifetime!\n");

	const size_t total = iterative_leak(args);

	if (args->should_print)
	{
		char buf[64];
		format_size(buf, sizeof(buf), total);
		printf("Total: %s leaked\n", buf);
		printf("Finished.\n");
	}

	free(args);

	return 0;
}
