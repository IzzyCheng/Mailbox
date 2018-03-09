#include "master.h"

int main(int argc, char **argv)
{
	//create mail
	char query_word[32] = "";
	char file_path[100] = "";
	int K = 2;
	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-q"))
			strcpy(query_word, argv[i+1]);
		else if (!strcmp(argv[i], "-d"))
			strcpy(file_path, argv[i+1]);
		else if (!strcmp(argv[i], "-s"))
			K = atoi(argv[i+1]);
	}

	int pid[K];
	for (int i=0; i<K; i++) {
		pid_t PID = fork();
		if (PID == 0) {
			int execreturn = execl("./slave", NULL);
			if (execreturn == -1)
				printf("exec fail\n");
		} else if (PID < 0)
			printf("fork fail\n");
		else {
			pid[i] = PID;
		}
	}

	int loop = 0;
	int textfile = 0;
	int totalcount = 0;
	struct dirent *dir;
	DIR*dp = opendir(file_path);
	if (dp == NULL)
		printf("open dir fail\n");
	while((dir = readdir(dp)) != NULL) {
		char name[20];
		strcpy(name, dir->d_name);
		if (strcmp(name, ".") && strcmp(name, "..")) {
			char path[200];
			strcpy(path, file_path);
			strcat(path, name);
			textfile++;
			while(1) {
				int recv_fd = open("/sys/kernel/hw2/mailbox", O_RDWR);
				struct mail_t temp, *mail;
				mail = &temp;
				if (receive_from_fd(recv_fd, mail)>0) {
					textfile--;
					totalcount += mail->data.word_count;
				}
				close(recv_fd);
				int send_fd = open("/sys/kernel/hw2/mailbox", O_RDWR);
				struct mail_t sendtemp, *sendmail;
				strcpy(sendtemp.data.query_word, query_word);
				strcpy(sendtemp.file_path, path);
				sendmail = &sendtemp;
				if (send_to_fd(send_fd, sendmail)>0) {
					close(send_fd);
					break;
				}
				close(send_fd);
			}
		}
	}
	closedir(dp);

	while (textfile>0) {
		struct mail_t temp, *mail;
		mail = &temp;
		int sysfs_fd = open("/sys/kernel/hw2/mailbox", O_RDWR);
		if (receive_from_fd(sysfs_fd, mail)>0) {
			textfile--;
			totalcount += mail->data.word_count;
		}
		close(sysfs_fd);
	}

	for (int i=0; i<K; i++)
		kill(pid[i], 9);
	printf("The number of %s is %d\n", query_word, totalcount);

	return 0;
}

int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	//word = query_word+file_path
	char buf[2000] = "";
	strcpy(buf, mail->data.query_word);
	strcat(buf, mail->file_path);

	//send word+path to mailbox
	int ret_val = write(sysfs_fd, buf, sizeof(buf));
	if (ret_val == 10) {
		return ret_val;
	} else {
		return ret_val;
	}
}

int receive_from_fd(int sysfs_fd, struct mail_t *mail)
{
	char buf[100] = "";
	int ret_val = read(sysfs_fd, buf, sizeof(buf));
	if (ret_val == 100) {
		char count[3] = "";
		for (int i=0; i<strlen(buf); i++)
			if (buf[i] == '/')
				break;
			else
				count[i] = buf[i];
		mail->data.word_count = atoi(count);
		for (int i=strlen(count), j=0; i<strlen(buf); i++, j++)
			mail->file_path[j] = buf[i];
		return ret_val;
	} else {
		return ret_val;
	}
}
