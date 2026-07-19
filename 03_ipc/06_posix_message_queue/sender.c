#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "protocol.h"

int main(){
	
	mqd_t mq;
	struct mq_attr attr;

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(IPCPacket);
	attr.mq_curmsgs = 0;

	mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY,
			0666, &attr);

	if(mq == (mqd_t)-1){
		perror("mq_open");
		exit(EXIT_FAILURE);
	}

	IPCPacket packet;

	memset(&packet, 0,sizeof(packet));

	packet.magic = MAGIC_NUMBER;
	packet.version = VERSION;
	packet.type = SENSOR_DATA;

	printf("Enter Sensor Message: ");
	fgets(packet.payload, sizeof(packet.payload), stdin);

	packet.payload[strcspn(packet.payload,"\n")] = '\0';
	packet.len = strlen(packet.payload);

	unsigned int priority = 10;

	if(mq_send(mq,
		   (const char *)&packet, 
		   sizeof(packet),
		   priority) == -1)
	{
		perror("mq_send");
		exit(EXIT_FAILURE);
	}

	printf("\n Message sent seuccessfully\n");
	mq_close(mq);

	return 0;
}


				
