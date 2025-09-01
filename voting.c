#include "voting.h"

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Enable in Makefile
#ifdef DEBUG_PRINTS_ENABLED
#define DebugPrint(...) printf(__VA_ARGS__);
#else
#define DebugPrint(...)
#endif

#define PRIMES_SZ 1024
#define DISTRICTS_SZ 56
#define PARTIES_SZ 5

typedef struct District District;
typedef struct Station Station;
typedef struct Voter Voter;
typedef struct Party Party;
typedef struct Candidate Candidate;
typedef struct ElectedCandidate ElectedCandidate;

struct District
{
    int did;
    int seats;
    int blanks;
    int invalids;
    int partyVotes[PARTIES_SZ];
};

struct Station
{
    int sid;
    int did;
    int registered;
    Voter *voters;
    Station *next;
};
struct Voter
{
    int vid;
    bool voted;
    Voter *parent;
    Voter *lc;
    Voter *rc;
};

struct Party
{
    int pid;
    int electedCount;
    Candidate *candidates;
};
struct Candidate
{
    int cid;
    int did;
    int votes;
    bool isElected;
    Candidate *lc;
    Candidate *rc;
};

struct ElectedCandidate
{
    int cid;
    int did;
    int pid;
    ElectedCandidate *next;
};

District Districts[DISTRICTS_SZ];
Station **StationsHT;
Party Parties[PARTIES_SZ];
ElectedCandidate *Parliament;

const int DefaultDid = -1;
const int BlankDid = -1;
const int InvalidDid = -2;

const int Primes[PRIMES_SZ] = {
    0, 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017, 8039, 8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111, 8117, 8123};
int MaxStationsCount;
int MaxSid;
// Custom Globals
int m;                       // Height of the hashtable
int a_elements_per_line = 3; // Average elements in each chain of the hashtable CHECK

int p; // Prime number used for hashing
int a;
int b;

//--------------------------------------------------------------------- Hash Function ------------------------------------------------------------------------
int Hash(int key)
{
    return ((a * key + b) % p) % m;
}
//--------------------------------------------------------------------- Hash Function ------------------------------------------------------------------------

//--------------------------------------------------------------------- A ------------------------------------------------------------------------
void EventAnnounceElections(int parsedMaxStationsCount, int parsedMaxSid)
{
    DebugPrint("A %d %d\n", parsedMaxStationsCount, parsedMaxSid);
    // TODO
    printf("A <%d> <%d>\n", parsedMaxStationsCount, parsedMaxSid);
    // First of all initialize the Districts array
    for (int i = 0; i < 56; i++)
    {
        Districts[i].did = -1;
        Districts[i].seats = 0;
        Districts[i].blanks = 0;
        Districts[i].invalids = 0;
        for (int j = 0; j < 5; j++)
        {
            Districts[i].partyVotes[j] = 0;
        }
    }
    // Initialize the Stations hashtable
    // Init the globals
    MaxStationsCount = parsedMaxStationsCount;
    MaxSid = parsedMaxSid;
    m = MaxStationsCount / a_elements_per_line;
    StationsHT = malloc(m * sizeof(struct Station *)); // First allocate m amount of pointers to stations
    for (int i = 0; i < m; i++)
    {
        StationsHT[i] = NULL; // Set the pointers of the array to null
    }
    // Choose a p > K
    for (int i = 0; i < PRIMES_SZ; i++)
    {
        if (Primes[i] > MaxSid)
        {
            p = Primes[i];
            break;
        }
    }
    // Random numbers rand() % (max+1 - min) + min
    srand(time(NULL));
    a = rand() % (p - 1 + 1 - 1) + 1; // [1,p)
    b = rand() % (p - 1 + 1 - 0) + 0; // [0,p)
    // Initialize the Parties array
    for (int i = 0; i < 5; i++)
    {
        Parties[i].pid = i;
        Parties[i].electedCount = 0;
        Parties[i].candidates = NULL;
    }
    // Initialize Parliament
    Parliament = NULL;
    printf("DONE\n");
}
//--------------------------------------------------------------------- A ------------------------------------------------------------------------
//--------------------------------------------------------------------- D ------------------------------------------------------------------------
int ReturnFirstEmptyIndex(int left, int right)
{
    if (left > right)
    {
        return -1; // Case where we surpassed the threshold
    }
    int index = (left + right) / 2;
    if (Districts[index].did == -1 && index == 0)
    {
        return 0; // First element is empty (index 0) case
    }
    else if (Districts[index].did == -1 && Districts[index - 1].did != -1)
    {
        return index; // Found the first empty element of the array
    }
    else if (Districts[index].did == -1)
    {
        return ReturnFirstEmptyIndex(left, index - 1); // Simply found a -1 in my array need to check if there are others or not to the left
    }
    else
    {
        return ReturnFirstEmptyIndex(index + 1, right); // Current index is on a valid District need to move to the right
    }
}

void EventCreateDistrict(int did, int seats)
{
    DebugPrint("D %d %d\n", did, seats);
    // TODO
    printf("D <%d> <%d>\n", did, seats);
    int index_to_place_district = ReturnFirstEmptyIndex(0, 55);
    if (index_to_place_district == -1)
    {
        printf("(D event) Array is Full\n");
        return;
    }
    // Place the values
    Districts[index_to_place_district].did = did;
    Districts[index_to_place_district].seats = seats;
    // -- Printing --
    printf(" Districts\n");
    printf(" ");
    int i = 0;
    while (Districts[i].did != -1 && i < DISTRICTS_SZ) // Avoid seg fault when array is full (i < 56)
    {
        if ((i + 1 < DISTRICTS_SZ && Districts[i + 1].did == -1) || i == 55) // For not printing the comma on the last district
        {
            printf("<%d>", Districts[i].did);
        }
        else
        {
            printf("<%d>, ", Districts[i].did);
        }
        i++;
    }
    printf("\nDONE\n");
    // -- Printing --
}
//--------------------------------------------------------------------- D ------------------------------------------------------------------------
//--------------------------------------------------------------------- S ------------------------------------------------------------------------
void InsertSorted(struct Station **head, struct Station *new)
{
    if (*head == NULL)
    {
        *head = new; // If head is NULL just return the first node which is the new one
        return;
    }

    if (new->sid < (*head)->sid)
    {
        new->next = *head; // Case where the new station needs to be in the first position of the list
        *head = new;
        return;
    }

    struct Station *station_traversal = *head;
    struct Station *prev = NULL;
    // Iterate while new value is greater than the others (increasing order)
    while (station_traversal != NULL && station_traversal->sid < new->sid)
    {
        prev = station_traversal;
        station_traversal = station_traversal->next;
    }

    if (station_traversal != NULL && station_traversal->sid == new->sid)
    {
        printf("(S event) This element already exists in the list I did not re-insert it\n");
        return; // Already there is this element in the list CHECK
    }

    // Adjust the pointers
    new->next = station_traversal;
    prev->next = new;
}

void EventCreateStation(int sid, int did)
{
    DebugPrint("S %d %d\n", sid, did);
    // TODO
    printf("S <%d> <%d>\n", sid, did);
    // Allocate memory for a new station and place the values
    struct Station *new_station = malloc(sizeof(struct Station));
    new_station->sid = sid;
    new_station->did = did;
    new_station->registered = 0;
    new_station->voters = NULL;
    new_station->next = NULL;
    // Position
    int h = Hash(sid);
    printf(" Stations[%d]\n", h);
    struct Station **head_of_chain = &StationsHT[h];
    // InsertSorted
    InsertSorted(head_of_chain, new_station);
    // -- Printing --
    struct Station *station_traversal = StationsHT[h];
    while (station_traversal->next != NULL)
    {
        printf(" <%d>,", station_traversal->sid);
        station_traversal = station_traversal->next;
    }
    printf(" <%d>\n", station_traversal->sid); // Print the last element without comma
    printf("DONE\n");
    // -- Printing --
}
//--------------------------------------------------------------------- S ------------------------------------------------------------------------
//--------------------------------------------------------------------- R ------------------------------------------------------------------------
struct Station *LookUpStation(int sid)
{
    struct Station *result = NULL;
    int pos = Hash(sid);      // Take the position of the sid inside the hashtable
    result = StationsHT[pos]; // Access the head of the list

    while (result != NULL && result->sid != sid) // Iterate through the list
    {
        result = result->next;
    }
    return result; // If it was not found NULL is returned because we reached the end of the traversal at the while loop
}

struct Voter *node_factory(int data)
{
    // Create a new voter and return it with all the fields filled
    struct Voter *new_voter = malloc(sizeof(struct Voter));
    new_voter->lc = NULL;
    new_voter->rc = NULL;
    new_voter->parent = NULL;
    new_voter->vid = data;
    new_voter->voted = false;
    return new_voter;
}

struct Voter *InsertCompleteTreeVoters(struct Voter *root, int data, int size)
{

    if (root == NULL)
    {
        root = node_factory(data); // Create root node
        return root;
    }

    // Count how many leafs I should have by first calculating the height
    struct Voter *temp = root;
    int height = 0;
    while (temp->lc != NULL)
    {
        temp = temp->lc;
        height++;
    }

    struct Voter *traversal = temp; // Only one node case
    if (size == 1)
    {
        traversal->lc = node_factory(data);
        traversal->lc->parent = traversal;
    }
    else if (size + 1 == pow(2, height + 1)) // Case where the tree is perfect so we need to go to the leftmost leaf (we already are from the height counting)
    {
        traversal->lc = node_factory(data);
        traversal->lc->parent = traversal; // Set parent pointer
    }
    else if (size + 2 == pow(2, height + 1)) // Case where there is only one missing node (we need to go to the rightest place of the tree)
    {
        traversal = root;
        while (traversal->rc != NULL)
        {
            traversal = traversal->rc;
        }
        traversal->rc = node_factory(data);
        traversal->rc->parent = traversal;
    }
    else
    {
        temp = root;
        int count_of_leafs = pow(2, height);
        int min = 0;
        // Count of leafs is used only in this specific case
        int max = count_of_leafs - 1;
        int value_to_search = (size + 1) % count_of_leafs;
        while (temp != NULL)
        {
            if (temp->lc == NULL)
            {
                temp->lc = node_factory(data);
                temp->lc->parent = temp;
                break;
            }
            else if (temp->rc == NULL)
            {
                temp->rc = node_factory(data);
                temp->rc->parent = temp;
                break;
            }

            // find current min current max
            int control_value = (int)ceil((double)(max + min) / 2);
            if (height == 1)
            {
                control_value = 1;
            }
            else if (height == 2)
            {
                control_value = 2;
            } // case where i need to put if only one is missing check or if tree is full
            if (value_to_search < control_value)
            {
                max = control_value - 1;
                temp = temp->lc;
            }
            else
            {
                min = control_value + 1;
                temp = temp->rc;
            }
        }
    }
    return root;
}

void InOrderPrintVoters(struct Voter *root, int *current_nodes, int max_count_nodes)
{
    if (root == NULL)
    {
        return;
    }

    InOrderPrintVoters(root->lc, current_nodes, max_count_nodes);
    (*current_nodes)++;
    if (*current_nodes == max_count_nodes)
    {
        printf(" <%d>\n", root->vid);
    }
    else
    {
        printf(" <%d>,", root->vid);
    }
    InOrderPrintVoters(root->rc, current_nodes, max_count_nodes);
}

void EventRegisterVoter(int vid, int sid) // CHECK commas and o(logn) and printing
{
    DebugPrint("R %d %d\n", vid, sid);
    // TODO
    printf("R <%d> <%d>\n", vid, sid);
    // First of all I need to look up the specific station
    struct Station *station_traversal = LookUpStation(sid);
    if (station_traversal == NULL)
    {
        printf("(R event) Station was not found in the hashtable\n");
        return;
    }
    // Add the new voter to the complete tree
    station_traversal->voters = InsertCompleteTreeVoters(station_traversal->voters, vid, station_traversal->registered);
    station_traversal->registered++; // Increment the number of registered Voters
    // -- Printing --
    printf(" Voters[%d]\n", sid);
    int current_count = 0; // For printing purposes
    InOrderPrintVoters(station_traversal->voters, &current_count, station_traversal->registered);
    printf("DONE\n");
    // -- Printing --
}
//--------------------------------------------------------------------- R ------------------------------------------------------------------------
//--------------------------------------------------------------------- C ------------------------------------------------------------------------
void InsertBSTCandidates(struct Candidate **root, struct Candidate *new)
{
    if (*root == NULL)
    {
        *root = new; // If head is NULL just return the first node which is the new one
        return;
    }

    struct Candidate *temp = *root;
    struct Candidate *prev = NULL;
    while (temp != NULL)
    {
        if (temp->cid == new->cid)
        {
            printf("(Event C) Candidate already exists in the BST\n");
            return;
        }
        prev = temp;
        if (new->cid < temp->cid)
        {
            temp = temp->lc;
        }
        else
        {
            temp = temp->rc;
        }
    }

    if (new->cid < prev->cid)
    {
        prev->lc = new;
    }
    else
    {
        prev->rc = new;
    }
}

void InOrderPrintCandidates(struct Candidate *root, int max_cid)
{
    if (root == NULL)
    {
        return;
    }

    InOrderPrintCandidates(root->lc, max_cid);
    if (root->cid == max_cid)
    {
        printf(" <%d> <%d>\n", root->cid, root->did);
    }
    else
    {
        printf(" <%d> <%d>,\n", root->cid, root->did);
    }

    InOrderPrintCandidates(root->rc, max_cid);
}

void EventRegisterCandidate(int cid, int pid, int did)
{
    DebugPrint("C %d %d %d\n", cid, pid, did);
    // TODO
    printf("C <%d> <%d> <%d>\n", cid, pid, did);
    printf(" Candidates[%d]\n", pid);
    // We know the indexes of the Parties array correspond to pid
    // Create an array of pointers to the max candidates of every Parties[pid].candidates candidate for printing purposes
    struct Candidate *max_candidates_party[5] = {NULL, NULL, NULL, NULL, NULL};
    // Create a new candidate and fill its field
    struct Candidate *new_candidate = malloc(sizeof(struct Candidate));
    new_candidate->cid = cid;
    new_candidate->did = did;
    new_candidate->isElected = false;
    new_candidate->lc = NULL;
    new_candidate->rc = NULL;
    new_candidate->votes = 0;
    // Update to the new max pointer (printing purposes)
    if (max_candidates_party[pid] == NULL)
    {
        max_candidates_party[pid] = new_candidate;
    }
    else if (new_candidate->cid > max_candidates_party[pid]->cid)
    {
        max_candidates_party[pid] = new_candidate;
    }
    InsertBSTCandidates(&Parties[pid].candidates, new_candidate);
    InOrderPrintCandidates(Parties[pid].candidates, max_candidates_party[pid]->cid);
    printf("DONE\n");
}
//--------------------------------------------------------------------- C ------------------------------------------------------------------------
//--------------------------------------------------------------------- V ------------------------------------------------------------------------
struct Voter *LookUpVoter(struct Voter *root, int vid)
{
    if (root == NULL)
    {
        return NULL; // Tree is empty or end of traversal
    }

    if (root->vid == vid) // No need to check if he has voter or not the parent will check for that
    {
        return root;
    }
    struct Voter *result = LookUpVoter(root->lc, vid);
    if (result != NULL) // Case where the voter was found in the left subtree
    {
        return result;
    }
    return LookUpVoter(root->rc, vid);
}
struct District *LookUpDistrict(int did)
{
    for (int i = 0; i < DISTRICTS_SZ; i++)
    {
        // Doesnt need to check for NULL because everything is allocated and has -1
        if (Districts[i].did == did)
        {
            return &Districts[i];
        }
    }
    return NULL;
}
struct Candidate *LookUpCandidate(struct Candidate *root, int cid)
{
    // BST LookUp
    if (root == NULL)
    {
        printf("(V event) The BST Tree of the Candidates is empty\n");
        return NULL;
    }
    struct Candidate *result = NULL;
    while (root != NULL)
    {
        if (cid == root->cid)
        {
            result = root;
            break; // Break so no infinite loop
        }
        else if (cid < root->cid)
        {
            root = root->lc;
        }
        else
        {
            root = root->rc;
        }
    }
    return result; // Could be NULL if we didn't find the Candidate in the BST
}
void EventVote(int vid, int sid, int cid, int pid)
{
    DebugPrint("V %d %d %d %d\n", vid, sid, cid, pid);
    // TODO
    printf("V <%d> <%d> <%d> <%d>\n", vid, sid, cid, pid);
    // Find the station where the Voter should be (hashtable)
    struct Station *station_traversal = LookUpStation(sid);
    if (station_traversal == NULL)
    {
        printf("(V event) Station was not found in the hashtable\n");
        return;
    }
    // Find the voter inside the station (complete tree)
    struct Voter *voter_traversal = LookUpVoter(station_traversal->voters, vid);
    if (voter_traversal == NULL)
    {
        printf("(V event) Voter was not found in the complete tree\n");
        return;
    }
    if (voter_traversal->voted == true)
    {
        printf("(V event) Voter has already voted\n");
        return;
    }
    // Find the district in the Districts array using linear search (array)
    int did = station_traversal->did; // Fetch the did from the station where the voter is
    struct District *district_traversal = LookUpDistrict(did);
    if (district_traversal == NULL)
    {
        printf("(V event) This district doesn't exist\n");
        return;
    }
    // Also check if the pid is OK (I saw it is already checked from main.c but left it there for compliteness purposes)
    if (pid < 0 || pid > 4)
    {
        printf("(V event) This Party is invalid\n");
        return;
    }
    // Now we can change our values
    voter_traversal->voted = true; // Make sure it is recorded he voted
    if (cid == -1)
    {
        district_traversal->blanks++;
    }
    else if (cid == -2)
    {
        district_traversal->invalids++;
    }
    else // Case where it is a valid vote so we need to find the candidate with the certain cid
    {
        // Find the candidate inside the Party
        struct Candidate *candidate_traversal = LookUpCandidate(Parties[pid].candidates, cid);
        if (candidate_traversal == NULL)
        {
            printf("(V event) The candidate wasn't found in the Parties.candidates BST\n");
            return;
        }
        // Change the district to the Candidate since its a valid vote
        district_traversal = LookUpDistrict(candidate_traversal->did);
        // Adjust the values finally
        candidate_traversal->votes++;          // Increment his votes
        district_traversal->partyVotes[pid]++; // Increment the votes of the party
    }
    // -- Printing --
    printf(" District[%d]\n", district_traversal->did);
    printf(" blanks <%d>\n", district_traversal->blanks);
    printf(" invalids <%d>\n", district_traversal->invalids);
    printf(" partyVotes\n");
    for (int i = 0; i < PARTIES_SZ - 1; i++)
    {
        printf(" <%d> <%d>,\n", i, district_traversal->partyVotes[i]);
    }
    printf(" <%d> <%d>\n", PARTIES_SZ - 1, district_traversal->partyVotes[PARTIES_SZ - 1]); // Printing purposes
    printf("DONE\n");
    // -- Printing --
}
//--------------------------------------------------------------------- V ------------------------------------------------------------------------
//--------------------------------------------------------------------- M ------------------------------------------------------------------------
struct Candidate **CandidateHeap = NULL;
int current_size = 0;
int CalculateValidVotesDistrict(struct District *district)
{
    int sum = 0;
    if (district == NULL)
    {
        // Won't happen because it is checked from its parent function
        printf("(M event) This district is NULL\n");
        return sum;
    }
    // Iterate through every party and add the votes it has
    for (int i = 0; i < PARTIES_SZ; i++)
    {
        sum = sum + district->partyVotes[i];
    }
    return sum;
}

void HeapInsert(struct Candidate *new, int N)
{
    if (current_size == N)
    {
        printf("(Event M) Heap is full\n");
        return;
    }
    int m = current_size;
    int parent = (m - 1) / 2;
    while (m > 0 && new->votes < (*CandidateHeap[parent]).votes)
    {
        CandidateHeap[m] = CandidateHeap[parent]; // Swap the pointers
        m = parent;
        parent = (m - 1) / 2; // Recalculate parent
    }
    CandidateHeap[m] = new;
    current_size++;
}
void HeapDeleteMin()
{
    if (current_size == 0)
    {
        printf("(Event M) Heap is empty\n");
        return;
    }
    struct Candidate *to_move_to_root = CandidateHeap[current_size - 1];
    if (current_size == 1)
    {
        current_size--;
        printf("(Event M) Heap contains 1 node after removal no need to do anything\n");
        CandidateHeap[0] = NULL;
        return;
    }
    current_size--;
    int m = 0;
    while ((2 * m + 1 < current_size && to_move_to_root->votes > CandidateHeap[2 * m + 1]->votes) || (2 * m + 2 < current_size && to_move_to_root->votes > CandidateHeap[2 * m + 2]->votes))
    {
        int p;
        if (2 * m + 2 < current_size)
        {
            if (CandidateHeap[2 * m + 1]->votes < CandidateHeap[2 * m + 2]->votes)
            {
                p = 2 * m + 1;
            }
            else
            {
                p = 2 * m + 2;
            }
        }
        else
        {
            p = current_size - 1;
        }
        CandidateHeap[m] = CandidateHeap[p]; // Swap
        m = p;
    }
    CandidateHeap[m] = CandidateHeap[current_size];
}
void FillHeapCandidatesSpecificDID(struct Candidate *root, int did, int max_size)
{
    if (root == NULL)
    {
        return; // Tree is empty or end of traversal
    }

    if (current_size < max_size && root->did == did && root->isElected == false)
    {
        HeapInsert(root, max_size); // If someone else cant fit then the Insertion algorithm will break but checked here as well
    }
    else if (root->did == did && root->isElected == false && CandidateHeap[0] != NULL && CandidateHeap[0]->votes < root->votes)
    {
        HeapDeleteMin();
        HeapInsert(root, max_size);
    }

    FillHeapCandidatesSpecificDID(root->lc, did, max_size);
    FillHeapCandidatesSpecificDID(root->rc, did, max_size);
}
void ElectPartyCandidatesInDistrict(int pid, int did, int will_be_elected_count)
{
    // Allocate memory for will_be_elected_count of pointers for the CandidateHeap and init its size
    CandidateHeap = malloc(will_be_elected_count * sizeof(struct Candidate *));
    current_size = 0;
    // Initialize with the first people we encounter during traversal with specified did and if we find someone with more votes act accordingly
    if (Parties[pid].candidates == NULL)
    {
        printf("(Event M) This party has no candidates\n");
        return;
    }
    else
    {
        FillHeapCandidatesSpecificDID(Parties[pid].candidates, did, will_be_elected_count);
    }
    // -- Printing --
    for (int i = 0; i < current_size - 1; i++)
    {
        printf("<%d> <%d> <%d>,\n", CandidateHeap[i]->cid, pid, CandidateHeap[i]->votes);
        // Elect everyone in the heap also
        CandidateHeap[i]->isElected = true;
    }
    printf("<%d> <%d> <%d>\n", CandidateHeap[current_size - 1]->cid, pid, CandidateHeap[current_size - 1]->votes);
    CandidateHeap[current_size - 1]->isElected = true;
    // -- Printing --

    // Need to free the Heap now (it will allocated again)
    free(CandidateHeap);
    CandidateHeap = NULL;
}

int CountNonElectedCandidates(struct Candidate *root, int did)
{
    if (root == NULL)
    {
        return 0;
    }

    int left = CountNonElectedCandidates(root->lc, did);
    int right = CountNonElectedCandidates(root->rc, did);

    if (root->isElected == false && root->did == did)
    {
        return left + right + 1;
    }
    else
    {
        return left + right;
    }
}

void EventCountVotes(int did)
{
    DebugPrint("M %d\n", did);
    // TODO
    printf("M <%d>\n", did);
    struct District *district_traversal = LookUpDistrict(did);
    if (district_traversal == NULL)
    {
        printf("(M event) This district doesn't exist\n");
        return;
    }
    printf(" seats\n");
    int sum_valid_votes_district = CalculateValidVotesDistrict(district_traversal);
    double electoral_quota = 0;
    // Change electoral quota from 0 to something else only if the district has a valid number of seats to give else it stays as 0
    if (district_traversal->seats != 0)
    {
        electoral_quota = (double)sum_valid_votes_district / district_traversal->seats;
    }
    // Could also return
    if (electoral_quota == 0)
    {
        printf("(M event) This district can't elect anyone\n");
    }
    int partyElected[5] = {0}; // Helping array that contains how many of each party will be elected
    // Iterate through every party and place in the array how many will be elected from every party in this district
    for (int i = 0; i < PARTIES_SZ; i++)
    {
        if (electoral_quota == 0)
        {
            partyElected[i] = 0;
        }
        else
        {
            partyElected[i] = district_traversal->partyVotes[i] / electoral_quota;
        }
        // Update party elected count and district seats left
        // Firstly need to check if partyElected is valid (new check)
        int count_non_elected_candidates = CountNonElectedCandidates(Parties[i].candidates, did);
        if (partyElected[i] > count_non_elected_candidates)
        {
            partyElected[i] = count_non_elected_candidates;
        }
        Parties[i].electedCount = Parties[i].electedCount + partyElected[i];
        district_traversal->seats = district_traversal->seats - partyElected[i];
        if (partyElected[i] != 0)
        {
            // Call the function to elect certain candidates
            ElectPartyCandidatesInDistrict(i, did, partyElected[i]);
        }
        else
        {
            printf("(M event) Party<%d> Elected is zero\n", i);
        }
    }
    printf("DONE\n");
}
//--------------------------------------------------------------------- M ------------------------------------------------------------------------
//--------------------------------------------------------------------- N ------------------------------------------------------------------------
void InsertAtHeadList(struct ElectedCandidate **head, int cid, int party_index, int did)
{

    // First of allocate memory for a new ElectedCandidate and place its values
    struct ElectedCandidate *new = malloc(sizeof(struct ElectedCandidate));
    new->cid = cid;
    new->pid = party_index;
    new->did = did;
    new->next = NULL;

    // Swap heads (The list is going to be in decreasing order with the cid because we put the bigger cid in the head each time
    new->next = *head;
    *head = new;
}

void MakeListSorted(struct ElectedCandidate **head, struct Candidate *root, int party_index)
{
    // InOrder traversal gives me the cid in increasing order
    if (root == NULL)
    {
        return;
    }
    MakeListSorted(head, root->lc, party_index);
    // Only add elected candidates
    if (root->isElected == true)
    {
        InsertAtHeadList(head, root->cid, party_index, root->did);
    }
    MakeListSorted(head, root->rc, party_index);
}

struct ElectedCandidate *MakeParliamentSorted(struct ElectedCandidate **head_l)
{
    struct ElectedCandidate *parliament_start = NULL;
    struct ElectedCandidate *parliament_end = NULL;
    while (1)
    {
        // Find the biggest value everytime and add it to the end of the newly created list so everything stays in descending order
        struct ElectedCandidate *elected_most_votes = NULL;
        int index_to_push = -1;
        if (head_l[0] != NULL)
        {
            elected_most_votes = head_l[0];
            index_to_push = 0;
        }
        if (head_l[1] != NULL && (elected_most_votes == NULL || head_l[1]->cid > elected_most_votes->cid))
        {
            elected_most_votes = head_l[1];
            index_to_push = 1;
        }
        if (head_l[2] != NULL && (elected_most_votes == NULL || head_l[2]->cid > elected_most_votes->cid))
        {
            elected_most_votes = head_l[2];
            index_to_push = 2;
        }
        if (head_l[3] != NULL && (elected_most_votes == NULL || head_l[3]->cid > elected_most_votes->cid))
        {
            elected_most_votes = head_l[3];
            index_to_push = 3;
        }
        if (head_l[4] != NULL && (elected_most_votes == NULL || head_l[4]->cid > elected_most_votes->cid))
        {
            elected_most_votes = head_l[4];
            index_to_push = 4;
        }

        if (elected_most_votes == NULL)
        {
            break; // Noone got more votes either all lists are empty
        }

        if (parliament_start == NULL)
        {
            parliament_start = elected_most_votes;
            parliament_end = elected_most_votes;
        }
        else
        {
            parliament_end->next = elected_most_votes;
            parliament_end = elected_most_votes;
        }
        head_l[index_to_push] = head_l[index_to_push]->next;
    }
    return parliament_start;
}

void EventFormParliament(void)
{
    DebugPrint("N\n");
    // TODO
    struct Candidate *root_t[5] = {Parties[0].candidates, Parties[1].candidates, Parties[2].candidates, Parties[3].candidates, Parties[4].candidates};
    struct ElectedCandidate *head_l[5] = {NULL, NULL, NULL, NULL, NULL};
    printf("N\n");
    printf(" members\n");
    // Listify all the parties
    // We make the candidates to ElectedCandidates
    for (int i = 0; i < 5; i++)
    {
        MakeListSorted(&head_l[i], root_t[i], i);
    }

    Parliament = MakeParliamentSorted(head_l);
    struct ElectedCandidate *temp = Parliament;
    while (temp->next != NULL)
    {
        printf(" <%d> <%d> <%d>,\n", temp->cid, temp->pid, temp->did);
        temp = temp->next;
    }
    printf(" <%d> <%d> <%d>\n", temp->cid, temp->pid, temp->did);
    printf("DONE\n"); // Printing purposes
}
//--------------------------------------------------------------------- N ------------------------------------------------------------------------
//--------------------------------------------------------------------- I ------------------------------------------------------------------------
void EventPrintDistrict(int did)
{
    DebugPrint("I %d\n", did);
    // TODO
    printf("I <%d>\n", did);
    struct District *district_traversal = LookUpDistrict(did);
    if (district_traversal == NULL)
    {
        printf("(I event) This district doesn't exist\n");
        return;
    }
    printf(" seats <%d>\n", district_traversal->seats);
    printf(" blanks <%d>\n", district_traversal->blanks);
    printf(" invalids <%d>\n", district_traversal->invalids);
    printf(" partyVotes\n");
    for (int i = 0; i < PARTIES_SZ - 1; i++)
    {
        printf(" <%d> <%d>,\n", i, district_traversal->partyVotes[i]);
    }
    printf(" <%d> <%d>\n", PARTIES_SZ - 1, district_traversal->partyVotes[PARTIES_SZ - 1]);
    printf("DONE\n");
}
//--------------------------------------------------------------------- I ------------------------------------------------------------------------
//--------------------------------------------------------------------- J ------------------------------------------------------------------------
void EventPrintStationVoters(struct Voter *root, int *current_nodes, int max_count_nodes)
{
    if (root == NULL)
    {
        return;
    }

    EventPrintStationVoters(root->lc, current_nodes, max_count_nodes);
    (*current_nodes)++;
    if (*current_nodes == max_count_nodes)
    {
        printf(" <%d> <%d>\n", root->vid, root->voted);
    }
    else
    {
        printf(" <%d> <%d>,\n", root->vid, root->voted);
    }
    EventPrintStationVoters(root->rc, current_nodes, max_count_nodes);
}
void EventPrintStation(int sid)
{
    DebugPrint("J %d\n", sid);
    // TODO
    printf("J <%d>\n", sid);
    struct Station *station_traversal = LookUpStation(sid);
    if (station_traversal == NULL)
    {
        printf("(J Event) This station doesn't exit\n");
        return;
    }
    printf(" registered %d\n", station_traversal->registered);
    printf(" voters\n");
    int current_nodes = 0;
    EventPrintStationVoters(station_traversal->voters, &current_nodes, station_traversal->registered);
    printf("DONE\n");
}
//--------------------------------------------------------------------- J ------------------------------------------------------------------------
//--------------------------------------------------------------------- K ------------------------------------------------------------------------
void EventPrintPartyCandidates(struct Candidate *root, int *current_nodes, int max_count_nodes)
{
    if (root == NULL)
    {
        return;
    }

    EventPrintPartyCandidates(root->lc, current_nodes, max_count_nodes);
    if (*current_nodes == max_count_nodes - 1 && root->isElected)
    {
        (*current_nodes)++; // No actual need but ok
        printf(" <%d> <%d>\n", root->cid, root->votes);
    }
    else if (root->isElected)
    {
        (*current_nodes)++;
        printf(" <%d> <%d>,\n", root->cid, root->votes);
    }
    EventPrintPartyCandidates(root->rc, current_nodes, max_count_nodes);
}
void EventPrintParty(int pid)
{
    DebugPrint("K %d\n", pid);
    // TODO
    printf("K <%d>\n", pid);
    printf(" elected\n");
    int current_nodes = 0; // For printing purposes
    EventPrintPartyCandidates(Parties[pid].candidates, &current_nodes, Parties[pid].electedCount);
    printf("DONE\n");
}
//--------------------------------------------------------------------- K ------------------------------------------------------------------------
//--------------------------------------------------------------------- L ------------------------------------------------------------------------
void EventPrintParliament(void)
{
    DebugPrint("L\n");
    // TODO
    printf("L\n");
    struct ElectedCandidate *elected_traversal = Parliament;
    if (elected_traversal == NULL)
    {
        printf("(L Event) This station doesn't exit\n");
        return;
    }
    while (elected_traversal->next != NULL)
    {
        printf(" <%d> <%d> <%d>,\n", elected_traversal->cid, elected_traversal->pid, elected_traversal->did);
        elected_traversal = elected_traversal->next;
    }
    printf(" <%d> <%d> <%d>\n", elected_traversal->cid, elected_traversal->pid, elected_traversal->did);
    printf("DONE\n");
}
//--------------------------------------------------------------------- L ------------------------------------------------------------------------
//--------------------------------------------------------------------- BU ------------------------------------------------------------------------
void DeepestNode(struct Voter *root, int levels, struct Voter **deepest)
{
    if (!root)
    {
        return;
    }
    if (levels == 1)
    {
        // printf("last node is: %d\n", root->vid);
        *deepest = root; // Update the deepest node
    }
    else if (levels > 1)
    {
        DeepestNode(root->lc, levels - 1, deepest);
        DeepestNode(root->rc, levels - 1, deepest);
    }
    return;
}
void LookUpValueToDeleteReplace(struct Voter **root, int value, struct Voter **new_node)
{
    if (root == NULL || *root == NULL || new_node == NULL || *new_node == NULL)
    {
        return;
    }

    // Check if the current node has the target value
    if ((*root)->vid == value) // Node to replace is the same to be removed case
    {
        if ((*root)->parent == NULL && (*root)->vid == (*new_node)->vid) // If that node is root
        {
            struct Voter *temp = *root;
            *root = NULL;
            free(temp);
            temp = NULL;
            return;
        }
        else if ((*root)->parent != NULL && (*root)->vid == (*new_node)->vid) // If that node is not root
        {
            struct Voter *temp = *root;
            if ((*root == (*root)->parent->lc))
            {
                (*root)->parent->lc = NULL;
            }
            else if ((*root == (*root)->parent->rc))
            {
                (*root)->parent->rc = NULL;
            }
            *root = NULL;
            free(temp);
            temp = NULL;
            return;
        }
        else if ((*new_node)->parent != NULL) // Normal case where a node replaces another pawn
        {
            // Check if right kid or left and nullify the pointers
            if (*new_node == (*new_node)->parent->lc)
            {
                (*new_node)->parent->lc = NULL;
            }
            else if (*new_node == (*new_node)->parent->rc)
            {
                (*new_node)->parent->rc = NULL;
            }
            // Take its data
            (*new_node)->lc = NULL;
            (*new_node)->rc = NULL;
            (*root)->vid = (*new_node)->vid;
            (*root)->voted = (*new_node)->voted;
            free(*new_node);
            new_node = NULL;
            return; // Exit after replacing
        }
    }

    // Recur on the left subtree
    LookUpValueToDeleteReplace(&((*root)->lc), value, new_node);

    // Recur on the right subtree
    LookUpValueToDeleteReplace(&((*root)->rc), value, new_node);
}
void EventBonusUnregisterVoter(int vid, int sid)
{
    DebugPrint("BU %d %d\n", vid, sid);
    // TODO
    printf("BU <%d> <%d>\n", vid, sid);
    struct Station *station_traversal = LookUpStation(sid);
    if (station_traversal == NULL)
    {
        printf("(BU Event) This station doesn't exit\n");
        return;
    }
    struct Voter *voter_tree = station_traversal->voters;
    if (voter_tree == NULL)
    {
        printf("(BU Event) The voter tree doesn't exist\n");
        return;
    }
    // Count which subtree has more height
    struct Voter *temp = voter_tree;
    int h1 = 0;
    while (temp != NULL)
    {
        h1++;
        temp = temp->lc;
    }

    temp = voter_tree;
    int h2 = 0;
    while (temp != NULL)
    {
        h2++;
        temp = temp->rc;
    }

    int max = (h1 > h2) ? h1 : h2; // Choose the biggest of the heights

    struct Voter *deepest = NULL;
    DeepestNode(voter_tree, max, &deepest);
    LookUpValueToDeleteReplace(&voter_tree, vid, &deepest);
    // Need to store it back too
    station_traversal->voters = voter_tree;
    station_traversal->registered--;
    int current_nodes = 0; // For printing purposes
    printf(" Voters[%d]\n", station_traversal->sid);
    InOrderPrintVoters(voter_tree, &current_nodes, station_traversal->registered);
    printf("DONE\n");
}
//--------------------------------------------------------------------- BU ------------------------------------------------------------------------
//--------------------------------------------------------------------- BF ------------------------------------------------------------------------
void FreePartyCandidates(struct Candidate *root)
{
    if (root == NULL)
    {
        return;
    }
    FreePartyCandidates(root->lc);
    FreePartyCandidates(root->rc);
    free(root);
}

void FreeStationVoters(struct Voter *root)
{
    if (root == NULL)
    {
        return;
    }
    FreeStationVoters(root->lc);
    FreeStationVoters(root->rc);
    free(root);
}
void EventBonusFreeMemory(void)
{
    DebugPrint("BF\n");
    // TODO
    printf("BF\n");
    // Free every station and its voters in each chain and then free the actual pointers of the hashtable
    for (int i = 0; i < m; i++) // M is the lines count of the Hashtable
    {
        struct Station *temp = StationsHT[i];

        while (temp != NULL)
        {
            // First free the voters
            FreeStationVoters(temp->voters);
            temp->voters = NULL;
            // Then free the actual Station
            struct Station *next = temp->next;
            free(temp);
            temp = next;
        }
    }
    free(StationsHT);
    // Free the Parties Candidates by traversing
    for (int i = 0; i < 5; i++)
    {
        FreePartyCandidates(Parties[i].candidates);
        Parties[i].candidates = NULL;
    }
    // Free the parliament
    struct ElectedCandidate *temp = Parliament;
    while (temp != NULL)
    {
        struct ElectedCandidate *next = temp->next;
        free(temp);
        temp = next;
    }
    printf("DONE\n");
}
//--------------------------------------------------------------------- BF ------------------------------------------------------------------------
