#include "functions.h"

void connlost(void *context, char *cause) {
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nConnection lost\n");
	if (cause)
		printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start connect, return code %d\n", rc);
		finished = 1;
	}
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
	if(strcmp(topicName, TOPIC) == 0){ //verifies if the message received is in the control_topic
		/*
			ID -> string to int
			Se a mensagem for um ID -> criar novo topico de conversa e enviar mensagem ao topico de controle do client que solicitou, 
				o id proprio para estabelecer conversasão
			Se for um topico de conversa -> Fazer publish no topico e adicionar "salvar" topico como conhecido
		*/ 
		int client_id_conversation = toInt((char*)message->payload);
		if (conversations[client_id_conversation] == 0) {
			conversations[client_id_conversation] = createId(toInt(CLIENTID), client_id_conversation);

			char conversation_id[30], conversation_topic_id[30], conversation_topic[30] = CONVERSATION_STR;
			sprintf(conversation_id, "%d", conversations[client_id_conversation]);

			MQTTAsync client = (MQTTAsync)context;
			MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
			MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
			int rc;
			strcat(conversation_topic, conversation_id);
			connectToTopic(client, conversation_topic);
			
			printf("Successful connection with client%d\n", client_id_conversation);
			opts.onSuccess = onSend;
			opts.onFailure = onSendFailure;
			opts.context = client;

			pubmsg.payload =CLIENTID;
			pubmsg.payloadlen = (int)strlen(CLIENTID);
			pubmsg.qos = QOS;
			pubmsg.retained = 0;
			strcpy(conversation_topic_id, TOPIC_STR);
			strcat(conversation_topic_id, (char*)message->payload);
			if ((rc = MQTTAsync_sendMessage(client, conversation_topic_id, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
				printf("Failed to start sendMessage, return code %d\n", rc);
				exit(EXIT_FAILURE);
			}
		}
	} else {
		printf("\nMessage arrived\n");
    	printf("     topic: %s\n", topicName);
		printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
	}
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void onDisconnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("Disconnect failed, rc %d\n", response->code);
	disc_finished = 1;
}

void onDisconnect(void* context, MQTTAsync_successData* response) {
	printf("Successful disconnection\n");
	disc_finished = 1;
}

void onSubscribe(void* context, MQTTAsync_successData* response) {
	printf("Subscribe succeeded\n");
	subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("Subscribe failed, rc %d\n", response->code);
	finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("Connect failed, rc %d\n%s\n", response->code, CLIENTID);
	finished = 1;
}


void onSendFailure(void* context, MQTTAsync_failureData* response) {
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message send failed token %d error code %d\n", response->token, response->code);
	opts.onSuccess = onDisconnect;
	opts.onFailure = onDisconnectFailure;
	opts.context = client;
	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

void onSend(void* context, MQTTAsync_successData* response) {
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message with token value %d delivery confirmed\n", response->token);
	opts.onSuccess = onDisconnect;
	opts.onFailure = onDisconnectFailure;
	opts.context = client;
}


void onConnect(void* context, MQTTAsync_successData* response) {
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	printf("Successful connection\n");

	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", TOPIC, CLIENTID, QOS);
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = client;
	if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start subscribe, return code %d\n", rc);
		finished = 1;
	}
}

void menu(int id) {
    printf("+-------------------------------------------------------+\n");
    printf("|\t\t\tCLIENT [%d]\t\t\t|\n", id);
    printf("+-------------------------------------------------------+\n");
    printf("|1 - Realizar conexão a outro client\t\t\t|\n");
    printf("|2 - Enviar mensagem\t\t\t\t\t|\n");
    printf("|0 - Sair\t\t\t\t\t\t|\n");
    printf("|-> ");
}

void connectToTopic(MQTTAsync client, char topic[]) {
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;
	printf("Subscribing to topic %s\nfor client %s using QoS%d\n", topic, CLIENTID, QOS);
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = client;
	if ((rc = MQTTAsync_subscribe(client, topic, QOS, &opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start subscribe, return code %d\n", rc);
		finished = 1;
	}
}

int toInt(char *str) {
  	int i, pot, ans;
  	ans = 0;
  	for(i = strlen(str) - 1, pot = 1; i >= 0; i--, pot *= 10)
    	ans += pot * (str[i] - '0');
  	return ans;
}

int createId(int a, int b) {
	return pow(2, a) + pow(2, b); 
}
