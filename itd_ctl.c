#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>

#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

static void __attribute__((__noreturn__)) usage(void);
static int convert_str_to_int(char* begin);
static void parse_cpu_config_list(char* cpu_list, cpu_set_t* cpu_set);
static void parse_group_config_list(char* group_list, cpu_set_t* cpu_set);
static void version(void);
static void itd_control(char* in);
static int  get_pid_by_process_name(char* process_name);
static void get_hw_state_class(char* group_list, cpu_set_t* cpu_set);
static int is_char_or_number(char* ins);
static void get_all_process_configuration(void);
static void get_hw_state(void);

#define GROUP_SETSIZE	1024

static void __attribute__((__noreturn__)) usage(void)
{

    fprintf(stderr, "Usage:\n"
		    "	 itd-ctl [options] [mask| cpu-list| group] [cmd] [args ...]\n"
		    "\n"
		    "\n"
		    "Options:\n"
		    "	-a, --all		display all the active configuration\n"
		    "	-v, --version		display the version and author\n"
		    "  	-e, --enable		1 --itd function enable; 0 --itd function disable\n"
		    "	-s, --status		display the pid or process name configuration\n"
		    "	-S, --all status	display all the process configuration\n"
		    "	-c, --cpu-list		display and specify cpus in list format\n"
		    "	-g, --group-list	display and specify groups in list format\n"
		    "	-p, --pid		give pid or process name\n"
		    "	-r, --remove		remove the core configuration for a specify pid or process name\n"
		    "	-h, --hfi table		get hardware feedback interface table and state\n"
		    "\n"
		    "\n"
		    "Show the help and display all the active configuration\n"
		    "	itd-ctl -a\n"
		    "\n"
		    "Show the version of tool\n"
		    "	itd-ctl -v\n"
		    "\n"
		    "Enable or disable ITD function\n"
		    "	itd-ctl -e 1 or 0\n"
		    "\n"
                    "Bond running process with a special CPU list:\n"
                    "	itd-ctl -c 0,2,4-6 -p firefox OR itd-ctl -c 0,2,4-6 -p 3299\n"
		    "\n"
		    "Bond running process with a special Group list reference HFI table\n"
		    "	itd-ctl -g 1-2 -p firefox OR itd-ctl -g 1-2 -p 3299\n"
		    "\n"
                    "Query running process's bonding CPU list:\n"
                    "	itd-ctl -s firefox OR itd-ctl -s 3299\n"
		    "\n"
		    "Query all the running process's bonding CPU list:\n"
		    "	itd-ctl	-S\n"
		    "\n"
		    "Remove the core configuration for a specify pid or process name\n"
		    "	itd-ctl -r 3299 OR itd-ctl -r firefox\n"
		    "\n"
		    "Get hardware feedback interface table and state\n"
		    "	itd-ctl -h"
		    "\n"
		    );
    exit(1);
}

static void  version(void)
{
	printf("itd-ctl version is 1.0.0\n"
			"Author: junhansh\n");
	exit(1);
}

/*itd control */
static void itd_control(char* in)
{
	int enable = 0;
	FILE *fd;
	int ret;

	if (!in)
	{
		errx(1, "Invalid arguments for %s", __func__);
	}

	enable = atoi(in);
	if(enable == 1)
	{
		fd = fopen("/sys/kernel/debug/sched/itd_ctl_enable","w+");
		if(fd != NULL)
		{
		       ret = fputs(in, fd);
		       if(ret < 0)
			       fprintf(stderr, "can't set value to /sys/kernel/debug/sched/itd_ctl_enable\n");
		       else
			       printf("itd function is enabled\n");
		}
		else
		{
			fprintf(stderr, "can't open /sys/kernel/debug/sched/itd_ctl_enable\n");
		}

	}
	else
	{
		 fd = fopen("/sys/kernel/debug/sched/itd_ctl_enable","r+");
		 if(fd != NULL)
		 {
			ret = fputs(in,fd);
			if(ret < 0)
				fprintf(stderr, "can't set value to /sys/kernel/debug/sched/itd_ctl_enable\n");
			else
				printf("itd function is disabled\n");
		 }
		 else
		 {
			fprintf(stderr, "can't open /sys/kernel/debug/sched/itd_ctl_enable\n");
		 }
	}
	exit(1);
}
/*get hfi table*/
static void get_hfi_table(void)
{
	FILE *hfi;
	char buf[256] = {0};

	hfi = fopen("/sys/kernel/debug/intel_hw_feedback/hw_state0", "r");

	if(!hfi)
	{
		fprintf(stderr, "can't open hfi table\n");
		exit(1);
	}
	while(fgets(buf, sizeof(buf), hfi) != NULL)
	{
		printf("%s\n", buf);
	}

	fclose(hfi);
	exit(1);
}
/*return pid reference to process name */
static int get_pid_by_process_name(char* process_name)
{
	char cmd[64] = {0};
	int pid = 0;
	FILE *file = NULL;
	char buff[64] = {0};

	sprintf(cmd, "ps -ef | grep %s | grep -v grep | awk '{print $2}'", process_name);
	file = popen(cmd, "r");
	if (!file){
		fprintf(stderr, "can't open ps file\n");
		return -1;
	}

	fgets(buff, sizeof(buff), file);
	pid = atoi(buff);
	fclose(file);
	printf("%s pid is %d\n", process_name, pid);
	
	return pid;
}

static void get_all_process_configuration(void)
{
	char cmda[64] = {0};
	FILE *fda = NULL;
	char buffa[128] = {0};
	pid_t pida;
	int i = 0;
	cpu_set_t cpu_set;

	sprintf(cmda, "ps -ef | grep -v grep | awk '{print $2}'");
	fda = popen(cmda, "r");
	if(!fda){
		fprintf(stderr, "can't open all  ps file\n");
		exit(1);
	}

	while(fgets(buffa, sizeof(buffa), fda) != NULL && fgets(buffa, sizeof(buffa), fda) != "PID")
	     {
	
		     pida =(pid_t)atoi(buffa);
		     CPU_ZERO(&cpu_set);
		     if (sched_getaffinity(pida, sizeof(cpu_set_t), &cpu_set))
		     {
			     err(1, "sched_getaffinity");
		     }
		     printf("Process (%d) bonds to CPU:", pida);
		     for (int j = 0; j < CPU_SETSIZE; j++)
		     {
			     if (CPU_ISSET(j, &cpu_set))
			     {
				     printf(" %d", j);
			     }
		     }
		     printf("\n");
		     
	     }
	fclose(fda);

}

/* Both Process and CPU ids should be positive numbers. */
static int convert_str_to_int(char* begin)
{
    if (!begin)
    {
        errx(1, "Invalid arguments for %s", __func__);
    }

    errno = 0;
    char *end = NULL;
    long num = strtol(begin, &end, 10);
   
    if (errno || (*end != '\0') || (num > INT_MAX) || (num < 0))
    {
        errx(1, "Invalid integer: %s", begin);
    }
    return (int)num;
}

/*
 * The cpu list should like 1-3,6
 */
static void parse_cpu_config_list(char* cpu_list, cpu_set_t* cpu_set)
{
    if (!cpu_list || !cpu_set)
    {
        errx(1, "Invalid arguments for %s", __func__);
    }

    char* begin = cpu_list;
    while (1)
    {
        bool last_token = false;
        char* end = strchr(begin, ',');
        if (!end)
        {
            last_token = true;
        }
        else
        {
            *end = '\0';
        }

        char* hyphen = strchr(begin, '-');
        if (hyphen)
        {
            *hyphen = '\0';
            int first_cpu = convert_str_to_int(begin);
            int last_cpu = convert_str_to_int(hyphen + 1);
            if ((first_cpu > last_cpu) || (last_cpu >= CPU_SETSIZE))
            {
                errx(1, "Invalid cpu list: %s", cpu_list);
            }
            for (int i = first_cpu; i <= last_cpu; i++)
            {
                CPU_SET(i, cpu_set);
            }
        }
        else
        {
            CPU_SET(convert_str_to_int(begin), cpu_set);
        }

        if (last_token)
        {
            break;
        }
        else
        {
            begin = end + 1;
        }
    }
}
/* the group list must be like 1-2,4*/
static void parse_group_config_list(char* group_list, cpu_set_t* cpu_set)
{
	char  group_name[12] = "group";
	if(!group_list )
	{
		errx(1, "Invalid arguments for %s\n", __func__);
	}
	printf("Process bonds to group %s\n", group_list);


	char* begin = group_list;

	while (1)
	{
		bool last_token = false;
		char* end =strchr(begin, ',');


		if(!end)
		{
			last_token = true;
		}
		else
		{
			*end = '\0';
		}
		
		char* hyphen = strchr(begin, '-');
		if(hyphen)
		{
			*hyphen = '\0';
			int first_group =convert_str_to_int(begin);
			int last_group = convert_str_to_int(hyphen + 1);
			if((first_group > last_group) || (last_group >= GROUP_SETSIZE))
			{
				errx(1, "Invalid group list: %s\n", group_list);
			}
			
			for (int i = first_group; i <= last_group; i++)
			{
				char str[10] = { 0 };
				sprintf(str, "%d", i);
				strcat(group_name, str);
				get_hw_state_class(group_name, cpu_set);
				strcpy(group_name, "group");
			}		

		}
		else
		{
			strcat(group_name, begin);
		
			get_hw_state_class(group_name, cpu_set);
			strcpy(group_name, "group");

	

		}
		if(last_token)
		{
			break;
		}
		else
		{
			begin = end + 1;
		}
	}

}

static void get_hw_state_class(char* group_name, cpu_set_t* cpu_set)
{
	char cmd1[256] = { 0 };

	FILE *file1 = NULL;
	char buff1[10]= { 0 };
	int g_number;
	

	sprintf(cmd1, "cat /sys/kernel/debug/intel_hw_feedback/hfi_group | grep %s | grep -v grep| awk '{print $2}'", group_name);

	file1 = popen(cmd1, "r");

	if (!file1){
		printf("can't get group value from this path\n");
		exit(1);
	}

	while( fgets(buff1, sizeof(buff1), file1) != NULL){

		g_number = atoi(buff1);
		CPU_SET(g_number, cpu_set);
	}


}

static int is_char_or_number(char* ins)
{

	pid_t pid;

	if (!ins)
	{
		fprintf(stderr, "error\n");
		exit(1);

	}


	if(isalpha(ins[0]))
	{
		pid = (pid_t)get_pid_by_process_name(ins);


	}else if(isdigit(ins[0]))
	{
		pid = (pid_t)convert_str_to_int(optarg);
	
	}
	return pid;

}

int main(int argc, char**argv)
{
    cpu_set_t cpu_set;
    pid_t pid;
    int input = 0;
    bool cflag = false;
    bool pflag = false;
    bool rflag = false;
    bool gflag = false;
    bool sflag = false;
   

    CPU_ZERO(&cpu_set);
    while ((input = getopt(argc, argv, "e:s:c:g:p:r:vahS")) != -1)
    {
        switch (input)
        {
	    case 'e':
	    {
		itd_control(optarg);
		break;
	    }
	    case 's':
	    {
		if(optarg != NULL){

			pid = (pid_t)is_char_or_number(optarg);

			sflag = true;
		}
		break;
	    }
	    case 'S':
	    {
	    	get_all_process_configuration();
		break;
	    }

            case 'c':
            {
                parse_cpu_config_list(optarg, &cpu_set);
                cflag = true;
                break;
            }
	    case 'g':
	    {
		parse_group_config_list(optarg, &cpu_set);
		gflag = true;
		break;
	    }
            case 'p':
            {
                pid = (pid_t)is_char_or_number(optarg);
                pflag = true;
                break;
            }
	    case 'r':
	    {
		pid = (pid_t)is_char_or_number(optarg);
		rflag = true;
		break;
	    }
	    case 'v':
	    {
		version();
		break;
	    }
	    case 'h':
	    {
		get_hfi_table();
		break;
	    }
            case 'a':
            case '?':
            default:
            {
                usage();
            }
        }
    }

    //printf("pflag=%d, cflag=%d, rflag=%d, gflag=%d\n",pflag, cflag, rflag, gflag);
    if (pflag || sflag)
    {
        /* pid and command are exclusive */
        if (optind != argc)
        {
            usage();
        }

        if (cflag || gflag)
        {
            if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpu_set))
            {
                err(1, "sched_setaffinity");
            }
	
	    CPU_ZERO(&cpu_set);
	    if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpu_set))
	    {
		    err(1, "sched_getaffinity");
	    }
	    printf("Process (%d) bonds to CPU:", pid);
	    for (int i = 0; i < CPU_SETSIZE; i++)
	    {
		    if (CPU_ISSET(i, &cpu_set))
		    {
			    printf(" %d", i);
		    }
	    }
	    printf("\n");

        }
        else
        {
            if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpu_set))
            {
                err(1, "sched_getaffinity");
            }
            printf("Process (%d) bonds to CPU:", pid);
            for (int i = 0; i < CPU_SETSIZE; i++)
            {
                if (CPU_ISSET(i, &cpu_set))
                {
                    printf(" %d", i);
                }
            }
            printf("\n");
        }
    }
    else if(rflag)
    {
	int num = sysconf(_SC_NPROCESSORS_CONF);
	int ret = 0;
	if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpu_set))
	{
	    err(1, "sched_getaffinity");
	}
	printf("Previous process (%d) bonds to CPU:", pid);
	for (int i = 0; i < CPU_SETSIZE; i++)
	{
		if(CPU_ISSET(i, &cpu_set))
		{
			printf(" %d", i);
		}
	}
	printf("\n");
	CPU_ZERO(&cpu_set);
	for(int j = 0; j < num; j++)
	{
		CPU_SET(j, &cpu_set);
	}

	if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpu_set))
	{
		err(1, "sched_setaffinity");
	}
	if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpu_set))
	{
		err(1, "sched_getaffinity");
	}
	printf("Now process (%d) bonds to CPU:", pid);
	for (int i = 0; i < CPU_SETSIZE; i++)
	{
		if(CPU_ISSET(i, &cpu_set))
			printf(" %d", i);
	}
	printf("\n");

    } 
    else
    {
        if ((optind == argc) || (!cflag) || (!gflag))
        {
            usage();
        }

        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set))
        {
            err(1, "sched_setaffinity");
        }
        if (execvp(argv[optind], &argv[optind]))
        {
            err(1, "execvp");
        }
    }


    return 0;
}
