/*
    This is a program stimulating the spread of 2019-nCov
    Authored by YangHui on 2020.02.26
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define zero_patient 50
struct node
{
    struct node *prior;
    int day;
    int people;
    int event;
    struct node *next;
}; //�¼���Ԫ��event��0������1ȷ�2��Ժ��3������4�ӼҸ����5�ӼҸ���������ȷ���¼��������Ƿ񴥷�����ȡ��ҽԺ�Ƿ��д�λ��
struct Linklist
{
    struct node *start;
    long long lenth;
    struct node *end;
};
void random(int *,int *);
int random_number(int);
void order_insert(struct Linklist *,int,int,int); //�¼������
void patient_to_hospital(struct Linklist *,int,int); //�������סԺ�¼�
void patient_to_home(struct Linklist *,int,int); //����ӼҸ����¼�
//�Ӽ��¼������Ժ󣬼�ʹҽԺ�д�λ���࣬�����ݽṹ�����ƣ������¼�������鷳���ʶ��Զ���ҽԺ�Ĺ���ʡ��
//��ʼ��������������
const int incubation = 0; //Ǳ����
const int delay = 2; //���������뷴Ӧ��
int hospital = 10000; //���봲λ����������̫��
const int day_limit = 300; //ģ������
const int home_track = 0; //�Ӽ��Ƿ����
//const int zero_patient = 50; //��ʼ����
const int flow_people = 50000; //ÿ���˿���������
const int low_limit_hospital = 7; //סԺ���ʱ��
const int high_limit_hospital = 21; //סԺ�ʱ��
const int immunity = 1;

struct node *Index[400];
int seed; //random������
int civilian[1000001][2] = {0}; //������״̬��¼��0�б�ʾ����״̬��-1���ߣ�0������1Ǳ����2��������1�б�ʾ������״̬��0�ǣ�1��
int ill_patient = 0; //������������
int incubation_patient = zero_patient; //Ǳ���ڻ�������
int all_patient = zero_patient; //��Ⱦ��������
int death_count = 0; //��������
int live_count = 0; //Ȭ������
int patient_know = 0; //ȷ������
int patient_home = 0; //�ڼ�����
int infection = zero_patient; //��Ⱦ�ۼ�
int day = 0; //��ǰ����

int main()
{
    long long open = clock();
    seed = 0;
    int i,j,k;
    //��ʼ���¼�����
    struct node *start = (struct node *) malloc (sizeof(struct node));
    struct Linklist *Eventlist = (struct Linklist *) malloc (sizeof(struct Linklist));
    start->prior = NULL;
    start->next = NULL;
    Eventlist->start = start;
    //��ʼ����Ⱦ�߲�д���¼�����
    for (i = 0;i < zero_patient;i ++)
    {
        j = random_number(100000);
        civilian[j][0] = 1; //����Ǳ����
        order_insert(Eventlist,day + incubation,j,0); //�����¼�����
        order_insert(Eventlist,day + incubation + delay,j,1); //ȷ���¼�����
    }
    while (day < day_limit)
    {
        int exit_flag = 0;
        for (i = 0;i < flow_people;i ++) //�����˿�����
        {
            int from,to;
            do
            {
                random(&from,&to);
            }while(civilian[from][1] == 1 || civilian[to][1] == 1); //�����˱�����������ҡ��
            if ((civilian[from][0] != 0 && civilian[from][0] != -1) || (civilian[to][0] != 0 && civilian[to][0] != -1)) //�����Ⱦ�¼�
            {
                int flag = 0;
                if (civilian[from][0] == 0) //from��Ⱦ
                {
                    civilian[from][0] = 1;
                    order_insert(Eventlist,day + incubation,from,0); //�����¼�д��
                    order_insert(Eventlist,day + incubation + delay,from,1); //ȷ���¼�д��
                    flag = 1;
                }
                else if(civilian[to][0] == 0) //to��Ⱦ
                {
                    civilian[to][0] = 1;
                    order_insert(Eventlist,day + incubation,to,0);
                    order_insert(Eventlist,day + incubation + delay,to,1);
                    flag = 1;
                }
                if (flag == 1)
                {
                    all_patient ++;
                    incubation_patient ++;
                    infection ++;
                }
            }
            if (ill_patient == 1)
            {
                break;
            }
        }
        //�����¼���

        struct node *p = Eventlist->start->next;
        while(p->day == day)
        {
            if (p->event == 0) //�������¼�
            {
                incubation_patient --;
                ill_patient ++;
            }
            else if (p->event == 1) //����ȷ������¼�
            {
                civilian[p->people][0] = 2;
                patient_know ++;
                if (hospital > 0) //ҽԺ���д�λ
                {
                    civilian[p->people][1] = 1; //����
                    patient_to_hospital(Eventlist,day,p->people); //������Ժ�Ժ��¼�
                }
                else
                {
                    patient_home ++;
                    civilian[p->people][1] = home_track; //����
                    patient_to_home(Eventlist,day,p->people);
                }
            }
            else if (p->event == 2) //�����Ժ�¼�
            {
                civilian[p->people][1] = 0; //����ȡ��
                civilian[p->people][0] = immunity; //����
                hospital ++;
                ill_patient --;
                all_patient --;
                live_count ++;
            }
            else if (p->event == 3) //����ҽԺ�����¼�
            {
                hospital ++;
                ill_patient --;
                all_patient --;
                death_count ++;
            }
            else if (p->event == 4) //����Ӽ�Ȭ���¼�
            {
                civilian[p->people][0] = immunity;
                civilian[p->people][1] = 0;
                ill_patient --;
                all_patient --;
                live_count ++;
                patient_home --;
            }
            else if (p->event == 5) //����Ӽ������¼�
            {
                ill_patient --;
                all_patient --;
                death_count ++;
                patient_home --;
            }
            i = 0;
            struct node *d = (struct node *) malloc (sizeof(struct node));
            d = p;
            p = p->next;
            if (p == NULL)
            {
                exit_flag = 1;
                break;
            }
            Eventlist->start->next = p;
            p->prior = Eventlist->start;
            free(d);
        }
        printf("%d�죺�ۼƸ�Ⱦ%d�� ʵ�ʸ�Ⱦ%d�� Ǳ����%d�� ����%d�� ȷ��%d�� ����%d�� ����%d�� ��λʣ��%d�� �ӼҸ���%d\n",day + 1,infection,all_patient,incubation_patient,ill_patient,patient_know,live_count,death_count,hospital,patient_home);
        day ++;
        if (exit_flag == 1)
        {
            printf("�������\n");
            break;
        }
        Index[day] = NULL;
    }
    long long end = clock();
    printf("ģ�����\n");
    printf("��ʱ%dms\n",end - open);
    return 0;
}

int random_number(int num)
{
    seed ++;
    srand(seed);
    int random = (int) (1.0 * rand()/RAND_MAX * num);
    return random;
}

void random(int *from,int *to)
{
    *from = random_number(1000000);
    *to = random_number(1000000);
    return;
}

void order_insert(struct Linklist *Eventlist,int day,int people,int event)
{
    int i;
    struct node *p = Eventlist->start->next;
    struct node *insert = (struct node *) malloc (sizeof(struct node));
    insert->day = day;
    insert->people = people;
    insert->event = event;
    if (p == NULL) //����ձ�����¼�
    {
        Eventlist->start->next = insert;
        insert->next = NULL;
        insert->prior = Eventlist->start;
        Index[day] = insert; //��Ӧ����д������
        return;
    }
    if (Index[day] == NULL)
    {
        Index[day] = insert;
        if (day < p->day) //Ӧ�ò�������ͷ
        { 
            Eventlist->start->next = insert;
            insert->prior = Eventlist->start;
            p->prior = insert;
            insert->next = p;
        }
        else
        {
            for (i = day - 1;Index[i] == NULL && i >= p->day;i --); //�ҵ�ǰһ����ΪNULL������
            if (Index[i]->next != NULL) //���ڱ�β����
            {
                Index[i]->next->prior = insert;
                insert->next = Index[i]->next;
                Index[i]->next = insert;
                insert->prior = Index[i];//��index����һλ�����¼�
            }
            else
            {
                Index[i]->next = insert;
                insert->prior = Index[i];
                insert->next = NULL;
            }
        }
    }
    else
    {
        if (day == p->day) //�����ͷ
        {
            Eventlist->start->next = insert;
            insert->prior = Eventlist->start;
            p->prior = insert;
            insert->next = p;
        }
        else
        {
            Index[day]->prior->next = insert;
            insert->prior = Index[day]->prior;
            Index[day]->prior = insert;
            insert->next = Index[day];
        }
    }

    /*
    �ɴ���
    while (day > p->day && p->next != NULL)
        p = p->next;
    if (day > p->day && p->next == NULL) //��������β
    {
        p->next = insert;
        insert->prior = p;
        insert->next = NULL;
    }
    else if (p->prior == Eventlist->start && day <= p->day) //��������ͷ
    {
        Eventlist->start->next = insert;
        insert->prior = Eventlist->start;
        p->prior = insert;
        insert->next = p;
    }
    else //һ�����
    {
        p->prior->next = insert;
        insert->prior = p->prior;
        p->prior = insert;
        insert->next = p;
    }
    return;
    */

}

void patient_to_hospital(struct Linklist *Eventlist,int day,int people)
{
    hospital --; //��λ-1
    srand(seed ++);
    int death_live = rand() % 2; //0����1��
    srand(seed ++);
    int in_hospital = rand() % (high_limit_hospital - low_limit_hospital + 1) + low_limit_hospital;
    order_insert(Eventlist,day + in_hospital,people,death_live + 2);
    return;
}

void patient_to_home(struct Linklist *Eventlist,int day,int people)
{
    srand(seed ++);
    int death_live = (rand() % 3 == 0) ? 0 : 1; //0����1��
    srand(seed ++);
    int in_hospital = rand() % (high_limit_hospital - low_limit_hospital + 1) + low_limit_hospital;
    order_insert(Eventlist,day + in_hospital,people,death_live + 4);
    return;
}