/*
	SOURCE:

	https://www.gnu.org/software/libc/manual/html_node/Inet-Example.html

	https://www.gnu.org/software/libc/manual/html_node/Server-Example.html

*/

# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <sys/select.h>
# include <strings.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>


# define false 0
# define true 1
# define bool int

typedef struct s_client	t_client;
typedef struct s_server	t_server;

void	goodbye_client(t_server *server, int client_fd);

/*
	store all clients in a linked list
	new client is always add at the end
*/

typedef struct s_client 
{
	int			fd;
	int			id;
	char		*send_buffer;
	char		*read_buffer;
	t_client	*prev;
	t_client	*next;
}	t_client;

typedef struct s_server {
	int 		sock;
	t_client	*lst;
	int			num;
	fd_set		all_fd;
}	t_server;

bool	add_client(t_client **lst, int fd, int id)
{
	t_client	*new_client;
	t_client	*last;

	new_client = malloc(sizeof(t_client));
	if (!new_client)
	{
		return (false);
	}
	new_client->fd = fd;
	new_client->id = id;
	new_client->read_buffer = NULL;
	new_client->send_buffer = NULL;
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
		prev->next = next;
	if (next)
		next->prev = prev;	
	if (curr == *lst)
		*lst = (*lst)->next;
	if (curr == NULL)
		return ;
	close(curr->fd);
	free(curr->read_buffer);
	free(curr->send_buffer);
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
		close(node->fd);
		free(node->read_buffer);
		free(node->send_buffer);
		free(node);
		node = next;
	}
	*lst = NULL;
}

void	free_quit(t_server *server, char *message)
{
	if (server)
	{
		free_lst(&(server->lst));
		close(server->sock);
	}
	write(2, message, strlen(message));
	write(2, "\n", 1);
	exit(1);
}

/*
	return pointer to client
	NULL if not found
*/
t_client	*get_client(t_client *lst, int fd)
{
	while (lst)
	{
		if (lst->fd == fd)
			return (lst);
		lst = lst->next;
	}
	return (NULL);
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

int	create_server(uint16_t port)
{
	int					sock;
	struct sockaddr_in	name;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		free_quit(NULL, "Fatal error");
	}
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		free_quit(NULL, "Fatal error");
	}
	return (sock);
}

// 'client %d: '
// Check if client quit as well
void	client_says(t_server *server, int client_fd)
{
	t_client	*lst = server->lst;
	t_client	*client = get_client(server->lst, client_fd);
	int			result;
	char		temp[11];
	char		header[30];
	char		*buffer = NULL;

	char		*mess = NULL;
	char		*line = NULL;

	printf("client_says\n");
	// read the message from client_fd into buffer, the join with client->read_buffer
	for (int i = 0; true; i++)
	{
		bzero(temp, 11);
		result = read(client_fd, temp, 10);
		if (result == 0 && i == 0)
			return (goodbye_client(server, client_fd));
		if (result <= 0)
			break ;
		buffer = str_join(buffer, temp);
	}
	client->read_buffer = str_join(client->read_buffer, buffer);
	// extract message from 'read_buffer' into 'mess', formatted
	sprintf(header, "client %d: ", client->id);
	while (true)
	{
		result = extract_message(&(client->read_buffer), &line);
		if (result == -1)
			free_quit(server, "malloc fails");
		if (result == 0)
			break ;
		str_join(mess, header);
		str_join(mess, line);
	}
	// send message to others 
	while (lst)
	{
		if (lst->id == client->id)
			continue ;
		lst->send_buffer = str_join(lst->send_buffer, mess);
		if (lst->send_buffer == NULL)
			free_quit(server, "malloc fails\n");
		lst = lst->next;
	}
}

// Accept new client, add to list, welcome message
void	hello_client(t_server *server)
{
	int					client_fd;
	struct sockaddr_in	clientname;
	size_t				size = sizeof (clientname);
	t_client			*lst = server->lst;
	char 				buffer[40];

	printf("hello client\n");
	client_fd = accept(server->sock, (struct sockaddr *) &clientname, (unsigned int *) (&size));
	if (client_fd < 0)
	{
		free_quit(server, "accept fails\n");
	}
	add_client(&(server->lst), client_fd, (server->num)++);
	FD_SET (client_fd, &(server->all_fd));
	sprintf(buffer, "server: client %d just arrived\n", server->num - 1);
	while (lst)
	{
		lst->send_buffer = str_join(lst->send_buffer, buffer);
		if (lst->send_buffer == NULL)
			free_quit(server, "Fatal error");
		lst = lst->next;
	}
}

void	goodbye_client(t_server *server, int client_fd)
{
	t_client	*lst = server->lst;
	char 		buffer[40];
	t_client	*client = get_client(server->lst, client_fd);

	printf("goodbye client\n");
	sprintf(buffer, "server: client %d just left\n", client->id);
	rm_client(&(server->lst), client_fd);
	FD_CLR(client_fd, &(server->all_fd));
	while (lst)
	{
		lst->send_buffer = str_join(lst->send_buffer, buffer);
		if (lst->send_buffer == NULL)
			free_quit(server, "Fatal error");
		lst = lst->next;
	}
}

/*
	send everything in the send_buffer to client
*/
void write_to_client(t_server *server, int client_fd)
{
	t_client	*client = get_client(server->lst, client_fd);

	if (client->send_buffer == NULL)
		return ;
	write(client_fd, client->send_buffer, strlen(client->send_buffer));
	free(client->send_buffer);
	client->send_buffer = NULL;
}


void	server_run(int sock)
{
	t_server	server;
	
	server.sock = sock;
	server.lst = NULL;
	server.num = 0;
	
	fd_set	read_fd;
	fd_set	write_fd;

	FD_ZERO (&(server.all_fd));
	FD_ZERO (&read_fd);
	FD_ZERO (&write_fd);
	FD_SET (sock, &(server.all_fd));

	if (listen(sock, 10) < 0)
	{
		free_quit(NULL, "Fatal error\n");
	}
	while (true)
	{
		read_fd = server.all_fd;
		write_fd = server.all_fd;
		if (select(FD_SETSIZE, &read_fd, &write_fd, NULL, NULL) < 0)
		{
			free_quit(&server, "select fails");
		}
		for (int i = 0; i < FD_SETSIZE; i++)
		{
			// can read
			if (FD_ISSET (i, &read_fd))
			{
				if (i == sock)
				{
					printf("new client\n");
					// hello_client(&server);
				}
				else
				{
					printf("client says sth\n");
					// client_says(&server, i);
				}
			}
			// can write
			if (FD_ISSET (i, &write_fd))
			{
				// write_to_client(&server, i);
			}
		}
	}

}

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	int	port = atoi(argv[1]);
	int	sock = create_server((uint16_t) port);
	server_run(sock);
}

