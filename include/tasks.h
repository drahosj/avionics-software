/* Tasks.h */

void Task_100ms();
void Task_1s();
void initializeTasks();

typedef enum 
{
	STATE_UNARMED,
	STATE_ARMING,
	STATE_ARMED,
	STATE_TEST_FIRE,
} State_t;