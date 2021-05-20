#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MQTTAsync.h"

#if !defined(_WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif

#define ADDRESS     		"tcp://localhost:1883"
#define TOPIC_STR			"topic"
#define CONVERSATION_STR	"conversation"
#define PAYLOAD    			"Hello World!"
#define QOS        			1
#define TIMEOUT     		10000L
#define MAX_CLIENTS 		200

int disc_finished = 0;
int subscribed = 0;
int finished = 0;

char CLIENTID[20];
char TOPIC[20];
char conversations[MAX_CLIENTS];

void connectToTopic(MQTTAsync client, char topic[]);
void connlost(void *context, char *cause);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void onDisconnectFailure(void* context, MQTTAsync_failureData* response);
void onDisconnect(void* context, MQTTAsync_successData* response);
void onSubscribe(void* context, MQTTAsync_successData* response);
void onSubscribeFailure(void* context, MQTTAsync_failureData* response);
void onConnectFailure(void* context, MQTTAsync_failureData* response);
void onSendFailure(void* context, MQTTAsync_failureData* response);
void onSend(void* context, MQTTAsync_successData* response);
void onConnect(void* context, MQTTAsync_successData* response);
void menu(int id);
void connectToTopic(MQTTAsync client, char topic[]);
int toInt(char *str);
int createId(int a, int b);