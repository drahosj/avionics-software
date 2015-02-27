/* Tasks.h */

void Task_100ms();
void Task_1s();
void initializeTasks();

typedef enum 
{
	STATE_UNARMED,
	STATE_ARMING,
	STATE_ARMED,
	STATE_POWER,
	STATE_COAST,
	STATE_DEPLOY_1,
    STATE_DEPLOY_2,
    STATE_DEPLOY_3,
    STATE_DEPLOY_4,
    STATE_DESCEND,
	STATE_TEST_FIRE,
} State_t;