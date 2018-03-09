#include "slave.h"

int main(int argc, char **argv)
{
	while(1) {
		struct mail_t temp, *mail;
		mail = &temp;
		int sysfs_fd = open("/sys/kernel/hw2/mailbox", O_RDWR);
		if (sysfs_fd<0)
			printf("openfail\n");
		if (receive_from_fd(sysfs_fd, mail) < 0) {
			close(sysfs_fd);
			continue;
		}

		unsigned int count = 0;
		FILE *readfile = fopen(mail->file_path, "r");
		char search[1000];
		while (fgets(search, 1000, readfile) != NULL) {
			while (1) {
				char *loc = strstr(search, mail->data.query_word);
				if (loc == NULL)
					break;
				char pos[3];
				sprintf(pos, "%lu", loc-search);
				int posi = atoi(pos);
				int last = posi+strlen(mail->data.query_word);
				if (posi>0)
					if (search[posi-1] == ' ' && (search[last] == ' ' || search[last] == '.'
					                              || search[last] == '!' || search[last] == '?' || search[last] == ','))
						count++;
				sprintf(search, "%s", loc+strlen(mail->data.query_word));
			}
		}
		fclose(readfile);

		readfile = fopen(mail->file_path, "r");
		mail->data.query_word[0] -=32;
		while (fgets(search, 1000, readfile) != NULL)
			while (1) {
				char *loc = strstr(search, mail->data.query_word);
				if (loc == NULL)
					break;
				char pos[3];
				sprintf(pos, "%lu", loc-search);
				int posi = atoi(pos);
				int last = posi+strlen(mail->data.query_word);
				if (posi>0) {
					if (search[posi-1] == ' ' && (search[last] == ' ' || search[last] == '.'
					                              || search[last] == ','))
						count++;
				} else if (posi == 0)
					if (search[last] == ' ' || search[last] == '.' || search[last] == ',')
						count++;
				sprintf(search, "%s", loc+strlen(mail->data.query_word));
			}
		fclose(readfile);

		struct mail_t result;
		strcpy(result.file_path, mail->file_path);
		result.data.word_count = count;
		mail = &result;

		sysfs_fd = open("/sys/kernel/hw2/mailbox", O_RDWR);
		send_to_fd(sysfs_fd, mail);
		close(sysfs_fd);
	}

	return 0;
}

int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	char buf[1000] = "";
	sprintf(buf, "%d%s", mail->data.word_count, mail->file_path);
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
		for (int i=0; i<strlen(buf); i++)
			if (buf[i] == '/')
				break;
			else
				mail->data.query_word[i] = buf[i];
		for (int i=strlen(mail->data.query_word), j=0; i<strlen(buf); i++, j++)
			mail->file_path[j] = buf[i];
		return ret_val;
	} else {
		return ret_val;
	}
}
