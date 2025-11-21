char addacc;
GtkWidget *addacc_window;
GtkWidget *addacc_label, *addacc_textb;
GtkWidget *addacc_addb;
char addacc_user[8];
int addacc_id;
char addacc_readen;
char addacc_online;

void addacc_add(
	GtkWidget *widget,
	gpointer data
)
{
	char fn[24];
	memset(fn, 0, sizeof(fn));
	sprintf(fn, "%s/users.txt", userns);
	FILE *fp = fopen(fn, "r");
	char str[8], c=fgetc(fp), i;
	for (;;) {
		for (i = 0; c != '\n'; str[i] = c, c=fgetc(fp), i++);
		str[i] = 0;
		if (!strcmp(str, addacc_user)) {
			puts("error: Account is already added");
			return;
		}
		if ((c=fgetc(fp)) == EOF) break;
	}
	fclose(fp);
	fp = fopen(fn, "a");
	fprintf(fp, "%s\n", addacc_user);
	fclose(fp);
	contact[cn].id = addacc_id;
	contact[cn].online = addacc_online;
	contact[cn].readen = addacc_readen;
	cn++;
	GtkTreeModel *model = fill_view();
	gtk_tree_view_set_model(
		GTK_TREE_VIEW(view),
		model
	);
}

void addacc_check(
	GtkWidget *widget,
	gpointer *data
)
{
	char r = info_user((char*)gtk_entry_get_text(GTK_ENTRY(addacc_textb)));
	if (!r) {
		addacc_id = bufi[3]
			| bufi[4] << 8
			| bufi[5] << 16
			| bufi[6] << 24;
		addacc_online = bufi[1];
		addacc_readen = bufi[2];
		memset(addacc_user, 0, sizeof(addacc_user));
		strcpy(addacc_user, gtk_entry_get_text(
				GTK_ENTRY(addacc_textb)
			)
		);
		gtk_label_set_text(
			GTK_LABEL(addacc_label),
			"Found the entered account."
		);
		gtk_widget_set_sensitive(addacc_addb, 1);
		gtk_widget_show(addacc_label);
		gtk_widget_show(addacc_addb);
	} else if (r == 3) {
		gtk_label_set_text(
			GTK_LABEL(addacc_label),
			"This account does not exist."
		);
		gtk_widget_set_sensitive(addacc_addb, 0);
		gtk_widget_show(addacc_label);
		gtk_widget_show(addacc_addb);
	}
}

void addacc_destroy(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer *data
)
{
	gtk_widget_destroy(addacc_window);
	addacc = 0;
}

void addacc_show()
{
	addacc = 1;
	addacc_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(addacc_window, 280, 100);
	gtk_window_set_resizable(GTK_WINDOW(addacc_window), 0);
	g_signal_connect(
		G_OBJECT(addacc_window),
		"destroy",
		G_CALLBACK(addacc_destroy),
		0
	);
	GtkWidget *addacc_fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(addacc_window), addacc_fixed);
	addacc_label = gtk_label_new("Please enter the account name to add.");
	gtk_fixed_put(GTK_FIXED(addacc_fixed), addacc_label, 10, 10);
	addacc_textb = gtk_entry_new();
	gtk_widget_set_size_request(addacc_textb, 80, 25);
	gtk_fixed_put(GTK_FIXED(addacc_fixed), addacc_textb, 10, 30);
	GtkWidget *checkb = gtk_button_new_with_label("Check");
	g_signal_connect(
		G_OBJECT(checkb),
		"clicked",
		G_CALLBACK(addacc_check),
		0
	);
	gtk_fixed_put(GTK_FIXED(addacc_fixed), checkb, 100, 30);
	addacc_addb = gtk_button_new_with_label("Add");
	g_signal_connect(
		G_OBJECT(addacc_addb),
		"clicked",
		G_CALLBACK(addacc_add),
		0
	);
	gtk_widget_set_sensitive(addacc_addb, 0);
	gtk_fixed_put(GTK_FIXED(addacc_fixed), addacc_addb, 10, 65);
	GtkWidget *cancelb = gtk_button_new_with_label("Cancel");
	g_signal_connect(
		G_OBJECT(cancelb),
		"clicked",
		G_CALLBACK(addacc_destroy),
		0
	);
	gtk_fixed_put(GTK_FIXED(addacc_fixed), cancelb, 60, 65);
	gtk_widget_show_all(addacc_window);
}
