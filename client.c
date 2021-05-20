#include "functions.c"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("Argumentos insuficientes, informe o ID do user a ser instanciado\n");
        return 1;
    }
    strcpy(CLIENTID, argv[1]);
	strcat(TOPIC, TOPIC_STR);
	strcat(TOPIC, CLIENTID);
	memset(conversations, 0, sizeof(conversations));
	
	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	int rc;

	if ((rc = MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS) {
		printf("Failed to create client, return code %d\n", rc);
		rc = EXIT_FAILURE;
		goto exit;
	}

	if ((rc = MQTTAsync_setCallbacks(client, client, connlost, msgarrvd, NULL)) != MQTTASYNC_SUCCESS) {
		printf("Failed to set callbacks, return code %d\n", rc);
		rc = EXIT_FAILURE;
		goto destroy_exit;
	}

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start connect, return code %d\n", rc);
		rc = EXIT_FAILURE;
		goto destroy_exit;
	}

	while (!subscribed && !finished)

	if (finished)
		goto exit;

	int option = -1;
	int while_true = 1;
	int client_id;
	char message[MESSAGE_SIZE];
	char topic_message[MAX_SIZE_CHAR], topic_message_id[MAX_SIZE_CHAR];

	while (while_true) {
		switch(option){
            case -1:
                menu(toInt(CLIENTID));
                while(scanf("%d", &option) != 1) 
                    getchar();
                break;
			case 1:
                printf("\t\t\tInforme o id do client\n(Os ids precisam estar entre 1 e %d!)\n", MAX_CLIENTS);
                scanf("%d", &client_id);

				if (conversations[client_id] == 0) {
					printf("Solicitando conexão com client%d\nAguarde a conexão ser concluida para enviar novas mensagens\n", client_id);
					opts.onSuccess = onSend;
					opts.onFailure = onSendFailure;
					opts.context = client;
					sprintf(message, "%d", client_id);
					pubmsg.payload = CLIENTID;
					pubmsg.payloadlen = (int)strlen(CLIENTID);
					pubmsg.qos = QOS;
					pubmsg.retained = 0;
					strcpy(topic_message, TOPIC_STR);
					strcat(topic_message, message);
					if ((rc = MQTTAsync_sendMessage(client, topic_message, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
						printf("Failed to start sendMessage, return code %d\n", rc);
						exit(EXIT_FAILURE);
					}
				} else {
					printf("Conexão com o client%d já foi estabelecida\n", client_id);
				}
				option = -1;
                sleep(1);
				break;
			case 2:
				printf("\t\t\tInforme o id do client\n(O clinte precisa estar conectado!)\n");
                scanf("%d", &client_id);
				if (conversations[client_id] == 0) {
					printf("O client%d não está conectado!\n", client_id);
					option = -1;
					break;
				}
				printf("Insira a mensagem:\n");
				getchar();
				fgets(message, MESSAGE_SIZE, stdin);

				opts.onSuccess = onSend;
				opts.onFailure = onSendFailure;
				opts.context = client;
				pubmsg.payload = message;
				pubmsg.payloadlen = (int)strlen(message);
				pubmsg.qos = QOS;
				pubmsg.retained = 0;
				sprintf(topic_message_id, "%d", conversations[client_id]);
				strcpy(topic_message, CONVERSATION_STR);
				strcat(topic_message, topic_message_id);
				if ((rc = MQTTAsync_sendMessage(client, topic_message, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
					printf("Failed to start sendMessage, return code %d\n", rc);
					exit(EXIT_FAILURE);
				}
				sleep(1);
                option = -1;
				break;
			case 0:
				while_true = 0;
				break;
			default:
                printf("\t\t\tOPÇÃO INVÁLIDA! \nInforme uma opção existente no menu!\n");
                sleep(2);
                option = -1;
                break;
		}
	}

	disc_opts.onSuccess = onDisconnect;
	disc_opts.onFailure = onDisconnectFailure;
	if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start disconnect, return code %d\n", rc);
		rc = EXIT_FAILURE;
		goto destroy_exit;
	}
 	while (!disc_finished) {
		usleep(10000L);
 	}

destroy_exit:
	MQTTAsync_destroy(&client);
exit:
 	return rc;
}