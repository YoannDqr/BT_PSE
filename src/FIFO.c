#include "pse.h"

int main(void)
{
	if(mkfifo("FIFO_SERVEUR", 0660) == -1)
	{
		perror("mkfifo");
		exit(EXIT_FAILURE);
	}
}
