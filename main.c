#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define NUM_LEN 20
#define NAME_LEN 50
#define TIME_LEN 50
#define STATUS_LEN 10
#define SPACE_DB_LEN 50
#define VEHICLE_DB_LEN 100
#define VEHICLE_FILE "vehicle_data.txt"
#define PARKING_FILE "parking_data.txt"
#define PARTS_LEN 5
#define DATA_LEN 4

typedef enum{FAILURE,SUCCESS} statusCode;
typedef enum{FALSE,TRUE} boolean;
typedef enum{BY_PARKINGS,BY_AMT_PAID} sort_vehicle;
typedef enum{BY_OCCUPANCY,BY_MAX_REVENUE} sort_spaces;
typedef enum{ASCENDING,DESCENDING} order;

typedef struct vehicle_details
{
    int vehicle_number;
    char owner_name[NUM_LEN];
    time_t arrival_time;
    time_t departure_time;
    char membership[STATUS_LEN];
    long int total_parking_hours;
    int parking_space_ID;
    int parking_count;
    double total_amount_paid;
    struct vehicle_details *next;//to maintain a linked list for vehicles with same keys
} vehicle;

typedef struct parking_space_status
{
    int space_ID;
    int status;
    int occupancy_count;
    double revenue_generated;
    struct parking_space_status *next;//to maintain a linked list for spaces with same keys
} space;

typedef struct data
{
    float key;
    vehicle *Vrecord; // pointer to vehicle record
} data;

typedef struct Sdata
{
    float key;
    space *Srecord; // pointer to space record
} Sdata;

typedef struct vehicleNode
{
    data **keyArr;                   // Array of POINTERS to vehicle record
    struct vehicleNode **partitions; // array of pointers for partitions(max 5)
    struct vehicleNode *parent;      // pointer to parent node
    boolean isLeaf;                  // 1 if leaf node (data node), 0 if index node
    struct vehicleNode *next;        // pointer to next node
    struct vehicleNode *prev;        // pointer to previous node
} vehicleNode;

typedef struct spaceNode
{
    Sdata **keyArr;                // Array of POINTERS to space record
    struct spaceNode **partitions; // array of pointers for partitions(max 5)
    struct spaceNode *parent;      // pointer to parent node
    boolean isLeaf;                // 1 if leaf node (data node), 0 if index node
    struct spaceNode *next;        // pointer to next node
    struct spaceNode *prev;        // pointer to previous node
} spaceNode;

// creating a node while inserting vehicle details for a new vehicle
statusCode createVehicleNode(int number, char *name, time_t time, int id, vehicle **newVehicle)
{
    statusCode sc = SUCCESS;
    *newVehicle = (vehicle *)malloc(sizeof(vehicle));
    if (*newVehicle == NULL)
        sc = FAILURE;
    
    else
    {
        (*newVehicle)->vehicle_number = number;
        strcpy((*newVehicle)->owner_name, name);
        (*newVehicle)->arrival_time = time;
        strcpy((*newVehicle)->membership, "GENERAL");
        (*newVehicle)->total_parking_hours = 0;
        (*newVehicle)->parking_count = 1;
        (*newVehicle)->total_amount_paid = 0;
        (*newVehicle)->parking_space_ID = id;
        (*newVehicle)->next = NULL;
    }
    return sc;
}

vehicleNode *createVTreeNode(vehicleNode *parentNode)
{
    vehicleNode *newNode = (vehicleNode *)malloc(sizeof(vehicleNode));
    newNode->parent = parentNode;
    newNode->keyArr = (data **)malloc(sizeof(data *) * DATA_LEN);
    for (int i = 0; i < DATA_LEN; i++)
    {
        newNode->keyArr[i] = (data *)malloc(sizeof(data)); // allocate each `data*`
        newNode->keyArr[i]->key = -1;
        newNode->keyArr[i]->Vrecord = NULL;
    }
    newNode->isLeaf = FALSE; // Use `false` from stdbool.h
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->partitions = (vehicleNode **)malloc(sizeof(vehicleNode *) * PARTS_LEN);
    for (int i = 0; i < PARTS_LEN; i++)
        newNode->partitions[i] = NULL;
    return newNode;
}

space *createSpaceNode(space *newNode, int i, int s, int count, double revenue)
{
    newNode = (space *)malloc(sizeof(space));
    newNode->space_ID = i;
    newNode->status = s; // Initially all spaces are free
    newNode->occupancy_count = count;
    newNode->revenue_generated = revenue;
    newNode->next = NULL;
    return newNode;
}

spaceNode *createSTreeNode(spaceNode *parentNode)
{
    spaceNode *newNode = (spaceNode *)malloc(sizeof(spaceNode));
    newNode->parent = parentNode;
    newNode->keyArr = (Sdata **)malloc(sizeof(Sdata *) * DATA_LEN);
    for (int i = 0; i < DATA_LEN; i++)
    {
        newNode->keyArr[i] = (Sdata *)malloc(sizeof(Sdata)); // allocate each `data*`
        newNode->keyArr[i]->key = -1;
        newNode->keyArr[i]->Srecord = NULL;
    }
    newNode->isLeaf = FALSE; // Use `false` from stdbool.h
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->partitions = (spaceNode **)malloc(sizeof(spaceNode *) * PARTS_LEN);
    for (int i = 0; i < PARTS_LEN; i++)
        newNode->partitions[i] = NULL;
    return newNode;
}

void copyData(data *node1, data *node2)
{
    node1->key = node2->key;
    node1->Vrecord = node2->Vrecord;
}
void copySData(Sdata *node1, Sdata *node2)
{
    node1->key = node2->key;
    node1->Srecord = node2->Srecord;
}

boolean compare(order o, float a, float b)
{
    boolean retVal = TRUE;
    if ((a >= b && o == ASCENDING) || (a <= b && o == DESCENDING))
        retVal = FALSE;

    return retVal;
}

void insert_V(data *value, data **keyArr, order o)
{
    int i = 0;
    while (keyArr[i]->key != -1 && compare(o, keyArr[i]->key, value->key) && i < DATA_LEN)
        i++;
    // i gives the position where value is to be inserted
    if (keyArr[i]->key != -1 || i != DATA_LEN - 1) // can not directly insert the value ,need to shift values on the right side
    {
        int j = DATA_LEN - 1;
        while (j != i)
        {
            copyData(keyArr[j], keyArr[j - 1]);
            j--;
        }
    }
    copyData(keyArr[i], value);
}

void insert_S(Sdata *value, Sdata **keyArr, order o)
{
    int i = 0;
    while (keyArr[i]->key != -1 && compare(o, keyArr[i]->key, value->key) && i < DATA_LEN)
        i++;
    // i gives the position where value is to be inserted
    if (keyArr[i]->key != -1 || i != DATA_LEN - 1) // can not directly insert the value ,need to shift values on the right side
    {
        int j = DATA_LEN - 1;
        while (j != i)
        {
            copySData(keyArr[j], keyArr[j - 1]);
            j--;
        }
    }
    copySData(keyArr[i], value);
}

data *findMedian_V(data *value, data *arr[], order o)
{
    data *retVal = (data *)malloc(sizeof(data));
    int i = 0;
    int e = value->key;
    while (i < DATA_LEN && arr[i] && compare(o, arr[i]->key, e))
        i++;
    if (i == 0 || i == 1)
        copyData(retVal, arr[1]);
    else if (i == 2)
        copyData(retVal, value);
    else
        copyData(retVal, arr[2]);

    return retVal;
}

Sdata *findMedian_S(Sdata *value, Sdata *arr[], order o)
{
    Sdata *retVal = (Sdata *)malloc(sizeof(Sdata));
    int i = 0;
    int e = value->key;
    while (i < DATA_LEN && arr[i] && compare(o, arr[i]->key, e))
        i++;
    if (i == 0 || i == 1)
        copySData(retVal, arr[1]);
    else if (i == 2)
        copySData(retVal, value);
    else
        copySData(retVal, arr[2]);

    return retVal;
}

void splitValues_V(data *A[], data *B[], int start1, int start2, data *med, data *value, order o)
{
    for (int i = 0; i < DATA_LEN; i++)
    {
        if (compare(o, A[i]->key, med->key))
        {
            copyData(A[start1], A[i]);
            start1++;
        }
        else if (!compare(o, A[i]->key, med->key) && A[i]->key != med->key)
        {
            copyData(B[start2], A[i]);
            start2++;
        }
    }
    if (!compare(o, value->key, med->key) && value->key != med->key)
        insert_V(value, B, o);

    else if (compare(o, value->key, med->key) && value->key != med->key)
        insert_V(value, A, o);
}

void splitValues_S(Sdata *A[], Sdata *B[], int start1, int start2, Sdata *med, Sdata *value, order o)
{
    for (int i = 0; i < DATA_LEN; i++)
    {
        if (compare(o, A[i]->key, med->key))
        {
            copySData(A[start1], A[i]);
            start1++;
        }
        else if (!compare(o, A[i]->key, med->key) && A[i]->key != med->key)
        {
            copySData(B[start2], A[i]);
            start2++;
        }
    }
    if (value->key != med->key && !compare(o, value->key, med->key))
        insert_S(value, B, o);

    else if (compare(o, value->key, med->key))
        insert_S(value, A, o);
}

void insertIntoList_V(vehicleNode *node1, vehicleNode *node2)
{
    vehicleNode *temp = node1->next;
    node1->next = node2;
    node2->prev = node1;
    node2->next = temp;
    if (temp != NULL)
        temp->prev = node2;
}

void insertIntoList_S(spaceNode *node1, spaceNode *node2)
{
    spaceNode *temp = node1->next;
    node1->next = node2;
    node2->prev = node1;
    node2->next = temp;
    if (temp != NULL)
        temp->prev = node2;
}

vehicleNode *traveseUpToLeaf_V(vehicleNode *curr, int num, order o)
{
    while (!curr->isLeaf)
    {
        int i = 0;
        while (i < DATA_LEN && curr->keyArr[i]->key != -1 && !compare(o, num, curr->keyArr[i]->key))
        {
            i++;
        }
        curr = curr->partitions[i];
    }
    return curr; // Now `curr` is the leaf node
}

spaceNode *traveseUpToLeaf_S(spaceNode *curr, int num, order o)
{
    while (!curr->isLeaf)
    {
        int i = 0;
        while (i < DATA_LEN && curr->keyArr[i]->key != -1 && !compare(o, num, curr->keyArr[i]->key))
        {
            i++;
        }
        curr = curr->partitions[i];
    }
    return curr; // Now `curr` is the leaf node
}

vehicleNode *insertIntoParent_V(vehicleNode *parent, data *value, vehicleNode *leftChild, vehicleNode *rightChild, vehicleNode *Vroot, order o); // handles insertion of internal nodes(Btree nodes)
vehicleNode *insertinVtree(vehicleNode *Vroot, data *value, vehicle *v, order o)
{
    vehicleNode *curr = Vroot;
    if (Vroot == NULL) // empty tree
    {
        vehicleNode *newNode = createVTreeNode(NULL);
        copyData(newNode->keyArr[0], value);
        newNode->isLeaf = TRUE;
        Vroot = newNode;
    }
    else
    {
        int e = value->key;
        curr = traveseUpToLeaf_V(curr, e, o); // traversing up to leaf node
        int inserted = 0;
        for (int i = 0; i < DATA_LEN && curr->keyArr[i]->key != -1 && !inserted; i++)
        {
            if (curr->keyArr[i]->key == e) // want to insert value with existing key
            {
                vehicle *temp = curr->keyArr[i]->Vrecord;
                while (temp->next != NULL)
                    temp = temp->next;

                temp->next = v;
                inserted = 1;
            }
        }
        if (!inserted)
        {
            if (curr->keyArr[DATA_LEN - 1]->key == -1) // check if space is available in leaf node
            {
                insert_V(value, curr->keyArr, o); // directly inserting into the node
            }
            else //(case 2) split the node across median
            {
                vehicleNode *sibling1 = curr;
                data *med = findMedian_V(value, curr->keyArr, o);
                vehicleNode *sibling2 = createVTreeNode(curr->parent);
                sibling2->isLeaf = TRUE;
                copyData(sibling2->keyArr[0], med);
                splitValues_V(sibling1->keyArr, sibling2->keyArr, 0, 1, med, value, o);
                sibling1->keyArr[2]->Vrecord = sibling1->keyArr[3]->Vrecord = NULL;
                sibling1->keyArr[2]->key = sibling1->keyArr[3]->key = -1;
                insertIntoList_V(sibling1, sibling2);                                        // maitaining the doubly linked list
                Vroot = insertIntoParent_V(curr->parent, med, sibling1, sibling2, Vroot, o); // inserting into a Btree node
            }
        }
    }
    return Vroot;
}

void printSrecord(space *s);
spaceNode *insertIntoParent_S(spaceNode *parent, Sdata *value, spaceNode *leftChild, spaceNode *rightChild, spaceNode *Sroot, order o); // handles insertion of internal nodes(Btree nodes)
spaceNode *insertinStree(spaceNode *Sroot, Sdata *value, space *s, order o)
{
    spaceNode *curr = Sroot;
    if (Sroot == NULL) // empty tree
    {
        spaceNode *newNode = createSTreeNode(NULL);
        copySData(newNode->keyArr[0], value);
        newNode->isLeaf = TRUE;
        Sroot = newNode;
    }
    else
    {
        int e = value->key;
        curr = traveseUpToLeaf_S(curr, e, o); // traversing up to leaf node
        int inserted = 0;
        for (int i = 0; i < DATA_LEN && curr->keyArr[i]->key != -1 && !inserted; i++)
        {
            if (curr->keyArr[i]->key == e) // want to insert value with existing key
            {
                space *temp = curr->keyArr[i]->Srecord;
                while (temp->next != NULL)
                {
                    temp = temp->next;
                }
                temp->next = s;
                inserted = 1;
            }
        }
        if (!inserted)
        {
            if (curr->keyArr[DATA_LEN - 1]->key == -1) // check if space is available in leaf node
            {
                insert_S(value, curr->keyArr, o); // directly inserting into the node
            }
            else //(case 2) split the node across median
            {
                spaceNode *sibling1 = curr;
                Sdata *med = findMedian_S(value, curr->keyArr, o);
                spaceNode *sibling2 = createSTreeNode(curr->parent);
                sibling2->isLeaf = TRUE;
                copySData(sibling2->keyArr[0], med);
                splitValues_S(sibling1->keyArr, sibling2->keyArr, 0, 1, med, value, o);
                sibling1->keyArr[2]->Srecord = sibling1->keyArr[3]->Srecord = NULL;
                sibling1->keyArr[2]->key = sibling1->keyArr[3]->key = -1;
                insertIntoList_S(sibling1, sibling2);                                        // maitaining the doubly linked list
                Sroot = insertIntoParent_S(curr->parent, med, sibling1, sibling2, Sroot, o); // inserting into a Btree node
            }
        }
    }
    return Sroot;
}

void shiftRight_V(vehicleNode *arr[], int index)
{
    for (int i = PARTS_LEN; i > index; i--)
        arr[i] = arr[i - 1];
}

void shiftRight_S(spaceNode *arr[], int index)
{
    for (int i = PARTS_LEN; i > index; i--)
        arr[i] = arr[i - 1];
}

void adjustPartitions_V(vehicleNode *leftParent, vehicleNode *rightParent, data *med, vehicleNode *leftChild, vehicleNode *rightChild)
{
    int i, j;
    if (med->key == leftParent->keyArr[1]->key)
    {
        i = 0;j = 2;
        while (i <= 2)
            rightParent->partitions[i++] = leftParent->partitions[j++];

        if (leftParent->partitions[0] == leftChild)
        {
            leftParent->partitions[2] = leftParent->partitions[1];
            leftParent->partitions[1] = rightChild;
        }
        else // eftParent -> partitions[1] == leftChild
            leftParent->partitions[2] = rightChild;
    }
    else if (med->key == leftParent->keyArr[2]->key)
    {
        int k, l;
        if (leftParent->partitions[3] == leftChild)
        {
            i = 1;j = 0;k = 2;l = 4;
        }
        else // leftParent -> partitions[3] == leftChild
        {
            i = 2;j = 1;k = 0;l = 3;
        }
        rightParent->partitions[i] = rightChild;
        rightParent->partitions[j] = leftChild;
        rightParent->partitions[k] = leftParent->partitions[l];
    }
    else // value == medium
    {
        i = 1, j = 3;
        while (i <= 2)
            rightParent->partitions[i++] = leftParent->partitions[j++];

        rightParent->partitions[0] = rightChild;
    }
    leftParent->partitions[3] = NULL;
    leftParent->partitions[4] = NULL;
    for (i = 0; i < 3; i++)
        (rightParent->partitions[i])->parent = rightParent;
}

void adjustPartitions_S(spaceNode *leftParent, spaceNode *rightParent, Sdata *med, spaceNode *leftChild, spaceNode *rightChild)
{
    int i, j;
    if (med->key == leftParent->keyArr[1]->key)
    {
        i = 0;j = 2;
        while (i <= 2)
            rightParent->partitions[i++] = leftParent->partitions[j++];

        if (leftParent->partitions[0] == leftChild)
        {
            leftParent->partitions[2] = leftParent->partitions[1];
            leftParent->partitions[1] = rightChild;
        }
        else // leftParent -> partitions[1] == leftChild
            leftParent->partitions[2] = rightChild;
    }
    else if (med->key == leftParent->keyArr[2]->key)
    {
        int k, l;
        if (leftParent->partitions[3] == leftChild)
        {
            i = 1;j = 0;k = 2;l = 4;
        }
        else // leftParent -> partitions[3] == leftChild
        {
            i = 2;j = 1;k = 0;l = 3;
        }
        rightParent->partitions[i] = rightChild;
        rightParent->partitions[j] = leftChild;
        rightParent->partitions[k] = leftParent->partitions[l];
    }
    else // value == median
    {
        i = 1, j = 3;
        while (i <= 2)
            rightParent->partitions[i++] = leftParent->partitions[j++];

        rightParent->partitions[0] = rightChild;
    }
    leftParent->partitions[3] = NULL;
    leftParent->partitions[4] = NULL;
    for (i = 0; i < 3; i++)
        (rightParent->partitions[i])->parent = rightParent;
}

vehicleNode *insertIntoParent_V(vehicleNode *parent, data *value, vehicleNode *leftChild, vehicleNode *rightChild, vehicleNode *Vroot, order o) // handles insertion of internal nodes(Btree nodes)
{
    if (parent == NULL)
    {
        vehicleNode *newRoot = createVTreeNode(NULL);
        copyData(newRoot->keyArr[0], value);
        newRoot->partitions[0] = leftChild;
        newRoot->partitions[1] = rightChild;
        leftChild->parent = newRoot;
        rightChild->parent = newRoot;
        Vroot = newRoot;
    }
    else if (parent->keyArr[DATA_LEN - 1]->key == -1) // space available at parent node
    {
        insert_V(value, parent->keyArr, o);
        int i = 0;
        while (parent->partitions[i] != leftChild)
            i++;

        if (parent->partitions[i + 1] != NULL)
            shiftRight_V(parent->partitions, i + 1);

        parent->partitions[i + 1] = rightChild;
    }
    else
    {
        vehicleNode *grandParent = parent->parent;
        vehicleNode *leftParent = parent;
        vehicleNode *rightParent = createVTreeNode(grandParent);
        data *med = findMedian_V(value, parent->keyArr, o);
        adjustPartitions_V(leftParent, rightParent, med, leftChild, rightChild);     // adjusting the partitions
        splitValues_V(leftParent->keyArr, rightParent->keyArr, 0, 0, med, value, o); // spliting the values across parent nodes
        leftParent->keyArr[2]->key = leftParent->keyArr[3]->key = -1;
        leftParent->keyArr[2]->Vrecord = leftParent->keyArr[3]->Vrecord = NULL;
        Vroot = insertIntoParent_V(grandParent, med, leftParent, rightParent, Vroot, o); // recursive call
    }
    return Vroot;
}

spaceNode *insertIntoParent_S(spaceNode *parent, Sdata *value, spaceNode *leftChild, spaceNode *rightChild, spaceNode *Sroot, order o) // handles insertion of internal nodes(Btree nodes)
{
    if (parent == NULL)
    {
        spaceNode *newRoot = createSTreeNode(NULL);
        copySData(newRoot->keyArr[0], value);
        newRoot->partitions[0] = leftChild;
        newRoot->partitions[1] = rightChild;
        leftChild->parent = newRoot;
        rightChild->parent = newRoot;
        Sroot = newRoot;
    }
    else if (parent->keyArr[DATA_LEN - 1]->key == -1) // space available at parent node
    {
        insert_S(value, parent->keyArr, o);
        int i = 0;
        while (parent->partitions[i] != leftChild)
            i++;

        if (parent->partitions[i + 1] != NULL)
            shiftRight_S(parent->partitions, i + 1);

        parent->partitions[i + 1] = rightChild;
    }
    else
    {
        spaceNode *grandParent = parent->parent;
        spaceNode *leftParent = parent;
        spaceNode *rightParent = createSTreeNode(grandParent);
        Sdata *med = findMedian_S(value, parent->keyArr, o);
        adjustPartitions_S(leftParent, rightParent, med, leftChild, rightChild);     // adjusting the partitions
        splitValues_S(leftParent->keyArr, rightParent->keyArr, 0, 0, med, value, o); // spliting the values across parent nodes
        leftParent->keyArr[2]->key = leftParent->keyArr[3]->key = -1;
        leftParent->keyArr[2]->Srecord = leftParent->keyArr[3]->Srecord = NULL;
        Sroot = insertIntoParent_S(grandParent, med, leftParent, rightParent, Sroot, o); // recursive call
    }
    return Sroot;
}

statusCode insertVehicle(vehicleNode *Vroot, int num, char *name, time_t time, int space_id, vehicle *newVehicle, order o)
{
    statusCode sc = createVehicleNode(num, name, time, space_id, &newVehicle);
    if (sc)
    {
        data *value = (data *)malloc(sizeof(data));
        value->key = num;
        value->Vrecord = newVehicle;
        Vroot = insertinVtree(Vroot, value, newVehicle, o);
        free(value);
    }
    return sc;
}

time_t convertToTimeT(const char *timeString)
{
    struct tm timeInfo;
    memset(&timeInfo, 0, sizeof(struct tm)); // Initialize struct to zero
    // Parse the input string
    if (sscanf(timeString, "%d-%d-%d_%d:%d", &timeInfo.tm_year, &timeInfo.tm_mon, &timeInfo.tm_mday, &timeInfo.tm_hour, &timeInfo.tm_min) != 5)
    {
        printf("Invalid time format!\n");
        return -1; // Return -1 to indicate error
    }
    timeInfo.tm_year -= 1900; // Adjust year (tm_year is years since 1900)
    timeInfo.tm_mon -= 1;     // Adjust month (tm_mon is 0-based)
    timeInfo.tm_sec = 0;      // Set seconds to 0
    return mktime(&timeInfo); // Convert to time_t
}

void convertTimeToString(time_t timeValue, char *output)
{
    struct tm *timeInfo = localtime(&timeValue); // Convert to local time
    if (timeInfo == NULL)
    {
        strcpy(output, "0000-00-00_00:00"); // Handle error case
        return;
    }
    sprintf(output, "%04d-%02d-%02d_%02d:%02d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, timeInfo->tm_hour, timeInfo->tm_min);
}

void printVrecord(vehicle *v)
{
    printf("Vehicle No.: %d  ", v->vehicle_number);
    printf("Name: %s  ", v->owner_name);
    printf("Pk.Count: %d  ", v->parking_count);
    printf("Amt.Paid: %lf  ", v->total_amount_paid);
    printf("\n");
}

void printVTree(vehicleNode *root);
vehicleNode *loadVehiclesFromFile(vehicleNode *Vroot)
{
    FILE *file = fopen(VEHICLE_FILE, "r");
    vehicleNode *retVal = NULL;
    vehicle *newVehicle = NULL;
    if (file != NULL)
    {
        int number, spaceID, count;
        char name[NAME_LEN], membership[STATUS_LEN];
        long int hours;
        double amount;
        time_t arrival, departure;
        char time1[TIME_LEN], time2[TIME_LEN];
        data *value = malloc(sizeof(data));
        int i = 1;
        while (fscanf(file, "%d %s %s %s %s %ld %d %d %lf", &number, name, time1, time2, membership, &hours, &spaceID, &count, &amount) == 9)
        {
            arrival = convertToTimeT(time1);
            departure = convertToTimeT(time2);
            createVehicleNode(number, name, arrival, spaceID, &newVehicle);
            if (newVehicle)
            {
                strcpy(newVehicle->membership, membership);
                newVehicle->departure_time = departure;
                newVehicle->total_parking_hours = hours;
                newVehicle->parking_space_ID = spaceID;
                newVehicle->parking_count = count;
                newVehicle->total_amount_paid = amount;
                value->key = number;
                value->Vrecord = newVehicle;
                Vroot = insertinVtree(Vroot, value, newVehicle, ASCENDING);
            }
        }
        retVal = Vroot;
        fclose(file);
        free(value);
        printf("Vehicles loaded successfully\n");
    }
    else
    {
        printf("Error opening file\n");
    }
    return retVal;
}

void printVTree(vehicleNode *root)
{
    vehicleNode *curr = root;
    vehicle *v;
    while (curr->isLeaf == 0)
    {
        curr = curr->partitions[0];
    }
    while (curr != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            if (curr->keyArr[i] != NULL && curr->keyArr[i]->key != -1)
            {
                v = curr->keyArr[i]->Vrecord;
                while (v)
                {
                    printVrecord(v);
                    v = v->next;
                }
            }
        }
        curr = curr->next;
    }
    printf("\n\n");
}

void printTree(spaceNode *root);
spaceNode *loadParkingSpacesFromFile(spaceNode *Sroot)
{
    FILE *file = fopen(PARKING_FILE, "r");
    spaceNode *retVal = NULL;
    if (file != NULL)
    {
        Sdata *value = malloc(sizeof(Sdata));
        space *newNode;
        spaceNode *Sroot = NULL;
        int id, status, occupancy;
        double revenue;
        int i = 1;
        while (fscanf(file, "%d %d %d %lf", &id, &status, &occupancy, &revenue) == 4)
        {
            newNode = createSpaceNode(newNode, i, status, occupancy, revenue);
            value->key = id;
            value->Srecord = newNode;
            Sroot = insertinStree(Sroot, value, value->Srecord, ASCENDING);
            i++;
        }
        fclose(file);
        free(value);
        retVal = Sroot;
        printf("Parking spaces loaded successfully !\n");
    }
    else
    {
        printf("Error opening file\n");
    }
    return retVal;
}

void printSrecord(space *s)
{
    printf("ID : %d  ", s->space_ID);
    printf("Status : %d  ", s->status);
    printf("revenue : %lf  ", s->revenue_generated);
    printf("occupancy : %d  ", s->occupancy_count);
    printf("\n");
}

void printTree(spaceNode *root)
{
    space *s;
    spaceNode *curr = root;
    while (curr->isLeaf == 0)
        curr = curr->partitions[0];

    while (curr != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            if (curr->keyArr[i] != NULL && curr->keyArr[i]->key != -1)
            {
                s = curr->keyArr[i]->Srecord;
                while (s)
                {
                    printSrecord(s);
                    s = s->next;
                }
            }
        }
        curr = curr->next;
    }
    printf("\n\n");
}

data *vehicleSearch(vehicleNode *Vroot, int num, order o)
{
    data *retVal = NULL;
    if (Vroot)
    {
        vehicleNode *curr = Vroot;
        curr = traveseUpToLeaf_V(curr, num, o); // Now `curr` is the leaf node; check if the value exists
        for (int i = 0; i < DATA_LEN && retVal == NULL; i++)
        {
            if (curr->keyArr[i]->key == num)
            {
                retVal = curr->keyArr[i]; // Value found
            }
        }
    } // else value not found returns retVal = NULL
    return retVal;
}

Sdata *spaceSearch(spaceNode *Sroot, int num, spaceNode **leaf, order o)
{
    Sdata *retVal = NULL;
    if (Sroot)
    {
        spaceNode *curr = Sroot;
        curr = traveseUpToLeaf_S(curr, num, o); // traversing up to leaf node
        *leaf = curr;                           // return the leaf node to start seaching from there
        for (int i = 0; i < DATA_LEN && retVal == NULL; i++)
        {
            if (curr->keyArr[i]->key == num)
            {
                retVal = curr->keyArr[i]; // Value found
            }
        }
    } // else value not found returns retVal = NULL
    return retVal;
}

space *searchFreeSpace(int start, spaceNode *Sroot)
{
    spaceNode *leaf;
    Sdata *s = spaceSearch(Sroot, start, &leaf, ASCENDING); // start from leaf node
    spaceNode *curr = leaf;
    space *freeSpace = NULL;
    space *sp;
    while (freeSpace == NULL && curr != NULL)
    {
        for (int i = 0; i < DATA_LEN && freeSpace == NULL; i++)
        {
            if (curr->keyArr[i]->key >= start && curr->keyArr[i]->key != -1)
            {
                sp = curr->keyArr[i]->Srecord;
                if (sp->status == 0) // space is free
                {
                    freeSpace = sp;
                    printf("\nFree space found : %d\n", freeSpace->space_ID);
                }
            }
        }
        curr = curr->next; // move to next leaf
    }
    return freeSpace;
}

space *findSpace(vehicle *vnode, int num, vehicleNode *Vroot, spaceNode *Sroot) // searches for free space where the vehicle can be parked
{
    space *freeSpace = NULL;
    if (vnode != NULL) // vehicle record is present in database
    {
        printVrecord(vnode); // contain the vehicle record if the vehicle is already present in the database
        if (strcmp(vnode->membership, "GOLDEN") == 0)
            freeSpace = searchFreeSpace(1, Sroot);

        else if (strcmp(vnode->membership, "PREMIUM") == 0)
            freeSpace = searchFreeSpace(11, Sroot);

        else // for general membership
            freeSpace = searchFreeSpace(21, Sroot);
    }
    else // new vehicle entered search in general space
        freeSpace = searchFreeSpace(21, Sroot);

    return freeSpace;
}

void updateDetails(time_t time, vehicle *vehicle, space *space)
{
    vehicle->arrival_time = time;
    vehicle->parking_space_ID = space->space_ID;
    vehicle->parking_count += 1;
    space->occupancy_count += 1;
    space->status = 1;
}

statusCode updateInsertVehicle(int number, char *name, time_t time, vehicleNode *Vroot, spaceNode *Sroot)
{
    vehicleNode *leaf = NULL;
    statusCode sc = SUCCESS;
    space *freeSpace = NULL;
    data *vdata = vehicleSearch(Vroot, number, ASCENDING);
    vehicle *vehicle = NULL;
    if (vdata)
        vehicle = vdata->Vrecord; // for vehicle node at which vehicle data is present if vehicle is already registered

    if (vehicle != NULL && vehicle->parking_space_ID != 0) // checking is car is already parked
    {
        printf("\nError :Can not insert , This vehicle is already parked !\n\n");
        sc = FAILURE;
    }
    else
    {
        freeSpace = findSpace(vehicle, number, Vroot, Sroot); // search for freeSpace in database
        if (freeSpace != NULL)
        {
            if (vehicle != NULL) // vechicle is already registerd
            {
                updateDetails(time, vehicle, freeSpace);
                printf("\nVehicle details updated successfully with space id : %d\n\n", freeSpace->space_ID);
            }
            else
            {
                sc = insertVehicle(Vroot, number, name, time, freeSpace->space_ID, vehicle, ASCENDING);
                printf("\nVehicle details inserted successfully with space id : %d\n\n", freeSpace->space_ID);
                freeSpace->status = 1; // now space is occupied
                freeSpace->occupancy_count = freeSpace->occupancy_count + 1;
            }
        }
        else // space not found
        {
            sc = FAILURE;
            printf("\nParking space exhausted can not insert or update the information.\n\n");
        }
    }
    return sc;
}

statusCode ExitVehicle(int number, time_t time, vehicleNode *Vroot, spaceNode *Sroot)
{
    vehicleNode *leaf;
    data *Vnode = vehicleSearch(Vroot, number, ASCENDING);
    statusCode sc = SUCCESS;
    if (Vnode == NULL)
    {
        printf("\nError: Vehicle not found in the records.\n\n");
        sc = FAILURE;
    }
    else // vehicle found in records
    {
        vehicle *Vehicle = Vnode->Vrecord; // vehicle record
        space *space;
        if (Vehicle->parking_space_ID == 0)
        {
            printf("\nError: Vehicle is not parked in any space.\n\n");
            sc = FAILURE;
        }
        else
        {
            spaceNode *leaf;
            Sdata *snode = spaceSearch(Sroot, Vehicle->parking_space_ID, &leaf, ASCENDING); // search for the space record
            space = snode->Srecord;                                                         // space record
            int hours_spent = difftime(time, Vehicle->arrival_time) / 3600;
            printf("\nHours spent: %d hours\n", hours_spent);
            // updating vehicle details
            Vehicle->departure_time = time;
            Vehicle->total_parking_hours = Vehicle->total_parking_hours + hours_spent;
            Vehicle->parking_space_ID = 0;
            if (Vehicle->total_parking_hours > 200) // updating membership
            {
                if (strcmp(Vehicle->membership, "GOLDEN") != 0) // Upgrade to GOLDEN if not already GOLDEN
                {
                    strcpy(Vehicle->membership, "GOLDEN");
                    printf("Membership upgraded to GOLDEN\n");
                }
            }
            else if (Vehicle->total_parking_hours > 100)
            {
                if (strcmp(Vehicle->membership, "PREMIUM") != 0) // Upgrade to PREMIUM if not already PREMIUM
                {
                    strcpy(Vehicle->membership, "PREMIUM");
                    printf("Membership upgraded to PREMIUM\n");
                }
            }
            // Calculate parking charges
            double charges = 0.0;
            if (hours_spent <= 3)
            {
                charges = 100.0;
            }
            else
            {
                charges = (hours_spent - 3) * 50.0 + 100.0;
            }
            if (strcmp(Vehicle->membership, "GENERAL") != 0)//for memberships other than general
            {
                charges = charges * 0.9;
            }
            printf("Parking charges: %.2lf Rs\n", charges);
            Vehicle->total_amount_paid = Vehicle->total_amount_paid + charges;
            // updating space details
            space->status = 0;                                             // now the space is unoccupied
            space->revenue_generated = space->revenue_generated + charges; // calculate current cost and add;
            printf("\n--------------------------------------------------------------------------------------\n");
            printf("Vehicle exited successfully!\nParking space ID %d is now free.\n\n", space->space_ID);
        }
    }
    return sc;
}

vehicle *copyVrecord(vehicle *v)
{
    vehicle *newVehicle = (vehicle *)malloc(sizeof(vehicle));
    newVehicle->vehicle_number = v->vehicle_number;
    strcpy(newVehicle->owner_name, v->owner_name);
    newVehicle->arrival_time = v->arrival_time;
    newVehicle->departure_time = v->departure_time;
    strcpy(newVehicle->membership, v->membership);
    newVehicle->total_parking_hours = v->total_parking_hours;
    newVehicle->parking_space_ID = v->parking_space_ID;
    newVehicle->parking_count = v->parking_count;
    newVehicle->total_amount_paid = v->total_amount_paid;
    newVehicle->next = NULL; // Initialize next pointer to NULL
    return newVehicle;
}

void freeVehicleTree(vehicleNode *root);
void vehicleReports(vehicleNode *Vroot, sort_vehicle parameter, double min, double max)
{
    vehicleNode *curr = Vroot, *leaf = NULL;
    data *value = (data *)malloc(sizeof(data));
    vehicle *newVehicle;
    vehicleNode *root = NULL;
    while (!curr->isLeaf)
        curr = curr->partitions[0];
    // current reaches leaf node
    while (curr)
    {
        for (int i = 0; i < DATA_LEN && curr->keyArr[i]->key != -1; i++)
        {
            newVehicle = copyVrecord(curr->keyArr[i]->Vrecord); // copy vehicle record
            value->Vrecord = newVehicle;
            if (parameter == BY_PARKINGS)
                value->key = newVehicle->parking_count;
            else
                value->key = newVehicle->total_amount_paid;

            root = insertinVtree(root, value, newVehicle, DESCENDING); // use new root
        }
        curr = curr->next;
    }
    free(value);
    value = NULL;
    if (parameter == BY_PARKINGS)
    {
        printf("\nSorting the vehciles based on the number of parkings done....\n\n");
        printVTree(root);
    }
    else // for amout paid need to print in given range
    {
        int found = 0;
        leaf = traveseUpToLeaf_V(root, max, DESCENDING); // Now `leaf` is the leaf node; check if the value exists
        printf("\nListing all the vehicle in given parking amount range....\n\n");
        curr = leaf;
        while (curr)
        {
            for (int i = 0; i < DATA_LEN; i++)
            {
                if (curr->keyArr[i]->key != -1 && curr->keyArr[i]->key >= min && curr->keyArr[i]->key <= max)
                {
                    newVehicle = curr->keyArr[i]->Vrecord;
                    found = 1;
                    while (newVehicle) // in case of duplicates need to print the list
                    {
                        printVrecord(newVehicle);
                        newVehicle = newVehicle->next;
                    }
                }
            }
            curr = curr->next; // move to next leaf
        }
        if (!found)
            printf("No vehicle found in the given range!!\n\n");
    }
    printf("\n---------------------------------------------------------------------------------------\n\n");
    freeVehicleTree(root);

}

space *copySrecord(space *s)
{
    space *newSpace = (space *)malloc(sizeof(space));
    newSpace->space_ID = s->space_ID;
    newSpace->status = s->status;
    newSpace->occupancy_count = s->occupancy_count;
    newSpace->revenue_generated = s->revenue_generated;
    newSpace->next = NULL; // Initialize next pointer to NULL
    return newSpace;
}

void freeSpaceTree(spaceNode *root);
void spaceReports(spaceNode *Sroot, sort_spaces parameter)
{
    spaceNode *curr = Sroot;
    Sdata *value = (Sdata *)malloc(sizeof(Sdata));
    space *newSpace;
    spaceNode *root = NULL;
    while (!curr->isLeaf)
        curr = curr->partitions[0];
    // current reaches leaf node
    while (curr)
    {
        for (int i = 0; i < DATA_LEN && curr->keyArr[i]->key != -1; i++)
        {
            newSpace = copySrecord(curr->keyArr[i]->Srecord); // copy vehicle record
            value->Srecord = newSpace;
            if (parameter == BY_OCCUPANCY)
                value->key = newSpace->occupancy_count;
            else
                value->key = newSpace->revenue_generated;

            root = insertinStree(root, value, newSpace, DESCENDING); // use new root
        }
        curr = curr->next;
    }
    free(value);
    if (parameter == BY_OCCUPANCY)
        printf("\nSorting the parking spaces based on the occupancy count....\n\n");
    else
        printf("\nSorting the parking spaces based on the revenue generated....\n\n");
    printTree(root);
    printf("\n---------------------------------------------------------------------------------------\n\n");
    freeSpaceTree(root);
}

void saveVehiclesToFile(vehicleNode *Vroot)
{
    FILE *file = fopen(VEHICLE_FILE, "w");
    if (!file)
    {
        printf("Error opening vehicle data file!\n");
    }
    else
    {
        char time1[TIME_LEN], time2[TIME_LEN];

        vehicleNode *curr = Vroot;
        while (curr->isLeaf == 0)
            curr = curr->partitions[0]; // Traverse to the first leaf node

        vehicle *v;
        while (curr)
        {
            for (int i = 0; i < DATA_LEN && curr->keyArr[i]->key != -1; i++)
            {
                v = curr->keyArr[i]->Vrecord;
                while (v)
                {
                    convertTimeToString(v->arrival_time, time1);
                    convertTimeToString(v->departure_time, time2);
                    fprintf(file, "%d %s %s %s %s %ld %d %d %lf\n",v->vehicle_number, v->owner_name, time1, time2,v->membership, v->total_parking_hours,v->parking_space_ID, v->parking_count, v->total_amount_paid);
                    v = v->next;
                }
            }
            curr = curr->next;
        }
    }
    fclose(file);
}

void saveParkingSpacesToFile(spaceNode *Sroot)
{

    FILE *file = fopen(PARKING_FILE, "w");
    if (!file)
    {
        printf("Error opening parking space data file!\n");
    }
    else
    {
        spaceNode *curr = Sroot;
        space *s;
        while (curr->isLeaf == 0)
            curr = curr->partitions[0]; // Traverse to the first leaf node

        while (curr)
        {
            for (int i = 0; i < DATA_LEN && curr->keyArr[i]->key != -1; i++)
            {
                s = curr->keyArr[i]->Srecord;
                while (s)
                {
                    fprintf(file, "%d %d %d %.2lf\n", s->space_ID, s->status, s->occupancy_count, s->revenue_generated);
                    s = s->next;
                }
            }
            curr = curr->next;
        }
    }
    fclose(file);
}

void freeVehicleList(vehicle *v) 
{
    while (v) 
	{
        vehicle *temp = v;
        v = v->next;
        free(temp);
    }
}

void freeVehicleTree(vehicleNode *root) 
{
    if(root == NULL)
        return;
    else 
    {
        for(int i=0;i<PARTS_LEN;i++)
        {
            freeVehicleTree(root->partitions[i]);
            free(root->partitions[i]);
        }
        for (int i = 0; i < DATA_LEN; i++)
        {
            if(root->isLeaf)
                freeVehicleList(root->keyArr[i]->Vrecord);
            free(root->keyArr[i]);
        }    
    }
}

void freeSpaceList(space *s)
{
    while (s) 
	{
        space *temp = s;
        s = s->next;
        free(temp);
    } 
}

void freeSpaceTree(spaceNode *root) 
{
    if(root == NULL)
        return;
    else 
    {
        for(int i=0;i<PARTS_LEN;i++)
        {
            freeSpaceTree(root->partitions[i]);
            free(root->partitions[i]);
        }
        for (int i = 0; i < DATA_LEN; i++)
        {
            if(root->isLeaf)
                freeSpaceList(root->keyArr[i]->Srecord);
            free(root->keyArr[i]);
        }    
    }
}


int main()
{
    spaceNode *Sroot = NULL;
    vehicleNode *Vroot = NULL;
    Sroot = loadParkingSpacesFromFile(Sroot);
    Vroot = loadVehiclesFromFile(Vroot);
    printf("\n Welcome to the Smart Car Parking Lot System!\n");
    int vehicle_num;
    char name[NAME_LEN];
    int choice, subchoice;
    double min = 0.0, max = 0.0;
    statusCode sc;
    do
    {
        printf("1. Vehicle Entry\n");
        printf("2. Vehicle Exit\n");
        printf("3. Sorting and analysis\n");
        printf("4. Exit system\n");
        printf("Please Enter your choice: \n");
        if (scanf("%d", &choice) != 1) 
        {
            printf("Invalid input! Please enter a valid number between 1 and 4.\n");
            while (getchar() != '\n');
        }
        else
		{
			switch (choice)
        	{
        		case 1: // vehicle entry
        	    	printf("Enter your vehicle details\n");
        	    	printf("Enter the vehicle number:\n");
        	    	scanf("%d", &vehicle_num);
        	    	printf("Enter the name of owner:\n");
        	    	scanf(" %[^\n]", name);

        	    	sc = updateInsertVehicle(vehicle_num, name, time(NULL), Vroot, Sroot);
        	    	if(sc)// Save updated data to files
        	    	{
        	    		saveVehiclesToFile(Vroot);
        	        	saveParkingSpacesToFile(Sroot);
        	    	}
        	    	break;
	
    	    	case 2:// Vehicle exit
            		printf("Enter your vehicle number:\n");
            		scanf("%d", &vehicle_num); // Prompt for the vehicle number during exit
            		sc = ExitVehicle(vehicle_num, time(NULL), Vroot, Sroot);
           			if(sc)// Save updated data to files
            		{
            			saveVehiclesToFile(Vroot);
                		saveParkingSpacesToFile(Sroot);
            		}
            		break;

        		case 3:// sorting and analysis
            		do
            		{
            		    printf("1: Sort the list of vehicles based on the number of parkings done\n");
                		printf("2: Print all the vehicles within two parking amounts.\n");
                		printf("3: Sort the parking spaces based on how often they are occupied\n");
                		printf("4: Sort the parking spaces based on the revenue generated \n");
                		printf("5:Exit sorting and analysis \n");

                		printf("Enter your choice:\n");
                		scanf("%d", &subchoice);
                		switch (subchoice)
               			{
                			case 1:
                			    vehicleReports(Vroot, BY_PARKINGS, min, max);
                    			break;

                			case 2:
                			    printf("enter the amount range\n");
                			    int valid = 0;
                    			do
                    			{
                        			printf("minimum amount: ");
                        			scanf("%lf", &min);
                        			printf("maximum amount: ");
                        			scanf("%lf", &max);
                        			if (max <= min || min < 0)
                        		    printf("Invalid input! Please enter a minimum amount greater than 0 and maximum amount greater than minimum amount.\n");
                        			else
                            		valid = 1;
                    			} while (!valid);

                    			vehicleReports(Vroot, BY_AMT_PAID, min, max);
                   				break;

                			case 3:
                			    spaceReports(Sroot, BY_OCCUPANCY);
                    			break;

                			case 4:
                    			spaceReports(Sroot, BY_MAX_REVENUE);
                    			break;

                			case 5:
                    			printf("Exiting analysis and sorting\n");
                    			break;

                			default:
                    		printf("Invalid choice! please enter a valid choice\n");
                    		break;
                		}
            		} while (subchoice != 5);
            		break;

        		case 4:
            		printf("Exiting the system.Thank you!\n");
            		break;

        		default:
            	printf("Invalid choice. Please enter a valid choice \n");
            	break;
        	}
    	}
    } while (choice != 4);  
    freeVehicleTree(Vroot);
    freeSpaceTree(Sroot);
    return 0;
}
