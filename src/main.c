// main.c - Fashion World GUI (GTK3 with cart update/remove + images)
// Build example (in MSYS2 MinGW 64-bit shell):
//   gcc main.c -o fashion_world.exe `pkg-config --cflags --libs gtk+-3.0`

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // for timestamp

/******************** PRODUCT CATALOG ********************/
typedef struct {
    int id;
    char name[64];
    double price;
    char desc[128];
    char image_path[512];
} Product;

Product catalog[] = {
    {1, "NIKE Black Oversized T-Shirt", 1399.00, "Comfort cotton oversized tee",
    "C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\1.png"},  
    {2, "Women Jeans", 1299.00, "Blue denim jeans", "C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\2.png"},
    {3, "Sneakers", 2499.00, "Lightweight sneakers", "C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\3.png"},
    {4, "Valentino Heels", 1510.00, "Women Embellished Block Heel Sandals", "C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\4.png"},
    {5, "KIDS Hoodie", 499.00, "Unisex Black Fleece Regular Fit Sweatshirt With Hoodie For Winter Wear","C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\5.png"},
    {6, "Calvin Klein Wallet", 4139.00,"Men Leather Two Fold Wallet","C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\6.png"},
    {7, "Lino Perros Womens Bag", 1798.00, "Off-White Quilted Handheld Bag", "C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\7.png"},
    {8, "Apple Airpods", 11900.00, "2nd Gen Bluetooth Headset with Charging Case AirPods","C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\8.png"}, 
    {9, "Shining Diva Fashion Earrings", 499.00,"Set Of 11 Gold-Plated Contemporary Studs Earrings","C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\9.png"},
    {10, "LITTLE GINNIE Soft Toy", 597.00,"Kids Cotton Soft Toy Soft Toys and Dolls","C:\\Users\\Shreeya\\OneDrive\\Desktop\\c++\\OnlineShopping\\image_scr\\10.png"}
    //Each {....} is one object of type Product 
};
size_t catalog_count = sizeof(catalog) / sizeof(catalog[0]);

/******************** CART (DOUBLY LINKED LIST) ********************/
typedef struct CartNode {
    int product_id;
    int qty;
    struct CartNode *prev;
    struct CartNode *next;
} CartNode;

CartNode *cart_head = NULL;

/********************Queue FUNCTIONS**********************/
typedef struct Order {
    int order_id;
    int item_count;
    double total;
    time_t timestamp;
    struct Order *next;
} Order;

static Order *order_front = NULL;
static Order *order_rear  = NULL;
static int next_order_id  = 1;

/* Enqueue a new order snapshot */
static Order* enqueue_order(int item_count, double total) {
    Order *o = (Order*)malloc(sizeof(Order));
    if (!o) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    o->order_id   = next_order_id++;
    o->item_count = item_count;
    o->total      = total;
    o->timestamp  = time(NULL);
    o->next       = NULL;

    if (order_rear) {
        order_rear->next = o;
    } else {
        order_front = o;
    }
    order_rear = o;
    return o;
}

/* Helper: build a printable string for the whole order queue */
static char* build_order_queue_string(void) {
    GString *msg = g_string_new("--- ORDER QUEUE ---\n");
    Order *cur = order_front;

    if (!cur) {
        g_string_append(msg, "(no orders in queue)\n");
        return g_string_free(msg, FALSE); // return heap string
    }

    while (cur) {
        char time_buf[64];
        struct tm *tm_info = localtime(&cur->timestamp);
        strftime(time_buf, sizeof(time_buf), "%a %b %d %H:%M:%S %Y", tm_info);

        g_string_append_printf(
            msg,
            "Order #%d | Items: %d | Total: %.2f | Time: %s\n",
            cur->order_id,
            cur->item_count,
            cur->total,
            time_buf
        );

        cur = cur->next;
    }
    return g_string_free(msg, FALSE);  // caller must free()
}


/******************** CART FUNCTIONS ********************/

static Product *find_product(int pid) {
    for (size_t i = 0; i < catalog_count; i++) {
        if (catalog[i].id == pid)
            return &catalog[i];
    }
    return NULL;
}

static CartNode* cart_find(int pid) {
    CartNode *c = cart_head;
    while (c) {
        if (c->product_id == pid) return c;
        c = c->next;
    }
    return NULL;
}

static void add_to_cart(int pid, int qty) {
    if (qty <= 0) {
        return;
    }

    Product *p = find_product(pid);
    if (!p) {
        return;
    }
    CartNode *n = cart_find(pid);
    if (n) {
        n->qty += qty;
        return;
    }

    n = (CartNode*)malloc(sizeof(CartNode));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    n->product_id = pid;
    n->qty = qty;
    n->prev = NULL;
    n->next = cart_head;
    if (cart_head) cart_head->prev = n;
    cart_head = n;

}

/* Set quantity for an item in cart; if new_qty <= 0, remove node */
static void cart_set_quantity(int pid, int new_qty) {
    CartNode *c = cart_find(pid);
    if (!c) return;

    if (new_qty <= 0) {
        // unlink from DLL
        if (c->prev) c->prev->next = c->next;
        else cart_head = c->next;

        if (c->next) c->next->prev = c->prev;

        free(c);
    } else {
        c->qty = new_qty;
    }
}

static double cart_total(void) {
    double total = 0.0;
    CartNode *c = cart_head;
    while (c) {
        Product *p = find_product(c->product_id);
        if (p) {
            total += p->price * c->qty;
        }
        c = c->next;
    }
    return total;
}

static gboolean cart_is_empty(void) {
    return cart_head == NULL;
}

static void cart_clear(void) {
    CartNode *c = cart_head;
    while (c) {
        CartNode *tmp = c;
        c = c->next;
        free(tmp);
    }
    cart_head = NULL;
}

/******************** GTK HELPERS ********************/

static void show_info_dialog(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        msg
    );
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Load scaled image (for thumbnails) */
static GtkWidget* create_scaled_image(const char *path, int width, int height) {
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(
        path,
        width,
        height,
        TRUE,
        &error
    );
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return gtk_image_new(); // empty placeholder
    }

    GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    return image;
}

/******************** VIEW CART (WITH IMAGES + UPDATE/REMOVE) ********************/

typedef struct {
    int product_id;
    GtkWidget *spin;
} CartItemControl;

static void on_view_cart_clicked(GtkButton *button, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Cart",
        parent,
        GTK_DIALOG_MODAL,
        "_Update Cart",
        GTK_RESPONSE_ACCEPT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL
    );

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, 500, 300);
    gtk_container_add(GTK_CONTAINER(content), scrolled);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(scrolled), box);

    CartItemControl *controls = NULL;
    int item_count = 0;

    if (cart_is_empty()) {
        GtkWidget *label = gtk_label_new("Cart is empty.");
        gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    } else {
        /* First pass: count items */
        CartNode *c = cart_head;
        while (c) {
            item_count++;
            c = c->next;
        }

        controls = (CartItemControl*)malloc(sizeof(CartItemControl) * item_count);
        if (!controls) {
            perror("malloc");
            gtk_widget_destroy(dialog);
            return;
        }

        /* Second pass: create rows with images + labels + spinbuttons */
        int idx = 0;
        c = cart_head;
        while (c) {
            Product *p = find_product(c->product_id);
            if (p) {
                GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

                // Thumbnail image
                GtkWidget *image = create_scaled_image(p->image_path, 80, 80);
                gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 4);

                // Text info + quantity spin
                GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
                gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

                char info[256];
                double sub = p->price * c->qty;
                snprintf(info, sizeof(info),
                         "%s\nPrice: %.2f  Subtotal: %.2f",
                         p->name, p->price, sub);
                GtkWidget *label = gtk_label_new(info);
                gtk_label_set_xalign(GTK_LABEL(label), 0.0);
                gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

                GtkWidget *qty_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
                gtk_box_pack_start(GTK_BOX(vbox), qty_box, FALSE, FALSE, 0);

                GtkWidget *qty_label = gtk_label_new(NULL);
                gtk_label_set_text(GTK_LABEL(qty_label), "Quantity:");
                gtk_box_pack_start(GTK_BOX(qty_box), qty_label, FALSE, FALSE, 0);

                GtkAdjustment *adj = gtk_adjustment_new(c->qty, 0, 100, 1, 5, 0);
                GtkWidget *spin = gtk_spin_button_new(adj, 1, 0);
                gtk_box_pack_start(GTK_BOX(qty_box), spin, FALSE, FALSE, 0);

                gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 4);

                controls[idx].product_id = p->id;
                controls[idx].spin = spin;
                idx++;
            }
            c = c->next;
        }

        // Total at the bottom (current before updates)
        double total = cart_total();
        char total_buf[128];
        snprintf(total_buf, sizeof(total_buf), "\nCurrent Total: %.2f", total);
        GtkWidget *total_label = gtk_label_new(total_buf);
        gtk_box_pack_start(GTK_BOX(box), total_label, FALSE, FALSE, 4);
    }

    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));

    if (resp == GTK_RESPONSE_ACCEPT && !cart_is_empty() && controls && item_count > 0) {
    for (int i = 0; i < item_count; i++) {
        int new_qty = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(controls[i].spin));
        cart_set_quantity(controls[i].product_id, new_qty);
    }
    show_info_dialog(parent, "Cart Updated",
                     "Cart quantities have been updated.");
}


    if (controls) free(controls);
    gtk_widget_destroy(dialog);
}

/******************** CHECKOUT ********************/

/******************** CHECKOUT WITH ORDER QUEUE ********************/

static void on_checkout_clicked(GtkButton *button, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);

    if (cart_is_empty()) {
        show_info_dialog(parent, "Checkout",
                         "Cart is empty. Add items before checkout.");
        return;
    }

    /* Count total items and total amount from cart */
    int item_count = 0;
    CartNode *c = cart_head;
    while (c) {
        item_count += c->qty;
        c = c->next;
    }

    double total = cart_total();

    /* Take snapshot and enqueue into order queue */
    Order *order = enqueue_order(item_count, total);

    /* Build the message text */

    // Part 1: order status lines
    GString *msg = g_string_new(NULL);
    g_string_append_printf(msg,
        "Order #%d is ready to process....\n"
        "Order #%d placed successfully! Total: %.2f\n\n",
        order->order_id, order->order_id, order->total);

    // Part 2: queue listing
    char *queue_str = build_order_queue_string();
    g_string_append(msg, queue_str);
    g_free(queue_str);

    // Part 3: processing messages
    g_string_append(msg,
        "\nSystem processing your order...\n"
        "It may take a few minutes to process your Order..\n"
        "Thank You for Shopping with us!");

    /* Show all of this in one dialog */
    show_info_dialog(parent, "Checkout", msg->str);

    g_string_free(msg, TRUE);

    /* Clear cart after checkout */
    cart_clear();
}


/******************** PRODUCT DETAILS DIALOG ********************/


static void on_product_row_activated(GtkWidget *widget,
                                     gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    Product *p = g_object_get_data(G_OBJECT(widget), "product");

    if (!p) return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        p->name,
        parent,
        GTK_DIALOG_MODAL,
        "_Add to Cart",
        GTK_RESPONSE_ACCEPT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL
    );

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    // Name + price
    char header[256];
    snprintf(header, sizeof(header), "%s\nPrice: %.2f", p->name, p->price);
    GtkWidget *title = gtk_label_new(header);
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    // Description
    GtkWidget *desc = gtk_label_new(p->desc);
    gtk_label_set_xalign(GTK_LABEL(desc), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), desc, FALSE, FALSE, 0);

    // Large image
    GtkWidget *image = create_scaled_image(p->image_path, 250, 250);
    gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 4);

    // Quantity selector
    GtkWidget *qty_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start(GTK_BOX(vbox), qty_box, FALSE, FALSE, 0);

    GtkWidget *qty_label = gtk_label_new("Quantity:");
    gtk_box_pack_start(GTK_BOX(qty_box), qty_label, FALSE, FALSE, 0);

    GtkAdjustment *adj = gtk_adjustment_new(1, 1, 100, 1, 5, 0);
    GtkWidget *spin = gtk_spin_button_new(adj, 1, 0);
    gtk_box_pack_start(GTK_BOX(qty_box), spin, FALSE, FALSE, 0);

    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));

    if (resp == GTK_RESPONSE_ACCEPT) {
        int qty = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        add_to_cart(p->id, qty);
        show_info_dialog(parent, "Cart", "Item added to cart.");
    }
    gtk_widget_destroy(dialog);
}
/********************* Exit Dialog *************************/
static void on_exit_clicked(GtkButton *button, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    // Build message for all pending orders
    GString *msg = g_string_new(NULL);

    if (!order_front) {
        g_string_append(msg, "No pending orders.\nExiting application...");
    } else {
        g_string_append(msg, "Processing all pending orders...\n\n");
        while (order_front) {
            Order *o = order_front;

            g_string_append_printf(msg, "Processing Order #%d...\n", o->order_id);

            order_front = order_front->next;
            if (!order_front) order_rear = NULL;

            free(o);
        }
        g_string_append(msg, "\nAll orders processed successfully.\nExiting application...");
    }
    // --- IMPORTANT: Use a modal dialog that BLOCKS until user presses OK ---
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        msg->str
    );

    gtk_window_set_title(GTK_WINDOW(dialog), "Exit");
    // Wait so message is shown
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_string_free(msg, TRUE);
    // Only close AFTER dialog is dismissed
    gtk_window_close(parent);
}

/******************** APPLICATION SETUP ********************/

static void activate(GtkApplication *app, gpointer user_data) {

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Fashion World");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);

    // --- window background: warm beige ---
    GdkRGBA beige;
    gdk_rgba_parse(&beige, "#F5EEDC");
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &beige);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header),
                        "<span font_desc='20' weight='bold' foreground='#333333'>Welcome to Fashion World</span>");
    gtk_label_set_xalign(GTK_LABEL(header), 0.5);
    gtk_widget_set_halign(header, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 15);

    // Product Grid
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), grid, TRUE, TRUE, 10);

    // color for product card background (white)
    GdkRGBA white_bg;
    gdk_rgba_parse(&white_bg, "#FFFFFF");

    int row = 0, col = 0;
    for (size_t i = 0; i < catalog_count; i++) {

        Product *p = &catalog[i];

        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

        GtkWidget *img = create_scaled_image(p->image_path, 180, 180);
        gtk_box_pack_start(GTK_BOX(vbox), img, FALSE, FALSE, 0);

        // product title: bold + dark charcoal
        char title_buf[128];
        snprintf(title_buf, sizeof(title_buf),
                 "<span foreground=\"#333333\" weight=\"bold\">%s</span>", p->name);
        GtkWidget *label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label), title_buf);
        gtk_label_set_xalign(GTK_LABEL(label), 0.5);
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

        GtkWidget *btn = gtk_button_new();
        gtk_container_add(GTK_CONTAINER(btn), vbox);
        g_object_set_data(G_OBJECT(btn), "product", p);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_product_row_activated), window);

        // make button look like a white card
        gtk_widget_override_background_color(btn, GTK_STATE_FLAG_NORMAL, &white_bg);
        gtk_button_set_relief(GTK_BUTTON(btn), GTK_RELIEF_NONE);  // remove 3D border
        gtk_widget_set_hexpand(btn, TRUE);

        gtk_grid_attach(GTK_GRID(grid), btn, col, row, 1, 1);

        col++;
        if (col == 3) { col = 0; row++; }
    }

    // Bottom button area
    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(bottom_box, GTK_ALIGN_CENTER);
    gtk_box_pack_end(GTK_BOX(main_box), bottom_box, FALSE, FALSE, 10);

    GtkWidget *cart_button = gtk_button_new_with_label("View Cart");
    GtkWidget *checkout_button = gtk_button_new_with_label("Checkout");
    GtkWidget *exit_button = gtk_button_new_with_label("Exit");

    gtk_box_pack_start(GTK_BOX(bottom_box), cart_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bottom_box), checkout_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bottom_box), exit_button, FALSE, FALSE, 0);

    g_signal_connect(cart_button, "clicked", G_CALLBACK(on_view_cart_clicked), window);
    g_signal_connect(checkout_button, "clicked", G_CALLBACK(on_checkout_clicked), window);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(on_exit_clicked), window);

    // --- caramel brown buttons with white text ---
    // --- caramel brown buttons with dark text (always visible) ---
    GdkRGBA caramel;
    gdk_rgba_parse(&caramel, "#C39A6B");

    GdkRGBA dark_text;
    gdk_rgba_parse(&dark_text, "#333333");   // dark charcoal text

    // try to set background; if theme ignores it, at least text is visible
    gtk_widget_override_background_color(cart_button, GTK_STATE_FLAG_NORMAL, &caramel);
    gtk_widget_override_color(cart_button, GTK_STATE_FLAG_NORMAL, &dark_text);

    gtk_widget_override_background_color(checkout_button, GTK_STATE_FLAG_NORMAL, &caramel);
    gtk_widget_override_color(checkout_button, GTK_STATE_FLAG_NORMAL, &dark_text);

    gtk_widget_override_background_color(exit_button, GTK_STATE_FLAG_NORMAL, &caramel);
    gtk_widget_override_color(exit_button, GTK_STATE_FLAG_NORMAL, &dark_text);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    app = gtk_application_new("com.example.fashionworld",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    cart_clear(); // free cart memory
    return status;
}
