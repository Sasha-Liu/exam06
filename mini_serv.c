/*
	SOURCE:

	https://www.gnu.org/software/libc/manual/html_node/Inet-Example.html

	https://www.gnu.org/software/libc/manual/html_node/Server-Example.html

*/

# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <sys/select.h>

/*
	store all clients in a linked list
	new client is always add at the end
*/
typedef struct s_client	t_client;

typedef struct s_client 
{
	int			fd;
	int			id;
	t_client	*prev;
	t_client	*next;
}	t_client;

bool	add_client(t_client **lst, int fd, int id)
{
	t_client	*new_client;
	t_client	*last;

	new_client = malloc(sizeof t_client);
	if (!new_client)
	{
		return (false)
	}
	new_client->fd = fd;
	new_client->id = id;
	new_client->prev = NULL;
	new_client->next = NULL;
	if (*lst == NULL)
	{
		*lst = new_client;
		return (true);
	}
	last = *lst;
	while (last->next)
	{
		last = last->next;
	}
	last->next = new_client;
	new_client->prev = last;
	return (true);
}

void	rm_client(t_client **lst, int fd)
{
	t_client	*prev;
	t_client	*curr;
	t_client	*next;

	prev = NULL;
	curr = *lst;
	next = NULL;
	while (curr && curr->fd != fd)
	{
		prev = curr;
		curr = curr->next;
		if (curr)
			next = curr->next;
	}
	if (prev)
		prev.next = next;
	if (next)
		next.prev = prev;	
	if (curr == *lst)
		*lst = (*lst)->next;
	free(curr);
}

/*
	free all node
	close all client fd
*/
void	free_lst(t_client **lst)
{
	t_client	*node;
	t_client	*next;

	node = *lst;
	while (node)
	{
		next = node->next;
		free(node);
		close(node->fd);
		node = next;
	}
	*lst = NULL;
}

/*
	return id of client
	-1 if not found
*/
int	get_client_id(t_client *lst, int fd)
{
	while (lst)
	{
		if (lst->fd == fd)
			return (lst->id);
		lst = lst->next;
	}
	return (-1);
}

/*
	Given by subject
	This function extract the first line from *buf and assign it to *msg
	*buf is then advanced by one line
	return 1 => extract one line
	return 0 => no line extracted
	return -1 => malloc fails
*/
int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

/*
	Given by subject

	return NULL => malloc fails
	return newbuf = buf + add
*/
char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void	fatal_error()
{
	write(2, 'Fatal error\n', 12);
	exit(1);
}

int	ft_atoi(char *str)
{
	int num;

	num = 0;
	for (int i = 0; str[i]; i++)
	{
		num = num * 10 + str[i] - '0';
	}
	return (num);
}

int	getPort(int argc, char **argv)
{
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	return (ft_atoi(argv[1]));
}

int	create_server(uint16_t port)
{
	int					sock;
	struct sockaddr_in	name;

	sock = socket(AF_INET, SOCK_STREAM, 0)
	if (socket < 0)
	{
		fatal_error();
	}
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		fatal_error();
	}
	if (listen(sock, 10) < 0)
	{
		fatal_error();
	}
	return (sock);
}

void	server_run(int sock)
{
	int 	client_num = 0;
	fd_set	all_fd;
	fd_set	read_fd;
	fd_set	write_fd;

	FD_ZERO (&all_fd);
	FD_ZERO (&read_fd);
	FD_ZERO (&write_fd);
	FD_SET (sock, &all_fd);

	while (true)
	{
		read_fd = all_fd;
		write_fd = all_fd;
		if (select(FD_SETSIZE, read_fd, write_fd, NULL, NULL) < 0)
		{
			// close all fds and exit
		}
		for (i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET (i, &read_fd))
			{
				if (i == sock)
				{
					// new client
				}
				else
				{
					// check end of file
				}
			}
			if (FD_ISSET (i, &write_fd))
			{

			}
		}
	}

}

int	main(int argc, char **argv)
{
	int	port = getPort(argc, argv);
	int	sock = create_server((uint16_t) port);
}
