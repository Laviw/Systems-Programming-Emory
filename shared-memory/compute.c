/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Lavi Wu */


#include "def.h"

share_mem *shm;
msg *msg_buffer;

int shm_id;
int msg_id;
int proc_idx;

int check_input(int argc, char* argv[]);
void check_sigaction(int val[3]);
void terminate(int sig);
void set_msg(int type, int content);
void send_reg_msg();
void update_shm(int start);


int main(int argc, char* argv[])
{
    int start;
    start = check_input(argc, argv);

    struct sigaction act;
    act.sa_handler = terminate;

    int act_check[3];
    act_check[0] = sigaction(SIGINT, &act, NULL);
    act_check[1] = sigaction(SIGHUP, &act, NULL);
    act_check[2] = sigaction(SIGQUIT, &act, NULL);

    check_sigaction(act_check);

    shm_id = shmget(M_KEY, sizeof(share_mem), 0660);
    shm = shmat(shm_id, NULL, 0);

    send_reg_msg();
    update_shm(start);

    shmdt(shm);
    free(msg_buffer);
    exit(0);
    
}

void set_msg(int type, int content)
{
    msg_buffer -> type = type;
    msg_buffer -> cont = content;
}

void send_reg_msg()
{
    msg_id = msgget(Q_KEY, 0660);
    msg_buffer = malloc(sizeof(msg));

    set_msg(REG_MSG, getpid());

    msgsnd(msg_id, msg_buffer, sizeof(msg_buffer -> cont), 0);
    printf("COMPUTE: REG MESSAGE SENT, P=%d\n", getpid());
}

int is_perfect(int curr)
{
    if (curr < 2)
    {
        return 0;
    }

    int sum = 1;

    for (int i = 2; i < curr / 2 + 1; i++)
    {
        if (!(curr % i))
        {
            sum += i;
        }
    }

    return sum == curr;
}

void send_per_msg(int curr)
{
    set_msg(PER_MSG, curr);
    msgsnd(msg_id, msg_buffer, sizeof(msg_buffer->cont), 0);
    printf("COMPUTE: PER MESSAGE SENT %d, P=%d\n", curr, getpid());
}

void update_shm(int start)
{
    int rcv_check = msgrcv(msg_id, msg_buffer, sizeof(msg_buffer -> cont), PRO_MSG, 0);

    if (rcv_check == -1)
    {
        perror("COMPUTE: msgrcv");
        exit(1);
    } 

    proc_idx = msg_buffer -> cont;

    int curr = start;

    while (curr < BITSIZE * 32)
    {   
        int seg_idx = (curr - 2) / 32;
        int bit_idx = (curr - 2) % 32;

        if (!(shm -> bitmap[seg_idx] & (1 << bit_idx)))
        {   
            if (is_perfect(curr))
            {
                send_per_msg(curr);
                shm -> processes[proc_idx].p_num++;
            }
            shm -> processes[proc_idx].t_num++;
            shm -> bitmap[seg_idx] |= (1 << bit_idx);
        }
        else
        {
            shm -> processes[proc_idx].s_num++;
        }

        curr = curr + 1;
    }
    shmdt(shm);
    free(msg_buffer);
    exit(0);
}


void check_sigaction(int val[3])
{
    if (val[0] == -1)
    {
        perror("COMPUTE: sigaction error -- SIGINT");
        exit(1);
    }

    if (val[1] == -1)
    {
        perror("COMPUTE: sigaction error -- SIGHUP");
        exit(1);
    }

    if (val[2] == -1)
    {
        perror("COMPUTE: sigaction error -- SIGQUIT");
        exit(1);
    }
}


int check_input(int argc, char* argv[])
{
    int start;

    argc != 2 ? printf("COMPUTE: no argument, start at 2 \n") : printf("");

    if (argc == 2)
    {
        start = atoi(argv[1]);
    }
    else
    {
        start = 2;
    }

    if (start > BITSIZE * 32 + 1 || start < 0)
    {
        printf("COMPUTE: invaild start number\n");
        exit(1);
    }

    return start;

}

void terminate(int sig)
{
    if (proc_idx < 0 || proc_idx >= 20)
    {
        exit(1);
    }

    memset(&shm -> processes[proc_idx], 0, sizeof(proc));

    if (shm_id != -1)
    {
        shmdt(shm);
    }

    free(msg_buffer);
    exit(0);

}



