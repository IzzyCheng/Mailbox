#include "mailbox.h"

MODULE_LICENSE("Dual BSD/GPL");

static void get_process_name(char *ouput_name);
static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf, size_t count);
static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count);

static struct kobject *hw2_kobject;
static struct kobj_attribute mailbox_attribute
    = __ATTR(mailbox, 0660, mailbox_read, mailbox_write);

static int num_entry_max = 2;

module_param(num_entry_max, int, S_IRUGO);

static void get_process_name(char *ouput_name)
{
	memcpy(ouput_name, current->comm, sizeof(current->comm));
}

//Start here////////////////////////////////

LIST_HEAD(boxhead);
static int entrycount = 0;


static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf, size_t count)
{
	printk("read\n");
	if (entrycount == 0)
		return -1;

	char from[sizeof(current->comm)];
	get_process_name(from);
	if (!strcmp(from, "master")) {
		struct list_head *listptr;
		struct mailbox_entry_t *entry;
		list_for_each(listptr, &boxhead) {
			entry = list_entry(listptr, struct mailbox_entry_t, list);
			if (entry->search) {
				list_del_init(listptr);
				sprintf(buf, "%d%s", entry->word_count, entry->file_path);
				printk("master read %s\n", buf);
				kfree(entry);
				entrycount--;
				return 100;
			}
		}
		return -1;          //no searched file
	} else if (!strcmp(from, "slave")) {
		struct list_head *listptr;
		struct mailbox_entry_t *entry;
		list_for_each(listptr, &boxhead) {
			entry = list_entry(listptr, struct mailbox_entry_t, list);
			if (!entry->search) {
				list_del_init(listptr);
				sprintf(buf, "%s%s", entry->query_word, entry->file_path);
				printk("slave read %s\n", buf);
				kfree(entry);
				entrycount--;
				return 100;
			}
		}
		return -1;          //no unsearch file
	}
	return -1;
}

static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count)
{
	if (entrycount < num_entry_max) {
		if (count == 2000) {        //from master
			struct mailbox_entry_t *newentry;
			newentry = (struct mailbox_entry_t*)kmalloc(sizeof(struct mailbox_entry_t),
			           GFP_KERNEL);
			list_add_tail(&newentry->list, &boxhead);
			strcpy(newentry->query_word, "");
			newentry->word_count = 0;
			strcpy(newentry->file_path, "");
			entrycount++;

			newentry->search = false;
			char tempword[10] = "";
			char temppath[100] = "";
			for (int i=0; i<strlen(buf); i++)
				if (buf[i] == '/')
					break;
				else
					tempword[i] = buf[i];
			strcpy(newentry->query_word, tempword);
			for (int i=strlen(tempword), j=0; i<strlen(buf); i++, j++)
				temppath[j] = buf[i];
			strcpy(newentry->file_path, temppath);
			printk("master : word %s\n", newentry->query_word);
			printk("master : path %s\n", newentry->file_path);
		} else if (count == 1000) { //from slave
			struct mailbox_entry_t *newentry;
			newentry = (struct mailbox_entry_t*)kmalloc(sizeof(struct mailbox_entry_t),
			           GFP_KERNEL);
			list_add_tail(&newentry->list, &boxhead);
			strcpy(newentry->query_word, "");
			newentry->word_count = 0;
			strcpy(newentry->file_path, "");
			entrycount++;

			newentry->search = true;
			char count[3] = "";
			int tempcount = 0;
			for (int i=0; i<strlen(buf); i++)
				if (buf[i] == '/')
					break;
				else
					count[i] = buf[i];
			for (int i=0; count[i] != '\0'; i++)
				tempcount = tempcount*10 + count[i] - '0';
			newentry->word_count = tempcount;
			for (int i=strlen(count), j=0; i<strlen(buf); i++, j++)
				newentry->file_path[j] = buf[i];
			printk("slave : count %d\n", newentry->word_count);
			printk("slave : path %s\n", newentry->file_path);
		}
		printk("entry count : %d\n", entrycount);
		return 10;
	} else return -1;
}

////////////////////////////////////////////

static int __init mailbox_init(void)
{
	spinlock_t lock;
	//spin_lock_init(&lock);
	hw2_kobject = kobject_create_and_add("hw2", kernel_kobj);
	sysfs_create_file(hw2_kobject, &mailbox_attribute.attr);
	return 0;
}

static void __exit mailbox_exit(void)
{
	kobject_put(hw2_kobject);
}

module_init(mailbox_init);
module_exit(mailbox_exit);
