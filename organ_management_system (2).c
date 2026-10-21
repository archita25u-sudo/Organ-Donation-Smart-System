/*
 * ORGAN DONATION MANAGEMENT SYSTEM
 *
 * Data structure used for each job:
 *   Hash Table + Linked List -> Donors (ID decides the bucket, bucket = linked list)
 *   Queue       -> Normal patients (first come, first served)
 *   Deque       -> Emergency patients (critical patient goes to the front)
 *   Stack       -> History (latest action on top)
 *   Binary Tree -> Patient records (sorted by ID)
 *   Graph       -> Hospital roads (shortest route)
 *
 * Every organ has its own queue and deque.
 * When an organ is available: emergency deque first, then normal queue.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 13
#define ORGANS 5
#define HOSPITALS 6
#define INF 9999


/* ---------------- Fixed data ---------------- */

const char *organ_names[ORGANS] = {"Kidney", "Liver", "Heart", "Lung", "Pancreas"};

const char *type_names[4] = {"", "Standard", "Emergency", "Critical"};

const char *hospital_names[HOSPITALS] = {
    "Andheri", "Bandra", "Dadar", "Parel", "Colaba", "Mulund"
};

/* Distance in km between hospitals (0 = no direct road) */
int roads[HOSPITALS][HOSPITALS] = {
    /*        And  Ban  Dad  Par  Col  Mul */
    /* And */ { 0,   9,  16,   0,   0,   0 },
    /* Ban */ { 9,   0,   6,   0,   0,   0 },
    /* Dad */ { 16,  6,   0,   3,   0,  14 },
    /* Par */ { 0,   0,   3,   0,  12,   0 },
    /* Col */ { 0,   0,   0,  12,   0,   0 },
    /* Mul */ { 0,   0,  14,   0,   0,   0 }
};


/* ---------------- Structures ---------------- */

/* Details of one person (donor or patient) */
typedef struct
{
    int id;
    char name[50];
    int organ;        /* 0 to 4, position in organ_names */
    int hospital;     /* 0 to 5, position in hospital_names */
    int type;         /* patients only: 1 Standard, 2 Emergency, 3 Critical */
} Person;

/* One donor in a linked list */
typedef struct Donor
{
    Person person;
    struct Donor *next;
} Donor;

/* One node of the queue */
typedef struct QNode
{
    Person person;
    struct QNode *next;
} QNode;

typedef struct
{
    QNode *front;
    QNode *rear;
    int count;
} Queue;

/* One node of the deque (has prev and next) */
typedef struct DNode
{
    Person person;
    struct DNode *prev;
    struct DNode *next;
} DNode;

typedef struct
{
    DNode *front;
    DNode *rear;
    int count;
} Deque;

/* One node of the tree */
typedef struct TreeNode
{
    Person person;
    int waiting;      /* 1 = waiting, 0 = received organ */
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/* One node of the stack */
typedef struct LogNode
{
    char text[200];
    struct LogNode *next;
} LogNode;


/* ---------------- Global variables ---------------- */

Donor *table[TABLE_SIZE];         /* hash table */
Queue standard_queue[ORGANS];     /* one queue for each organ */
Deque emergency_deque[ORGANS];    /* one deque for each organ */
TreeNode *root = NULL;            /* top of the tree */
LogNode *log_top = NULL;          /* top of the stack */


/* ======================= STACK (history) ======================= */

/* Put a message on top of the stack */
void push_history(const char *message)
{
    LogNode *node = malloc(sizeof(LogNode));

    strcpy(node->text, message);
    node->next = log_top;
    log_top = node;
}

/* Remove the message on top */
void pop_history(void)
{
    LogNode *node = log_top;

    if (node == NULL)
    {
        printf("History is empty.\n");
        return;
    }

    printf("Removed: %s\n", node->text);
    log_top = node->next;
    free(node);
}

/* Print all messages, latest first */
void show_history(void)
{
    LogNode *current = log_top;

    printf("\n--- History (latest first) ---\n");

    if (current == NULL)
    {
        printf("History is empty.\n");
    }

    while (current != NULL)
    {
        printf("%s\n", current->text);
        current = current->next;
    }
}


/* ======================= QUEUE ======================= */

/* Add a person at the rear */
void enqueue(Queue *q, Person p)
{
    QNode *node = malloc(sizeof(QNode));

    node->person = p;
    node->next = NULL;

    if (q->rear == NULL)
    {
        q->front = node;
        q->rear = node;
    }
    else
    {
        q->rear->next = node;
        q->rear = node;
    }

    q->count++;
}

/* Remove the person at the front. Returns 0 if the queue is empty. */
int dequeue(Queue *q, Person *p)
{
    QNode *node = q->front;

    if (node == NULL)
    {
        return 0;
    }

    *p = node->person;
    q->front = node->next;

    if (q->front == NULL)
    {
        q->rear = NULL;
    }

    q->count--;
    free(node);
    return 1;
}

/* Look at the front person without removing. Returns 0 if empty. */
int queue_peek(Queue *q, Person *p)
{
    if (q->front == NULL)
    {
        return 0;
    }

    *p = q->front->person;
    return 1;
}


/* ======================= DEQUE ======================= */

/* Add a person at the front */
void insert_front(Deque *d, Person p)
{
    DNode *node = malloc(sizeof(DNode));

    node->person = p;
    node->prev = NULL;
    node->next = d->front;

    if (d->front == NULL)
    {
        d->rear = node;
    }
    else
    {
        d->front->prev = node;
    }

    d->front = node;
    d->count++;
}

/* Add a person at the rear */
void insert_rear(Deque *d, Person p)
{
    DNode *node = malloc(sizeof(DNode));

    node->person = p;
    node->prev = d->rear;
    node->next = NULL;

    if (d->rear == NULL)
    {
        d->front = node;
    }
    else
    {
        d->rear->next = node;
    }

    d->rear = node;
    d->count++;
}

/* Remove the person at the front. Returns 0 if the deque is empty. */
int remove_front(Deque *d, Person *p)
{
    DNode *node = d->front;

    if (node == NULL)
    {
        return 0;
    }

    *p = node->person;
    d->front = node->next;

    if (d->front == NULL)
    {
        d->rear = NULL;
    }
    else
    {
        d->front->prev = NULL;
    }

    d->count--;
    free(node);
    return 1;
}

/* Look at the front person without removing. Returns 0 if empty. */
int deque_peek_front(Deque *d, Person *p)
{
    if (d->front == NULL)
    {
        return 0;
    }

    *p = d->front->person;
    return 1;
}


/* ================= HASH TABLE + LINKED LIST (donors) ================= */

/* The donor ID decides the bucket number */
int hash(int id)
{
    return id % TABLE_SIZE;
}

/* Find a donor by ID. Only one bucket is checked. */
Donor *find_donor(int id)
{
    Donor *current = table[hash(id)];

    while (current != NULL)
    {
        if (current->person.id == id)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void add_donor(Person p)
{
    Donor *node;
    int index = hash(p.id);
    char message[200];

    if (find_donor(p.id) != NULL)
    {
        printf("This donor ID already exists.\n");
        return;
    }

    node = malloc(sizeof(Donor));
    node->person = p;

    /* add at the start of the bucket's linked list */
    node->next = table[index];
    table[index] = node;

    printf("Donor %s registered (bucket %d).\n", p.name, index);

    sprintf(message, "Donor %d (%s) registered", p.id, organ_names[p.organ]);
    push_history(message);
}

/* Delete a donor from the bucket's linked list */
void remove_donor(int id)
{
    int index = hash(id);
    Donor *current = table[index];
    Donor *previous = NULL;

    while (current != NULL && current->person.id != id)
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
    {
        return;
    }

    if (previous == NULL)
    {
        table[index] = current->next;
    }
    else
    {
        previous->next = current->next;
    }

    free(current);
}

void show_donors(void)
{
    int i;
    int total = 0;
    Donor *current;

    printf("\n--- Donors ---\n");

    for (i = 0; i < TABLE_SIZE; i++)
    {
        current = table[i];

        while (current != NULL)
        {
            printf("[Bucket %d] ID %d, %s, %s, %s\n",
                   i,
                   current->person.id,
                   current->person.name,
                   organ_names[current->person.organ],
                   hospital_names[current->person.hospital]);
            current = current->next;
            total++;
        }
    }

    if (total == 0)
    {
        printf("No donors yet.\n");
    }
}

/* Search by ID (hash search) */
void search_donor(int id)
{
    Donor *donor = find_donor(id);

    if (donor == NULL)
    {
        printf("Donor not found.\n");
        return;
    }

    printf("Found: ID %d, %s, %s, %s\n",
           donor->person.id,
           donor->person.name,
           organ_names[donor->person.organ],
           hospital_names[donor->person.hospital]);
}

/* Search by organ (linear search: checks every donor) */
void search_donors_by_organ(int organ)
{
    int i;
    int found = 0;
    Donor *current;

    for (i = 0; i < TABLE_SIZE; i++)
    {
        current = table[i];

        while (current != NULL)
        {
            if (current->person.organ == organ)
            {
                printf("ID %d, %s, %s\n",
                       current->person.id,
                       current->person.name,
                       hospital_names[current->person.hospital]);
                found++;
            }
            current = current->next;
        }
    }

    if (found == 0)
    {
        printf("No donors found for this organ.\n");
    }
}


/* ================= BINARY TREE (patient records) ================= */

/* Smaller ID goes left, bigger ID goes right */
TreeNode *tree_insert(TreeNode *node, Person p)
{
    if (node == NULL)
    {
        node = malloc(sizeof(TreeNode));
        node->person = p;
        node->waiting = 1;
        node->left = NULL;
        node->right = NULL;
        return node;
    }

    if (p.id < node->person.id)
    {
        node->left = tree_insert(node->left, p);
    }
    else
    {
        node->right = tree_insert(node->right, p);
    }

    return node;
}

/* Find a patient by ID */
TreeNode *tree_search(TreeNode *node, int id)
{
    if (node == NULL || node->person.id == id)
    {
        return node;
    }

    if (id < node->person.id)
    {
        return tree_search(node->left, id);
    }

    return tree_search(node->right, id);
}

/* Left, then node, then right: prints patients sorted by ID */
void tree_inorder(TreeNode *node)
{
    if (node == NULL)
    {
        return;
    }

    tree_inorder(node->left);

    printf("ID %d, %s, needs %s, %s, %s\n",
           node->person.id,
           node->person.name,
           organ_names[node->person.organ],
           type_names[node->person.type],
           node->waiting ? "waiting" : "received organ");

    tree_inorder(node->right);
}

void tree_free(TreeNode *node)
{
    if (node == NULL)
    {
        return;
    }

    tree_free(node->left);
    tree_free(node->right);
    free(node);
}

void search_recipient(int id)
{
    TreeNode *node = tree_search(root, id);

    if (node == NULL)
    {
        printf("Patient not found.\n");
        return;
    }

    printf("Found: ID %d, %s, needs %s, %s, %s\n",
           node->person.id,
           node->person.name,
           organ_names[node->person.organ],
           type_names[node->person.type],
           node->waiting ? "waiting" : "received organ");
}


/* ======================= PATIENT WAITLISTS ======================= */

void add_recipient(Person p)
{
    char message[200];

    if (tree_search(root, p.id) != NULL)
    {
        printf("This patient ID already exists.\n");
        return;
    }

    if (p.type == 1)
    {
        enqueue(&standard_queue[p.organ], p);        /* standard: queue rear */
    }
    else if (p.type == 2)
    {
        insert_rear(&emergency_deque[p.organ], p);   /* emergency: deque rear */
    }
    else
    {
        insert_front(&emergency_deque[p.organ], p);  /* critical: deque front */
    }

    root = tree_insert(root, p);

    printf("Patient %s added (%s).\n", p.name, type_names[p.type]);

    sprintf(message, "%s patient %d (%s) added", type_names[p.type], p.id, p.name);
    push_history(message);
}

void show_waitlists(void)
{
    int i;
    DNode *d;
    QNode *q;

    printf("\n--- Waitlists ---\n");

    for (i = 0; i < ORGANS; i++)
    {
        printf("%s:\n", organ_names[i]);

        printf("  Emergency Deque: ");
        for (d = emergency_deque[i].front; d != NULL; d = d->next)
        {
            printf("%s ", d->person.name);
        }
        printf("\n");

        printf("  Standard Queue: ");
        for (q = standard_queue[i].front; q != NULL; q = q->next)
        {
            printf("%s ", q->person.name);
        }
        printf("\n");
    }
}

/* Show who gets this organ next, without removing anyone */
void show_next_in_line(int organ)
{
    Person next;

    if (deque_peek_front(&emergency_deque[organ], &next) == 1)
    {
        printf("Next: %s (%s) from the Emergency Deque.\n",
               next.name, type_names[next.type]);
    }
    else if (queue_peek(&standard_queue[organ], &next) == 1)
    {
        printf("Next: %s from the Standard Queue.\n", next.name);
    }
    else
    {
        printf("Nobody is waiting for this organ.\n");
    }
}


/* ======================= GRAPH (shortest route) ======================= */

/* Finds the shortest distance from one hospital to every other hospital */
void dijkstra(int source, int dist[], int previous[])
{
    int visited[HOSPITALS];
    int i;
    int round;
    int u;
    int v;

    /* start: every hospital is "far away" and not visited */
    for (i = 0; i < HOSPITALS; i++)
    {
        dist[i] = INF;
        previous[i] = -1;
        visited[i] = 0;
    }

    dist[source] = 0;

    for (round = 0; round < HOSPITALS; round++)
    {
        /* step 1: pick the closest hospital that is not visited yet */
        u = -1;
        for (i = 0; i < HOSPITALS; i++)
        {
            if (visited[i] == 0 && (u == -1 || dist[i] < dist[u]))
            {
                u = i;
            }
        }

        if (dist[u] == INF)
        {
            break;
        }

        visited[u] = 1;

        /* step 2: check if going through u gives a shorter way to its neighbours */
        for (v = 0; v < HOSPITALS; v++)
        {
            if (roads[u][v] > 0 && dist[u] + roads[u][v] < dist[v])
            {
                dist[v] = dist[u] + roads[u][v];
                previous[v] = u;
            }
        }
    }
}

/* Print the route by going backwards using "previous" */
void print_path(int previous[], int target)
{
    if (previous[target] != -1)
    {
        print_path(previous, previous[target]);
        printf(" -> ");
    }

    printf("%s", hospital_names[target]);
}

void show_route(int from, int to)
{
    int dist[HOSPITALS];
    int previous[HOSPITALS];

    dijkstra(from, dist, previous);

    if (dist[to] == INF)
    {
        printf("No route found.\n");
        return;
    }

    printf("Shortest route (%d km): ", dist[to]);
    print_path(previous, to);
    printf("\n");
}


/* ======================= ORGAN ALLOCATION ======================= */

void allocate_organ(int id)
{
    Donor *donor = find_donor(id);
    Person giver;
    Person receiver;
    TreeNode *record;
    int found;
    char message[200];

    if (donor == NULL)
    {
        printf("Donor not found.\n");
        return;
    }

    giver = donor->person;

    /* first try the emergency deque */
    found = remove_front(&emergency_deque[giver.organ], &receiver);

    /* if it is empty, try the standard queue */
    if (found == 0)
    {
        found = dequeue(&standard_queue[giver.organ], &receiver);
    }

    if (found == 0)
    {
        printf("Nobody is waiting for %s yet.\n", organ_names[giver.organ]);
        return;
    }

    printf("Match: %s's %s goes to %s (%s).\n",
           giver.name,
           organ_names[giver.organ],
           receiver.name,
           hospital_names[receiver.hospital]);

    printf("Transport: ");
    show_route(giver.hospital, receiver.hospital);

    /* the patient has received an organ */
    record = tree_search(root, receiver.id);
    if (record != NULL)
    {
        record->waiting = 0;
    }

    /* the donor's organ is used, so remove the donor */
    remove_donor(id);

    sprintf(message, "Allocated %s from donor %d to %s",
            organ_names[giver.organ], id, receiver.name);
    push_history(message);
}


/* ======================= INPUT ======================= */

/* Shows the organ list and returns the chosen organ (0 to 4), or -1 if invalid */
int choose_organ(void)
{
    int i;
    int choice;

    for (i = 0; i < ORGANS; i++)
    {
        printf("  %d. %s\n", i + 1, organ_names[i]);
    }

    printf("Choose organ: ");
    scanf("%d", &choice);

    if (choice < 1 || choice > ORGANS)
    {
        printf("Invalid organ.\n");
        return -1;
    }

    return choice - 1;
}

/* Shows the hospital list and returns the chosen hospital (0 to 5), or -1 if invalid */
int choose_hospital(void)
{
    int i;
    int choice;

    for (i = 0; i < HOSPITALS; i++)
    {
        printf("  %d. %s\n", i + 1, hospital_names[i]);
    }

    printf("Choose hospital: ");
    scanf("%d", &choice);

    if (choice < 1 || choice > HOSPITALS)
    {
        printf("Invalid hospital.\n");
        return -1;
    }

    return choice - 1;
}

/* Reads the details of a donor or patient. Returns 0 if something is invalid. */
int get_person(Person *p)
{
    printf("Enter ID: ");
    scanf("%d", &p->id);

    printf("Enter name (one word): ");
    scanf("%49s", p->name);

    p->organ = choose_organ();
    if (p->organ == -1)
    {
        return 0;
    }

    p->hospital = choose_hospital();
    if (p->hospital == -1)
    {
        return 0;
    }

    p->type = 0;
    return 1;
}

/* Free all memory before the program ends */
void free_memory(void)
{
    int i;
    Person temp;

    for (i = 0; i < TABLE_SIZE; i++)
    {
        while (table[i] != NULL)
        {
            Donor *node = table[i];
            table[i] = node->next;
            free(node);
        }
    }

    for (i = 0; i < ORGANS; i++)
    {
        while (dequeue(&standard_queue[i], &temp) == 1)
        {
        }

        while (remove_front(&emergency_deque[i], &temp) == 1)
        {
        }
    }

    tree_free(root);

    while (log_top != NULL)
    {
        LogNode *node = log_top;
        log_top = node->next;
        free(node);
    }
}


/* ======================= MAIN ======================= */

int main(void)
{
    int choice;
    int type;
    int id;
    int organ;
    int from;
    int to;
    Person p;

    do
    {
        printf("\n===== ORGAN DONATION MANAGEMENT =====\n");
        printf("1. Register donor\n");
        printf("2. Add patient\n");
        printf("3. Search\n");
        printf("4. Allocate organ\n");
        printf("5. Shortest route between hospitals\n");
        printf("6. History\n");
        printf("7. Next in line for an organ\n");
        printf("8. Show all\n");
        printf("0. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            while (getchar() != '\n');   /* throw away wrong input like letters */
            choice = -1;
        }

        switch (choice)
        {
            case 1:
                if (get_person(&p) == 1)
                {
                    add_donor(p);
                }
                break;

            case 2:
                if (get_person(&p) == 1)
                {
                    printf("Type: 1. Standard  2. Emergency  3. Critical\n");
                    printf("Choose type: ");
                    scanf("%d", &type);

                    if (type < 1 || type > 3)
                    {
                        printf("Invalid type.\n");
                    }
                    else
                    {
                        p.type = type;
                        add_recipient(p);
                    }
                }
                break;

            case 3:
                printf("1. Donor by ID\n");
                printf("2. Patient by ID\n");
                printf("3. Donors by organ\n");
                printf("Choose: ");
                scanf("%d", &type);

                if (type == 1)
                {
                    printf("Enter donor ID: ");
                    scanf("%d", &id);
                    search_donor(id);
                }
                else if (type == 2)
                {
                    printf("Enter patient ID: ");
                    scanf("%d", &id);
                    search_recipient(id);
                }
                else if (type == 3)
                {
                    organ = choose_organ();
                    if (organ != -1)
                    {
                        search_donors_by_organ(organ);
                    }
                }
                else
                {
                    printf("Invalid choice.\n");
                }
                break;

            case 4:
                printf("Enter donor ID: ");
                scanf("%d", &id);
                allocate_organ(id);
                break;

            case 5:
                printf("From:\n");
                from = choose_hospital();
                printf("To:\n");
                to = choose_hospital();

                if (from != -1 && to != -1)
                {
                    show_route(from, to);
                }
                break;

            case 6:
                printf("1. View history\n");
                printf("2. Remove latest entry\n");
                printf("Choose: ");
                scanf("%d", &type);

                if (type == 1)
                {
                    show_history();
                }
                else if (type == 2)
                {
                    pop_history();
                }
                else
                {
                    printf("Invalid choice.\n");
                }
                break;

            case 7:
                organ = choose_organ();
                if (organ != -1)
                {
                    show_next_in_line(organ);
                }
                break;

            case 8:
                show_donors();
                show_waitlists();
                printf("\n--- Patient Records (sorted by ID) ---\n");
                tree_inorder(root);
                break;

            case 0:
                printf("Goodbye!\n");
                break;

            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 0);

    free_memory();
    return 0;
}
