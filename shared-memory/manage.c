/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Lavi Wu */


#include "def.h"

share_mem *shm;
msg *msg_buffer;
int shm_id;
int msg_id;
int mem = 0;


void terminate(int sig);
void check_sigaction(int val[3]);
void manage_shared_mem();

int main(int argc, char *argv[])
{
    shm_id = shmget(M_KEY, sizeof(share_mem), 0660|IPC_EXCL|IPC_CREAT);
    // terminate(0);

    if (shm_id == -1)
    {
        perror("MANAGE:  allocating shared memory error.");
        exit(1);
    }

    shm = shmat(shm_id, NULL, 0);
    memset(shm, 0, sizeof(share_mem));
    shm -> m_id = getpid();

    struct sigaction act;
    act.sa_handler = terminate;

    int act_check[3];
    act_check[0] = sigaction(SIGINT, &act, NULL);
    act_check[1] = sigaction(SIGHUP, &act, NULL);
    act_check[2] = sigaction(SIGQUIT, &act, NULL);

    check_sigaction(act_check);

    msg_id = msgget(Q_KEY, 0660|IPC_CREAT);

    if (msg_id == -1)
    {
        perror("MANAGE: msgget");
        exit(1);
    }

    msg_buffer = malloc(sizeof(msg));

    while(1)
    {
        manage_shared_mem();
    }
    terminate(0);
    // exit(0);
}

void reg_segment()
{
    printf("---------------- P(%d) ----------------\n", msg_buffer -> cont);
    printf("COMPUTE: REG MESSAHE RECIEVED!\n");
    if (shm -> proc_num < 20)
    {
        shm -> processes[shm -> proc_num].pid = msg_buffer -> cont;
    }
    else
    {
        printf("MANAGE: processes full \n");
    }

    msg_buffer -> type = PRO_MSG;
    msg_buffer -> cont = shm -> proc_num++;
    msgsnd(msg_id, msg_buffer, sizeof(msg_buffer -> cont), 0);
}

int update_segment()
{
    printf("COMPUTE: UPDATE MEMORY SEGMENT: %d\n", msg_buffer -> cont);

    int i;
    for (i = 0; i < 20; i++)
    {
        if (shm -> p_num_found[i] == msg_buffer -> cont)
        {
            break;
        }
        else if(shm -> p_num_found[i] == 0)
        {
            shm -> p_num_found[i] = msg_buffer -> cont;
            break;
        }
    }

    return i;
}

void manage_shared_mem()
{
    memset(msg_buffer, '\0', sizeof(msg));
    int n_read;
    n_read = msgrcv(msg_id, msg_buffer, sizeof(msg_buffer -> cont), -2, 0);

    if (n_read == -1)
    {
        perror("MANAGE: msgrcv");
    }

    if (msg_buffer -> type == REG_MSG)
    {
        reg_segment();
    }
    else if (msg_buffer -> type == PER_MSG)
    {
        int cur_p_num = update_segment();

        if (cur_p_num >= 20)
        {
            printf("MANAGE: 20 perfect numbers already! \n");
        }
    }

}

void check_sigaction(int val[3])
{
    if (val[0] == -1)
    {
        perror("MANAGE: sigaction error -- SIGINT");
        exit(1);
    }

    if (val[1] == -1)
    {
        perror("MANAGE: sigaction error -- SIGHUP");
        exit(1);
    }

    if (val[2] == -1)
    {
        perror("MANAGE: sigaction error -- SIGQUIT");
        exit(1);
    }
}



void terminate(int sig)
{
    printf("---------------- TERMINATE ----------------\n");
    for (int i = 0; i < 20; i++)
    {
        if (shm -> processes[i].pid)
        {
            printf("KILLING PROCESSES %d \n", shm -> processes[i].pid);
            kill(shm -> processes[i].pid, SIGINT);
        }
    }

    sleep(3);

    shmdt(shm);
    printf("* DISTORY MEMORY SEGMENT %d\n", shm_id);
    shmctl(shm_id, IPC_RMID, NULL);
    printf("* KILLING MESSAGE QUEUE %d\n", msg_id);
    msgctl(msg_id, IPC_RMID, NULL);

    free(msg_buffer);
    exit(0);
}
