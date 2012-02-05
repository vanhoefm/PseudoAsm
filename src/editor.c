#include <assert.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

// Creates all widgets
void openEditWindow(void);
static GtkWidget * createToolbarBox(void);
static GtkWidget *button_with_image_and_label(char *stock, char *label);

// Callbacks
static void eventOpen(GtkWidget *widget, gpointer data);
static void eventSave(GtkWidget *widget, gpointer data);
static void eventSaveAs(GtkWidget *widget, gpointer data);
static void eventExit(GtkWidget *widget, gpointer data);

// Helper functions
static void saveContents(char *filename);
static void messageBox(char *message, GtkMessageType type, GtkButtonsType buttons);
static void setNewFile(char *filename);

// Global variables of this module
static GtkWidget *text;			// Text widget
static GtkWidget *currFileLabel;	// Label that displayes the current file
static char currfile[128];		// Current file being edited
static int isfileopen = 0;		// Is a file open?

static gint destroy(GtkWidget *widget, gpointer data)
{
	isfileopen = 0;
	gtk_main_quit();
	return FALSE;
}

/** Creates an edit window. Function returns when edit window is closed. */
void openEditWindow(void)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *toolbar;
	GtkWidget *textWindow;

	/* Create main window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
	gtk_window_set_title(GTK_WINDOW(window), "PseudoAsm: Edit File");
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(destroy), NULL);

	// Create the text box
	text = gtk_text_view_new();
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(text), GTK_WRAP_NONE);

	textWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
		GTK_SCROLLED_WINDOW(textWindow),
		GTK_POLICY_AUTOMATIC,
		GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(textWindow), text);
	gtk_widget_show(text);

	// Create the "toolbar"
	toolbar = createToolbarBox();

	// Create label of current file
	currFileLabel = gtk_label_new("No file open");

	// Add everything a the vbox
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),toolbar, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), currFileLabel, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), textWindow, TRUE, TRUE, 5);

	// Add vbox to window
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);

	// When gtk is closed, the window should be destroyed
	gtk_quit_add_destroy(1, GTK_OBJECT(window));

	// Show and start it all
	gtk_main();
}

/** Create the "toolbar" */
static GtkWidget * createToolbarBox(void)
{
	GtkWidget *hbox;
	GtkWidget *open;
	GtkWidget *save;
	GtkWidget *saveAs;
	GtkWidget *exit;

	hbox = gtk_hbox_new(FALSE, 0);

	// Create all the buttons
	open = button_with_image_and_label(GTK_STOCK_OPEN, "Open");
	save = button_with_image_and_label(GTK_STOCK_SAVE, "Save");
	saveAs = button_with_image_and_label(GTK_STOCK_SAVE_AS, "Save As");
	exit = button_with_image_and_label(GTK_STOCK_CLOSE, "Close");
	g_signal_connect(G_OBJECT(open), "clicked", G_CALLBACK(eventOpen), NULL);
	g_signal_connect(G_OBJECT(save), "clicked", G_CALLBACK(eventSave), NULL);
	g_signal_connect(G_OBJECT(saveAs), "clicked", G_CALLBACK(eventSaveAs), NULL);
	g_signal_connect(G_OBJECT(exit), "clicked", G_CALLBACK(eventExit), NULL);
	gtk_widget_show(open);
	gtk_widget_show(save);
	gtk_widget_show(saveAs);
	gtk_widget_show(exit);

	// Put the buttons in a hbox
	gtk_box_pack_start(GTK_BOX(hbox), open, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), save, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), saveAs, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), exit, TRUE, FALSE, 0);
	gtk_widget_show(hbox);

	return hbox;
}

/** Create a button with a label and an image */
static GtkWidget *button_with_image_and_label(char *stock, char *label)
{
	GtkWidget *button;
	GtkWidget *image;
	GtkWidget *gtklabel;
	GtkWidget *hbox;

	assert(stock != NULL);
	assert(label != NULL);

	button = gtk_button_new();
	image = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_BUTTON);
	gtklabel = gtk_label_new(label);
	
	hbox = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), image, TRUE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), gtklabel, TRUE, FALSE, 0);

	gtk_widget_show(gtklabel);
	gtk_widget_show(image);
	
	gtk_container_add(GTK_CONTAINER(button), hbox);
	gtk_widget_show(hbox);
	
	return button;
}

static void eventExit(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static void eventOpen(GtkWidget *widget, gpointer data)
{
	GtkWidget	*dialog;
	gchar 		*filename;
	gint 		response;
	
	dialog = gtk_file_chooser_dialog_new (
		"Open een bestand",
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN,
		GTK_RESPONSE_ACCEPT,
		NULL);

	response = gtk_dialog_run(GTK_DIALOG (dialog));

	if(response == GTK_RESPONSE_ACCEPT)
	{
		FILE *fp = NULL;	

		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		fp = fopen(filename, "r");
		if(!fp)
		{
			messageBox("Error opening file", GTK_MESSAGE_ERROR, GTK_BUTTONS_OK);
		}
		else
		{
			char buff[1024];
			GtkTextBuffer	*buffer;
			GtkTextIter	iter;
			GtkTextIter	enditer;

			// Clear the text box
			buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));

			gtk_text_buffer_get_start_iter(buffer, &iter);
			gtk_text_buffer_get_end_iter(buffer, &enditer);
			gtk_text_buffer_delete(buffer, &iter, &enditer);

			// Copy the file to the text box
			while(fgets(buff, 1024, fp))
			{
				gtk_text_buffer_get_end_iter(buffer, &iter);
				gtk_text_buffer_insert(buffer, &iter, buff, -1);
			}

			fclose(fp);

			// Update label and global vars
			setNewFile(filename);
			isfileopen = 1;
		}
	}

	gtk_widget_destroy (dialog);	
}

static void eventSave(GtkWidget *widget, gpointer data)
{
	if(isfileopen)
	{
		saveContents(currfile);
	}
	else
	{
		messageBox("No file is open: don't know where to save.", GTK_MESSAGE_ERROR, GTK_BUTTONS_OK);
	}
}

static void eventSaveAs(GtkWidget *widget, gpointer data)
{
	GtkWidget	*dialog;
	gchar 		*filename;
	gint 		response;
	
	dialog = gtk_file_chooser_dialog_new (
		"Save file",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE,
		GTK_RESPONSE_ACCEPT,
		NULL);

	response = gtk_dialog_run(GTK_DIALOG (dialog));

	if(response == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		// Save the file
		saveContents(filename);

		// Update label and global vars
		setNewFile(filename);
		isfileopen = 1;
	}

	gtk_widget_destroy(dialog);
}

static void saveContents(char *filename)
{
	FILE *fp = NULL;	

	fp = fopen(filename, "w");
	if(!fp)
	{
		messageBox("Error saving file", GTK_MESSAGE_ERROR, GTK_BUTTONS_OK);
	}
	else
	{
		char		*line = NULL;
		GtkTextBuffer	*buffer;
		GtkTextIter	startiter, enditer;
		int		i, lines = 0;

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
		lines = gtk_text_buffer_get_line_count(buffer);

		// Get all the lines, except the last one
		for(i = 0; i < lines - 1; i++)
		{
			gtk_text_buffer_get_iter_at_line(buffer, &startiter, i);
			gtk_text_buffer_get_iter_at_line(buffer, &enditer, i + 1);
			line = gtk_text_buffer_get_text(buffer, &startiter, &enditer, FALSE);
			fprintf(fp, "%s", line);
		}

		// Get the last line
		gtk_text_buffer_get_iter_at_line(buffer, &startiter, lines - 1);
		gtk_text_buffer_get_end_iter(buffer, &enditer);
		line = gtk_text_buffer_get_text(buffer, &startiter, &enditer, FALSE);
		fprintf(fp, "%s", line);

		messageBox("File has been saved!", GTK_MESSAGE_OTHER, GTK_BUTTONS_OK);

		fclose(fp);
	}	
}

/** Create a simple message box */
static void messageBox(char *message, GtkMessageType type, GtkButtonsType buttons)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (NULL,
				   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				   type,
				   buttons,
				   message);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static void setNewFile(char *filename)
{
	strcpy(currfile, filename);
	gtk_label_set_text(GTK_LABEL(currFileLabel), filename);
}
