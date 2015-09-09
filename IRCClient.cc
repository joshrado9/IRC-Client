#include <gtk/gtk.h>
#include <time.h>
//#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

GtkListStore * list_rooms;
GtkListStore * list_users;

int room = 0; 
char * username = (char*)" "; 
char * password = (char*)" "; 
char * roomname; 
char * message; 
char * command;
char * host = (char*)"localhost";
int port = 9999;
char * response = (char*)malloc(sizeof(char)*100);

int open_client_socket(char * host, int port) {
        // Initialize socket address structure
        struct  sockaddr_in socketAddress;

        // Clear sockaddr structure
        memset((char *)&socketAddress,0,sizeof(socketAddress));
                
        // Set family to Internet
        socketAddress.sin_family = AF_INET;
        // Set port
        socketAddress.sin_port = htons((u_short)port);

        // Get host table entry for this host
        struct  hostent  *ptrh = gethostbyname(host);
        if ( ptrh == NULL ) {
                perror("gethostbyname");
                exit(1);
        }

        // Copy the host ip address to socket address structure
        memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

        // Get TCP transport protocol entry
        struct  protoent *ptrp = getprotobyname("tcp");
        if ( ptrp == NULL ) {
                perror("getprotobyname");
                exit(1);
        }

        // Create a tcp socket
        int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
        if (sock < 0) {
                perror("socket");
                exit(1);
        }

        // Connect the socket to the specified server
        if (connect(sock, (struct sockaddr *)&socketAddress,
                    sizeof(socketAddress)) < 0) {
                perror("connect");
                exit(1);
        }

        return sock;
}


#define MAX_RESPONSE (10 * 1024)
int sendCommand(char *  host, int port, char * command, char * response) {

        int sock = open_client_socket( host, port);
        
        if (sock<0) {
                return 0;
        }

        // Send command
        write(sock, command, strlen(command));
        write(sock, "\r\n",2);

        //Print copy to stdout
        write(1, command, strlen(command));
        write(1, "\r\n",2);
         
        // Keep reading until connection is closed or MAX_REPONSE
        int n = 0;
        int len = 0;
        while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
                len += n;
        }
        response[len]=0;

        printf("response:\n%s\n", response);
 
        close(sock);

        return 1;
}


void update_list_rooms()
{

}

/* Create the list of "messages" */
static GtkWidget *create_list( const char * titleColumn, GtkListStore *model )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    //GtkListStore *model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    int i;
   
    /* Create a new scrolled window, with scrollbars only if needed */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC, 
				    GTK_POLICY_AUTOMATIC);
   
    //model = gtk_list_store_new (1, G_TYPE_STRING);
    tree_view = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
    gtk_widget_show (tree_view);
   
    cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (titleColumn,
                                                       cell,
                                                       "text", 0,
                                                       NULL);
  
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
	  		         GTK_TREE_VIEW_COLUMN (column));

    return scrolled_window;
}
   
/* Add some text to our text widget - this is a callback that is invoked
when our window is realized. We could also force our window to be
realized with gtk_widget_realize, but it would have to be part of
a hierarchy first */

static void insert_text( GtkTextBuffer *buffer, const char * initialText )
{
   GtkTextIter iter;
 
   gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
   gtk_text_buffer_insert (buffer, &iter, initialText,-1);
}
   
/* Create a scrolled text area that displays a "message" */
static GtkWidget *create_text( const char * initialText )
{
   GtkWidget *scrolled_window;
   GtkWidget *view;
   GtkTextBuffer *buffer;

   view = gtk_text_view_new ();
   buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

   scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
		   	           GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);

   gtk_container_add (GTK_CONTAINER (scrolled_window), view);
   insert_text (buffer, initialText);

   gtk_widget_show_all (scrolled_window);

   return scrolled_window;
}

static gboolean
time_handler(GtkWidget *widget)
{
  if (widget->window == NULL) return FALSE;

  //gtk_widget_queue_draw(widget);

  //fprintf(stderr, "Hi\n");
  //update_list_rooms();

  return TRUE;
}

void createInput(GtkWidget *widget, gpointer data)
{
	GtkWidget *temp = (GtkWidget*) data;
	const gchar * text = gtk_entry_get_text(GTK_ENTRY (temp));

	if (strcmp("ADD-USER", command) == 0)
	{
		if (*username == ' ')
		{
			username = (char*)text;
			return;
		}
		else
		{
			password = (char*)text;
			strcat(response, username);
			strcat(response, password);
			sendCommand(host, port, command, response);
			return;
		}
	}
	else if (strcmp("SET-LOGIN", command) == 0)
	{
		if (*username == ' ')
                {
                        username = (char*)text;
			return;
                }   
                else
                {
			password = (char*)text;
			return;
		}
	}

	sendCommand(host, port, command, response);
	
}

void enterRoom(GtkWidget * widget, gpointer data)
{
	GtkWidget *enterWindow;
        enterWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title (GTK_WINDOW (enterWindow), "Enter A Room");
        g_signal_connect (enterWindow, "destroy", G_CALLBACK (gtk_main_quit), NULL);
        gtk_container_set_border_width (GTK_CONTAINER (enterWindow), 10);
        gtk_widget_set_size_request (GTK_WIDGET (enterWindow), 250, 200);
        gtk_widget_show(enterWindow);
        
        GtkWidget *table = gtk_table_new (4, 4, TRUE);
        gtk_container_add (GTK_CONTAINER (enterWindow), table);
        gtk_table_set_row_spacings(GTK_TABLE (table), 5);
        gtk_table_set_col_spacings(GTK_TABLE (table), 5);
        gtk_widget_show (table);
 
        GtkWidget *labelName = gtk_label_new("Which Room Do You Want to Enter");
        gtk_table_attach_defaults (GTK_TABLE (table), labelName, 0, 4, 0, 1);
        gtk_widget_show (labelName);

	list_rooms = gtk_list_store_new (1, G_TYPE_STRING);
	//update_list_rooms();
	GtkWidget *listRooms = create_list ("Rooms", list_rooms);
	gtk_table_attach_defaults (GTK_TABLE (table), listRooms, 0, 4, 1, 4);
	gtk_widget_show (listRooms);

	command = (char*) "ENTER-ROOM";

	GtkWidget *done_button = gtk_button_new_with_label ("Enter Room");
        gtk_table_attach_defaults(GTK_TABLE (table), done_button, 0, 4, 4, 5);
        gtk_signal_connect(GTK_OBJECT (done_button), "clicked", GTK_SIGNAL_FUNC (createInput), listRooms);
	g_signal_connect_swapped(GTK_OBJECT (done_button), "clicked", G_CALLBACK (gtk_widget_destroy), enterWindow);
        gtk_widget_show (done_button);

	gtk_widget_show(enterWindow);
        gtk_widget_show(table);
}

void leaveRoom(GtkWidget * widget, gpointer data)
{
        GtkWidget *leaveWindow;
        leaveWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title (GTK_WINDOW (leaveWindow), "Leave A Room");
        g_signal_connect (leaveWindow, "destroy", G_CALLBACK (gtk_main_quit), NULL);
        gtk_container_set_border_width (GTK_CONTAINER (leaveWindow), 10);
        gtk_widget_set_size_request (GTK_WIDGET (leaveWindow), 250, 200);
        gtk_widget_show(leaveWindow);
        
        GtkWidget *table = gtk_table_new (4, 4, TRUE);
        gtk_container_add (GTK_CONTAINER (leaveWindow), table);
        gtk_table_set_row_spacings(GTK_TABLE (table), 5);
        gtk_table_set_col_spacings(GTK_TABLE (table), 5);
        gtk_widget_show (table);
 
        GtkWidget *labelName = gtk_label_new("Which Room Do You Want to Leave");
        gtk_table_attach_defaults (GTK_TABLE (table), labelName, 0, 4, 0, 1);
        gtk_widget_show (labelName);

        list_rooms = gtk_list_store_new (1, G_TYPE_STRING);
        //update_list_rooms();
        GtkWidget *listRooms = create_list ("Rooms", list_rooms);
        gtk_table_attach_defaults (GTK_TABLE (table), listRooms, 0, 4, 1, 4);
        gtk_widget_show (listRooms);
	command = (char*) "LEAVE-ROOM";
   
        GtkWidget *done_button = gtk_button_new_with_label ("Leave Room");
        gtk_table_attach_defaults(GTK_TABLE (table), done_button, 0, 4, 4, 5);
        gtk_signal_connect(GTK_OBJECT (done_button), "clicked", GTK_SIGNAL_FUNC (createInput), listRooms);
	g_signal_connect_swapped(GTK_OBJECT (done_button), "clicked", G_CALLBACK (gtk_widget_destroy), leaveWindow);
        gtk_widget_show (done_button);
        
        gtk_widget_show(leaveWindow);
        gtk_widget_show(table);
}

void createRoom(GtkWidget * widget, gpointer data)
{
	GtkWidget *roomWindow;
	roomWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title (GTK_WINDOW (roomWindow), "Create Your Room");
        gtk_container_set_border_width (GTK_CONTAINER (roomWindow), 10);
        gtk_widget_set_size_request (GTK_WIDGET (roomWindow), 250, 200);
        gtk_widget_show(roomWindow);

        GtkWidget *table = gtk_table_new (2, 2, TRUE);
        gtk_container_add (GTK_CONTAINER (roomWindow), table);
        gtk_table_set_row_spacings(GTK_TABLE (table), 5);
        gtk_table_set_col_spacings(GTK_TABLE (table), 5);
        gtk_widget_show (table);
  
        GtkWidget *labelName = gtk_label_new("Enter A Room Name");
        gtk_table_attach_defaults (GTK_TABLE (table), labelName, 0, 3, 0, 2);
        gtk_widget_show (labelName);
 
	GtkWidget *roomName = gtk_entry_new();
	gtk_table_attach_defaults (GTK_TABLE (table), roomName, 0, 3, 1, 2);
        gtk_widget_show (roomName);

	command = (char*) "CREATE-ACCOUNT";

	GtkWidget *done_button = gtk_button_new_with_label ("Create Room");
        gtk_table_attach_defaults(GTK_TABLE (table), done_button, 0, 3, 2, 3);
	gtk_signal_connect(GTK_OBJECT (done_button), "clicked", GTK_SIGNAL_FUNC (createInput), roomName);
	g_signal_connect_swapped(GTK_OBJECT (done_button), "clicked", G_CALLBACK (gtk_widget_destroy), roomWindow);
	gtk_widget_show (done_button);

        gtk_widget_show(roomWindow);
        gtk_widget_show(table);
}

void loginAccount(GtkWidget * widget, gpointer data)
{
	GtkWidget *loginWindow;
        loginWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title (GTK_WINDOW (loginWindow), "Login to Your Account");
        g_signal_connect (loginWindow, "destroy", G_CALLBACK (gtk_main_quit), NULL);
        gtk_container_set_border_width (GTK_CONTAINER (loginWindow), 10);
        gtk_widget_set_size_request (GTK_WIDGET (loginWindow), 250, 200);
        gtk_widget_show(loginWindow);

        GtkWidget *table = gtk_table_new (4, 4, TRUE);
        gtk_container_add (GTK_CONTAINER (loginWindow), table);
        gtk_table_set_row_spacings(GTK_TABLE (table), 5);
        gtk_table_set_col_spacings(GTK_TABLE (table), 5);
        gtk_widget_show (table);
        
        GtkWidget *labelUser = gtk_label_new("Username");
        gtk_table_attach_defaults (GTK_TABLE (table), labelUser, 0, 4, 0, 1);
        gtk_widget_show (labelUser);
        
        GtkWidget *username = gtk_entry_new();
        gtk_table_attach_defaults (GTK_TABLE (table), username, 0, 4, 1, 2);
        gtk_widget_show (username);
        
        GtkWidget *labelPassword = gtk_label_new("Password");
        gtk_table_attach_defaults (GTK_TABLE (table), labelPassword, 0, 4, 2, 3);
        gtk_widget_show (labelPassword);
        
        GtkWidget *password = gtk_entry_new();
        gtk_table_attach_defaults (GTK_TABLE (table), password, 0, 4, 3, 4);
        gtk_widget_show (password);

	command = (char*)"SET-LOGIN";

	GtkWidget *login_button = gtk_button_new_with_label ("Login");
        gtk_table_attach_defaults(GTK_TABLE (table), login_button, 0, 4, 4, 5);
	gtk_signal_connect(GTK_OBJECT (login_button), "clicked", GTK_SIGNAL_FUNC (createInput), username);
        gtk_signal_connect(GTK_OBJECT (login_button), "clicked", GTK_SIGNAL_FUNC (createInput), password);
        gtk_signal_connect(GTK_OBJECT (login_button), "clicked", G_CALLBACK (gtk_widget_destroy), loginWindow);
        gtk_widget_show (login_button);
 
        gtk_widget_show(loginWindow);
        gtk_widget_show(table);
}

void createAccount(GtkWidget * widget, gpointer data)
{
        GtkWidget *accountWindow;
	accountWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (accountWindow), "Create Your Account");
	g_signal_connect (accountWindow, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (accountWindow), 10);
	gtk_widget_set_size_request (GTK_WIDGET (accountWindow), 250, 200);
	gtk_widget_show(accountWindow);

	GtkWidget *table = gtk_table_new (4, 4, TRUE);
	gtk_container_add (GTK_CONTAINER (accountWindow), table);
	gtk_table_set_row_spacings(GTK_TABLE (table), 5);
	gtk_table_set_col_spacings(GTK_TABLE (table), 5);
	gtk_widget_show (table);

	GtkWidget *labelUser = gtk_label_new("Username");
	gtk_table_attach_defaults (GTK_TABLE (table), labelUser, 0, 4, 0, 1);
	gtk_widget_show (labelUser);

	GtkWidget *username = gtk_entry_new();
	gtk_table_attach_defaults (GTK_TABLE (table), username, 0, 4, 1, 2);
	gtk_widget_show (username);

	GtkWidget *labelPassword = gtk_label_new("Password");
        gtk_table_attach_defaults (GTK_TABLE (table), labelPassword, 0, 4, 2, 3);
        gtk_widget_show (labelPassword);

	GtkWidget *password = gtk_entry_new();
        gtk_table_attach_defaults (GTK_TABLE (table), password, 0, 4, 3, 4);
        gtk_widget_show (password);

	command = (char*)"ADD-USER";

	GtkWidget *done_button = gtk_button_new_with_label ("Create");
	gtk_table_attach_defaults(GTK_TABLE (table), done_button, 0, 4, 4, 5);
	gtk_signal_connect(GTK_OBJECT (done_button), "clicked", GTK_SIGNAL_FUNC (createInput), username);
	gtk_signal_connect(GTK_OBJECT (done_button), "clicked", GTK_SIGNAL_FUNC (createInput), password);
	g_signal_connect_swapped(GTK_OBJECT (done_button), "clicked", G_CALLBACK (gtk_widget_destroy), accountWindow);
	gtk_widget_show (done_button);

	gtk_widget_show(accountWindow);
	gtk_widget_show(table);
}


int main( int   argc,
          char *argv[] )
{
    GtkWidget *window;
    GtkWidget *listRooms;
    GtkWidget *listUsers;
    GtkWidget *messages;
    GtkWidget *myMessage;

    gtk_init (&argc, &argv);
   
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Paned Windows");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (GTK_WIDGET (window), 450, 400);

    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    GtkWidget *table = gtk_table_new (8, 4, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    // Add list of rooms. Use columns 0 to 4 (exclusive) and rows 0 to 4 (exclusive)
    list_rooms = gtk_list_store_new (1, G_TYPE_STRING);
    //update_list_rooms();
    listRooms = create_list ("Rooms", list_rooms);
    gtk_table_attach_defaults (GTK_TABLE (table), listRooms, 0, 2, 0, 2);
    gtk_widget_show (listRooms);

    // Add list of USERS. 
    list_users = gtk_list_store_new (1, G_TYPE_STRING);
    //update_list_rooms();
    listUsers = create_list ("Users", list_users);
    gtk_table_attach_defaults (GTK_TABLE (table), listUsers, 2, 4, 0, 2);
    gtk_widget_show (listUsers);
   
    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive) 
    messages = create_text ("Peter: Hi how are you\nMary: I am fine, thanks and you?\nPeter: Fine thanks.\n");
    gtk_table_attach_defaults (GTK_TABLE (table), messages, 0, 4, 2, 5);
    gtk_widget_show (messages);

    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive) 
    myMessage = create_text ("I am fine, thanks and you?\n");
    gtk_table_attach_defaults (GTK_TABLE (table), myMessage, 0, 4, 5, 7);
    gtk_widget_show (myMessage);

    // Add send button. Use columns 0 to 1 (exclusive) and rows 4 to 7 (exclusive)
    GtkWidget *send_button = gtk_button_new_with_label ("Send");
    gtk_table_attach_defaults(GTK_TABLE (table), send_button, 0, 1, 7, 9);
    gtk_widget_show (send_button);

    //Add Create Account Button
    GtkWidget *create_account_button = gtk_button_new_with_label ("Create Account");
    gtk_table_attach_defaults(GTK_TABLE (table), create_account_button, 1, 2, 7, 8);
    gtk_signal_connect(GTK_OBJECT (create_account_button), "clicked", GTK_SIGNAL_FUNC (createAccount),(gpointer) "please work!!!");
    gtk_widget_show (create_account_button);

    //Add Login Button
    GtkWidget *login_button = gtk_button_new_with_label ("Login");
    gtk_table_attach_defaults(GTK_TABLE (table), login_button, 1, 2, 8, 9);
    gtk_signal_connect(GTK_OBJECT (login_button), "clicked", GTK_SIGNAL_FUNC (loginAccount),(gpointer) "");
    gtk_widget_show (login_button);

    //Add Create Room Button
    GtkWidget *create_room_button = gtk_button_new_with_label ("Create Room");
    gtk_table_attach_defaults(GTK_TABLE (table), create_room_button, 2, 3, 7, 8);
    gtk_signal_connect(GTK_OBJECT (create_room_button), "clicked", GTK_SIGNAL_FUNC (createRoom),(gpointer)"");
    gtk_widget_show (create_room_button);

    //Add Enter Room Button
    GtkWidget *enter_room_button = gtk_button_new_with_label ("Enter Room");
    gtk_table_attach_defaults(GTK_TABLE (table), enter_room_button, 3, 4, 7, 8);
    gtk_signal_connect(GTK_OBJECT (enter_room_button), "clicked", GTK_SIGNAL_FUNC (enterRoom),(gpointer)"");
    gtk_widget_show (enter_room_button);

    //Add Leave Room Button
    GtkWidget *leave_room_button = gtk_button_new_with_label ("Leave Room");
    gtk_table_attach_defaults(GTK_TABLE (table), leave_room_button, 3, 4, 8, 9);
    gtk_signal_connect(GTK_OBJECT (leave_room_button), "clicked", GTK_SIGNAL_FUNC (leaveRoom),(gpointer)"");
    gtk_widget_show (leave_room_button);

    gtk_widget_show (table);
    gtk_widget_show (window);

    g_timeout_add(5000, (GSourceFunc) time_handler, (gpointer) window);

    gtk_main ();

    return 0;
}
