

/* THIS CODE IS MY OWN WORK, IT WAS WRITTEM WOTHOUT CONSULTING
   A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - LAVI WU */


#include "def.h"
share_mem *shm;
msg *msg_buffer;

int shm_id;
int t_total;
int s_total;
int p_total;

void print_process();
void print_summary();

int main(int argc, char *argv[])
{
	shm_id = shmget(M_KEY, sizeof(share_mem), 0660);
	if (shm_id == -1)
	{
		perror("REPORT: error accessing shared memory");
	}

	shm = shmat(shm_id, NULL, 0);

	print_process();
	print_summary();

	if (argc >= 2)
	{
		if (strcmp(argv[1], "-k") == 0)
		{
			kill(shm -> m_id, SIGINT);
		}
	}
}

void print_process()
{
	printf("-------------------- RESULT ---------------------\n");
    printf("pid\ttested\tskipped\tfound\n");
    int i;
    for ( i = 0; i < 20; i ++) 
    {
	    if(shm -> processes[i].pid > 0)
	    {
            printf("%d\t%d\t%d\t%d\t\n", 
            shm -> processes[i].pid,
            shm -> processes[i].t_num,
            shm -> processes[i].s_num,
            shm -> processes[i].p_num);

            t_total += shm->processes[i].t_num;
	        s_total += shm->processes[i].s_num;
	        p_total += shm->processes[i].p_num;
	    }
	}
	
}

void print_summary()
{
	printf("-------------------- TOTAL ----------------------\n");
    printf("%s", "All Perfect Number Found:\t");
    int i;
    for (i = 0; i < 20; i++)
    {
    	if(shm -> p_num_found[i] > 0)
    	{
    		printf("%d ", shm -> p_num_found[i]);
    	}
    }
    printf("\n");
    printf("Total Number Tested:\t%d\n", t_total);
    printf("Total Number Skipped:\t%d\n", s_total);
    printf("Total Number Found:\t%d\n", p_total);
}




