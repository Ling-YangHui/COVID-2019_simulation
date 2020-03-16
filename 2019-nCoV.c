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
}; //事件表单元，event：0发病，1确诊，2出院，3死亡，4居家隔离存活，5居家隔离死亡（确诊事件触发后是否触发隔离取决医院是否还有床位）
struct Linklist
{
    struct node *start;
    long long lenth;
    struct node *end;
};
void random(int *,int *);
int random_number(int);
void order_insert(struct Linklist *,int,int,int); //事件表插入
void patient_to_hospital(struct Linklist *,int,int); //处理隔离住院事件
void patient_to_home(struct Linklist *,int,int); //处理居家隔离事件
//居家事件产生以后，即使医院有床位空余，受数据结构的限制，调动事件表过于麻烦，故而自动送医院的过程省略
//初始变量，可以修正
const int incubation = 0; //潜伏期
const int delay = 2; //发病到隔离反应期
int hospital = 10000; //隔离床位数量（不能太大）
const int day_limit = 300; //模拟天数
const int home_track = 0; //居家是否隔离
//const int zero_patient = 50; //初始患者
const int flow_people = 50000; //每日人口流动次数
const int low_limit_hospital = 7; //住院最短时长
const int high_limit_hospital = 21; //住院最长时长
const int immunity = 1;

struct node *Index[400];
int seed; //random的种子
int civilian[1000001][2] = {0}; //所有人状态记录，0列表示患病状态（-1免疫，0健康，1潜伏，2发病），1列表示可流动状态（0是，1否）
int ill_patient = 0; //发病患者数量
int incubation_patient = zero_patient; //潜伏期患者数量
int all_patient = zero_patient; //感染者总数量
int death_count = 0; //死亡数量
int live_count = 0; //痊愈数量
int patient_know = 0; //确诊数量
int patient_home = 0; //在家数量
int infection = zero_patient; //感染累计
int day = 0; //当前天数

int main()
{
    long long open = clock();
    seed = 0;
    int i,j,k;
    //初始化事件链表
    struct node *start = (struct node *) malloc (sizeof(struct node));
    struct Linklist *Eventlist = (struct Linklist *) malloc (sizeof(struct Linklist));
    start->prior = NULL;
    start->next = NULL;
    Eventlist->start = start;
    //初始化感染者并写入事件链表
    for (i = 0;i < zero_patient;i ++)
    {
        j = random_number(100000);
        civilian[j][0] = 1; //进入潜伏期
        order_insert(Eventlist,day + incubation,j,0); //发病事件插入
        order_insert(Eventlist,day + incubation + delay,j,1); //确诊事件插入
    }
    while (day < day_limit)
    {
        int exit_flag = 0;
        for (i = 0;i < flow_people;i ++) //处理人口流动
        {
            int from,to;
            do
            {
                random(&from,&to);
            }while(civilian[from][1] == 1 || civilian[to][1] == 1); //若有人被隔离则重新摇号
            if ((civilian[from][0] != 0 && civilian[from][0] != -1) || (civilian[to][0] != 0 && civilian[to][0] != -1)) //处理感染事件
            {
                int flag = 0;
                if (civilian[from][0] == 0) //from感染
                {
                    civilian[from][0] = 1;
                    order_insert(Eventlist,day + incubation,from,0); //发病事件写入
                    order_insert(Eventlist,day + incubation + delay,from,1); //确诊事件写入
                    flag = 1;
                }
                else if(civilian[to][0] == 0) //to感染
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
        //处理事件表

        struct node *p = Eventlist->start->next;
        while(p->day == day)
        {
            if (p->event == 0) //处理发病事件
            {
                incubation_patient --;
                ill_patient ++;
            }
            else if (p->event == 1) //处理确诊隔离事件
            {
                civilian[p->people][0] = 2;
                patient_know ++;
                if (hospital > 0) //医院还有床位
                {
                    civilian[p->people][1] = 1; //隔离
                    patient_to_hospital(Eventlist,day,p->people); //插入入院以后事件
                }
                else
                {
                    patient_home ++;
                    civilian[p->people][1] = home_track; //隔离
                    patient_to_home(Eventlist,day,p->people);
                }
            }
            else if (p->event == 2) //处理出院事件
            {
                civilian[p->people][1] = 0; //隔离取消
                civilian[p->people][0] = immunity; //免疫
                hospital ++;
                ill_patient --;
                all_patient --;
                live_count ++;
            }
            else if (p->event == 3) //处理医院死亡事件
            {
                hospital ++;
                ill_patient --;
                all_patient --;
                death_count ++;
            }
            else if (p->event == 4) //处理居家痊愈事件
            {
                civilian[p->people][0] = immunity;
                civilian[p->people][1] = 0;
                ill_patient --;
                all_patient --;
                live_count ++;
                patient_home --;
            }
            else if (p->event == 5) //处理居家死亡事件
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
        printf("%d天：累计感染%d人 实际感染%d人 潜伏期%d人 发病%d人 确诊%d人 治愈%d人 死亡%d人 床位剩余%d张 居家隔离%d\n",day + 1,infection,all_patient,incubation_patient,ill_patient,patient_know,live_count,death_count,hospital,patient_home);
        day ++;
        if (exit_flag == 1)
        {
            printf("疫情结束\n");
            break;
        }
        Index[day] = NULL;
    }
    long long end = clock();
    printf("模拟结束\n");
    printf("用时%dms\n",end - open);
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
    if (p == NULL) //处理空表插入事件
    {
        Eventlist->start->next = insert;
        insert->next = NULL;
        insert->prior = Eventlist->start;
        Index[day] = insert; //对应日期写入索引
        return;
    }
    if (Index[day] == NULL)
    {
        Index[day] = insert;
        if (day < p->day) //应该插入链表头
        { 
            Eventlist->start->next = insert;
            insert->prior = Eventlist->start;
            p->prior = insert;
            insert->next = p;
        }
        else
        {
            for (i = day - 1;Index[i] == NULL && i >= p->day;i --); //找到前一个不为NULL的日期
            if (Index[i]->next != NULL) //不在表尾插入
            {
                Index[i]->next->prior = insert;
                insert->next = Index[i]->next;
                Index[i]->next = insert;
                insert->prior = Index[i];//在index的下一位插入事件
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
        if (day == p->day) //插入表头
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
    旧代码
    while (day > p->day && p->next != NULL)
        p = p->next;
    if (day > p->day && p->next == NULL) //插入链表尾
    {
        p->next = insert;
        insert->prior = p;
        insert->next = NULL;
    }
    else if (p->prior == Eventlist->start && day <= p->day) //插入链表头
    {
        Eventlist->start->next = insert;
        insert->prior = Eventlist->start;
        p->prior = insert;
        insert->next = p;
    }
    else //一般插入
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
    hospital --; //床位-1
    srand(seed ++);
    int death_live = rand() % 2; //0生，1死
    srand(seed ++);
    int in_hospital = rand() % (high_limit_hospital - low_limit_hospital + 1) + low_limit_hospital;
    order_insert(Eventlist,day + in_hospital,people,death_live + 2);
    return;
}

void patient_to_home(struct Linklist *Eventlist,int day,int people)
{
    srand(seed ++);
    int death_live = (rand() % 3 == 0) ? 0 : 1; //0生，1死
    srand(seed ++);
    int in_hospital = rand() % (high_limit_hospital - low_limit_hospital + 1) + low_limit_hospital;
    order_insert(Eventlist,day + in_hospital,people,death_live + 4);
    return;
}